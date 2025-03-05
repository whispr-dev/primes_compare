fn sieve_of_eratosthenes(n: usize) -> Vec<usize> {
    // Create a boolean vector for marking primality
    let mut is_prime = vec![true; n + 1];
    is_prime[0] = false; // 0 is not prime
    if n >= 1 {
        is_prime[1] = false; // 1 is not prime
    }

    let limit = (n as f64).sqrt() as usize;
    for p in 2..=limit {
        if is_prime[p] {
            let mut multiple = p * p;
            while multiple <= n {
                is_prime[multiple] = false;
                multiple += p;
            }
        }
    }

    // Collect and return all primes up to n
    (2..=n).filter(|&i| is_prime[i]).collect()
}

fn main() {
    let n = 500_000;
    let primes = sieve_of_eratosthenes(n);

    print!("Primes up to {}: ", n);
    for prime in primes {
        print!("{} ", prime);
    }
    println!();
}
