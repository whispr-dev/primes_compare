use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::{self, Read};
use std::path::Path;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};
use itertools::Itertools;
use primal::Primes;
use rayon::prelude::*;
use regex::Regex;
use sha2::{Digest, Sha256};
use zip::read::ZipArchive;
use zip::result::ZipError;

// Copied/adapted from quantum_local_search mods (tokenizer.rs, prime_hilbert.rs)
// PrimeTokenizer
#[derive(Debug)]
pub struct PrimeTokenizer {
    token_to_prime: HashMap<String, u64>,
    prime_to_token: HashMap<u64, String>,
    current_prime: u64,
    word_regex: Regex,
    primal_generator: Primes,
}

impl PrimeTokenizer {
    pub fn new() -> Self {
        let word_regex = Regex::new(r"\b\w+\b").expect("Failed to create word regex");
        PrimeTokenizer {
            token_to_prime: HashMap::new(),
            prime_to_token: HashMap::new(),
            current_prime: 2,
            word_regex,
            primal_generator: Primes::all(),
        }
    }

    pub fn tokenize(&mut self, text: &str) -> Vec<u64> {
        let lower_text = text.to_lowercase();
        let mut primes_list = Vec::new();

        for mat in self.word_regex.find_iter(&lower_text) {
            let token = mat.as_str().to_string();
            if !self.token_to_prime.contains_key(&token) {
                let next_p_usize = self.primal_generator
                    .by_ref()
                    .find(|&p| p as u64 > self.current_prime)
                    .expect("Next prime");
                let next_p = next_p_usize as u64;
                self.token_to_prime.insert(token.clone(), next_p);
                self.prime_to_token.insert(next_p, token.clone());
                self.current_prime = next_p;
            }
            primes_list.push(*self.token_to_prime.get(&token).unwrap());
        }
        primes_list
    }
}

// PrimeVector type
pub type PrimeVector = HashMap<u64, f64>;

// build_vector
pub fn build_vector(primes: &[u64]) -> PrimeVector {
    if primes.is_empty() {
        return HashMap::new();
    }
    let mut counts = HashMap::new();
    for &prime in primes {
        *counts.entry(prime).or_insert(0) += 1;
    }
    let norm: f64 = (counts.values().map(|&c| (c * c) as f64).sum::<f64>()).sqrt();
    let mut vector = HashMap::new();
    if norm > 0.0 {
        for (&prime, &count) in &counts {
            vector.insert(prime, count as f64 / norm);
        }
    }
    vector
}

// dot_product
pub fn dot_product(vec1: &PrimeVector, vec2: &PrimeVector) -> f64 {
    let keys1: HashSet<_> = vec1.keys().cloned().collect();
    let keys2: HashSet<_> = vec2.keys().cloned().collect();
    let all_keys = keys1.union(&keys2).cloned().collect::<Vec<_>>();
    all_keys.iter().fold(0.0, |acc, key| {
        let val1 = vec1.get(key).unwrap_or(&0.0);
        let val2 = vec2.get(key).unwrap_or(&0.0);
        acc + val1 * val2
    })
}

// Candidate type: (score, candidate bytes) for pass or hash
type Candidate = (f64, Vec<u8>);

// Generate resonant candidates
fn generate_resonant_candidates(query: &str) -> Vec<Candidate> {
    let mut tokenizer = PrimeTokenizer::new();
    let query_primes = tokenizer.tokenize(query);
    let unique_primes: Vec<u64> = query_primes.into_iter().collect::<HashSet<_>>().into_iter().collect();

    let query_vec = build_vector(&unique_primes);

    let mut candidates = Vec::new();

    for k in (1..=unique_primes.len()).rev() {  // Prioritize larger subsets
        for perm in unique_primes.iter().permutations(k) {
            let concat_pass: String = perm.iter().map(|&&p| p.to_string()).collect();
            let pass_bytes = concat_pass.as_bytes().to_vec();
            let mut hasher = Sha256::new();
            hasher.update(&concat_pass);
            let hash_bytes = hasher.finalize().to_vec();

            let perm_primes: Vec<u64> = perm.iter().cloned().cloned().collect();
            let candidate_vec = build_vector(&perm_primes);
            let score = dot_product(&query_vec, &candidate_vec);

            candidates.push((score, pass_bytes));
            candidates.push((score, hash_bytes));
        }
    }

    candidates.sort_by(|a, b| b.0.partial_cmp(&a.0).unwrap_or(std::cmp::Ordering::Equal));
    candidates
}

// Try candidate on ZIP: Success if can decrypt and read a file without error
fn try_candidate(zip_path: &Path, candidate: &[u8]) -> Result<bool, ZipError> {
    let file = File::open(zip_path)?;
    let mut archive = ZipArchive::new(file)?;

    if archive.len() == 0 {
        return Ok(false);
    }

    // Try decrypt first file
    let mut zip_file = archive.by_index_decrypt(0, candidate)?;
    let mut buffer = Vec::new();
    zip_file.read_to_end(&mut buffer)?;

    Ok(true)  // If read succeeds, pass is good
}

// Main cracker
fn main() -> io::Result<()> {
    let zip_path = Path::new("your_precious.zip");
    let query = "memories of a beloved passed on fren";

    let candidates = generate_resonant_candidates(query);
    if candidates.is_empty() {
        println!("No resonant candidates, fren. Tweak query?");
        return Ok(());
    }

    println!("Grinding {} resonant candidates, fren... Hold tight.", candidates.len());

    let start = Instant::now();
    let zip_path_arc = Arc::new(zip_path.to_path_buf());
    let found = Arc::new(Mutex::new(None::<Vec<u8>>));

    candidates.par_iter().for_each(|&(_, ref cand)| {
        let zip_path = zip_path_arc.clone();
        let found_clone = found.clone();

        if let Ok(true) = try_candidate(&zip_path, cand) {
            let mut guard = found_clone.lock().unwrap();
            if guard.is_none() {
                *guard = Some(cand.clone());
            }
        }
    });

    let elapsed = start.elapsed();
    let guard = found.lock().unwrap();
    if let Some(cracked) = &*guard {
        println!("Cracked in {:?}! Use: {:?}", elapsed, cracked);
    } else {
        println!("No luck in {:?}, fren. Resonance off? Tweak and retry.", elapsed);
    }

    Ok(())
}