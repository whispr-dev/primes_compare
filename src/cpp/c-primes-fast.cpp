// to compile`g++ -O3 -march=native -flto -fopenmp c_sieve.cpp -o c-primes.exe`

#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

// Helper macros for bit manipulation
#define SET_BIT(arr, idx)   (arr[(idx) >> 6] |= (1ULL << ((idx) & 63)))
#define CLEAR_BIT(arr, idx) (arr[(idx) >> 6] &= ~(1ULL << ((idx) & 63)))
#define TEST_BIT(arr, idx)  (arr[(idx) >> 6] & (1ULL << ((idx) & 63)))

// This function returns a list of primes up to n.
std::vector<unsigned long long> sieve_odd_bitset_parallel(unsigned long long n) {
    if (n < 2) {
        return {};
    }
    if (n == 2) {
        return {2ULL};
    }

    // We'll store only odd numbers in the bitset.
    // Index i corresponds to the odd number = 2*i + 1.
    // So we need (n+1)/2 bits to represent all odd numbers up to n.
    // We'll skip 0 and 1 as well, but that's minor overhead.

    unsigned long long size = (n >> 1) + 1; // roughly (n/2 + 1)
    // Each 64-bit block can hold 64 bits
    std::vector<unsigned long long> bitset((size + 63) / 64, 0ULL);

    // Initialize all bits to 1 (mark as potential primes) except index 0 => number 1 => not prime
    // We'll do this by setting them all, then clearing the 1 index.
    #pragma omp parallel for
    for (std::size_t i = 0; i < bitset.size(); i++) {
        bitset[i] = 0xFFFFFFFFFFFFFFFFULL; // set all 64 bits to 1
    }
    CLEAR_BIT(bitset, 0);  // index 0 corresponds to number 1 (not prime)

    // Now let's do the sieve
    unsigned long long limit = (unsigned long long)std::sqrt((long double)n);

    // We handle p=2 separately:
    // 2 is prime, but we're only marking odd numbers in the bitset.
    // We'll skip marking multiples of 2. (Though you could if you want.)

    // For p = 3, 5, 7, ..., up to limit
    // index i => p = 2*i+1
    unsigned long long max_i = (limit >> 1);
    
    // Parallel outer loop
    #pragma omp parallel for schedule(dynamic)
    for (long long i = 1; i <= (long long)max_i; i++) {
        // i=1 => p=3, i=2 => p=5, etc.
        if (TEST_BIT(bitset, i)) { 
            unsigned long long p = (i << 1) + 1ULL; 
            unsigned long long start = (p * p) >> 1; // index of p^2
            // Mark multiples of p as composite
            for (unsigned long long multiple = start; multiple < size; multiple += p) {
                CLEAR_BIT(bitset, multiple);
            }
        }
    }

    // Collect primes from the bitset
    std::vector<unsigned long long> primes;
    primes.reserve(size / 10); // rough guess for capacity
    primes.push_back(2ULL);    // add the even prime

    for (unsigned long long i = 1; i < size; i++) {
        if (TEST_BIT(bitset, i)) {
            unsigned long long p = (i << 1) + 1ULL;
            if (p <= n) {
                primes.push_back(p);
            } else {
                break;
            }
        }
    }
    return primes;
}

int main() {
    unsigned long long n = 500000ULL; // example up to 10 million
    auto primes = sieve_odd_bitset_parallel(n);
    std::cout << "Found " << primes.size() << " primes up to " << n << ".\n";
    if (!primes.empty()) {
        std::cout << "Last few primes: ";
        for (size_t i = primes.size() > 5 ? primes.size() - 5 : 0; i < primes.size(); i++) {
            std::cout << primes[i] << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
