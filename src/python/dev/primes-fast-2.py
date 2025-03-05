import numpy as np

def sieve_numpy(n):
    sieve = np.ones(n+1, dtype=bool)
    sieve[0] = False
    sieve[1] = False
    limit = int(n**0.5)
    
    for p in range(2, limit + 1):
        if sieve[p]:
            # Vectorized assignment for multiples
            sieve[p*p : n+1 : p] = False
    return np.flatnonzero(sieve)  # or [i for i in range(n+1) if sieve[i]]

# Add this at the end of the file
if __name__ == "__main__":
    primes = sieve_numpy(500_000)  # Change 500_000 to whatever limit you want
    print(f"Found {len(primes)} primes up to 500,000. Last few primes: {primes[-10:]}")