#pragma once
#include <vector>
#include <imgui.h>
#include <algorithm>
#include <set>
#include <string>
#include <optional>
#include <cfloat>

#include "VisualNode.hpp"
#include "VisualDevice.hpp"
#include "VisualSensor.hpp"

namespace NodeSystem::Visual {

    struct Flight {
        float emitTime;
        float arrivalTime;
        std::string traceName;
        PinKey fromPin;
        PinKey toPin;
        Core::types::ProcessData data;

        std::string emitEventStr;
        std::string emitParentStr;
        std::string acceptEventStr;
    };

    struct ActiveFlight {
        Flight flight;
        float progress = 0.0f;
    };

    class GraphCanvas {
    public:
        std::vector<VisualDevice> devices;
        std::map<uint64_t, VisualLink> traceLinks;
        SelectionInfo currentSelection;

        std::vector<Flight> flights;
        std::vector<ActiveFlight> activeFlights;
        std::set<std::string> availableTraces;
        std::string selectedTrace = "All";
        std::optional<Flight> inspectedFlight;

        float currentLogTime = 0.0f;
        bool isPlaying = false;
        float playbackSpeed = 1.0f;

        ImVec2 scrolling = ImVec2(0.0f, 0.0f);

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

        void syncFlightsToTime() {
            activeFlights.clear();

            for (auto& dev : devices) {
                for (auto* b : dev.blocks) {
                    if (b->blockType == SelectionType::Node) {
                        static_cast<VisualNode*>(b)->lastReceivedData.clear();
                    }
                }
            }

            for (size_t i = 0; i < flights.size(); ++i) {
                if (flights[i].emitTime > currentLogTime) break;

                if (flights[i].arrivalTime > currentLogTime) {
                    if (selectedTrace == "All" || flights[i].traceName == selectedTrace) {
                        ActiveFlight af;
                        af.flight = flights[i];
                        float dur = flights[i].arrivalTime - flights[i].emitTime;
                        af.progress = dur > 0 ? (currentLogTime - flights[i].emitTime) / dur : 1.0f;
                        activeFlights.push_back(af);
                    }
                } else {
                    auto& tPin = flights[i].toPin;
                    for (auto& dev : devices) {
                        if (dev.blockId == std::get<0>(tPin)) {
                            for (auto* b : dev.blocks) {
                                if (b->blockType == SelectionType::Node && b->getLevel() == std::get<1>(tPin) && b->blockId == std::get<2>(tPin)) {
                                    static_cast<VisualNode*>(b)->lastReceivedData[std::get<3>(tPin)] = flights[i].data;
                                }
                            }
                        }
                    }
                }
            }
        }

        void render() {
            ImGui::Columns(2, "MainLayout", true);
            if (ImGui::GetColumnWidth() == 0) ImGui::SetColumnWidth(0, ImGui::GetIO().DisplaySize.x * 0.75f);

            ImGui::BeginChild("CanvasRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            float minTime = getTraceMinTime();
            float maxTime = getTraceMaxTime();
            if (maxTime <= minTime) maxTime = minTime + 1.0f;

            // --- ПЛЕЄР ---
            ImGui::SetCursorPos(ImVec2(10, 10));
            ImGui::BeginGroup();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImU32)colorsMap[Keys::Colors::PlayerBackground]);
            ImGui::BeginChild("Player", ImVec2(400, 180), true);

            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "Playing Trace: %s", selectedTrace.c_str());
            ImGui::Separator();

            ImGui::Checkbox(isPlaying ? "PAUSE" : "PLAY", &isPlaying);
            ImGui::SameLine();
            if (ImGui::Button("Restart")) {
                currentLogTime = minTime;
                isPlaying = false;
                syncFlightsToTime();
            }
            ImGui::SliderFloat("Speed", &playbackSpeed, 0.1f, 10.0f, "%.1f Ticks/sec");

            if (ImGui::SliderFloat("Timeline", &currentLogTime, minTime, maxTime, "Time: %.2f")) {
                isPlaying = false;
            }

            ImGui::Text("Flights Rendered: %zu", activeFlights.size());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::EndGroup();

            ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
            ImVec2 canvasSz = ImGui::GetContentRegionAvail();
            ImVec2 canvasP1 = canvasP0 + canvasSz;
            drawList->AddRectFilled(canvasP0, canvasP1, colorsMap[Keys::Colors::CanvasBackground]);
            auto& zoom = floatsMap[Keys::Floats::zoom];
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
            std::map<PinKey, ImVec2> pinRegistry;

            std::map<PinKey, float> activeFlashes;
            float flashDuration = 0.6f;

            for (const auto& f : flights) {
                if (f.emitTime > currentLogTime) break;
                if (selectedTrace != "All" && f.traceName != selectedTrace) continue;

                if (currentLogTime - f.emitTime <= flashDuration) {
                    float intensity = 1.0f - ((currentLogTime - f.emitTime) / flashDuration);
                    activeFlashes[f.fromPin] = std::max(activeFlashes[f.fromPin], intensity);
                }

                if (currentLogTime >= f.arrivalTime && currentLogTime - f.arrivalTime <= flashDuration) {
                    float intensity = 1.0f - ((currentLogTime - f.arrivalTime) / flashDuration);
                    activeFlashes[f.toPin] = std::max(activeFlashes[f.toPin], intensity);
                }
            }

            for (auto& dev : devices) {
                dev.draw(drawList, origin, zoom, currentSelection, pinRegistry, activeFlashes);
            }

            bool isAnythingSelected = (currentSelection.type != SelectionType::None && currentSelection.type != SelectionType::Device);
            for (const auto& [traceId, link] : traceLinks) {
                PinKey fromKey = {link.fromDevice, link.fromLevel, link.fromIndex, -1};
                PinKey toKey = {link.toDevice, link.toLevel, link.toIndex, link.toSlot};
                if (pinRegistry.contains(fromKey) && pinRegistry.contains(toKey)) {
                    ImU32 lineColor = colorsMap[Keys::Colors::Link];
                    if (currentSelection.block) {
                        bool mFrom = (currentSelection.block->parentDeviceId == link.fromDevice && currentSelection.block->getLevel() == link.fromLevel && currentSelection.block->blockId == link.fromIndex);
                        bool mTo = (currentSelection.block->parentDeviceId == link.toDevice && currentSelection.block->getLevel() == link.toLevel && currentSelection.block->blockId == link.toIndex);

                        if (currentSelection.type == SelectionType::Node || currentSelection.type == SelectionType::Sensor) {
                            if (mFrom || mTo) lineColor = colorsMap[Keys::Colors::LinkHighlight];
                        } else if (currentSelection.type == SelectionType::OutputSlot) {
                            if (mFrom) lineColor = colorsMap[Keys::Colors::LinkHighlight];
                        } else if (currentSelection.type == SelectionType::InputSlot) {
                            if (mTo && currentSelection.slotIndex == link.toSlot) lineColor = colorsMap[Keys::Colors::LinkHighlight];
                        }
                    } else if (isAnythingSelected) lineColor = IM_COL32(100,100,100,50);

                    drawList->AddLine(pinRegistry[fromKey], pinRegistry[toKey], lineColor, floatsMap[Keys::Floats::linkThin] * zoom);
                }
            }

            if (selectedTrace != "All") {
                for (const auto& f : flights) {
                    if (f.traceName == selectedTrace) {
                        if (pinRegistry.contains(f.fromPin) && pinRegistry.contains(f.toPin)) {
                            ImVec2 p1 = pinRegistry[f.fromPin];
                            ImVec2 p2 = pinRegistry[f.toPin];
                            drawList->AddLine(p1, p2, colorsMap[Keys::Colors::TraceHighlight], 5.0f * zoom);
                            drawList->AddCircleFilled(p1, 7.0f * zoom, colorsMap[Keys::Colors::TraceHighlightDot]);
                            drawList->AddCircleFilled(p2, 7.0f * zoom, colorsMap[Keys::Colors::TraceHighlightDot]);
                        }
                    }
                }
            }

            // --- ЗАЦИКЛЕННЯ АБО ЗУПИНКА ---
            if (isPlaying && !flights.empty()) {
                currentLogTime += ImGui::GetIO().DeltaTime * playbackSpeed;
                if (currentLogTime > maxTime) {
                    if (selectedTrace != "All") {
                        currentLogTime = minTime; // Loop if single trace selected
                    } else {
                        currentLogTime = maxTime;
                        isPlaying = false;
                    }
                }
            }

            static float lastRenderTime = -1.0f;
            if (currentLogTime != lastRenderTime) {
                syncFlightsToTime();
                lastRenderTime = currentLogTime;
            }

            for (auto& af : activeFlights) {
                if (pinRegistry.contains(af.flight.fromPin) && pinRegistry.contains(af.flight.toPin)) {
                    ImVec2 p1 = pinRegistry[af.flight.fromPin];
                    ImVec2 p2 = pinRegistry[af.flight.toPin];
                    ImVec2 curr = ImVec2(p1.x + (p2.x - p1.x) * af.progress, p1.y + (p2.y - p1.y) * af.progress);

                    float size = 6.0f * zoom;
                    ImVec2 minSq = curr - ImVec2(size, size);
                    ImVec2 maxSq = curr + ImVec2(size, size);

                    bool isHovered = ImGui::IsMouseHoveringRect(minSq, maxSq);
                    ImU32 color = isHovered ? colorsMap[Keys::Colors::FlightHover] : colorsMap[Keys::Colors::FlightNormal];

                    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        currentSelection = { SelectionType::Flight, nullptr, -1 };
                        inspectedFlight = af.flight;
                        isPlaying = false;
                    }

                    drawList->AddRectFilled(minSq, maxSq, color);
                    drawList->AddRect(minSq, maxSq, IM_COL32(255, 255, 255, 255));
                }
            }

            drawList->PopClipRect();
            ImGui::EndChild();
            ImGui::NextColumn();

            ImGui::BeginChild("RightPanelRegion");

            ImGui::TextDisabled("TRACE FILTER");
            ImGui::Separator();

            float traceListHeight = ImGui::GetContentRegionAvail().y * 0.35f;
            ImGui::BeginChild("TraceListWindow", ImVec2(0, traceListHeight), true);

            if (ImGui::Selectable("All", selectedTrace == "All")) {
                if (selectedTrace != "All") {
                    selectedTrace = "All";
                    currentLogTime = getTraceMinTime();
                    syncFlightsToTime();
                }
            }
            for (const auto& t : availableTraces) {
                if (ImGui::Selectable(t.c_str(), selectedTrace == t)) {
                    if (selectedTrace != t) {
                        selectedTrace = t;
                        currentLogTime = getTraceMinTime(); // Телепорт на початок трейсу
                        isPlaying = true; // Можемо відразу вмикати авто-відтворення!
                        syncFlightsToTime();
                    }
                }
            }
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::Spacing();

            drawInspector();

            ImGui::EndChild();
            ImGui::Columns(1);
        }

    private:
        void drawInspector() {
            ImGui::TextDisabled("INSPECTOR");
            ImGui::Separator();

            if (currentSelection.type == SelectionType::Flight && inspectedFlight.has_value()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Data Packet Selected");
                ImGui::Separator();
                ImGui::Text("Trace Group: %s", inspectedFlight->traceName.c_str());
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Causality (Logical Chain):");
                ImGui::BulletText("Emit ParentId: %s", inspectedFlight->emitParentStr.c_str());
                ImGui::BulletText("Emit EventId:  %s", inspectedFlight->emitEventStr.c_str());
                ImGui::BulletText("Accept EventId: %s", inspectedFlight->acceptEventStr.c_str());
                ImGui::Separator();
                ImGui::Text("Payload Data:");
                ImGui::BulletText("Alpha Value: %.4f", inspectedFlight->data.alpha.get());
                ImGui::BulletText("Theta Value: %llu", inspectedFlight->data.theta);
                return;
            }

            if (currentSelection.type == SelectionType::None || !currentSelection.block) {
                ImGui::Text("Select an item to see details.");
                return;
            }

            VisualBlock* selBlock = currentSelection.block;

            if (currentSelection.type == SelectionType::InputSlot) {
                if (selBlock->blockType == SelectionType::Node) {
                    auto* vNode = static_cast<VisualNode*>(selBlock);
                    if (currentSelection.slotIndex >= 0 && currentSelection.slotIndex < vNode->coreNode->inputs.size()) {
                        const auto& kgIn = vNode->coreNode->inputs[currentSelection.slotIndex].input;
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "Input Slot %d", currentSelection.slotIndex);
                        ImGui::Separator();
                        ImGui::Text("Structure Data (KgInput):");
                        ImGui::BulletText("c_i: %s", kgIn.c_i.get() ? "true" : "false");
                        ImGui::BulletText("a_i: %.3f", kgIn.a_i.get());

                        if (vNode->lastReceivedData.contains(currentSelection.slotIndex)) {
                            ImGui::Separator();
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Runtime ProcessData (Received):");
                            auto& pData = vNode->lastReceivedData[currentSelection.slotIndex];
                            ImGui::BulletText("Alpha: %.4f", pData.alpha.get());
                            ImGui::BulletText("Theta: %llu", pData.theta);
                        }
                    }
                }
            }
            else if (currentSelection.type == SelectionType::Device) {
                auto *dev = static_cast<VisualDevice *>(selBlock);
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 0.8f, 1.0f), "Device Selected");
                ImGui::Text("Device ID: %d", dev->blockId);
                ImGui::Text("Workers: %d", dev->workersNumber);
                ImGui::Text("IP: %s", dev->ipAddress.c_str());
            } else if (currentSelection.type == SelectionType::Sensor) {
                auto *vSensor = static_cast<VisualSensor *>(selBlock);
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Sensor Selected");
                ImGui::Separator();
                ImGui::Text("Sensor ID: S0.%d", vSensor->blockId);
                ImGui::Text("Period: %llu ms", vSensor->period);
                ImGui::Text("To Emit: %.3f", vSensor->toEmit);
            } else if (currentSelection.type == SelectionType::Node) {
                auto *vNode = static_cast<VisualNode *>(selBlock);
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Node Selected");
                ImGui::Separator();
                ImGui::Text("Node ID: N%d.%d", vNode->coreNode->id.level, vNode->coreNode->id.index);
                ImGui::Text("Algorithm ID: %d", vNode->coreNode->algoType);
                ImGui::Text("Inputs Count: %zu", vNode->coreNode->inputs.size());
            } else if (currentSelection.type == SelectionType::OutputSlot) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "Output Slot");
                ImGui::Separator();

                if (selBlock->blockType == SelectionType::Sensor) {
                    auto *vSensor = static_cast<VisualSensor *>(selBlock);
                    ImGui::Text("Sensor Output Value: %.3f", vSensor->toEmit);
                } else if (selBlock->blockType == SelectionType::Node) {
                    auto *vNode = static_cast<VisualNode *>(selBlock);
                    const auto &kgOut = vNode->coreNode->output.output;

                    ImGui::Text("Data (KgOutput):");
                    ImGui::BulletText("c: %s", kgOut.c.get() ? "true" : "false");
                    ImGui::BulletText("a: %.3f", kgOut.a.get());
                }
            }
        }
    };
}