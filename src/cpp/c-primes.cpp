#include <iostream>
#include <vector>
using namespace std;

vector<int> sieve_of_eratosthenes(int n) {
    // Create a boolean vector for marking primality
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = false; // 0 is not prime
    is_prime[1] = false; // 1 is not prime

    for (int p = 2; p * p <= n; p++) {
        if (is_prime[p]) {
            // Mark multiples of p as not prime
            for (int multiple = p * p; multiple <= n; multiple += p) {
                is_prime[multiple] = false;
            }
        }
    }

    // Gather the primes into a separate vector
    vector<int> primes;
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) {
            primes.push_back(i);
        }
    }
    return primes;
}

int main() {
    int n = 500000;
    vector<int> primes = sieve_of_eratosthenes(n);

    cout << "Primes up to " << n << ": ";
    for (int prime : primes) {
        cout << prime << " ";
    }
    cout << endl;

    return 0;
}
