def sieve_of_eratosthenes(n):
    # Create an array of True for primality checks
    is_prime = [True] * (n + 1)
    is_prime[0] = False
    is_prime[1] = False

    p = 2
    while p * p <= n:
        if is_prime[p]:
            # Mark multiples of p as not prime
            for multiple in range(p * p, n + 1, p):
                is_prime[multiple] = False
        p += 1

    # Gather all primes into a list
    return [i for i, prime in enumerate(is_prime) if prime]

# Example usage:
max_number = 500_000
primes_up_to_500_000 = sieve_of_eratosthenes(max_number)
print(primes_up_to_500_000)
