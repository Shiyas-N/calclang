#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "optimizer.h"
#include "backend.h"
#include "jit.h"
#include "benchmark.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input.calc> [options]" << endl;
        cerr << "Options:" << endl;
        cerr << "  -O<level>      Optimization level (0-3)" << endl;
        cerr << "  -o <file>      Output file name" << endl;
        cerr << "  --jit          Execute using JIT" << endl;
        cerr << "  --asm          Generate assembly file" << endl;
        cerr << "  --benchmark    Run full benchmark suite" << endl;
        return 1;
    }
    
    try {
        string inputFile = argv[1];
        string outputBase = "output";
        int optLevel = 2;
        bool useJIT = false;
        bool genAssembly = false;
        bool runBenchmark = false;
        
        for (int i = 2; i < argc; i++) {
            string arg = argv[i];
            if (arg.substr(0, 2) == "-O") {
                optLevel = stoi(arg.substr(2));
            } else if (arg == "-o" && i + 1 < argc) {
                outputBase = argv[++i];
            } else if (arg == "--jit") {
                useJIT = true;
            } else if (arg == "--asm") {
                genAssembly = true;
            } else if (arg == "--benchmark") {
                runBenchmark = true;
            }
        }
        
        string source = readFile(inputFile);
        cout << "=== Source Code ===" << endl;
        cout << source << endl << endl;
        
        cout << "=== Lexing ===" << endl;
        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();
        cout << "Generated " << tokens.size() << " tokens" << endl << endl;
        
        cout << "=== Parsing ===" << endl;
        Parser parser(tokens);
        auto program = parser.parse();
        cout << "Successfully parsed!" << endl << endl;
        
        if (runBenchmark) {
            Benchmarker benchmarker;
            
            for (int level = 0; level <= 3; level++) {
                cout << "\n=== BENCHMARK: Optimization Level -O" << level << " ===" << endl;
                
                CodeGenerator codegen("CalcLang");
                benchmarker.startTimer();
                codegen.generate(program.get());
                
                Optimizer optimizer(codegen.getModule());
                optimizer.optimizeLevel(level);
                double compileTime = benchmarker.stopTimer();
                
                CodeGenBackend backend(codegen.getModule());
                backend.initializeTarget();
                
                string objFile = outputBase + "_O" + to_string(level) + ".o";
                string exeFile = outputBase + "_O" + to_string(level);
                
                backend.emitObjectFile(objFile);
                backend.linkExecutable(objFile, exeFile);
                
                int instCount = 0;
                for (auto &F : *codegen.getModule()) {
                    for (auto &BB : F) {
                        for (auto &I : BB) {
                            instCount++;
                        }
                    }
                }
                
                BenchmarkResult result;
                result.name = "CalcLang";
                result.optimizationLevel = level;
                result.compilationTime = compileTime;
                result.executionTime = 0;
                result.instructionCount = instCount;
                result.binarySize = Benchmarker::getFileSize(exeFile);
                
                benchmarker.addResult(result);
            }
            
            benchmarker.printResults();
            benchmarker.printComparison();
            
            return 0;
        }
        
        CodeGenerator codegen("CalcLang");
        codegen.generate(program.get());
        
        Optimizer optimizer(codegen.getModule());
        optimizer.optimizeLevel(optLevel);
        
        cout << "=== Optimized LLVM IR ===" << endl;
        codegen.dump();
        codegen.writeToFile(outputBase + ".ll");
        
        if (useJIT) {
            JITCompiler jit(codegen.getModule());
            if (jit.initialize()) {
                jit.executeMain();
            }
        } else {
            CodeGenBackend backend(codegen.getModule());
            
            if (!backend.initializeTarget()) {
                cerr << "Failed to initialize target" << endl;
                return 1;
            }
            
            backend.printTargetInfo();
            
            string objFile = outputBase + ".o";
            if (!backend.emitObjectFile(objFile)) {
                cerr << "Failed to generate object file" << endl;
                return 1;
            }
            
            if (genAssembly) {
                string asmFile = outputBase + ".s";
                backend.emitAssembly(asmFile);
            }
            
            string exeFile = outputBase;
#ifdef _WIN32
            exeFile += ".exe";
#endif
            if (!backend.linkExecutable(objFile, exeFile)) {
                cerr << "Failed to link executable" << endl;
                return 1;
            }
            
            cout << "\n=== Compilation Complete ===" << endl;
            cout << "Executable: " << exeFile << endl;
            cout << "Run with: ./" << exeFile << endl;
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}