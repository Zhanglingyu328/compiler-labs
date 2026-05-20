#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string readFile(const fs::path& p) {
    std::ifstream in(p);
    if (!in) {
        throw std::runtime_error("cannot open input file: " + p.string());
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void writeText(const fs::path& p, const std::string& s) {
    std::ofstream out(p);
    if (!out) {
        throw std::runtime_error("cannot write file: " + p.string());
    }

    out << s;
}

static int analyzeOne(const fs::path& input, const fs::path& outDir, bool verbose) {
    std::string source = readFile(input);

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    SemanticContext sem;
    for (const auto& e : lexer.errors()) {
        sem.errors.push_back(e);
    }

    Parser parser(tokens, sem);
    AST root = parser.parseProgram();

    fs::create_directories(outDir);
    std::string stem = input.stem().string();

    std::ostringstream astOut;
    printAST(root, astOut);
    writeText(outDir / (stem + ".ast.txt"), astOut.str());

    std::ostringstream dotOut;
    writeDot(root, dotOut);
    writeText(outDir / (stem + ".ast.dot"), dotOut.str());

    std::ostringstream symOut;
    sem.symbols.print(symOut);
    writeText(outDir / (stem + ".symbols.txt"), symOut.str());

    std::ostringstream errOut;
    if (sem.errors.empty()) {
        errOut << "No semantic errors.\n";
    } else {
        errOut << "Semantic errors:\n";
        for (const auto& e : sem.errors) {
            errOut << "- " << e << "\n";
        }
    }

    if (!sem.warnings.empty()) {
        errOut << "\nWarnings:\n";
        for (const auto& w : sem.warnings) {
            errOut << "- " << w << "\n";
        }
    }

    writeText(outDir / (stem + ".errors.txt"), errOut.str());

    IRGenerator ir;
    ir.generate(root);
    ir.writeToFile((outDir / (stem + ".ir.txt")).string());

    if (verbose) {
        std::cout << "[OK] " << input << " -> " << outDir << "\n";
        std::cout << "     ast:     " << (outDir / (stem + ".ast.txt")) << "\n";
        std::cout << "     dot:     " << (outDir / (stem + ".ast.dot")) << "\n";
        std::cout << "     symbols: " << (outDir / (stem + ".symbols.txt")) << "\n";
        std::cout << "     errors:  " << (outDir / (stem + ".errors.txt")) << "\n";
        std::cout << "     ir:      " << (outDir / (stem + ".ir.txt")) << "\n";

        if (!sem.errors.empty()) {
            std::cout << "     semantic errors: " << sem.errors.size() << "\n";
        }

        if (!sem.warnings.empty()) {
            std::cout << "     warnings: " << sem.warnings.size() << "\n";
        }
    }

    return sem.errors.empty() ? 0 : 1;
}

static void usage(const char* exe) {
    std::cout << "Usage:\n"
              << "  " << exe << " <file.src> [-o output_dir]\n"
              << "  " << exe << " --batch <examples_dir> [-o output_dir]\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    try {
        fs::path outDir = "output";

        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "-o" && i + 1 < argc) {
                outDir = argv[++i];
            }
        }

        std::string first = argv[1];

        if (first == "--batch") {
            if (argc < 3) {
                usage(argv[0]);
                return 2;
            }

            fs::path dir = argv[2];

            if (!fs::exists(dir) || !fs::is_directory(dir)) {
                throw std::runtime_error("examples directory not found: " + dir.string());
            }

            std::vector<fs::path> inputs;

            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".src") {
                    inputs.push_back(entry.path());
                }
            }

            std::sort(inputs.begin(), inputs.end());

            int failCount = 0;

            for (const auto& p : inputs) {
                failCount += analyzeOne(p, outDir, true) ? 1 : 0;
            }

            std::cout << "Done. Total files: " << inputs.size()
                      << ", files with semantic errors: " << failCount << "\n";
            return 0;
        }

        fs::path input = first;

        if (!fs::exists(input)) {
            throw std::runtime_error("input file not found: " + input.string());
        }

        analyzeOne(input, outDir, true);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}