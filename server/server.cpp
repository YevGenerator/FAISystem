#include "../coreі/ZMCore/RpController.hpp"
#include "../coreі/Core/Timer.hpp"
using namespace NodeSystem::Core;
using RPServer = RpController<true>;
int main() {
    RPServer server;
    Timer::init();
    server.launchBlocking();
    return 0;
}
