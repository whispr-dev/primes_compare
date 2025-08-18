// AVX2 optimized version (only if AVX2 is available)
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