use std::vec::Vec;
use std::string::String;
use std::collections::BTreeMap;
use std::io::{prelude::*, BufReader, stdin};
use std::fmt::{self, Formatter};

use text_io::scan;

macro_rules! dbgprintln(
    ($($arg:tt)*) => (if cfg!(debug_assertions) { eprintln!($($arg)*) })
);
macro_rules! dbgprint(
    ($($arg:tt)*) => (if cfg!(debug_assertions) { eprint!($($arg)*) })
);

#[derive(Clone, Debug)]
struct RouteMemory {
    visit_twice: bool,
    visits: Vec<String>,
    from: String,
}

impl RouteMemory {
    pub fn new() -> Self {
        RouteMemory {
            visit_twice: false,
            visits: vec!(),
            from: "start".to_string(),
        }
    }
}

#[derive(Clone)]
struct Graph {
    _impl: BTreeMap<String, Vec<String>>,
}

impl fmt::Debug for Graph {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "Graph ({} elements)\n", self._impl.len()).unwrap();

        for (s, vs) in self._impl.iter() {
            write!(f, "{} -> ", s).unwrap();

            let mut i = 0;
            while i < vs.len() {
                if i != vs.len() - 1 {
                    write!(f, "{},", vs[i]).unwrap();
                } else {
                    write!(f, "{}\n", vs[i]).unwrap();
                }
                i += 1;
            }
        }

        Ok(())
    }
}

fn debug_trace(trace: &Vec<String>) {
    let mut i = 0;
    for s in trace.iter() {
        if i != trace.len() - 1 {
            dbgprint!("{},", s);
        } else {
            dbgprint!("{}\n", s);
        }
        i += 1;
    }
}

impl Graph {
    fn new() -> Self {
        Graph {
            _impl: BTreeMap::new()
        }
    }

    // Assuming a String representation of an edge, insert the undirected edge.
    fn insert(&mut self, line: &str) -> &mut Self {
        let e1: String;
        let e2: String;
        // NOTE: While the C/C++ equivalent using sscanf(...) suffered from a buffer overflow
        // problem, this one has a problem with accepting potential whitespace in between
        // though it's fine because no whitespace may occur around the hyphen (-) as per
        // the spec.
        scan!(line.bytes() => "{}-{}", e1, e2);
        dbgprintln!("{} - {} (read)", e1, e2);
        self._impl.entry(e1.clone()).or_default().push(e2.clone());
        self._impl.entry(e2.clone()).or_default().push(e1.clone());
        return self;
    }

    fn end_paths(&mut self, mut rm: RouteMemory, trace: &mut Vec<String>) -> i32 {
        trace.push(rm.from.clone());

        dbgprint!("ENTER: ");
        debug_trace(trace);
        dbgprint!("VISITED: ");
        debug_trace(&rm.visits);

        if rm.from == "end" {
            dbgprintln!("\"end\" reached, local return 1.");
            trace.pop();
            return 1;
        }

        if rm.visits.contains(&rm.from) {
            if !rm.visit_twice && rm.from != "start" {
                dbgprintln!("\"{}\" was already visited, but it can be visited again.", rm.from);
            } else {
                dbgprintln!("Something was visited twice or the keyword (\"{}\") was \"start\", local return 0.", rm.from);
                trace.pop();
                return 0;
            }
        }

        if !rm.from.is_empty() && rm.from.as_bytes()[0].is_ascii_lowercase() {
            if rm.from == "start" {
                dbgprintln!("\"start\" --> never visit again.");
            } else {
                dbgprintln!("\"{}\" lowercase, mark as never visit more than twice.", rm.from);
            }
            rm.visits.push(rm.from.clone());

            // Does this cause two or more elements to exist?
            let mut it = rm.visits.iter().filter(|&s| s == &rm.from);
            let test1 = it.next();
            let test2 = it.next();
            if let Some(_) = test1 {
                if let Some(_) = test2 {
                    dbgprintln!("\"{}\" will cause this cave to have been visited twice.", rm.from);
                    rm.visit_twice = true;
                }
            }
        }

        let mut sum = 0;
        let it = self._impl.entry(rm.from.clone()).or_default().clone();
        for neigh in it {
            let mut rm2 = rm.clone();
            rm2.from = neigh;
            sum += self.end_paths(rm2, trace);
        }

        trace.pop();
        return sum;
    }
}

fn main() {
    let mut g = Graph::new();
    let s = stdin();
    let r = BufReader::new(s);
    for line in r.lines() {
        if let Ok(l) = line {
            g.insert(&l);
        }
    }
    dbgprintln!("\n{:?}\n", g.clone());

    let mut trace = vec![];
    let rm = RouteMemory::new();
    let start = rm.from.clone();
    let n = g.end_paths(rm, &mut trace);
    println!("There are {} many ways to go from {} to end.", n, start);
}
