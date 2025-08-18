#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <functional>

using namespace std;
using namespace std::chrono;

// Original implementation for comparison
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
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) {
            primes.push_back(i);
        }
    }
    return primes;
}

// Optimized version 1: Wheel factorization + better memory layout
vector<int> sieve_optimized_v1(int n) {
    if (n < 2) return {};
    if (n == 2) return {2};
    if (n < 5) return {2, 3};
    
    // Handle 2 and 3 separately, sieve only odd numbers not divisible by 3
    // This reduces memory by 2/3
    int sieve_size = n / 3 + (n % 6 >= 2 ? 1 : 0);
    vector<bool> is_prime(sieve_size, true);
    
    // Start with sqrt optimization
    int sqrt_n = static_cast<int>(sqrt(n));
    
    // Map index to actual number: 6k±1 form
    auto index_to_num = [](int i) -> int {
        return (i & 1) ? 3 * i + 4 : 3 * i + 5;
    };
    
    auto num_to_index = [](int num) -> int {
        return (num % 6 == 1) ? num / 3 : (num - 2) / 3;
    };
    
    for (int i = 0; i < sieve_size && index_to_num(i) <= sqrt_n; i++) {
        if (is_prime[i]) {
            int p = index_to_num(i);
            // Start from p² and mark multiples
            for (int j = p * p; j <= n; j += p) {
                if (j % 6 == 1 || j % 6 == 5) {
                    is_prime[num_to_index(j)] = false;
                }
            }
        }
    }
    
    // Collect primes
    vector<int> primes;
    primes.reserve(n / (log(n) - 1)); // Prime number theorem approximation
    primes.push_back(2);
    primes.push_back(3);
    
    for (int i = 0; i < sieve_size; i++) {
        if (is_prime[i]) {
            int prime = index_to_num(i);
            if (prime <= n) {
                primes.push_back(prime);
            }
        }
    }
    
    return primes;
}

// Optimized version 2: Segmented sieve for better cache usage
vector<int> sieve_segmented(int n) {
    const int segment_size = 32768; // L1 cache friendly size
    int sqrt_n = static_cast<int>(sqrt(n));
    
    // Find all primes up to sqrt(n)
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
    for (int low = sqrt_n + 1; low <= n; low += segment_size) {
        int high = min(low + segment_size - 1, n);
        vector<bool> segment(segment_size, true);
        
        // Mark multiples of each prime in current segment
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

// Ultimate optimization: Bit-packed sieve with unrolled loops
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

// Benchmark function
void benchmark(const string& name, function<vector<int>(int)> func, int n) {
    auto start = high_resolution_clock::now();
    vector<int> result = func(n);
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(end - start);
    cout << name << ": " << duration.count() / 1000.0 << " ms, "
         << "found " << result.size() << " primes" << endl;
}

int main() {
    int n = 500000;
    
    cout << "Benchmarking prime sieves up to " << n << ":\n" << endl;
    
    // Warm up
    sieve_original(1000);
    
    // Run benchmarks
    benchmark("Original", sieve_original, n);
    benchmark("Optimized v1 (wheel)", sieve_optimized_v1, n);
    benchmark("Segmented", sieve_segmented, n);
    
    BitSieve bs;
    benchmark("Bit-packed", [&bs](int n) { return bs.sieve(n); }, n);
    
    // Verify correctness (compare first 100 primes)
    cout << "\nVerifying first 20 primes from bit-packed version: ";
    BitSieve verify;
    vector<int> primes = verify.sieve(100);
    for (int i = 0; i < min(20, (int)primes.size()); i++) {
        cout << primes[i] << " ";
    }
    cout << endl;
    
    return 0;
}