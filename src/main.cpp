#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "optimizer.h"

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
        cerr << "Usage: " << argv[0] << " <input.calc> [-o output.ll]" << endl;
        return 1;
    }
    
    try {
        string source = readFile(argv[1]);
        cout << "=== Source Code ===" << endl;
        cout << source << endl << endl;
        
        cout << "=== Lexing ===" << endl;
        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();
        cout << "Generated " << tokens.size() << " tokens" << endl << endl;
        
        cout << "=== Parsing ===" << endl;
        Parser parser(tokens);
        auto program = parser.parse();
        cout << "Successfully parsed!" << endl;
        cout << "Functions: " << program->functions.size() << endl;
        cout << "Statements: " << program->statements.size() << endl << endl;
        
        cout << "=== Generating LLVM IR ===" << endl;
        CodeGenerator codegen("CalcLang");
        codegen.generate(program.get());
        cout << "Code generation successful!" << endl << endl;
        
        cout << "=== LLVM IR Output ===" << endl;
        codegen.dump();


        // Run optimizations
        
        Optimizer optimizer(codegen.getModule());
        optimizer.optimize();
        
        // Output optimized IR
        cout << "\n=== Optimized LLVM IR ===" << endl;
        codegen.dump();

        
        string outputFile = "output.ll";
        if (argc >= 4 && string(argv[2]) == "-o") {
            outputFile = argv[3];
        }
        
        codegen.writeToFile(outputFile);
        cout << "\nIR written to: " << outputFile << endl;
        
        cout << "\n=== Compilation Complete ===" << endl;
        cout << "To execute: " << endl;
        cout << "  clang " << outputFile << " -o program" << endl;
        cout << "  ./program" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}