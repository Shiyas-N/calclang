mkdir build

cd build

cmake ..

cmake --build . --config Release

.\bin\calclang ..\examples\test.calc

clang output.ll -o program.exe

.\program.exe

Basic Compilation:

cd bin
./calclang ../../examples/comprehensive_test.calc -O2 -o myprogram
./myprogram

Jit execution:

./calclang ../../examples/comprehensive_test.calc --jit

Generate Assembly:

./calclang ../../examples/comprehensive_test.calc -O3 --asm -o optimized
cat optimized.s

Full Benchmark:

./calclang ../../examples/comprehensive_test.calc --benchmark
