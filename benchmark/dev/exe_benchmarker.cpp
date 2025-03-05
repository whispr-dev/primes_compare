#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>
#include <thread>

// Helper function to execute a command and get its output
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

class ExeBenchmarker {
private:
    struct Program {
        std::string name;
        std::string path;
        std::string arguments;
    };
    std::vector<Program> programs;
    int num_runs;

public:
    ExeBenchmarker(int runs = 10) : num_runs(runs) {}

    // Add a program to benchmark
    void addProgram(const std::string& name, const std::string& path, const std::string& args = "") {
        programs.push_back({name, path, args});
    }

    void runBenchmarks() {
        std::cout << "Benchmarking Executables:\n";
        std::cout << std::left << std::setw(30) << "Program" 
                  << std::setw(15) << "Total Time (ms)" 
                  << "Average Time (ms)\n";
        std::cout << std::string(60, '-') << '\n';

        for (const auto& program : programs) {
            std::vector<long long> run_times;

            for (int i = 0; i < num_runs; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                
                // Run the executable
                std::string cmd = "\"" + program.path + "\" " + program.arguments;
                exec(cmd.c_str());
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                run_times.push_back(duration);

                // Small delay between runs
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Calculate statistics
            long long total_time = 0;
            for (auto time : run_times) {
                total_time += time;
            }

            double avg_time = static_cast<double>(total_time) / num_runs / 1000.0;
            double total_time_ms = total_time / 1000.0;

            // Print results
            std::cout << std::left << std::setw(30) << program.name 
                      << std::setw(15) << std::fixed << std::setprecision(3) << total_time_ms
                      << avg_time << "\n";
        }
    }
};

int main() {
    ExeBenchmarker benchmark;

    // Add your executables to benchmark
    benchmark.addProgram("p-primes.exe", "C:/users/phine/desktop/primes_ben/dist/p-primes.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("p-primes-fast.exe", "C:/users/phine/desktop/primes_ben/dist/p-primes-fast.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("p-primes-faster.exe", "C:/users/phine/desktop/primes_ben/dist/p-primes-faster.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("p-primes-fastest.exe", "C:/users/phine/desktop/primes_ben/dist/p-primes-fastest.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("c-primes.exe", "C:/users/phine/desktop/primes_ben/dist/c-primes.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("c-primes-fast.exe", "C:/users/phine/desktop/primes_ben/dist/c-primes-fast.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("r-primes.exe", "C:/users/phine/desktop/primes_ben/dist/r-primes.exe");  // [arguments go in 3rd place in the brackets if there were any]
    benchmark.addProgram("r-primes-fast.exe", "C:/users/phine/desktop/primes_ben/dist/r-primes-fast.exe");  // [arguments go in 3rd place in the brackets if there were any]
  // Run benchmarks
    benchmark.runBenchmarks();

    return 0;
}
