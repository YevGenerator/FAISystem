#pragma once
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <iostream>
#include <format>
#include <algorithm>

#include "ArchTypes.hpp"
#include "../Core/logger/LogRecordList.hpp"
#include "GraphCanvas.hpp"
#include "VisualSensor.hpp"
#include "VisualNode.hpp"

namespace NodeSystem::Visual {
    using GlobalEventKey = std::pair<uint32_t, uint64_t>;
    inline GlobalEventKey makeKey(const Core::EventCounter::EventId &id) { return {id.deviceId, id.localEventId}; }

    struct ParsedEvent {
        Core::EventCounter::EventHeader header;
        uint64_t timeStamp;
        Core::types::Byte logType;
        std::function<void(GraphCanvas &)> applyToCanvas;

        int devId = -1;
        int level = -1;
        int index = -1;
        int slot = -1;
        Core::types::ProcessData data{};

        uint64_t traceKey() const {
            return ((uint64_t) header.traceId.deviceId << 32) | header.traceId.localEventId;
        }
    };

    template<Core::types::Byte value>
    void read() {
    }

    class LogParser {
        template<typename P>
        static constexpr auto readLogRecord(std::ifstream &in) {
            Core::LogRecordList::LogRecord<Core::LogRecordList::Variant> record;
            in.read(reinterpret_cast<char *>(&record.header), sizeof(record.header));
            in.read(reinterpret_cast<char *>(&record.data), sizeof(P));
            return record;
        }

        template<typename... T>
        static constexpr auto readHandlers() {
            return std::array{readLogRecord<T>...};
        }

    public:
        static ArchPinKey getArchPin(const ParsedEvent &ev) {
            using namespace Core::types::logs;
            using Registry = Core::LogRecordList;
            if (ev.logType == Registry::id<LogSensorOutEvent>) return {ev.devId, ArchCompType::Sensor, ev.index};
            if (ev.logType == Registry::id<LogRouterAccept>) return {ev.devId, ArchCompType::Router, 0};
            if (ev.logType == Registry::id<LogRouterSendInner>) return {ev.devId, ArchCompType::Router, 0};
            if (ev.logType == Registry::id<LogWorkerAccept>) return {ev.devId, ArchCompType::Worker, ev.slot};
            // slot = workerId
            if (ev.logType == Registry::id<LogWorkerSend>) return {ev.devId, ArchCompType::Worker, ev.slot};
            if (ev.logType == Registry::id<LogRouterSendForward>) return {ev.devId, ArchCompType::TcpTx, 0};
            if (ev.logType == Registry::id<LogWorkerAccept> ||
                ev.logType == Registry::id<LogWorkerSend> ||
                ev.logType == Registry::id<LogNodeProcessed> ||
                ev.logType == Registry::id<LogNodeInputFilled>) {
                int workerId = (ev.slot >= 0) ? ev.slot : 0;
                return {ev.devId, ArchCompType::Worker, workerId};
            }
            return {0, ArchCompType::None, 0};
        }

        static void parseAndExecute(const std::vector<std::string> &filenames, GraphCanvas &canvas,
                                    ArchitectureCanvas &archCanvas) {
            std::vector<ParsedEvent> allEvents;
            std::map<std::pair<int, uint32_t>, int> machineToLogicalId;
            for (int fileIndex = 0; fileIndex < filenames.size(); ++fileIndex) {
                std::ifstream in(filenames[fileIndex], std::ios::binary);
                if (!in) continue;
                Core::ID currentDeviceId{0};
                while (in.peek() != EOF) {
                    Core::types::Byte logType;
                    in.read(reinterpret_cast<char *>(&logType), 1);
                    in.seekg(-1, std::ios::cur);

                    using namespace Core::types::logs;
                    using Registry = Core::LogRecordList;

                    switch (logType) {
                        case Registry::id<LogSetDeviceId>: {
                            Registry::LogRecord<LogSetDeviceId> r;
                            in.read((char *) &r, sizeof(r));
                            machineToLogicalId[{fileIndex, r.header.eventId.deviceId}] = r.data.deviceId;
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data](GraphCanvas &c) {
                                    auto it = std::find_if(c.devices.begin(), c.devices.end(),
                                                           [&](const VisualDevice &dev) {
                                                               return dev.blockId == d.deviceId;
                                                           });
                                    if (it == c.devices.end()) c.devices.emplace_back(d.deviceId, ImVec2(0, 0));
                                }
                            });
                            currentDeviceId = r.data.deviceId;
                            archCanvas.devices[currentDeviceId].deviceId = currentDeviceId;
                            break;
                        }
                        case Registry::id<LogSetWorkers>: {
                            Registry::LogRecord<LogSetWorkers> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType,
                                [d=r.data, physDev=r.header.eventId.deviceId, fileIndex, &machineToLogicalId
                                ](GraphCanvas &c) {
                                    int targetId = machineToLogicalId[{fileIndex, physDev}];
                                    for (auto &dev: c.devices)
                                        if (dev.blockId == targetId)
                                            dev.workersNumber = d.workerAmount;
                                }
                            });
                            archCanvas.devices[currentDeviceId].workerCount = r.data.workerAmount;
                            break;
                        }
                        case Registry::id<LogSetIp>: {
                            Registry::LogRecord<LogSetIp> r;
                            in.read((char *) &r, sizeof(r));
                            break;
                        }
                        case Registry::id<LogRun>: {
                            Registry::LogRecord<LogRun> r;
                            in.read((char *) &r, sizeof(r));
                            break;
                        }
                        case Registry::id<LogCreateSensor>: {
                            Registry::LogRecord<LogCreateSensor> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data](GraphCanvas &c) {
                                    for (auto &dev: c.devices) {
                                        if (dev.blockId == (int) d.device) {
                                            bool exists = false;
                                            for (auto *b: dev.blocks)
                                                if (
                                                    b->blockType == SelectionType::Sensor && b->blockId == (int) d.
                                                    id_val) {
                                                    exists = true;
                                                    break;
                                                }
                                            if (!exists) {
                                                auto *s = new VisualSensor(d.device, d.id_val, {0, 0});
                                                s->period = d.period;
                                                s->toEmit = d.toEmit.get();
                                                dev.blocks.push_back(s);
                                            }
                                        }
                                    }
                                }
                            });
                            break;
                        }
                        case Registry::id<LogCreateNode>: {
                            Registry::LogRecord<LogCreateNode> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data](GraphCanvas &c) {
                                    for (auto &dev: c.devices) {
                                        if (dev.blockId == (int) d.device) {
                                            bool exists = false;
                                            for (auto *b: dev.blocks)
                                                if (
                                                    b->blockType == SelectionType::Node && b->getLevel() == d.id_lvl &&
                                                    b->
                                                    blockId == (int) d.id_val) {
                                                    exists = true;
                                                    break;
                                                }
                                            if (!exists) {
                                                auto *n = new Core::Nodes(
                                                    Core::types::NodeId{d.id_lvl, d.id_val}, d.inputs_count);
                                                n->algoType = d.algo;
                                                n->output.output = d.output;
                                                dev.blocks.push_back(new VisualNode(d.device, n, {0, 0}));
                                            }
                                        }
                                    }
                                }
                            });
                            break;
                        }
                        case Registry::id<LogNodeAddInput>: {
                            Registry::LogRecord<LogNodeAddInput> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data](GraphCanvas &c) {
                                    for (auto &dev: c.devices) {
                                        if (dev.blockId == (int) d.device) {
                                            for (auto *b: dev.blocks) {
                                                if (b->blockType == SelectionType::Node && b->blockId == (int) d.id_val
                                                    && b->getLevel() == d.id_lvl) {
                                                    auto *vNode = static_cast<VisualNode *>(b);
                                                    if (d.input_index < vNode->coreNode->inputs.size())
                                                        vNode->coreNode
                                                                ->addInput(d.input_data, d.input_index);
                                                }
                                            }
                                        }
                                    }
                                }
                            });
                            break;
                        }
                        case Registry::id<LogDoubleBind>: {
                            Registry::LogRecord<LogDoubleBind> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data, tr=r.header.traceId](GraphCanvas &c) {
                                    uint64_t tk = ((uint64_t) tr.deviceId << 32) | tr.localEventId;
                                    c.traceLinks[tk] = {
                                        (int) d.from.device, (int) d.from.id_lvl, (int) d.from.id_val,
                                        (int) d.to.device, (int) d.to.id_lvl, (int) d.to.id_val, (int) d.to.in_index
                                    };
                                }
                            });
                            break;
                        }
                        case Registry::id<LogSingleBind>: {
                            Registry::LogRecord<LogSingleBind> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({
                                r.header, r.timeStamp, logType, [d=r.data, tr=r.header.traceId](GraphCanvas &c) {
                                    uint64_t tk = ((uint64_t) tr.deviceId << 32) | tr.localEventId;
                                    if (!c.traceLinks.contains(tk) || c.traceLinks[tk].toSlot == -1) {
                                        c.traceLinks[tk] = {
                                            (int) d.from.device, (int) d.from.id_lvl, (int) d.from.id_val,
                                            (int) d.deviceTo, -1, (int) d.deviceTo, -1
                                        };
                                    }
                                }
                            });
                            break;
                        }

                        case Registry::id<LogSensorOutEvent>: {
                            Registry::LogRecord<LogSensorOutEvent> r;
                            in.read((char *) &r, sizeof(r));
                            ParsedEvent ev{};
                            ev.header = r.header;
                            ev.timeStamp = r.timeStamp;
                            ev.logType = logType;
                            ev.devId = (int) r.header.eventId.deviceId;
                            ev.level = 0;
                            ev.index = (int) r.data.result.from.index;
                            ev.data = r.data.result.data;
                            allEvents.push_back(ev);
                            break;
                        }
                        case Registry::id<LogWorkerAccept>: {
                            Registry::LogRecord<LogWorkerAccept> r;
                            in.read((char *) &r, sizeof(r));
                            ParsedEvent ev{};
                            ev.header = r.header;
                            ev.timeStamp = r.timeStamp;
                            ev.logType = logType;
                            ev.devId = (int) r.header.eventId.deviceId;
                            ev.level = (int) r.data.message.nodeId.level;
                            ev.index = (int) r.data.message.nodeId.index;
                            ev.slot = (int) r.data.message.slotId;
                            ev.data = r.data.message.data;
                            allEvents.push_back(ev);
                            break;
                        }

                        case Registry::id<LogWorkerSend>: {
                            Registry::LogRecord<LogWorkerSend> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogRouterAccept>: {
                            Registry::LogRecord<LogRouterAccept> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogRouterSendInner>: {
                            Registry::LogRecord<LogRouterSendInner> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogRouterSendForward>: {
                            Registry::LogRecord<LogRouterSendForward> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogNodeInputFilled>: {
                            Registry::LogRecord<LogNodeInputFilled> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogNodeProcessed>: {
                            Registry::LogRecord<LogNodeProcessed> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogNodeDenied>: {
                            Registry::LogRecord<LogNodeDenied> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogReceivedTcpCommand>: {
                            Registry::LogRecord<LogReceivedTcpCommand> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }
                        case Registry::id<LogSendTcpCommand>: {
                            Registry::LogRecord<LogSendTcpCommand> r;
                            in.read((char *) &r, sizeof(r));
                            allEvents.push_back({r.header, r.timeStamp, logType});
                            break;
                        }

                        default: {
                            std::cerr << "CRITICAL ERROR: Unknown Log Type.\n";
                            in.seekg(0, std::ios::end);
                            break;
                        }
                    }
                }
            }

            std::sort(allEvents.begin(), allEvents.end(), [](const ParsedEvent &a, const ParsedEvent &b) {
                return a.timeStamp < b.timeStamp;
            });
            for (auto &ev: allEvents) if (ev.applyToCanvas) ev.applyToCanvas(canvas);

            // ==========================================
            // ЕТАП 1: СИРЕ ДЕРЕВО (RAW CAUSALITY TREE)
            // ==========================================
            std::map<GlobalEventKey, ParsedEvent *> ptrMap;
            std::map<ParsedEvent *, std::vector<ParsedEvent *> > rawChildren;

            std::vector<ParsedEvent *> traceRoots; // Тільки SensorOut
            std::vector<ParsedEvent *> floatingAccepts; // TCP розриви

            // ПРОХІД 1: Реєструємо ВСІ події в ptrMap, незалежно від їхнього часу
            // Це вирішує "часовий парадокс" розсинхронізованих годинників
            for (auto &ev: allEvents) {
                ptrMap[makeKey(ev.header.eventId)] = &ev;
            }

            // ПРОХІД 2: Будуємо родинні зв'язки.
            // Тепер ми точно знаємо, що якщо батько існує десь у системі, ми його знайдемо!
            for (auto &ev: allEvents) {
                if (ev.header.parentId.isNull()) {
                    if (ev.logType == Core::LogRecordList::id<Core::types::logs::LogSensorOutEvent>) {
                        traceRoots.push_back(&ev);
                    }
                } else {
                    auto parentKey = makeKey(ev.header.parentId);
                    // Тепер parentKey гарантовано буде знайдено, навіть якщо дитина "старша" за батька в часі
                    if (ptrMap.contains(parentKey)) {
                        rawChildren[ptrMap[parentKey]].push_back(&ev);
                    } else if (ev.logType == Core::LogRecordList::id<Core::types::logs::LogWorkerAccept>) {
                        floatingAccepts.push_back(&ev);
                    }
                }
            }

            // СОРТУВАННЯ: Гарантує правильний порядок виконання дочірніх подій в межах одного пристрою
            for (auto &pair: rawChildren) {
                std::sort(pair.second.begin(), pair.second.end(), [](ParsedEvent *a, ParsedEvent *b) {
                    return a->header.eventId.localEventId < b->header.eventId.localEventId;
                });
            }

            // ==========================================
            // ЕТАП 2: КОЛАПСУВАННЯ (CHECKPOINT TREE)
            // Будуємо нове дерево тільки з Вузлів.
            // ==========================================
            std::map<ParsedEvent *, std::vector<ParsedEvent *> > checkpointChildren;
            std::vector<ParsedEvent *> allCheckpoints;

            std::function<void(ParsedEvent *, ParsedEvent *)> collapseTree = [&
                    ](ParsedEvent *current, ParsedEvent *lastCheckpoint) {
                ParsedEvent *nextCheckpoint = lastCheckpoint;

                // Якщо це приймання у вузлі - фіксуємо чекпоінт
                if (current->logType == Core::LogRecordList::id<Core::types::logs::LogWorkerAccept>) {
                    if (lastCheckpoint && lastCheckpoint != current) {
                        // Запобігаємо дублюванню (на випадок складних маршрутів)
                        auto &children = checkpointChildren[lastCheckpoint];
                        if (std::find(children.begin(), children.end(), current) == children.end()) {
                            children.push_back(current);
                        }
                    }
                    allCheckpoints.push_back(current);
                    nextCheckpoint = current; // Усі наступні події (включно з WorkerSend) успадкують цей чекпоінт
                }

                // Завдяки оновленому логуванню (CreateNetworkPacket з передачею parent),
                // дерево є строго ієрархічним. "Костиль" для братів (siblings) більше не потрібен!
                for (auto *child: rawChildren[current]) {
                    collapseTree(child, nextCheckpoint);
                }
            };

            for (auto *root: traceRoots) {
                allCheckpoints.push_back(root);
                collapseTree(root, root);
            }

            // ==========================================
            // ЕТАП 3: РЕМОНТ TCP (Fallback)
            // Тепер, коли ParentId передається мережею, цей етап потрібен
            // лише якщо якийсь з log.bin файлів не був завантажений у візуалізатор.
            // ==========================================

            for (auto *acc: floatingAccepts) {
                allCheckpoints.push_back(acc);
                uint64_t targetTrace = acc->traceKey();

                for (const auto &[bindTraceKey, link]: canvas.traceLinks) {
                    if (link.toDevice == acc->devId && link.toLevel == acc->level && link.toIndex == acc->index && link.
                        toSlot == acc->slot) {
                        for (auto *potentialParent: allCheckpoints) {
                            if (potentialParent->traceKey() == targetTrace &&
                                potentialParent->devId == link.fromDevice &&
                                potentialParent->level == link.fromLevel &&
                                potentialParent->index == link.fromIndex &&
                                potentialParent->timeStamp <= acc->timeStamp) {
                                auto &children = checkpointChildren[potentialParent];
                                if (std::find(children.begin(), children.end(), acc) == children.end()) {
                                    children.push_back(acc);
                                }

                                for (auto *child: rawChildren[acc]) {
                                    collapseTree(child, acc);
                                }
                                break;
                            }
                        }
                    }
                }
            }

            // ==========================================
            // ЕТАП 4: ГЕНЕРАЦІЯ ПОЛЬОТІВ (ЛОГІЧНИЙ ЧАС)
            // ==========================================
            std::map<ParsedEvent *, float> logicalTime;

            std::function<void(ParsedEvent *, float)> calculateTime = [&](ParsedEvent *ev, float timeBase) {
                if (!logicalTime.contains(ev) || logicalTime[ev] < timeBase) {
                    logicalTime[ev] = timeBase;
                }

                float childOffset = 0.0f;
                std::sort(checkpointChildren[ev].begin(), checkpointChildren[ev].end(),
                          [](ParsedEvent *a, ParsedEvent *b) {
                              return a->header.eventId.localEventId < b->header.eventId.localEventId;
                          });

                for (auto *child: checkpointChildren[ev]) {
                    float dt = 1.0f + childOffset;
                    calculateTime(child, logicalTime[ev] + dt);
                    childOffset += 0.3f;
                }
            };

            float globalOffset = 0.0f;
            for (auto *root: traceRoots) {
                // ФІЛЬТР: Якщо пакет не дійшов до жодного вузла (немає дітей-чекпоінтів)
                if (!checkpointChildren.contains(root) || checkpointChildren[root].empty()) {
                    continue;
                }

                calculateTime(root, globalOffset);
                globalOffset += 1.0f;

                canvas.availableTraces.insert(std::format("Trace-{}:{}", root->header.traceId.deviceId,
                                                          root->header.traceId.localEventId));
            }

            for (const auto &[parent, children]: checkpointChildren) {
                for (auto *child: children) {
                    std::string traceStr = std::format("Trace-{}:{}", parent->header.traceId.deviceId,
                                                       parent->header.traceId.localEventId);

                    PinKey fromPin = {parent->devId, parent->level, parent->index, -1};
                    PinKey toPin = {child->devId, child->level, child->index, child->slot};

                    std::string emitIdStr = std::format("{}:{}", parent->header.eventId.deviceId,
                                                        parent->header.eventId.localEventId);
                    std::string emitParentStr = parent->header.parentId.isNull()
                                                    ? "NULL"
                                                    : std::format("{}:{}", parent->header.parentId.deviceId,
                                                                  parent->header.parentId.localEventId);
                    std::string accIdStr = std::format("{}:{}", child->header.eventId.deviceId,
                                                       child->header.eventId.localEventId);

                    canvas.flights.push_back({
                        logicalTime[parent],
                        logicalTime[child],
                        traceStr,
                        fromPin,
                        toPin,
                        parent->data,
                        emitIdStr,
                        emitParentStr,
                        accIdStr
                    });

                    archCanvas.availableTraces.insert(traceStr);

                   // --- ГЕНЕРАЦІЯ АРХІТЕКТУРНИХ ФАЗ (БЕЗПЕРЕРВНИЙ ПОТІК) ---
                    float T_BASE = logicalTime[parent] * 10.0f;
                    float T_NEXT = logicalTime[child] * 10.0f;
                    float dt = T_NEXT - T_BASE;
                    if (dt <= 0.0f) dt = 2.0f; // Запобіжник, щоб час завжди йшов вперед

                    int devA = parent->devId;
                    int devB = child->devId;

                    ArchPinKey startPin;
                    if (parent->logType == Core::LogRecordList::id<Core::types::logs::LogSensorOutEvent>) {
                        startPin = {devA, ArchCompType::Sensor, parent->index};
                    } else {
                        startPin = {devA, ArchCompType::Worker, parent->slot};
                    }
                    ArchPinKey endPin = {devB, ArchCompType::Worker, child->slot};

                    if (devA == devB) {
                        // ЛОКАЛЬНИЙ МАРШРУТ (Dev A -> Dev A) - 4 відрізки
                        float step = dt / 4.0f;
                        float t1 = T_BASE + step;
                        float t2 = t1 + step;
                        float t3 = t2 + step;
                        float t4 = T_NEXT; // Точний збіг із початком наступного етапу!

                        ArchPinKey rq = {devA, ArchCompType::Queue_Router, 0};
                        ArchPinKey r  = {devA, ArchCompType::Router, 0};
                        ArchPinKey wq = {devA, ArchCompType::Queue_Worker, 0};

                        archCanvas.flights.push_back({T_BASE, t1, traceStr, startPin, rq, ""});
                        archCanvas.flights.push_back({t1, t2, traceStr, rq, r, ""});
                        archCanvas.flights.push_back({t2, t3, traceStr, r, wq, ""});
                        archCanvas.flights.push_back({t3, t4, traceStr, wq, endPin, ""});
                    } else {
                        // МЕРЕЖЕВИЙ МАРШРУТ (Dev A -> Dev B) - 9 відрізків
                        float step = dt / 9.0f;
                        float t1 = T_BASE + step;
                        float t2 = t1 + step;
                        float t3 = t2 + step;
                        float t4 = t3 + step;
                        float t5 = t4 + step; // Переліт по мережі
                        float t6 = t5 + step;
                        float t7 = t6 + step;
                        float t8 = t7 + step;
                        float t9 = T_NEXT; // Точний збіг!

                        ArchPinKey rqA = {devA, ArchCompType::Queue_Router, 0};
                        ArchPinKey rA  = {devA, ArchCompType::Router, 0};
                        ArchPinKey fqA = {devA, ArchCompType::Queue_Forward, 0};
                        ArchPinKey txA = {devA, ArchCompType::TcpTx, 0};

                        archCanvas.flights.push_back({T_BASE, t1, traceStr, startPin, rqA, ""});
                        archCanvas.flights.push_back({t1, t2, traceStr, rqA, rA, ""});
                        archCanvas.flights.push_back({t2, t3, traceStr, rA, fqA, ""});
                        archCanvas.flights.push_back({t3, t4, traceStr, fqA, txA, ""});

                        ArchPinKey rxB = {devB, ArchCompType::TcpRx, 0};
                        archCanvas.flights.push_back({t4, t5, traceStr, txA, rxB, ""});

                        ArchPinKey rqB = {devB, ArchCompType::Queue_Router, 0};
                        ArchPinKey rB  = {devB, ArchCompType::Router, 0};
                        ArchPinKey wqB = {devB, ArchCompType::Queue_Worker, 0};

                        archCanvas.flights.push_back({t5, t6, traceStr, rxB, rqB, ""});
                        archCanvas.flights.push_back({t6, t7, traceStr, rqB, rB, ""});
                        archCanvas.flights.push_back({t7, t8, traceStr, rB, wqB, ""});
                        archCanvas.flights.push_back({t8, t9, traceStr, wqB, endPin, ""});
                    }
                }
            }

            std::ranges::sort(canvas.flights, [](const Flight &a, const Flight &b) {
                return a.emitTime < b.emitTime;
            });
            if (!canvas.flights.empty()) canvas.currentLogTime = canvas.flights.front().emitTime;


            std::ranges::sort(archCanvas.flights, [](const ArchFlight &a, const ArchFlight &b) {
                return a.emitTime < b.emitTime;
            });

            float currentArchX = 50.0f;
            for (auto &graphDev: canvas.devices) {
                int devId = graphDev.blockId;
                auto &archDev = archCanvas.devices[devId];
                archDev.deviceId = devId;

                // --- ВИПРАВЛЕННЯ МАПІНГУ СЕНСОРІВ ---
                archDev.sensors.clear();
                for (auto *block: graphDev.blocks) {
                    if (block->blockType == SelectionType::Sensor) {
                        ArchBlock sBlock;
                        // Ми беремо справжній blockId (напр. 5), а не просто індекс по порядку!
                        sBlock.pin = {devId, ArchCompType::Sensor, block->blockId};
                        archDev.sensors.push_back(sBlock);
                    }
                }

                if (archDev.workerCount <= 0) archDev.workerCount = 1;

                archDev.performLayout();

                archDev.pos = ImVec2(currentArchX, 50.0f);
                currentArchX += archDev.size.x + 100.0f;
            }
        }
    };
}
