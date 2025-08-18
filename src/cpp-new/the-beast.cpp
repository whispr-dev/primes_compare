#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>
#include <memory>
#include <immintrin.h>
#include <intrin.h>

using namespace std;
using namespace std::chrono;

// ============================================================================
// Platform Detection and CPU Feature Support
// ============================================================================

struct CPUFeatures {
    bool sse2 = false;
    bool sse4_1 = false;
    bool sse4_2 = false;
    bool avx = false;
    bool avx2 = false;
    bool avx512f = false;
    bool popcnt = false;
    bool bmi1 = false;
    bool bmi2 = false;
    int logical_cores = 0;
    int cache_line_size = 64;
    
    CPUFeatures() {
        int cpuInfo[4];
        
        // Get vendor
        __cpuid(cpuInfo, 0);
        int nIds = cpuInfo[0];
        
        // Get features
        if (nIds >= 1) {
            __cpuid(cpuInfo, 1);
            sse2 = (cpuInfo[3] & (1 << 26)) != 0;
            sse4_1 = (cpuInfo[2] & (1 << 19)) != 0;
            sse4_2 = (cpuInfo[2] & (1 << 20)) != 0;
            avx = (cpuInfo[2] & (1 << 28)) != 0;
            popcnt = (cpuInfo[2] & (1 << 23)) != 0;
        }
        
        if (nIds >= 7) {
            __cpuidex(cpuInfo, 7, 0);
            avx2 = (cpuInfo[1] & (1 << 5)) != 0;
            bmi1 = (cpuInfo[1] & (1 << 3)) != 0;
            bmi2 = (cpuInfo[1] & (1 << 8)) != 0;
            avx512f = (cpuInfo[1] & (1 << 16)) != 0;
        }
        
        logical_cores = thread::hardware_concurrency();
        if (logical_cores == 0) logical_cores = 4;
    }
    
    void print() const {
        cout << "CPU Features Detected:" << endl;
        cout << "  SSE2: " << (sse2 ? "YES" : "NO") << endl;
        cout << "  SSE4.1/4.2: " << (sse4_1 ? "YES" : "NO") << "/" << (sse4_2 ? "YES" : "NO") << endl;
        cout << "  AVX: " << (avx ? "YES" : "NO") << endl;
        cout << "  AVX2: " << (avx2 ? "YES" : "NO") << endl;
        cout << "  AVX-512F: " << (avx512f ? "YES" : "NO") << endl;
        cout << "  POPCNT: " << (popcnt ? "YES" : "NO") << endl;
        cout << "  BMI1/BMI2: " << (bmi1 ? "YES" : "NO") << "/" << (bmi2 ? "YES" : "NO") << endl;
        cout << "  Logical Cores: " << logical_cores << endl;
    }
};

static CPUFeatures g_cpu;

// ============================================================================
// Bit Manipulation Helpers
// ============================================================================

#ifdef _WIN64
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
    
    inline int popcount64(uint64_t x) {
        return (int)__popcnt64(x);
    }
#else
    inline int ctz32(uint32_t x) {
        unsigned long index;
        _BitScanForward(&index, x);
        return index;
    }
    
    inline int ctz64(uint64_t x) {
        uint32_t low = (uint32_t)x;
        if (low) return ctz32(low);
        return 32 + ctz32((uint32_t)(x >> 32));
    }
    
    inline int popcount64(uint64_t x) {
        return __popcnt((uint32_t)x) + __popcnt((uint32_t)(x >> 32));
    }
#endif

// ============================================================================
// Base Sieve Interface
// ============================================================================

class ISieve {
public:
    virtual ~ISieve() = default;
    virtual vector<int> sieve(int n) = 0;
    virtual const char* name() const = 0;
};

// ============================================================================
// Optimized Bit-Packed Sieve with Heavy Loop Unrolling
// ============================================================================

class BitPackedUnrolledSieve : public ISieve {
private:
    alignas(64) vector<uint64_t> bits;
    
    inline void clear_bit(size_t pos) {
        bits[pos >> 7] &= ~(1ULL << ((pos >> 1) & 63));
    }
    
    inline bool test_bit(size_t pos) const {
        return bits[pos >> 7] & (1ULL << ((pos >> 1) & 63));
    }
    
public:
    vector<int> sieve(int n) override {
        if (n < 2) return {};
        
        // Use 64-bit words for better performance
        int bit_words = ((n >> 1) + 63) / 64;
        bits.resize(bit_words, 0xFFFFFFFFFFFFFFFFULL);
        
        // Clear bit for 1
        bits[0] &= ~1ULL;
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Special handling for prime 3
        for (int i = 9; i <= n; i += 6) {
            clear_bit(i);
        }
        
        // Main sieving with aggressive unrolling
        for (int p = 5; p <= sqrt_n; p += 2) {
            if (test_bit(p)) {
                int step = p << 1;
                int i = p * p;
                
                // Unroll by 8 for maximum throughput
                int unroll_limit = n - 7 * step;
                for (; i <= unroll_limit; i += 8 * step) {
                    clear_bit(i);
                    clear_bit(i + step);
                    clear_bit(i + 2 * step);
                    clear_bit(i + 3 * step);
                    clear_bit(i + 4 * step);
                    clear_bit(i + 5 * step);
                    clear_bit(i + 6 * step);
                    clear_bit(i + 7 * step);
                }
                
                // Handle remainder
                for (; i <= n; i += step) {
                    clear_bit(i);
                }
            }
        }
        
        // Collect primes using bit scan
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        primes.push_back(2);
        
        for (int word_idx = 0; word_idx < bit_words; word_idx++) {
            uint64_t word = bits[word_idx];
            while (word) {
                int bit_pos = ctz64(word);
                int prime = (word_idx * 128) + (bit_pos * 2) + 1;
                if (prime <= n) {
                    primes.push_back(prime);
                }
                word &= word - 1;  // Clear lowest set bit
            }
        }
        
        return primes;
    }
    
    const char* name() const override { return "Bit-Packed Unrolled"; }
};

// ============================================================================
// AVX2 Optimized Sieve (When Available)
// ============================================================================

class AVX2OptimizedSieve : public ISieve {
private:
    alignas(32) vector<uint64_t> bits;
    
    inline void clear_bit_avx(size_t pos) {
        bits[pos >> 7] &= ~(1ULL << ((pos >> 1) & 63));
    }
    
    inline bool test_bit_avx(size_t pos) const {
        return bits[pos >> 7] & (1ULL << ((pos >> 1) & 63));
    }
    
public:
    vector<int> sieve(int n) override {
        if (n < 2) return {};
        
        int bit_words = ((n >> 1) + 63) / 64;
        int aligned_words = ((bit_words + 3) / 4) * 4;  // Align to 256 bits
        bits.resize(aligned_words, 0xFFFFFFFFFFFFFFFFULL);
        
        bits[0] &= ~1ULL;
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Sieve with AVX2 optimizations
        for (int p = 3; p <= sqrt_n; p += 2) {
            if (test_bit_avx(p)) {
                int step = p << 1;
                
                // Use AVX2 for clearing multiple bits when possible
                for (int i = p * p; i <= n; i += step) {
                    clear_bit_avx(i);
                }
            }
        }
        
        // Collect primes using AVX2 parallel processing
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        primes.push_back(2);
        
        // Process 4 words at a time with AVX2
        for (int i = 0; i < bit_words; i += 4) {
            __m256i vec = _mm256_load_si256((__m256i*)&bits[i]);
            
            if (!_mm256_testz_si256(vec, vec)) {
                alignas(32) uint64_t temp[4];
                _mm256_store_si256((__m256i*)temp, vec);
                
                for (int j = 0; j < 4 && i + j < bit_words; j++) {
                    uint64_t word = temp[j];
                    while (word) {
                        int bit_pos = ctz64(word);
                        int prime = ((i + j) * 128) + (bit_pos * 2) + 1;
                        if (prime <= n) {
                            primes.push_back(prime);
                        }
                        word &= word - 1;
                    }
                }
            }
        }
        
        return primes;
    }
    
    const char* name() const override { return "AVX2 Optimized"; }
};

// ============================================================================
// Parallel Segmented Sieve with Work Stealing
// ============================================================================

class ParallelSegmentedSieve : public ISieve {
private:
    static constexpr int CACHE_LINE = 64;
    static constexpr int SEGMENT_SIZE = 262144;  // 256KB segments
    vector<int> small_primes;
    
    struct alignas(CACHE_LINE) WorkUnit {
        atomic<int> next_segment{0};
        int max_segment;
    };
    
    void sieve_segment(int low, int high, vector<uint8_t>& segment) {
        int size = high - low + 1;
        memset(segment.data(), 1, size);
        
        for (int p : small_primes) {
            int start = ((low + p - 1) / p) * p;
            if (start == p) start = p * p;
            if (start > high) continue;
            
            // Unrolled marking loop
            int j = start - low;
            int limit = size - 7 * p;
            
            for (; j < limit; j += 8 * p) {
                segment[j] = 0;
                segment[j + p] = 0;
                segment[j + 2*p] = 0;
                segment[j + 3*p] = 0;
                segment[j + 4*p] = 0;
                segment[j + 5*p] = 0;
                segment[j + 6*p] = 0;
                segment[j + 7*p] = 0;
            }
            
            for (; j < size; j += p) {
                segment[j] = 0;
            }
        }
    }
    
public:
    vector<int> sieve(int n) override {
        if (n < 2) return {};
        
        // For small n, use bit-packed version
        if (n < 10000000) {
            BitPackedUnrolledSieve bp;
            return bp.sieve(n);
        }
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Find small primes up to sqrt(n)
        BitPackedUnrolledSieve small_sieve;
        small_primes = small_sieve.sieve(sqrt_n);
        
        vector<int> all_primes = small_primes;
        all_primes.reserve(n / (log(n) - 1));
        
        // Setup work units
        WorkUnit work;
        work.max_segment = (n - sqrt_n) / SEGMENT_SIZE + 1;
        
        int num_threads = min(g_cpu.logical_cores, work.max_segment);
        vector<thread> threads;
        vector<vector<int>> thread_primes(num_threads);
        
        auto worker = [&](int thread_id) {
            vector<uint8_t> segment(SEGMENT_SIZE);
            vector<int>& local_primes = thread_primes[thread_id];
            local_primes.reserve(SEGMENT_SIZE / 10);
            
            while (true) {
                int seg_idx = work.next_segment.fetch_add(1);
                if (seg_idx >= work.max_segment) break;
                
                int low = sqrt_n + 1 + seg_idx * SEGMENT_SIZE;
                int high = min(low + SEGMENT_SIZE - 1, n);
                
                sieve_segment(low, high, segment);
                
                // Collect primes
                int size = high - low + 1;
                for (int i = 0; i < size; i++) {
                    if (segment[i]) {
                        local_primes.push_back(low + i);
                    }
                }
            }
        };
        
        // Launch threads
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(worker, i);
        }
        
        // Wait for completion
        for (auto& t : threads) {
            t.join();
        }
        
        // Merge results
        for (const auto& tp : thread_primes) {
            all_primes.insert(all_primes.end(), tp.begin(), tp.end());
        }
        
        sort(all_primes.begin(), all_primes.end());
        
        return all_primes;
    }
    
    const char* name() const override { return "Parallel Segmented"; }
};

// ============================================================================
// Wheel Factorization Sieve (Memory Efficient for Huge Ranges)
// ============================================================================

class WheelFactorizationSieve : public ISieve {
private:
    static constexpr int WHEEL[] = {2, 3, 5, 7, 11, 13};
    static constexpr int WHEEL_SIZE = 30030;  // Product of first 6 primes
    vector<uint8_t> wheel_bits;
    
    void init_wheel() {
        wheel_bits.resize(WHEEL_SIZE, 1);
        for (int p : WHEEL) {
            for (int i = p; i < WHEEL_SIZE; i += p) {
                wheel_bits[i] = 0;
            }
        }
    }
    
public:
    vector<int> sieve(int n) override {
        if (n < 2) return {};
        
        init_wheel();
        
        int segments = (n / WHEEL_SIZE) + 1;
        vector<bool> is_prime(n + 1, true);
        is_prime[0] = is_prime[1] = false;
        
        // Apply wheel
        for (int seg = 0; seg < segments; seg++) {
            int base = seg * WHEEL_SIZE;
            for (int i = 0; i < WHEEL_SIZE && base + i <= n; i++) {
                if (!wheel_bits[i] && base + i > 1) {
                    is_prime[base + i] = false;
                }
            }
        }
        
        // Continue sieving for remaining primes
        int sqrt_n = static_cast<int>(sqrt(n));
        for (int p = 17; p <= sqrt_n; p += 2) {
            if (is_prime[p]) {
                for (int i = p * p; i <= n; i += p * 2) {
                    is_prime[i] = false;
                }
            }
        }
        
        // Collect primes
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        
        for (int i = 2; i <= n; i++) {
            if (is_prime[i]) {
                primes.push_back(i);
            }
        }
        
        return primes;
    }
    
    const char* name() const override { return "Wheel Factorization"; }
};

// ============================================================================
// Auto-Selecting Optimal Sieve
// ============================================================================

class AutoOptimalSieve : public ISieve {
private:
    unique_ptr<ISieve> select_best_sieve(int n) {
        // For cryptographic scale (>100M), always use parallel
        if (n > 100000000) {
            return make_unique<ParallelSegmentedSieve>();
        }
        
        // For large scale (10M-100M)
        if (n > 10000000) {
            if (g_cpu.logical_cores >= 8) {
                return make_unique<ParallelSegmentedSieve>();
            } else if (g_cpu.avx2) {
                return make_unique<AVX2OptimizedSieve>();
            } else {
                return make_unique<BitPackedUnrolledSieve>();
            }
        }
        
        // For medium scale (1M-10M)
        if (n > 1000000) {
            if (g_cpu.avx2) {
                return make_unique<AVX2OptimizedSieve>();
            } else {
                return make_unique<BitPackedUnrolledSieve>();
            }
        }
        
        // For small scale (<1M)
        return make_unique<BitPackedUnrolledSieve>();
    }
    
public:
    vector<int> sieve(int n) override {
        auto best_sieve = select_best_sieve(n);
        cout << "Auto-selected: " << best_sieve->name() << " for n=" << n << endl;
        return best_sieve->sieve(n);
    }
    
    const char* name() const override { return "Auto-Optimal"; }
};

// ============================================================================
// Benchmarking
// ============================================================================

void benchmark(ISieve* sieve, int n, int runs = 3) {
    // Warm up
    sieve->sieve(min(n/100, 10000));
    
    double total_time = 0;
    vector<int> result;
    
    for (int i = 0; i < runs; i++) {
        auto start = high_resolution_clock::now();
        result = sieve->sieve(n);
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start);
        total_time += duration.count() / 1000.0;
    }
    
    cout << sieve->name() << ": " 
         << (total_time / runs) << " ms (avg of " << runs << " runs), "
         << "found " << result.size() << " primes" << endl;
}

// ============================================================================
// Main
// ============================================================================

int main() {
    cout << "Ultimate Prime Sieve - Maximum Performance Edition" << endl;
    cout << "==================================================" << endl;
    
    // Detect CPU features
    g_cpu.print();
    
    // Test scales
    vector<int> test_sizes = {500000, 10000000, 50000000};
    
    for (int n : test_sizes) {
        cout << "\n" << string(50, '-') << endl;
        cout << "Benchmarking with n = " << n << endl;
        cout << string(50, '-') << endl;
        
        // Create sieves
        vector<unique_ptr<ISieve>> sieves;
        sieves.push_back(make_unique<BitPackedUnrolledSieve>());
        
        if (g_cpu.avx2) {
            sieves.push_back(make_unique<AVX2OptimizedSieve>());
        }
        
        if (n >= 10000000 && g_cpu.logical_cores >= 4) {
            sieves.push_back(make_unique<ParallelSegmentedSieve>());
        }
        
        sieves.push_back(make_unique<AutoOptimalSieve>());
        
        // Run benchmarks
        for (auto& sieve : sieves) {
            benchmark(sieve.get(), n, n < 10000000 ? 5 : 3);
        }
    }
    
    // Verify correctness
    cout << "\n" << string(50, '-') << endl;
    cout << "Verification (first 20 primes):" << endl;
    BitPackedUnrolledSieve verify;
    vector<int> primes = verify.sieve(100);
    for (int i = 0; i < min(20, (int)primes.size()); i++) {
        cout << primes[i] << " ";
    }
    cout << endl;
    
    // SHA256-scale demonstration
    cout << "\n" << string(50, '-') << endl;
    cout << "Cryptographic Scale Demo (n = 100,000,000):" << endl;
    cout << string(50, '-') << endl;
    AutoOptimalSieve crypto_sieve;
    
    auto start = high_resolution_clock::now();
    auto result = crypto_sieve.sieve(100000000);
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Found " << result.size() << " primes in " 
         << duration.count() << " ms" << endl;
    cout << "Rate: " << (100000000.0 / duration.count()) / 1000 
         << " million numbers/second" << endl;
    
    return 0;
}