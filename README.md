# primes_compare
 comparing numbercrunching in c++; py; rust


Prime Number Generators Benchmark
This project compares the performance of prime number generators implemented in Python, C++, and Rust. It includes various optimization strategies and a comprehensive benchmarking system.
Project Structure
Copyprimes_benchmark/
├── src/
│   ├── python/
│   │   ├── p-primes.py          # Basic Python implementation
│   │   ├── p-primes-fast.py     # NumPy optimized version
│   │   ├── p-primes-faster.py   # Numba optimized version
│   │   └── p-primes-smart.py    # Adaptive implementation
│   ├── cpp/
│   │   ├── c-primes.cpp         # Basic C++ implementation
│   │   └── c-primes-fast.cpp    # Optimized C++ version
│   └── rust/
│       ├── r-primes.rs          # Basic Rust implementation
│       └── r-primes-fast.rs     # Optimized Rust version
├── benchmark/
│   └── cs_exe_benchmarker.cpp   # C++ benchmarking tool
└── README.md
Implementations
Python Versions

Basic (p-primes.py): Standard Sieve of Eratosthenes implementation
Fast (p-primes-fast.py): Uses NumPy for vectorized operations
Faster (p-primes-faster.py): Uses Numba for JIT compilation
Smart (p-primes-smart.py): Adaptive implementation that chooses:

Basic sieve for n < 1,000
NumPy for 1,000 ≤ n < 100,000
Numba for n ≥ 100,000



C++ Versions

Basic (c-primes.cpp): Standard implementation with basic optimizations
Fast (c-primes-fast.cpp): Optimized version with improved memory management

Rust Versions

Basic (r-primes.rs): Standard Rust implementation
Fast (r-primes-fast.rs): Optimized version with improved algorithms

Benchmarking Tool
The project includes a C++ benchmarking tool that:

Runs multiple iterations of each implementation
Calculates average, min, max, and standard deviation
Supports command-line arguments for different ranges
Handles timeouts and error conditions
Provides detailed performance metrics

Building and Running
Python Implementations
bashCopy# Install requirements
pip install numpy numba

# Build executables
pyinstaller --onefile src/python/p-primes.py
pyinstaller --onefile src/python/p-primes-fast.py
pyinstaller --onefile src/python/p-primes-faster.py
pyinstaller --onefile src/python/p-primes-smart.py
C++ Implementations
bashCopy# Build basic version
g++ -o c-primes.exe src/cpp/c-primes.cpp -std=c++17

# Build fast version
g++ -o c-primes-fast.exe src/cpp/c-primes-fast.cpp -std=c++17
Rust Implementations
bashCopy# Build basic version
rustc src/rust/r-primes.rs -o r-primes.exe

# Build fast version
rustc src/rust/r-primes-fast.rs -o r-primes-fast.exe
Building the Benchmarker
bashCopyg++ -o cs_exe_bm.exe benchmark/cs_exe_benchmarker.cpp -std=c++17
Running Benchmarks
The benchmarking tool can be run with:
bashCopy./cs_exe_bm.exe
For the smart Python implementation, you can specify the range:
bashCopy./p-primes-smart.exe 500     # Find primes up to 500 
./p-primes-smart.exe 50000   # Find primes up to 50,000
./p-primes-smart.exe 500000  # Find primes up to 500,000
Performance Results
Recent benchmark results show:

C++ and Rust implementations perform similarly well (50-60ms range)
Python implementations show higher overhead but better scaling with the smart version:

500 numbers: ~0.14ms (algorithm only)
50k numbers: ~0.37ms (algorithm only)
500k numbers: ~840ms (algorithm only)



Under system load, Rust shows excellent consistency with the lowest standard deviation.
Contributing
Feel free to contribute by:

Adding new implementations
Optimizing existing implementations
Improving the benchmarking tool
Adding new features or fixes

License
cc