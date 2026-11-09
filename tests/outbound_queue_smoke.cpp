#include <cassert>
#include <thread>
#include <vector>
#include "ZMCore/OutboundQueue.hpp"
#include "commands/CommandList.hpp"

int main() {
    using namespace NodeSystem;
    ZMCore::OutboundQueue queue;
    const auto packet = Core::Commands::CommandList::CreatePacket(Core::Commands::CmdRun{2, true});
    queue.push(2, packet);
    queue.push(3, packet);
    auto initial = queue.drain();
    assert(initial.size() == 2);
    assert(initial[0].deviceId == 2);
    assert(initial[1].deviceId == 3);
    assert(initial[0].bytes.size() == sizeof(packet));
    assert(queue.drain().empty());

    constexpr int senders = 4;
    constexpr int packetsPerSender = 100;
    std::vector<std::thread> threads;
    for (int id = 0; id < senders; ++id) {
        threads.emplace_back([&, id] {
            for (int i = 0; i < packetsPerSender; ++i)
                queue.push(static_cast<Core::types::ID>(id), packet);
        });
    }
    for (auto& thread : threads) thread.join();
    assert(queue.drain().size() == senders * packetsPerSender);
}
