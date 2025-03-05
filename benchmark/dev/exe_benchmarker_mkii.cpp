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
#include <algorithm>

static std::string exec(const char* cmd) {
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

    int num_runs;         // e.g. 10
    int repeats_per_run;  // e.g. 10

public:
    ExeBenchmarker(int runs = 10, int repeats = 10)
        : num_runs(runs), repeats_per_run(repeats) {}

    // Add a program to benchmark
    void addProgram(const std::string& name, const std::string& path, const std::string& args = "") {
        programs.push_back({name, path, args});
    }

    void runBenchmarks() {
        // We'll store the average time for each of the num_runs runs, for each program
        // results[programIndex][runIndex]
        std::vector<std::vector<long long>> results(
            programs.size(),
            std::vector<long long>(num_runs, 0)
        );

        // Perform the runs
        for (int runIndex = 0; runIndex < num_runs; ++runIndex) {
            for (size_t p = 0; p < programs.size(); ++p) {
                long long total_time = 0;
                for (int r = 0; r < repeats_per_run; ++r) {
                    auto start = std::chrono::high_resolution_clock::now();

                    // Build and execute command
                    std::string cmd = "\"" + programs[p].path + "\" " + programs[p].arguments;
                    exec(cmd.c_str());

                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    total_time += duration;
                }
                // Store the average microseconds for these repeats
                long long avg_time_us = total_time / repeats_per_run;
                results[p][runIndex] = avg_time_us;

                // Optional short delay between programs
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            // Optional short delay between runs
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Now print out a table comparing times for each run
        // We'll convert microseconds to milliseconds in the table
        std::cout << std::left << std::setw(10) << "Run #";
        for (auto& prog : programs) {
            // Print each program name as a column header
            std::cout << std::setw(20) << prog.name;
        }
        std::cout << "\n";

        // Print a simple separator
        std::cout << std::string(10 + programs.size() * 20, '-') << "\n";

        // For each run, print row: runIndex, then times for each program
        for (int runIndex = 0; runIndex < num_runs; ++runIndex) {
            std::cout << std::left << std::setw(10) << (runIndex + 1);
            for (size_t p = 0; p < programs.size(); ++p) {
                double time_ms = results[p][runIndex] / 1000.0; // microseconds -> milliseconds
                std::cout << std::setw(20) << std::fixed << std::setprecision(3) << time_ms;
            }
            std::cout << "\n";
        }

        // Print a row of averages across all runs for each program
        std::cout << std::string(10 + programs.size() * 20, '-') << "\n";
        std::cout << std::left << std::setw(10) << "Average";
        for (size_t p = 0; p < programs.size(); ++p) {
            long long sum_us = 0;
            for (int runIndex = 0; runIndex < num_runs; ++runIndex) {
                sum_us += results[p][runIndex];
            }
            // Average (in microseconds) of the 10 run averages
            double avg_us = static_cast<double>(sum_us) / num_runs;
            double avg_ms = avg_us / 1000.0;
            std::cout << std::setw(20) << std::fixed << std::setprecision(3) << avg_ms;
        }
        std::cout << "\n";
    }
};

int main() {
    // We'll do 10 runs, each run repeats the program 10 times
    ExeBenchmarker benchmark(10, 10);

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
