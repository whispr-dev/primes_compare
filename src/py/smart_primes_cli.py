import math
import numpy as np
import numba
from numba import njit
import argparse
import time

def basic_sieve(n):
    """Basic sieve for small numbers (n < 1000)"""
    if n < 2:
        return []
    sieve = [True] * (n + 1)
    sieve[0] = sieve[1] = False
    
    for i in range(2, int(math.sqrt(n)) + 1):
        if sieve[i]:
            for j in range(i * i, n + 1, i):
                sieve[j] = False
                
    return [i for i, prime in enumerate(sieve) if prime]

def numpy_sieve(n):
    """NumPy sieve for medium numbers (1000 <= n < 100000)"""
    sieve = np.ones(n + 1, dtype=bool)
    sieve[0] = sieve[1] = False
    
    for p in range(2, int(np.sqrt(n)) + 1):
        if sieve[p]:
            sieve[p*p::p] = False
            
    return np.flatnonzero(sieve)

@njit
def numba_sieve(n):
    """Numba sieve for large numbers (n >= 100000)"""
    # Pre-allocate boolean array
    sieve = np.ones(n + 1, dtype=np.bool_)
    sieve[0] = False
    sieve[1] = False
    
    # Standard sieve implementation
    for i in range(2, int(math.sqrt(n)) + 1):
        if sieve[i]:
            sieve[i*i::i] = False
    
    # Convert to array of prime numbers
    return np.nonzero(sieve)[0]

def smart_sieve(n):
    """Choose the most efficient implementation based on input size"""
    start = time.perf_counter()  # More precise timing
    
    if n < 1000:
        result = basic_sieve(n)
    elif n < 100_000:
        result = numpy_sieve(n)
    else:
        result = numba_sieve(n)
    
    end = time.perf_counter()  # More precise timing
    return result, (end - start) * 1000  # Return time in milliseconds

def main():
    parser = argparse.ArgumentParser(description='Find prime numbers up to n using the most efficient method')
    parser.add_argument('n', type=int, help='Find primes up to this number')
    args = parser.parse_args()
    
    primes, elapsed = smart_sieve(args.n)
    
    print(f"Found {len(primes)} primes up to {args.n}")
    print(f"Last few primes: {primes[-10:] if len(primes) >= 10 else primes}")
    print(f"Time taken: {elapsed:.2f}ms")

if __name__ == "__main__":
    main()