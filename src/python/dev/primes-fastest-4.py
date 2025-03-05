import math
import numba
import numpy as np

@numba.njit(parallel=True)
def sieve_skip_evens_numba(n):
    # Handle small n first, ensuring we always return the same dtype
    if n < 2:
        return np.zeros(0, dtype=np.int64)
    elif n == 2:
        arr = np.zeros(1, dtype=np.int64)
        arr[0] = 2
        return arr

    # For n >= 3:
    # We'll track only odds >= 3 => index i corresponds to number (2*i + 3)
    size = n // 2  # # of odd numbers up to n
    is_prime = np.ones(size, dtype=np.bool_)  # True = "potential prime"
    
    limit = min((int(math.sqrt(n)) - 1) // 2 + 1, size)
    
    # Outer loop: i => prime p = 2*i + 3
    # Now we use the pre-calculated limit instead of a break
    for i in numba.prange(limit):
        p = 2 * i + 3
        if is_prime[i]:
            # Mark multiples of p, starting at p*p
            start = (p * p - 3) // 2
            if start < size:  # Add bounds check
                for multiple_idx in range(start, size, p):
                    is_prime[multiple_idx] = False

    # Count how many primes, including 2
    count = 1  # "2" is prime
    for i in range(size):
        if is_prime[i]:
            count += 1

    # Allocate a result array
    primes = np.zeros(count, dtype=np.int64)
    primes[0] = 2

    # Fill with odd primes
    idx = 1
    for i in range(size):
        if is_prime[i]:
            primes[idx] = 2 * i + 3
            idx += 1

    return primes

def main():
    n = 500_000  # Fixed the number (removed underscore)
    primes = sieve_skip_evens_numba(n)
    print(f"Found {len(primes)} primes up to {n}. Example of last few primes: {primes[-10:]}")

if __name__ == "__main__":
    main()