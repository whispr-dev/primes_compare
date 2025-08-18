// Optimized version 2: Segmented sieve for better cache usage
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <functional>

using namespace std;
using namespace std::chrono;

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
