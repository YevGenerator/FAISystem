#include "calculate/2SNO/pipeline.hpp"

int main() {
    using namespace NodeSystem::Testing::Calculate::twoSNO;
    using namespace std::chrono_literals;

    Simulation sim(10s);

    sim.step(0, 0.9, 100ms)
       .step(1, 0.65, 500ms)
       .step(2, 0.65, 900ms)
       .save("matrix_result");

    return 0;
}