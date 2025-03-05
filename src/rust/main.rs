use bitvec::prelude::*;
use rayon::prelude::*;
use std::cmp;
use std::sync::Arc;

fn small_primes_up_to(limit: usize) -> Vec<usize> {
    // Standard simple sieve to get primes up to limit
    let mut sieve = bitvec![1; limit+1];
    sieve.set(0, false);
    if limit >= 1 {
        sieve.set(1, false);
    }
    let max_check = (limit as f64).sqrt() as usize;

    for p in 2..=max_check {
        if sieve[p] {
            for multiple in ((p*p)..=limit).step_by(p) {
                sieve.set(multiple, false);
            }
        }
    }
    sieve.iter_ones().collect()
}

fn segmented_sieve_parallel(n: usize, segment_size: usize) -> Vec<usize> {
    // 1) Generate small primes up to sqrt(n)
    let limit = (n as f64).sqrt() as usize;
    let small_primes = Arc::new(small_primes_up_to(limit));

    // 2) We'll process [2..n] in chunks of size segment_size
    //    Each chunk [start..end] is sieved using small_primes
    let mut segments = vec![];
    let mut start = 2;
    while start <= n {
        let end = cmp::min(start + segment_size - 1, n);
        segments.push((start, end));
        start = end + 1;
    }

    // 3) Parallel process each segment with rayon
    let partial_results: Vec<Vec<usize>> = segments
        .into_par_iter()
        .map(|(seg_start, seg_end)| {
            // For segment [seg_start..seg_end], build a local bitvec
            let length = seg_end - seg_start + 1;
            let mut bits = bitvec![1; length]; // all set to 1 initially

            // Mark off multiples of each prime in small_primes
            for &p in small_primes.iter() {
                // If p^2 > seg_end, no need to mark further
                let p_sq = p * p;
                if p_sq > seg_end {
                    break;
                }
                // Find the first multiple of p in [seg_start..seg_end]
                // We want k*p >= seg_start => k >= seg_start/p
                // let's call that "start_mult"
                let mut start_mult = (seg_start + (p - 1)) / p * p; 
                if start_mult < p_sq {
                    start_mult = p_sq;
                }
                // Mark multiples in the local bitvec
                for multiple in (start_mult..=seg_end).step_by(p) {
                    bits.set(multiple - seg_start, false);
                }
            }

            // Collect primes from this segment
            let mut local_primes = vec![];
            for i in bits.iter_ones() {
                local_primes.push(seg_start + i);
            }
            local_primes
        })
        .collect();

    // 4) Flatten all partial results
    let mut all_primes = partial_results.into_iter().flatten().collect::<Vec<_>>();
    // remove special cases below 2 if necessary, or you can handle them upstream
    all_primes.retain(|&x| x >= 2);
    all_primes
}

fn main() {
    let n = 500_000; 
    let segment_size = 50_000; // tweak as you wish
    let primes = segmented_sieve_parallel(n, segment_size);

    println!("Found {} primes up to {}.", primes.len(), n);
    if !primes.is_empty() {
        println!("Last 5 primes: {:?}", &primes[primes.len().saturating_sub(5)..]);
    }
}
