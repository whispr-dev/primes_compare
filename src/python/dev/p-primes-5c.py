import math
import numpy as np
import numba
from numba import njit, prange

# Basic implementation for small numbers
def sieve_basic(n):
    if n < 2:
        return []
    # Initialize boolean array "isPrime[0..n]" 
    sieve = [True] * (n + 1)
    sieve[0] = sieve[1] = False
    
    # Use simple sieving for small numbers
    for i in range(2, int(math.sqrt(n)) + 1):
        if sieve[i]:
            for j in range(i * i, n + 1, i):
                sieve[j] = False
                
    return [i for i, prime in enumerate(sieve) if prime]

# NumPy implementation for medium-sized numbers
def sieve_numpy(n):
    sieve = np.ones(n + 1, dtype=bool)
    sieve[0] = sieve[1] = False
    limit = int(np.sqrt(n))
    
    for p in range(2, limit + 1):
        if sieve[p]:
            sieve[p*p::p] = False
            
    return np.flatnonzero(sieve)

# Numba parallel implementation for large numbers
@njit(parallel=True)
def sieve_numba_parallel(n):
    size = n // 2  # Track only odds
    is_prime = np.ones(size, dtype=np.bool_)
    limit = min((int(math.sqrt(n)) - 1) // 2 + 1, size)
    
    for i in prange(limit):
        p = 2 * i + 3
        if is_prime[i]:
            start = (p * p - 3) // 2
            if start < size:
                for multiple_idx in range(start, size, p):
                    is_prime[multiple_idx] = False

    # Count primes and create result array
    count = 1  # Include 2
    for i in range(size):
        if is_prime[i]:
            count += 1

    # Allocate and fill result array
    primes = np.zeros(count, dtype=np.int64)
    primes[0] = 2
    idx = 1
    for i in range(size):
        if is_prime[i]:
            primes[idx] = 2 * i + 3
            idx += 1

    return primes

def smart_sieve(n):
    """
    Choose the most efficient implementation based on input size.
    
    Args:
        n (int): Upper limit for finding primes
        
    Returns:
        list: List of prime numbers up to n
    """
    if n < 1000:
        return sieve_basic(n)
    elif n < 100_000:
        return sieve_numpy(n)
    else:
        return sieve_numba_parallel(n)

def main():
    n = 50_000  # Adjust this number to test different ranges
    primes = smart_sieve(n)
    print(f"Found {len(primes)} primes up to {n}.")
    print(f"Last few primes: {primes[-10:] if len(primes) >= 10 else primes}")

if __name__ == "__main__":
    main()