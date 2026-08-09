#include <fstream>

#include "../Timer.hpp"
#include "../nodes/Node.hpp"
#include <iostream>
void print(NodeSystem::Core::Algo::AlgoResult result) {
    std::cout << "a: " << result.alpha << "  toSwitch:" << result.toSwitch << '\n';
}

auto main2() -> int {
    double alpha_test = 0.9;
    constexpr NodeSystem::Core::Nodes::KgOutput defaultOut{.c = true, .a = 0.85};
    NodeSystem::Core::Nodes::InputDataSlot input;
    input.kgt = {.c = true, .a = 0.75, .b = 0, .v = 0.1, .g = 0.33};

    NodeSystem::Core::Nodes::Node node1;
    node1.nodeId = {1, 1};
    node1.algoType = 0;
    node1.output = defaultOut;
    node1.inputs = {input, input, input};
    NodeSystem::Core::Nodes::NodeTimedProcessMessage message;
    auto startTime = NodeSystem::Core::Timer::elapsedNow();
    node1.inputs[0].theta = startTime;
    node1.inputs[1].theta = startTime;
    node1.inputs[2].theta = startTime;

    message.alpha = alpha_test;
    const auto t_dif = 200;
    message.slotId = 0;
    message.theta = NodeSystem::Core::Timer::add<std::chrono::microseconds, std::chrono::seconds>
            (message.theta, t_dif).count();
    auto result4 = node1.acceptMessage(message);
    print(result4);

    message.slotId = 1;
    message.theta = NodeSystem::Core::Timer::add<std::chrono::microseconds, std::chrono::seconds>
        (message.theta, t_dif).count();

    auto result5 = node1.acceptMessage(message);
    print(result5);

    message.slotId = 2;
    message.theta = NodeSystem::Core::Timer::add<std::chrono::microseconds, std::chrono::seconds>
        (message.theta, t_dif).count();

    auto result6 = node1.acceptMessage(message);
    print(result6);

    return 0;
}

auto main3()-> int {
    double b[] = {0.0, 5, 10, 15, 20};
    auto a = 0.5;
    for (int i = 0; i < 5; i++) {
        auto mx_mean = NodeSystem::Core::Algo::AlgoTwoSNO::m_x_mean(a, b[i]);
        std::cout << "\t" << mx_mean << "\n";
        auto k_beta = NodeSystem::Core::Algo::AlgoTwoSNO::k_beta(0.5, mx_mean);
        std::cout << k_beta << "\n";
    }
    return 0;
}

struct Row {
    double alpha;
    double beta;
    double mx;
};

int main() {
    std::vector<Row> table;
    table.reserve(100);

    for (double alpha = -1.0; alpha <= 1.0; alpha += 0.25) {
        for (double beta = 0.0; beta <= 1.0; beta += 0.1) {
            auto mx_mean = NodeSystem::Core::Algo::AlgoTwoSNO::m_x_mean(alpha, beta);
            table.push_back({alpha, beta, mx_mean});
        }
    }

    std::ofstream file("results.csv");
    if (file.is_open()) {
        file << "alpha,beta,mx_mean\n"; // Хедер
        for (const auto &[alpha, beta, mx_mean]: table) {
            file << std::format("{:.4f},{:.4f},{:.4f}\n", alpha, beta, mx_mean);
        }
    }

    std::ofstream md_file("results.md");
    if (md_file.is_open()) {
        md_file << "|  alpha  |  beta  |  mx_mean  |\n";
        md_file << "| :-----: | :----: | :-------: |\n";

        for (const auto &[alpha, beta, mx_mean] : table) {
            md_file << std::format("| {:>7.4f} | {:>6.4f} | {:>9.4f} |\n",
                                   alpha, beta, mx_mean);
        }
    }
}