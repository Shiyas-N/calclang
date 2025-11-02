mkdir build

cd build

cmake ..

cmake --build . --config Release

.\bin\calclang ..\examples\test.calc

clang output.ll -o program.exe

.\program.exe
