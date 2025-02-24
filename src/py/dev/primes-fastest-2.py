import math
import numba
import numpy as np

@numba.njit(parallel=True)
def sieve_skip_evens_numba(n):
    if n < 2:
        return np.array([], dtype=np.int64)
    if n == 2:
        return np.array([2], dtype=np.int64)
    
    # We handle "2" as a special prime:
    size = (n // 2)  # number of odds up to n
    is_prime = np.ones(size, dtype=np.bool_)
    
    # index i corresponds to the odd number (2*i + 3)
    # because i=0 => 3, i=1 => 5, etc.
    
    limit = int(math.sqrt(n))  # replaced math.isqrt(n) with math.sqrt(n)

    # For p = 3 to sqrt(n), step by 2
    # p index => i = (p - 3) // 2
    for i in numba.prange((limit - 1) // 2 + 1):  
        p = 2*i + 3
        if p*p > n:
            break
        if is_prime[i]:
            # Mark multiples of p starting at p*p
            start = (p*p - 3) // 2
            for multiple_idx in range(start, size, p):
                is_prime[multiple_idx] = False

    # Gather results
    primes_list = [2]  # 2 is prime
    for i in range(size):
        if is_prime[i]:
            primes_list.append(2*i + 3)
    return np.array(primes_list, dtype=np.int64)

def main():
    n = 100_0000
    primes = sieve_skip_evens_numba(n)
    print(f"Found {len(primes)} primes up to {n}. Example last primes: {primes[-10:]}")

if __name__ == "__main__":
    main()
