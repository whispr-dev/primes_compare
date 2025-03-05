from numba import njit

@njit
def sieve_numba(n):
    sieve = [True] * (n+1)
    sieve[0] = False
    sieve[1] = False
    limit = int(n**0.5)

    for p in range(2, limit + 1):
        if sieve[p]:
            for multiple in range(p*p, n+1, p):
                sieve[multiple] = False
    
    return [i for i, val in enumerate(sieve) if val]

if __name__ == "__main__":
    primes = sieve_numba(500_000)  # Change 500_000 to whatever limit you want
    print(f"Found {len(primes)} primes up to 500,000. Last few primes: {primes[-10:]}")