// Original implementation for comparison
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <functional>

using namespace std;
using namespace std::chrono;

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
