// Optimized version 1: Wheel factorization + better memory layout
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <functional>

using namespace std;
using namespace std::chrono;

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
