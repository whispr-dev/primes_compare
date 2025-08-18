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

// Platform detection and bit scan functions
#ifdef _WIN64
    // 64-bit Windows
    inline int ctz32(uint32_t x) {
        unsigned long index;
        _BitScanForward(&index, x);
        return index;
    }
    
    inline int ctz64(uint64_t x) {
        unsigned long index;
        _BitScanForward64(&index, x);
        return index;
    }
#else
    // 32-bit Windows - no _BitScanForward64
    inline int ctz32(uint32_t x) {
        unsigned long index;
        _BitScanForward(&index, x);
        return index;
    }
    
    inline int ctz64(uint64_t x) {
        // For 32-bit, handle 64-bit values in two parts
        uint32_t low = (uint32_t)x;
        if (low) {
            return ctz32(low);
        }
        uint32_t high = (uint32_t)(x >> 32);
        return 32 + ctz32(high);
    }
#endif

// CPU feature detection
bool has_avx2() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);
    int nIds = cpuInfo[0];
    
    if (nIds >= 7) {
        __cpuidex(cpuInfo, 7, 0);
        return (cpuInfo[1] & (1 << 5)) != 0;  // AVX2 is EBX bit 5
    }
    return false;
}

// Original baseline implementation
vector<int> sieve_original(int n) {
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = false;
    is_prime[1] = false;

    for (int p = 2; p * p <= n; p++) {
        if (is_prime[p]) {
            for (int multiple = p * p; multiple <= n; multiple += p) {
                is_prime[multiple] = false;
            }
        }
    }

    vector<int> primes;
    primes.reserve(n / (log(n) - 1));
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) {
            primes.push_back(i);
        }
    }
    return primes;
}

// Bit-packed version (works on both x86 and x64)
class BitPackedSieve {
private:
    vector<uint32_t> bits;
    int size;
    
public:
    vector<int> sieve(int n) {
        if (n < 2) return {};
        
        size = n;
        int bit_words = (n >> 6) + 1;
        bits.resize(bit_words, 0xFFFFFFFF);
        
        // Clear bit for 1
        bits[0] &= ~1u;
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Mark composites
        for (int p = 3; p <= sqrt_n; p += 2) {
            int word_idx = p >> 6;
            int bit_idx = (p >> 1) & 31;
            
            if (bits[word_idx] & (1u << bit_idx)) {
                int step = p << 1;
                for (int num = p * p; num <= n; num += step) {
                    int idx_word = num >> 6;
                    int idx_bit = (num >> 1) & 31;
                    bits[idx_word] &= ~(1u << idx_bit);
                }
            }
        }
        
        // Collect primes
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        primes.push_back(2);
        
        for (int word = 0; word < bit_words; word++) {
            uint32_t bits_word = bits[word];
            while (bits_word) {
                int bit_pos = ctz32(bits_word);
                int prime = (word * 64) + (bit_pos * 2) + 1;
                if (prime <= n) {
                    primes.push_back(prime);
                }
                bits_word &= bits_word - 1;
            }
        }
        
        return primes;
    }
};

// Segmented sieve for better cache usage
class SegmentedSieve {
private:
    static constexpr int SEGMENT_SIZE = 32768;
    
public:
    vector<int> sieve(int n) {
        if (n < 2) return {};
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Find primes up to sqrt(n)
        vector<bool> is_prime_small(sqrt_n + 1, true);
        is_prime_small[0] = is_prime_small[1] = false;
        
        for (int p = 2; p * p <= sqrt_n; p++) {
            if (is_prime_small[p]) {
                for (int i = p * p; i <= sqrt_n; i += p) {
                    is_prime_small[i] = false;
                }
            }
        }
        
        vector<int> primes_small;
        for (int i = 2; i <= sqrt_n; i++) {
            if (is_prime_small[i]) {
                primes_small.push_back(i);
            }
        }
        
        vector<int> primes = primes_small;
        primes.reserve(n / (log(n) - 1));
        
        // Process segments
        for (int low = sqrt_n + 1; low <= n; low += SEGMENT_SIZE) {
            int high = min(low + SEGMENT_SIZE - 1, n);
            vector<bool> segment(SEGMENT_SIZE, true);
            
            // Mark multiples in segment
            for (int p : primes_small) {
                int start = ((low + p - 1) / p) * p;
                if (start == p) start = p * p;
                
                for (int j = start; j <= high; j += p) {
                    segment[j - low] = false;
                }
            }
            
            // Collect primes from segment
            for (int i = low; i <= high; i++) {
                if (segment[i - low]) {
                    primes.push_back(i);
                }
            }
        }
        
        return primes;
    }
};

// Parallel segmented sieve using threads - FIXED VERSION
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

// AVX2 optimized version (only if AVX2 is available)
#ifdef __AVX2__
class AVX2Sieve {
private:
    alignas(32) vector<uint64_t> bits;
    int size;
    
public:
    vector<int> sieve(int n) {
        if (n < 2) return {};
        
        size = n;
        int bit_words = ((n >> 1) >> 6) + 1;
        int aligned_words = ((bit_words + 3) / 4) * 4;
        bits.resize(aligned_words, 0xFFFFFFFFFFFFFFFFULL);
        
        bits[0] &= ~1ULL;
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        for (int p = 3; p <= sqrt_n; p += 2) {
            int bit_idx = p >> 1;
            if (bits[bit_idx >> 6] & (1ULL << (bit_idx & 63))) {
                for (int num = p * p; num <= n; num += p * 2) {
                    int idx = num >> 1;
                    bits[idx >> 6] &= ~(1ULL << (idx & 63));
                }
            }
        }
        
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        primes.push_back(2);
        
        for (int i = 0; i < bit_words; i++) {
            uint64_t word = bits[i];
            while (word) {
                int bit_pos = ctz64(word);
                int prime = (i * 128) + (bit_pos * 2) + 1;
                if (prime <= n) {
                    primes.push_back(prime);
                }
                word &= word - 1;
            }
        }
        
        return primes;
    }
};
#endif

// Benchmark function
void benchmark(const string& name, function<vector<int>(int)> func, int n) {
    // Warm up
    func(1000);
    
    const int runs = 3;
    double total_time = 0;
    vector<int> result;
    
    for (int i = 0; i < runs; i++) {
        auto start = high_resolution_clock::now();
        result = func(n);
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start);
        total_time += duration.count() / 1000.0;
    }
    
    cout << name << ": " << (total_time / runs) << " ms (avg of " << runs << " runs), "
         << "found " << result.size() << " primes" << endl;
}

int main() {
    cout << "Prime Sieve Optimizations Benchmark" << endl;
    cout << "====================================" << endl;
    
    // System info
    cout << "\nSystem Information:" << endl;
#ifdef _WIN64
    cout << "Platform: Windows x64 (64-bit)" << endl;
#else
    cout << "Platform: Windows x86 (32-bit)" << endl;
#endif
    cout << "Hardware threads: " << thread::hardware_concurrency() << endl;
    cout << "AVX2 support: " << (has_avx2() ? "YES" : "NO") << endl;
    
    int n = 500000;
    cout << "\nBenchmarking with n = " << n << endl;
    cout << "----------------------------------------" << endl;
    
    // Run benchmarks
    benchmark("Original", sieve_original, n);
    
    BitPackedSieve bp;
    benchmark("Bit-packed", [&bp](int n) { return bp.sieve(n); }, n);
    
    SegmentedSieve seg;
    benchmark("Segmented", [&seg](int n) { return seg.sieve(n); }, n);
    
    ParallelSieve par;
    benchmark("Parallel", [&par](int n) { return par.sieve(n); }, n);
    
#ifdef __AVX2__
    if (has_avx2()) {
        AVX2Sieve avx2;
        benchmark("AVX2", [&avx2](int n) { return avx2.sieve(n); }, n);
    }
#endif
    
    // Test with larger value
    n = 10000000;
    cout << "\nBenchmarking with n = " << n << endl;
    cout << "----------------------------------------" << endl;
    
    benchmark("Original", sieve_original, n);
    benchmark("Parallel", [&par](int n) { return par.sieve(n); }, n);
    
    // Verify correctness
    cout << "\nVerification (first 20 primes):" << endl;
    BitPackedSieve verify;
    vector<int> primes = verify.sieve(100);
    for (int i = 0; i < min(20, (int)primes.size()); i++) {
        cout << primes[i] << " ";
    }
    cout << endl;
    
    return 0;
}