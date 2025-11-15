#include "benchmark.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Format.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

using namespace llvm;

Benchmarker::Benchmarker() {}

void Benchmarker::startTimer() {
    startTime = std::chrono::high_resolution_clock::now();
}

double Benchmarker::stopTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime);
    return duration.count() / 1000.0;
}

void Benchmarker::addResult(const BenchmarkResult& result) {
    results.push_back(result);
}

void Benchmarker::printSeparator() {
    errs() << "+";
    for (int i = 0; i < 80; i++) errs() << "-";
    errs() << "+\n";
}

void Benchmarker::printResults() {
    errs() << "\n";
    printSeparator();
    
    std::string header = "BENCHMARK RESULTS";
    int padding = (78 - header.length()) / 2;
    errs() << "| ";
    for (int i = 0; i < padding; i++) errs() << " ";
    errs() << header;
    for (int i = 0; i < (78 - header.length() - padding); i++) errs() << " ";
    errs() << "|\n";
    
    printSeparator();
    
    for (const auto& result : results) {
        std::stringstream ss;
        
        ss << "Optimization Level: -O" << result.optimizationLevel;
        errs() << "| " << ss.str();
        for (size_t i = 0; i < (78 - ss.str().length()); i++) errs() << " ";
        errs() << "|\n";
        
        ss.str("");
        ss << "  Compilation Time:  " << std::fixed << std::setprecision(2) 
           << result.compilationTime << " ms";
        errs() << "| " << ss.str();
        for (size_t i = 0; i < (78 - ss.str().length()); i++) errs() << " ";
        errs() << "|\n";
        
        ss.str("");
        ss << "  Execution Time:    " << std::fixed << std::setprecision(2)
           << result.executionTime << " ms";
        errs() << "| " << ss.str();
        for (size_t i = 0; i < (78 - ss.str().length()); i++) errs() << " ";
        errs() << "|\n";
        
        ss.str("");
        ss << "  Instructions:      " << result.instructionCount;
        errs() << "| " << ss.str();
        for (size_t i = 0; i < (78 - ss.str().length()); i++) errs() << " ";
        errs() << "|\n";
        
        ss.str("");
        ss << "  Binary Size:       " << result.binarySize << " bytes";
        errs() << "| " << ss.str();
        for (size_t i = 0; i < (78 - ss.str().length()); i++) errs() << " ";
        errs() << "|\n";
        
        printSeparator();
    }
}

void Benchmarker::printComparison() {
    if (results.empty()) return;
    
    errs() << "\n=== OPTIMIZATION COMPARISON ===\n\n";
    
    std::stringstream ss;
    
    ss << std::left << std::setw(15) << "Level"
       << std::setw(15) << "Compile(ms)"
       << std::setw(15) << "Execute(ms)"
       << std::setw(15) << "Instructions"
       << std::setw(15) << "Size(bytes)";
    errs() << ss.str() << "\n";
    
    errs() << std::string(75, '-') << "\n";
    
    // Data rows
    for (const auto& result : results) {
        ss.str("");
        ss << std::left << std::setw(15) << ("-O" + std::to_string(result.optimizationLevel))
           << std::fixed << std::setprecision(2)
           << std::setw(15) << result.compilationTime
           << std::setw(15) << result.executionTime
           << std::setw(15) << result.instructionCount
           << std::setw(15) << result.binarySize;
        errs() << ss.str() << "\n";
    }
    
    errs() << "\n";
    
    // Speedup analysis
    if (results.size() > 1) {
        double baseExecTime = results[0].executionTime;
        double bestExecTime = results.back().executionTime;
        
        if (baseExecTime > 0 && bestExecTime > 0) {
            double speedup = baseExecTime / bestExecTime;
            
            ss.str("");
            ss << "Performance Improvement: " 
               << std::fixed << std::setprecision(2) 
               << speedup << "x faster";
            errs() << ss.str() << "\n";
        }
        
        int baseInstructions = results[0].instructionCount;
        int optInstructions = results.back().instructionCount;
        double reduction = (baseInstructions - optInstructions) * 100.0 / baseInstructions;
        
        ss.str("");
        ss << "Instruction Reduction: " 
           << std::fixed << std::setprecision(1)
           << reduction << "%";
        errs() << ss.str() << "\n\n";
    }
}

size_t Benchmarker::getFileSize(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}