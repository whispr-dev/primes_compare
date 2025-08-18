// Ultimate optimization: Bit-packed sieve with unrolled loops
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <functional>

using namespace std;
using namespace std::chrono;

class BitSieve {
private:
    vector<uint32_t> bits;
    int size;
    
    void unmark(int n) {
        bits[n >> 6] &= ~(1u << ((n >> 1) & 31));
    }
    
    bool is_marked(int n) const {
        return bits[n >> 6] & (1u << ((n >> 1) & 31));
    }
    
public:
    vector<int> sieve(int n) {
        if (n < 2) return {};
        
        size = n;
        int bit_size = (n >> 6) + 1;
        bits.resize(bit_size, 0xFFFFFFFF);
        
        // Clear even numbers (except 2)
        unmark(1);
        
        int sqrt_n = static_cast<int>(sqrt(n));
        
        // Unrolled loop for small primes
        for (int i = 3; i <= sqrt_n; i += 2) {
            if (is_marked(i)) {
                int step = i << 1;
                int j = i * i;
                
                // Unroll by 4 for better pipeline usage
                for (; j + 3 * step <= n; j += 4 * step) {
                    unmark(j);
                    unmark(j + step);
                    unmark(j + 2 * step);
                    unmark(j + 3 * step);
                }
                
                // Handle remainder
                for (; j <= n; j += step) {
                    unmark(j);
                }
            }
        }
        
        // Collect primes
        vector<int> primes;
        primes.reserve(n / (log(n) - 1));
        primes.push_back(2);
        
        for (int i = 3; i <= n; i += 2) {
            if (is_marked(i)) {
                primes.push_back(i);
            }
        }
        
        return primes;
    }
};
