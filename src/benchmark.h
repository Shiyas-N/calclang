#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <string>
#include <chrono>
#include <vector>

using namespace std;

struct BenchmarkResult {
    string name;
    int optimizationLevel;
    double compilationTime;
    double executionTime;
    int instructionCount;
    size_t binarySize;
};

class Benchmarker {
public:
    Benchmarker();
    
    void startTimer();
    
    double stopTimer();
    
    void addResult(const BenchmarkResult& result);
    
    void printResults();
    
    void printComparison();
    
    static size_t getFileSize(const string& filename);
    
private:
    chrono::high_resolution_clock::time_point startTime;
    vector<BenchmarkResult> results;
    
    void printSeparator();
};

#endif 