#include "calculate/2SNO/pipeline.hpp"

int main() {
    using namespace NodeSystem::Testing::Calculate::twoSNO;

    Simulation sim(10000);
    sim.step(0, 0.9, 1)
       .step(1, 0.9, 5)
       .step(0, 0.9, 7)
       .save("matrix_result");

    return 0;
}