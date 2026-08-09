#pragma once
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <imgui.h>
#include <cfloat>
#include "VisualArchDevice.hpp"

namespace NodeSystem::Visual {

    class ArchitectureCanvas {
    public:
        std::map<int, VisualArchDevice> devices;
        std::vector<ArchFlight> flights;

        std::set<std::string> availableTraces;
        std::string selectedTrace = "All";

        float currentLogTime = 0.0f;
        bool isPlaying = false;
        float playbackSpeed = 1.0f;
        ImVec2 scrolling = ImVec2(0.0f, 0.0f);
        float zoom = 1.0f;

        float getTraceMinTime() const {
            if (flights.empty()) return 0.0f;
            if (selectedTrace == "All") return flights.front().emitTime;
            float m = FLT_MAX;
            for (const auto& f : flights) {
                if (f.traceName == selectedTrace && f.emitTime < m) m = f.emitTime;
            }
            return m == FLT_MAX ? 0.0f : m;
        }

        float getTraceMaxTime() const {
            if (flights.empty()) return 1.0f;
            float m = -FLT_MAX;
            for (const auto& f : flights) {
                if ((selectedTrace == "All" || f.traceName == selectedTrace) && f.arrivalTime > m) m = f.arrivalTime;
            }
            return m == -FLT_MAX ? 1.0f : m;
        }

        static ImVec2 interpolatePolyline(const std::vector<ImVec2>& points, float progress) {
            if (points.empty()) return ImVec2(0,0);
            if (points.size() == 1) return points[0];
            if (progress <= 0.0f) return points.front();
            if (progress >= 1.0f) return points.back();

            float totalLen = 0.0f;
            std::vector<float> segmentLens;
            for (size_t i = 0; i < points.size() - 1; ++i) {
                float dx = points[i+1].x - points[i].x;
                float dy = points[i+1].y - points[i].y;
                float len = std::sqrt(dx*dx + dy*dy);
                segmentLens.push_back(len);
                totalLen += len;
            }

            if (totalLen == 0.0f) return points.back();

            float targetLen = totalLen * progress;
            float currentLen = 0.0f;
            for (size_t i = 0; i < points.size() - 1; ++i) {
                if (currentLen + segmentLens[i] >= targetLen) {
                    float segProg = (targetLen - currentLen) / segmentLens[i];
                    return ImVec2(
                        points[i].x + (points[i+1].x - points[i].x) * segProg,
                        points[i].y + (points[i+1].y - points[i].y) * segProg
                    );
                }
                currentLen += segmentLens[i];
            }
            return points.back();
        }

        void render() {
            ImGui::Columns(2, "ArchMainLayout", true);
            if (ImGui::GetColumnWidth() == 0) ImGui::SetColumnWidth(0, ImGui::GetIO().DisplaySize.x * 0.75f);

            ImGui::BeginChild("ArchCanvasRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            float minTime = getTraceMinTime();
            float maxTime = getTraceMaxTime();
            if (maxTime <= minTime) maxTime = minTime + 1.0f;

            ImGui::SetCursorPos(ImVec2(10, 10));
            ImGui::BeginGroup();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 20, 25, 220));
            ImGui::BeginChild("ArchPlayer", ImVec2(400, 140), true);

            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "Playing Trace: %s", selectedTrace.c_str());
            ImGui::Separator();

            ImGui::Checkbox(isPlaying ? "PAUSE" : "PLAY", &isPlaying); ImGui::SameLine();
            if (ImGui::Button("Restart")) currentLogTime = minTime;
            ImGui::SliderFloat("Speed", &playbackSpeed, 0.1f, 10.0f, "%.1f Ticks/sec");
            if (ImGui::SliderFloat("Timeline", &currentLogTime, minTime, maxTime, "Time: %.2f")) isPlaying = false;
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::EndGroup();

            ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
            ImVec2 canvasSz = ImGui::GetContentRegionAvail();
            ImVec2 canvasP1 = canvasP0 + canvasSz;
            drawList->AddRectFilled(canvasP0, canvasP1, IM_COL32(25, 25, 25, 255));

            if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive()) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    scrolling.x += ImGui::GetIO().MouseDelta.x;
                    scrolling.y += ImGui::GetIO().MouseDelta.y;
                }
                if (ImGui::GetIO().MouseWheel != 0.0f) {
                    zoom += ImGui::GetIO().MouseWheel * 0.1f;
                    zoom = std::clamp(zoom, 0.2f, 3.0f);
                }
            }

            drawList->PushClipRect(canvasP0, canvasP1, true);
            ImVec2 origin = canvasP0 + scrolling;
            std::map<ArchPinKey, ImVec2> pinRegistry;

            std::map<ArchPinKey, int> activeQueues;
            std::map<ArchPinKey, bool> busyWorkers;
            std::vector<const ArchFlight*> activeFlights;

            for (const auto& f : flights) {
                if (selectedTrace != "All" && f.traceName != selectedTrace) continue;

                if (currentLogTime >= f.emitTime && currentLogTime <= f.arrivalTime) {
                    if (f.fromPin == f.toPin) {
                        activeQueues[f.toPin]++;
                        if (std::get<1>(f.toPin) == ArchCompType::Worker) busyWorkers[f.toPin] = true;
                    } else {
                        activeFlights.push_back(&f);
                    }
                }
            }
            std::map<ArchPinKey, float> emitFlashes;
            std::map<ArchPinKey, float> recvFlashes;
            float flashDuration = 0.6f;

            for (const auto& f : flights) {
                if (selectedTrace != "All" && f.traceName != selectedTrace) continue;

                if (currentLogTime >= f.emitTime && currentLogTime <= f.arrivalTime) {
                    if (f.fromPin == f.toPin) {
                        activeQueues[f.toPin]++;
                        if (std::get<1>(f.toPin) == ArchCompType::Worker) busyWorkers[f.toPin] = true;
                    } else {
                        activeFlights.push_back(&f);
                    }
                }

                // Розрахунок затухань:
                if (f.fromPin != f.toPin) { // Тільки для реальних перельотів
                    if (currentLogTime >= f.emitTime && currentLogTime - f.emitTime <= flashDuration) {
                        float intensity = 1.0f - ((currentLogTime - f.emitTime) / flashDuration);
                        emitFlashes[f.fromPin] = std::max(emitFlashes[f.fromPin], intensity);
                    }
                    if (currentLogTime >= f.arrivalTime && currentLogTime - f.arrivalTime <= flashDuration) {
                        float intensity = 1.0f - ((currentLogTime - f.arrivalTime) / flashDuration);
                        recvFlashes[f.toPin] = std::max(recvFlashes[f.toPin], intensity);
                    }
                }
            }

            // Малюємо пристрої, ПЕРЕДАЄМО ФЛЕШІ
            for (auto& [id, dev] : devices) {
                dev.draw(drawList, origin, zoom, pinRegistry, activeQueues, busyWorkers, emitFlashes, recvFlashes);
            }

            // Малюємо зовнішні мережеві труби та АНІМУЄМО ПОЛЬОТИ
            for (const auto& f : flights) {
                if (std::get<1>(f.fromPin) == ArchCompType::TcpTx && std::get<1>(f.toPin) == ArchCompType::TcpRx) {
                    if (pinRegistry.contains(f.fromPin) && pinRegistry.contains(f.toPin)) {
                        ImVec2 p1 = pinRegistry[f.fromPin]; ImVec2 p2 = pinRegistry[f.toPin];
                        std::vector<ImVec2> netPath = { p1, ImVec2(p1.x, p1.y + 60.0f * zoom), ImVec2(p2.x, p1.y + 60.0f * zoom), p2 };
                        drawList->AddPolyline(netPath.data(), netPath.size(), IM_COL32(200, 150, 255, 150), 0, 3.0f * zoom);
                    }
                }
            }

            for (const auto* af : activeFlights) {
                ImVec2 curr;
                bool routed = false;
                int devId = std::get<0>(af->fromPin);

                if (devices.contains(devId)) {
                    auto& dev = devices[devId];
                    if (dev.routes.contains({af->fromPin, af->toPin})) {
                        curr = origin + dev.pos * zoom + interpolatePolyline(dev.routes.at({af->fromPin, af->toPin}), progress(af)) * zoom;
                        routed = true;
                    }
                }

                if (!routed && pinRegistry.contains(af->fromPin) && pinRegistry.contains(af->toPin)) {
                    ImVec2 p1 = pinRegistry[af->fromPin]; ImVec2 p2 = pinRegistry[af->toPin];
                    if (std::get<1>(af->fromPin) == ArchCompType::TcpTx) {
                        std::vector<ImVec2> extPath = { p1, ImVec2(p1.x, p1.y + 60.0f*zoom), ImVec2(p2.x, p1.y + 60.0f*zoom), p2 };
                        curr = interpolatePolyline(extPath, progress(af));
                    } else {
                        curr = ImVec2(p1.x + (p2.x - p1.x)*progress(af), p1.y + (p2.y - p1.y)*progress(af));
                    }
                }

                if (routed || (pinRegistry.contains(af->fromPin) && pinRegistry.contains(af->toPin))) {
                    drawList->AddCircleFilled(curr, 6.0f * zoom, IM_COL32(0, 255, 255, 255));
                    drawList->AddCircle(curr, 8.0f * zoom, IM_COL32(255, 255, 255, 255));
                }
            }

            drawList->PopClipRect();
            ImGui::EndChild();
            ImGui::NextColumn();

            ImGui::BeginChild("ArchRightPanelRegion");
            ImGui::TextDisabled("TRACE FILTER");
            ImGui::Separator();

            float traceListHeight = ImGui::GetContentRegionAvail().y * 0.8f;
            ImGui::BeginChild("ArchTraceListWindow", ImVec2(0, traceListHeight), true);

            if (ImGui::Selectable("All", selectedTrace == "All")) {
                if (selectedTrace != "All") { selectedTrace = "All"; currentLogTime = getTraceMinTime(); }
            }
            for (const auto& t : availableTraces) {
                if (ImGui::Selectable(t.c_str(), selectedTrace == t)) {
                    if (selectedTrace != t) { selectedTrace = t; currentLogTime = getTraceMinTime(); isPlaying = true; }
                }
            }
            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::Columns(1);

            if (isPlaying && !flights.empty()) {
                currentLogTime += ImGui::GetIO().DeltaTime * playbackSpeed;
                if (currentLogTime > maxTime) {
                    if (selectedTrace != "All") currentLogTime = minTime;
                    else { currentLogTime = maxTime; isPlaying = false; }
                }
            }


        }

        float progress(const ArchFlight* af) {
            return (af->arrivalTime - af->emitTime) > 0 ? (currentLogTime - af->emitTime) / (af->arrivalTime - af->emitTime) : 1.0f;
        }
    };
}