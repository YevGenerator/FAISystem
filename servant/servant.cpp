#include "../ZMCore/RpController.hpp"
#include "../coreі/Core/Timer.hpp"
using namespace NodeSystem::Core;
using RpServant = RpController<false>;
int main() {
    RpServant servant;
    Timer::init();
    servant.launchBlocking();
    return 0;
}
