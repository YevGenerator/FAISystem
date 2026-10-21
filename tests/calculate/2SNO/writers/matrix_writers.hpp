#pragma once
#include <fstream>
#include <iomanip>
#include "../matrix_trace.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {

    class MatrixExporter {
    public:
        static void exportCsv(const std::string& filepath, const ProfessorMatrixBuilder& builder, char sep = ',') {
            std::ofstream out(filepath);
            const auto& headers = builder.headers();
            const auto rows = builder.buildRows();

            out << "Algr" << sep << "Phase" << sep << "Variable";
            for (const auto& h : headers) {
                out << sep << h;
            }
            out << "\n";

            for (const auto& r : rows) {
                out << r.algr << sep << r.phase << sep << r.variable;
                for (const auto& cell : r.cells) {
                    out << sep << cell;
                }
                out << "\n";
            }
        }

        static void exportMarkdown(const std::string& filepath, const ProfessorMatrixBuilder& builder) {
            std::ofstream out(filepath);
            const auto& headers = builder.headers();
            const auto rows = builder.buildRows();

            out << "| Algr | Phase | Variable |";
            for (const auto& h : headers) {
                out << " " << h << " |";
            }
            out << "\n|:---|:---|:---|";
            for (std::size_t i = 0; i < headers.size(); ++i) {
                out << ":---:|";
            }
            out << "\n";

            for (const auto& r : rows) {
                out << "| " << r.algr << " | " << r.phase << " | " << r.variable << " |";
                for (const auto& cell : r.cells) {
                    out << " " << (cell.empty() ? " " : cell) << " |";
                }
                out << "\n";
            }
        }

        static void exportText(const std::string& filepath, const ProfessorMatrixBuilder& builder) {
            std::ofstream out(filepath);
            const auto& headers = builder.headers();
            const auto rows = builder.buildRows();

            constexpr int W_Algr = 8;
            constexpr int W_Phase = 8;
            constexpr int W_Var = 11;
            constexpr int W_Col = 10;

            auto print_sep = [&]() {
                out << "+" << std::string(W_Algr + 2, '-')
                    << "+" << std::string(W_Phase + 2, '-')
                    << "+" << std::string(W_Var + 2, '-');
                for (std::size_t i = 0; i < headers.size(); ++i) {
                    out << "+" << std::string(W_Col + 2, '-');
                }
                out << "+\n";
            };

            print_sep();
            out << "| " << std::setw(W_Algr) << "Algr"
                << " | " << std::setw(W_Phase) << "Phase"
                << " | " << std::setw(W_Var) << "Variable";
            for (const auto& h : headers) {
                out << " | " << std::setw(W_Col) << h;
            }
            out << " |\n";
            print_sep();

            for (const auto& r : rows) {
                out << "| " << std::setw(W_Algr) << r.algr
                    << " | " << std::setw(W_Phase) << r.phase
                    << " | " << std::setw(W_Var) << r.variable;
                for (const auto& cell : r.cells) {
                    out << " | " << std::setw(W_Col) << cell;
                }
                out << " |\n";
            }
            print_sep();
        }

        static void exportAll(const std::string& base_filepath, const ProfessorMatrixBuilder& builder) {
            exportCsv(base_filepath + ".csv", builder);
            exportMarkdown(base_filepath + ".md", builder);
            exportText(base_filepath + ".txt", builder);
        }
    };

}