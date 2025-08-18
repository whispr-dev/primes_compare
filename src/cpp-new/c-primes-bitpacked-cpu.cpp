// bitpacked cpu-detect version
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
