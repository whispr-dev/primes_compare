#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>
#include <thread>
#include <algorithm>
#include <sstream>
#include <cmath>

class ExeBenchmarker {
    private:
    struct Program {
        std::string name;
        std::string path;
        std::string arguments;
        std::vector<long long> run_times;
    };
    std::vector<Program> programs;
    int num_runs;
    int repeats_per_run;
    bool verbose;
    int timeout_ms;  // Added timeout as class member

    // Improved process execution with configurable timeout
    static std::string exec(const char* cmd, int timeout_ms) {
        std::array<char, 128> buffer;
        std::string result;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
            
            // Check if we've exceeded timeout
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
            if (elapsed > timeout_ms) {
                pclose(pipe.release());  // Force close the pipe
                throw std::runtime_error("Process execution timed out");
            }
        }
        return result;
    }

public:
    // Updated constructor with timeout parameter
    ExeBenchmarker(int runs = 10, int repeats = 10, bool verbose_output = false, int timeout = 30000)
        : num_runs(runs), repeats_per_run(repeats), verbose(verbose_output), timeout_ms(timeout) {}

    void addProgram(const std::string& name, const std::string& path, const std::string& args) {
        Program prog{name, path, args, std::vector<long long>()};
        prog.run_times.reserve(num_runs);
        programs.push_back(prog);
    }
    
    void printProgress(int current_run, int total_runs, const std::string& program_name) {
        if (verbose) {
            std::cout << "\rProgress: Run " << current_run << "/" << total_runs 
                      << " - Testing: " << program_name << std::flush;
        }
    }

    void runBenchmarks() {
        std::cout << "Starting benchmarks with " << num_runs << " runs, " 
                  << repeats_per_run << " repeats per run...\n\n";

        // Perform the runs
        for (int run = 0; run < num_runs; ++run) {
            for (auto& program : programs) {
                printProgress(run + 1, num_runs, program.name);
                
                try {
                    long long total_time = 0;
                    for (int r = 0; r < repeats_per_run; ++r) {
                        auto start = std::chrono::high_resolution_clock::now();
                        
                        std::string cmd = "\"" + program.path + "\" " + program.arguments;
                        exec(cmd.c_str(), timeout_ms);  // Using class member timeout
                        
                        auto end = std::chrono::high_resolution_clock::now();
                        total_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                        
                        // Small delay between repeats
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    
                    program.run_times.push_back(total_time / repeats_per_run);
                }
                catch (const std::runtime_error& e) {
                    std::cerr << "\nError running " << program.name << ": " << e.what() << "\n";
                    program.run_times.push_back(-1);  // Mark as failed
                }
                
                // Delay between programs
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        if (verbose) {
            std::cout << "\n\n";
        }

        // Print results table
        printResults();
    }

private:
    void printResults() {
        // Print header
        std::cout << std::left << std::setw(25) << "Program";
        std::cout << std::setw(15) << "Avg (ms)";
        std::cout << std::setw(15) << "Min (ms)";
        std::cout << std::setw(15) << "Max (ms)";
        std::cout << "StdDev (ms)\n";
        std::cout << std::string(70, '-') << "\n";

        // Calculate and print statistics for each program
        for (const auto& program : programs) {
            // Filter out failed runs
            std::vector<long long> valid_times;
            std::copy_if(program.run_times.begin(), program.run_times.end(),
                        std::back_inserter(valid_times),
                        [](long long t) { return t >= 0; });

            if (valid_times.empty()) {
                std::cout << std::left << std::setw(25) << program.name << "Failed to run\n";
                continue;
            }

            // Calculate statistics
            double avg = 0, min = valid_times[0], max = valid_times[0], variance = 0;
            
            // Calculate average
            for (auto time : valid_times) {
                avg += time;
                min = std::min(min, (double)time);
                max = std::max(max, (double)time);
            }
            avg /= valid_times.size();

            // Calculate variance
            for (auto time : valid_times) {
                variance += (time - avg) * (time - avg);
            }
            variance /= valid_times.size();
            double stddev = std::sqrt(variance);

            // Convert to milliseconds and print
            std::cout << std::left << std::setw(25) << program.name;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << std::setw(15) << avg / 1000.0;
            std::cout << std::setw(15) << min / 1000.0;
            std::cout << std::setw(15) << max / 1000.0;
            std::cout << stddev / 1000.0 << "\n";
        }
    }
};

int main() {
    try {
        ExeBenchmarker benchmark(10, 10, true, 30000);

        // Smart Python implementation with different ranges
        benchmark.addProgram("p-primes-smart-500", "C:/users/phine/desktop/primes_ben/dist/p-primes-smart.exe", "500");
        benchmark.addProgram("p-primes-smart-50k", "C:/users/phine/desktop/primes_ben/dist/p-primes-smart.exe", "50000");
        benchmark.addProgram("p-primes-smart-500k", "C:/users/phine/desktop/primes_ben/dist/p-primes-smart.exe", "500000");

        // Original implementations (assuming they don't take arguments)
        benchmark.addProgram("c-primes", "C:/users/phine/desktop/primes_ben/dist/c-primes.exe", "");
        benchmark.addProgram("c-primes-fast", "C:/users/phine/desktop/primes_ben/dist/c-primes-fast.exe", "");
        benchmark.addProgram("r-primes", "C:/users/phine/desktop/primes_ben/dist/r-primes.exe", "");
        benchmark.addProgram("r-primes-fast", "C:/users/phine/desktop/primes_ben/dist/r-primes-fast.exe", "");

        benchmark.runBenchmarks();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}