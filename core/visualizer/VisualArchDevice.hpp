#pragma once
#include <imgui.h>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include "ArchTypes.hpp"

namespace NodeSystem::Visual {

    struct ArchBlock {
        ImVec2 pos;
        ImVec2 size;
        std::string name;
        ArchPinKey pin;
    };

    class VisualArchDevice {
    public:
        int deviceId = 0;
        int workerCount = 1;

        ImVec2 pos;
        ImVec2 size{ 820.0f, 500.0f };

        std::vector<ArchBlock> sensors;
        std::vector<ArchBlock> workers;
        ArchBlock router;
        ArchBlock tcpBridge;

        ArchBlock queueRouter, rqPortWorker, rqPortSensor, rqPortTcp;
        ArchBlock queueWorker;
        ArchBlock queueForward;

        ImVec2 sensorOutHub, workerInHub, workerOutHub;
        float sGrpTop, sGrpBot, wGrpTop, wGrpBot;

        std::map<std::pair<ArchPinKey, ArchPinKey>, std::vector<ImVec2>> routes;

        ArchPinKey getPin(ArchCompType t, int idx = 0) const { return {deviceId, t, idx}; }
        void buildRoute(ArchPinKey from, ArchPinKey to, const std::vector<ImVec2>& pts) { routes[{from, to}] = pts; }

        void performLayout() {
            float totalSHeight = sensors.size() * 60.0f - 20.0f;
            float totalWHeight = workerCount * 60.0f - 20.0f;
            size.y = std::max({460.0f, totalSHeight + 100.0f, totalWHeight + 100.0f});

            // --- SENSORS GROUP ---
            float sStartY = (size.y - totalSHeight) / 2.0f;
            float currentY = sStartY;
            for(size_t i = 0; i < sensors.size(); ++i) {
                sensors[i].pos = ImVec2(30.0f, currentY);
                sensors[i].size = ImVec2(60.0f, 40.0f);
                sensors[i].name = "S" + std::to_string(std::get<2>(sensors[i].pin));
                currentY += 60.0f;
            }
            sGrpTop = sStartY - 20.0f;
            sGrpBot = currentY - 20.0f;
            sensorOutHub = ImVec2(110.0f, (sGrpTop + sGrpBot) / 2.0f);

            // --- WORKERS GROUP ---
            float wStartY = (size.y - totalWHeight) / 2.0f;
            currentY = wStartY;
            workers.resize(workerCount);
            for(int i = 0; i < workerCount; ++i) {
                workers[i].pos = ImVec2(650.0f, currentY);
                workers[i].size = ImVec2(60.0f, 40.0f);
                workers[i].name = "W" + std::to_string(i);
                workers[i].pin = {deviceId, ArchCompType::Worker, i};
                currentY += 60.0f;
            }
            wGrpTop = wStartY - 20.0f;
            wGrpBot = currentY - 20.0f;
            workerInHub = ImVec2(635.0f, (wGrpTop + wGrpBot) / 2.0f);
            workerOutHub = ImVec2(725.0f, (wGrpTop + wGrpBot) / 2.0f);

            // --- MAIN BLOCKS ---
            router.pos = ImVec2(350.0f, size.y / 2.0f - 35.0f);
            router.size = ImVec2(100.0f, 70.0f);
            router.name = "Router";

            tcpBridge.pos = ImVec2(350.0f, router.pos.y + 120.0f);
            tcpBridge.size = ImVec2(100.0f, 60.0f);
            tcpBridge.name = "TCP Bridge";

            // --- QUEUES ---
            queueRouter.pos = ImVec2(240.0f, 60.0f);
            queueRouter.size = ImVec2(25.0f, size.y - 120.0f);
            queueRouter.name = "RQ";

            queueWorker.pos = ImVec2(530.0f, 60.0f);
            queueWorker.size = ImVec2(25.0f, size.y - 120.0f);
            queueWorker.name = "WQ";

            queueForward.pos = ImVec2(router.pos.x + 40.0f, router.pos.y + 85.0f);
            queueForward.size = ImVec2(20.0f, 15.0f);
            queueForward.name = "FQ";

            // --- RQ PORTS ---
            rqPortWorker.pos = ImVec2(215.0f, 80.0f); rqPortWorker.size = ImVec2(25.0f, 25.0f);
            rqPortSensor.pos = ImVec2(215.0f, sensorOutHub.y - 12.5f); rqPortSensor.size = ImVec2(25.0f, 25.0f);
            rqPortTcp.pos    = ImVec2(215.0f, tcpBridge.pos.y + 17.5f); rqPortTcp.size = ImVec2(25.0f, 25.0f);

            generateRoutes();
        }

        void generateRoutes() {
            routes.clear();
            auto c  = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y + b.size.y/2); };
            auto rc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x, b.pos.y + b.size.y/2); };
            auto lc = [](const ArchBlock& b) { return ImVec2(b.pos.x, b.pos.y + b.size.y/2); };
            auto tc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y); };
            auto bc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y + b.size.y); };

            for(auto& s : sensors) buildRoute(s.pin, getPin(ArchCompType::Queue_Router), { rc(s), sensorOutHub, lc(rqPortSensor), c(rqPortSensor), ImVec2(c(queueRouter).x, c(rqPortSensor).y) });

            for(auto& w : workers) buildRoute(w.pin, getPin(ArchCompType::Queue_Router), { rc(w), workerOutHub, ImVec2(workerOutHub.x + 20.0f, workerOutHub.y), ImVec2(workerOutHub.x + 20.0f, 30.0f), ImVec2(c(rqPortWorker).x, 30.0f), tc(rqPortWorker), c(rqPortWorker), ImVec2(c(queueRouter).x, c(rqPortWorker).y) });

            buildRoute(getPin(ArchCompType::TcpRx), getPin(ArchCompType::Queue_Router), { lc(tcpBridge), rc(rqPortTcp), c(rqPortTcp), ImVec2(c(queueRouter).x, c(rqPortTcp).y) });
            buildRoute(getPin(ArchCompType::Queue_Router), getPin(ArchCompType::Router), { ImVec2(c(queueRouter).x, c(router).y), rc(queueRouter), lc(router) });
            buildRoute(getPin(ArchCompType::Router), getPin(ArchCompType::Queue_Worker), { rc(router), lc(queueWorker), c(queueWorker) });
            buildRoute(getPin(ArchCompType::Router), getPin(ArchCompType::Queue_Forward), { bc(router), tc(queueForward), c(queueForward) });
            buildRoute(getPin(ArchCompType::Queue_Forward), getPin(ArchCompType::TcpTx), { c(queueForward), bc(queueForward), tc(tcpBridge) });

            for(auto& w : workers) buildRoute(getPin(ArchCompType::Queue_Worker), w.pin, { c(queueWorker), rc(queueWorker), workerInHub, lc(w) });
        }

        // ОНОВЛЕНО: додано emitFlashes та recvFlashes
        void draw(ImDrawList* dl, ImVec2 origin, float zoom, std::map<ArchPinKey, ImVec2>& pinRegistry,
                  const std::map<ArchPinKey, int>& activeQueues, const std::map<ArchPinKey, bool>& busyWorkers,
                  const std::map<ArchPinKey, float>& emitFlashes, const std::map<ArchPinKey, float>& recvFlashes) {

            ImVec2 p0 = origin + pos * zoom;

            // --- ФОН ДЕВАЙСУ І НАЗВА ---
            dl->AddRectFilled(p0, p0 + size * zoom, IM_COL32(30, 30, 35, 255), 10.0f * zoom);
            dl->AddRect(p0, p0 + size * zoom, IM_COL32(80, 80, 90, 255), 10.0f * zoom, 0, 2.0f * zoom);
            dl->AddText(p0 + ImVec2(15, 15) * zoom, IM_COL32(150, 200, 255, 255), ("DEVICE " + std::to_string(deviceId)).c_str());

            // --- ВІЗУАЛЬНІ РАМКИ ГРУП ---
            dl->AddRect(p0 + ImVec2(15.0f, sGrpTop) * zoom, p0 + ImVec2(sensorOutHub.x, sGrpBot) * zoom, IM_COL32(200, 200, 200, 100), 5.0f*zoom, 0, 1.5f*zoom);
            dl->AddText(p0 + ImVec2(20.0f, sGrpTop + 5.0f)*zoom, IM_COL32(200, 200, 200, 150), "Sensors Grp");

            dl->AddRect(p0 + ImVec2(workerInHub.x, wGrpTop) * zoom, p0 + ImVec2(workerOutHub.x, wGrpBot) * zoom, IM_COL32(200, 200, 200, 100), 5.0f*zoom, 0, 1.5f*zoom);
            dl->AddText(p0 + ImVec2(workerInHub.x + 5.0f, wGrpTop + 5.0f)*zoom, IM_COL32(200, 200, 200, 150), "Workers Grp");

            // --- ТРУБИ ---
            ImU32 pipeCol = IM_COL32(110, 140, 160, 255);
            float pt = 3.0f * zoom;
            auto drawPipe = [&](ImVec2 p1, ImVec2 p2) { dl->AddLine(p0 + p1*zoom, p0 + p2*zoom, pipeCol, pt); };
            auto c  = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y + b.size.y/2); };
            auto rc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x, b.pos.y + b.size.y/2); };
            auto lc = [](const ArchBlock& b) { return ImVec2(b.pos.x, b.pos.y + b.size.y/2); };
            auto tc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y); };
            auto bc = [](const ArchBlock& b) { return ImVec2(b.pos.x + b.size.x/2, b.pos.y + b.size.y); };

            for(auto& s : sensors) drawPipe(rc(s), sensorOutHub);
            drawPipe(sensorOutHub, lc(rqPortSensor));

            drawPipe(lc(tcpBridge), rc(rqPortTcp));

            for(auto& w : workers) drawPipe(rc(w), workerOutHub);
            drawPipe(workerOutHub, ImVec2(workerOutHub.x + 20.0f, workerOutHub.y));
            drawPipe(ImVec2(workerOutHub.x + 20.0f, workerOutHub.y), ImVec2(workerOutHub.x + 20.0f, 30.0f));
            drawPipe(ImVec2(workerOutHub.x + 20.0f, 30.0f), ImVec2(c(rqPortWorker).x, 30.0f));
            drawPipe(ImVec2(c(rqPortWorker).x, 30.0f), tc(rqPortWorker));

            drawPipe(rc(queueWorker), workerInHub);
            for(auto& w : workers) drawPipe(workerInHub, lc(w));

            drawPipe(ImVec2(queueRouter.pos.x + queueRouter.size.x, c(router).y), lc(router));
            drawPipe(rc(router), ImVec2(queueWorker.pos.x, c(router).y));
            drawPipe(bc(router), tc(queueForward));
            drawPipe(bc(queueForward), tc(tcpBridge));

            // --- ХАБИ ---
            auto drawHub = [&](ImVec2 hPos) { dl->AddRectFilled(p0 + hPos*zoom - ImVec2(4,4)*zoom, p0 + hPos*zoom + ImVec2(4,4)*zoom, IM_COL32(200,200,200,255)); };
            drawHub(sensorOutHub); drawHub(workerInHub); drawHub(workerOutHub);

            // --- БЛОКИ ТА АНІМАЦІЯ ЗАТУХАННЯ ---
            auto drawBlock = [&](const ArchBlock& b, ImU32 col) {
                ImVec2 bp0 = p0 + b.pos * zoom; ImVec2 bp1 = bp0 + b.size * zoom;
                bool isBusy = busyWorkers.contains(b.pin) && busyWorkers.at(b.pin);
                ImU32 baseCol = isBusy ? IM_COL32(0, 200, 100, 255) : col;
                dl->AddRectFilled(bp0, bp1, baseCol, 3.0f * zoom);

                // Receive Flash (Green)
                if (recvFlashes.contains(b.pin)) {
                    float intensity = recvFlashes.at(b.pin);
                    dl->AddRectFilled(bp0, bp1, IM_COL32(0, 255, 100, static_cast<int>(200 * intensity)), 3.0f * zoom);
                }
                // Emit Flash (Yellow)
                if (emitFlashes.contains(b.pin)) {
                    float intensity = emitFlashes.at(b.pin);
                    dl->AddRectFilled(bp0, bp1, IM_COL32(255, 200, 0, static_cast<int>(200 * intensity)), 3.0f * zoom);
                }

                dl->AddRect(bp0, bp1, IM_COL32(255, 255, 255, 100), 3.0f * zoom);
                if (!b.name.empty()) {
                    ImVec2 tSz = ImGui::CalcTextSize(b.name.c_str());
                    dl->AddText(bp0 + ImVec2((b.size.x*zoom - tSz.x)/2.0f, (b.size.y*zoom - tSz.y)/2.0f), IM_COL32(255, 255, 255, 255), b.name.c_str());
                }
            };

            for (auto& s : sensors) drawBlock(s, IM_COL32(60, 60, 80, 255));
            for (auto& w : workers) drawBlock(w, IM_COL32(60, 80, 60, 255));
            drawBlock(router, IM_COL32(150, 100, 50, 255));
            drawBlock(tcpBridge, IM_COL32(80, 80, 120, 255));
            drawBlock(queueForward, IM_COL32(200, 100, 100, 200));

            drawBlock(queueRouter, IM_COL32(200, 150, 50, 150));
            drawBlock(rqPortWorker, IM_COL32(200, 150, 50, 200)); drawBlock(rqPortSensor, IM_COL32(200, 150, 50, 200)); drawBlock(rqPortTcp, IM_COL32(200, 150, 50, 200));
            drawBlock(queueWorker, IM_COL32(100, 200, 100, 150));

            // --- ЛІЧИЛЬНИКИ ---
            if (activeQueues.contains(getPin(ArchCompType::Queue_Router)) && activeQueues.at(getPin(ArchCompType::Queue_Router)) > 0) {
                std::string qStr = std::to_string(activeQueues.at(getPin(ArchCompType::Queue_Router)));
                dl->AddText(p0 + queueRouter.pos*zoom + ImVec2(5, queueRouter.size.y/2)*zoom, IM_COL32(255, 50, 50, 255), qStr.c_str());
            }

            // --- РЕЄСТРАЦІЯ TCP ПІНІВ ---
            pinRegistry[getPin(ArchCompType::TcpRx)] = p0 + ImVec2(tcpBridge.pos.x + tcpBridge.size.x * 0.3f, tcpBridge.pos.y + tcpBridge.size.y) * zoom;
            pinRegistry[getPin(ArchCompType::TcpTx)] = p0 + ImVec2(tcpBridge.pos.x + tcpBridge.size.x * 0.7f, tcpBridge.pos.y + tcpBridge.size.y) * zoom;
        }
    };
}