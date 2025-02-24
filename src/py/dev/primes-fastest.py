import math
import numba
import numpy as np

@numba.njit(parallel=True)
def sieve_skip_evens_numba(n):
    """
    Returns a NumPy array of primes up to n (inclusive).
    Uses skipping of even numbers and Numba parallel loops.
    """
    if n < 2:
        return np.array([], dtype=np.int64)
    if n == 2:
        return np.array([2], dtype=np.int64)
    
    # We handle "2" as a special prime:
    # Then we only track odd numbers [3, 5, 7, ..., n or n-1 if n is even]
    size = (n // 2)  # number of odds up to n (not counting the even ones)
    is_prime = np.ones(size, dtype=np.bool_)  
    # index i corresponds to the odd number (2*i + 3)

    limit = int(math.isqrt(n))  # integer sqrt

    # For p = 3 to sqrt(n), step by 2
    for i in numba.prange((limit - 1) // 2 + 1):  # index in [0, limit//2]
        p = 2*i + 3  # the actual odd number
        if p*p > n:
            break
        if is_prime[i]:
            # Mark multiples of p starting at p*p
            start = (p*p - 3) // 2  # index of p*p in the odd array
            for multiple_idx in range(start, size, p):
                is_prime[multiple_idx] = False

    # Gather results
    primes_list = [2]  # 2 is prime
    for i in range(size):
        if is_prime[i]:
            primes_list.append(2*i + 3)
    return np.array(primes_list, dtype=np.int64)

def main():
    n = 100_0000  # 1 million, for example
    primes = sieve_skip_evens_numba(n)
    print(f"Found {len(primes)} primes up to {n}. Example last primes: {primes[-10:]}")
    
if __name__ == "__main__":
    main()
