//! Program to solve day 14 (parts 1 and 2)
//!
//! Copyright YuJin Kim (C) 2022. All rights reserved.
//!
//! The core idea in this program is that two ascii characters make up a single 16-bit word.
//! There are 65536 such words. Therefore it is completely computationally feasible to
//! iterate each and every single such word representing a potentially valid two-character string.
//!
//! It's very fast (operating under 10 milliseconds), constant space and linear time w.r.t.
//! number of iterations.

use std::cmp::{max, min};
use std::env;
use std::io::{stdin, BufRead, BufReader};
use std::process::exit;
use std::str::FromStr;

#[allow(unused_macros)]
macro_rules! dbgprint {
    ($($arg:tt)*) => ( if cfg!(debug_assertions) { eprint!($($arg)*) })
}

fn atoi(s: &str) -> i32 {
    match i32::from_str(s) {
        Ok(n) => n,
        Err(_) => 0,
    }
}

fn unionize(cc: (u8, u8)) -> usize {
    ((cc.0 as usize) << 8) + cc.1 as usize
}

fn separate(uu: usize) -> (u8, u8) {
    ((uu >> 8) as u8, (uu & 0xff) as u8)
}

struct Problem {
    pattern_counts: [isize; u16::MAX as usize],
    rewrite_rules: [u8; u16::MAX as usize],
    char_counts: [isize; ('Z' as usize) + 1],
}

fn main() {
    let mut args = env::args();
    let max_iterations;
    if args.len() == 2 {
        max_iterations = atoi(&args.nth(1).unwrap());
    } else {
        max_iterations = 1;
    }
    if max_iterations <= 0 || args.len() > 2 {
        eprintln!("Usage: (program) [steps] < in > out 2> trace");
        exit(1);
    }

    let mut problem = Problem {
        pattern_counts: [0; u16::MAX as usize],
        rewrite_rules: [0; u16::MAX as usize],
        char_counts: [0; ('Z' as usize) + 1],
    };

    let line_reader = BufReader::new(stdin());
    let mut lines = line_reader.lines();
    let line: String = lines.next().unwrap().unwrap();
    let line = line.as_bytes();
    for c in line {
        problem.char_counts[*c as usize] += 1;
    }
    for c1c2 in line.windows(2) {
        let c1 = c1c2[0];
        let c2 = c1c2[1];
        problem.pattern_counts[unionize((c1, c2))] += 1;
    }

    // Skip one line.
    lines.next();

    for _rewrite in lines.map(|s| s.unwrap()) {
        let (c1c2, cr) = _rewrite.split_once(" -> ").unwrap();
        let c1c2 = c1c2.as_bytes();
        let c1c2 = (c1c2[0], c1c2[1]);
        let cr = cr.as_bytes();
        problem.rewrite_rules[unionize(c1c2)] = cr[0];
    }

    // END interpretation of initial string and rewrite rules.
    // START evolution of the string rewriting system

    for iteration in 1..=max_iterations {
        let mut updates = [0 as isize; u16::MAX as usize];

        for pattern in 0..u16::MAX as usize {
            if problem.rewrite_rules[pattern] > 0 && problem.pattern_counts[pattern] > 0 {
                // Once detect a pattern that matches a known rule, do two things:
                //  1. Insert the two other patterns. E.g. Matching XY against XY -> Z creates XZ
                //     and ZY patterns.
                //  2. Increment the character count for Z.
                let n = problem.pattern_counts[pattern];
                let (c1, c2) = separate(pattern as usize);
                let cr = problem.rewrite_rules[pattern];
                let new_pattern1 = unionize((c1, cr));
                let new_pattern2 = unionize((cr, c2));
                problem.char_counts[cr as usize] += n;
                updates[pattern] -= n;
                updates[new_pattern1] += n;
                updates[new_pattern2] += n;
            }
        }

        for pattern in 0..u16::MAX as usize {
            if updates[pattern] != 0 {
                problem.pattern_counts[pattern] += updates[pattern];
            }
            debug_assert!(problem.pattern_counts[pattern] >= 0);
        }

        let mut c_most = isize::MIN;
        let mut c_least = isize::MAX;
        for c in ('A' as usize)..('Z' as usize) {
            let n = problem.char_counts[c];
            if n > 0 {
                c_least = min(c_least, n);
                c_most = max(c_most, n);
            }
        }

        println!(
            "After iteration {},\n\tMost {}\n\tLeast {}\n\tDelta {}",
            iteration,
            c_most,
            c_least,
            c_most - c_least
        );
    }
}
