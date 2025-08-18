// Parallel segmented sieve using threads
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>
#include <immintrin.h>
#include <intrin.h>

using namespace std;
using namespace std::chrono;

class ParallelSieve {
private:
    static constexpr int SEGMENT_SIZE = 131072;  // Larger segments to reduce overhead
    vector<int> small_primes;
    
    void sieve_segment(int low, int high, vector<bool>& segment) {
        int segment_size = high - low + 1;
        fill(segment.begin(), segment.begin() + segment_size, true);
        
        for (int p : small_primes) {
            int start = ((low + p - 1) / p) * p;
            if (start == p) start = p * p;
            
            for (int j = start; j <= high; j += p) {
                segment[j - low] = false;
            }
        }
    }
    
public:
    vector<int> sieve(int n) {
        if (n < 2) return {};
        
        // For small n, just use the bit-packed version
        if (n < 1000000) {
            BitPackedSieve bp;
            return bp.sieve(n);
        }
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Find small primes
        vector<bool> is_prime_small(sqrt_n + 1, true);
        is_prime_small[0] = is_prime_small[1] = false;
        
        for (int p = 2; p * p <= sqrt_n; p++) {
            if (is_prime_small[p]) {
                for (int i = p * p; i <= sqrt_n; i += p) {
                    is_prime_small[i] = false;
                }
            }
        }
        
        for (int i = 2; i <= sqrt_n; i++) {
            if (is_prime_small[i]) {
                small_primes.push_back(i);
            }
        }
        
        vector<int> all_primes = small_primes;
        all_primes.reserve(n / (log(n) - 1));
        
        // Calculate segments properly
        int num_threads = thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        
        // Reduce thread count for smaller ranges to avoid overhead
        int total_range = n - sqrt_n;
        int segments_needed = (total_range + SEGMENT_SIZE - 1) / SEGMENT_SIZE;
        if (segments_needed < num_threads) {
            num_threads = max(1, segments_needed);
        }
        
        vector<thread> threads;
        vector<vector<int>> thread_primes(num_threads);
        atomic<int> next_segment(0);
        
        auto worker = [&](int thread_id) {
            vector<bool> segment(SEGMENT_SIZE);
            vector<int>& local_primes = thread_primes[thread_id];
            local_primes.reserve(SEGMENT_SIZE / 10);  // Avoid reallocations
            
            while (true) {
                int segment_idx = next_segment.fetch_add(1);
                int low = sqrt_n + 1 + segment_idx * SEGMENT_SIZE;
                
                if (low > n) break;
                
                int high = min(low + SEGMENT_SIZE - 1, n);
                sieve_segment(low, high, segment);
                
                // Only iterate through actual segment range
                int segment_end = high - low + 1;
                for (int i = 0; i < segment_end; i++) {
                    if (segment[i]) {
                        local_primes.push_back(low + i);
                    }
                }
            }
        };
        
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(worker, i);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        for (const auto& tp : thread_primes) {
            all_primes.insert(all_primes.end(), tp.begin(), tp.end());
        }
        
        sort(all_primes.begin(), all_primes.end());
        
        return all_primes;
    }
};
