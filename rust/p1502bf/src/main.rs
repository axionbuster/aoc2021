use std::fmt::{Debug, Formatter};
use std::io::{stdin, BufRead, BufReader};

macro_rules! dbgprint {
    ($($e:tt)*) => { if cfg!(debug_assertions) { eprint!($($e)*); }}
}

#[derive(Copy, Clone)]
struct Position {
    r: i32,
    c: i32,
}

static INVALID: Position = Position { r: -1, c: -1 };

impl Position {
    fn valid(&self, g: &Grid) -> bool {
        self.r >= 0 && self.r < g.h && self.c >= 0 && self.c < g.w
    }
    fn left(&self, g: &Grid) -> Option<Self> {
        let m = Self {
            r: self.r,
            c: self.c - 1,
        };
        if m.valid(g) {
            Option::Some(m)
        } else {
            None
        }
    }
    fn right(&self, g: &Grid) -> Option<Self> {
        let m = Self {
            r: self.r,
            c: self.c + 1,
        };
        if m.valid(g) {
            Option::Some(m)
        } else {
            None
        }
    }
    fn up(&self, g: &Grid) -> Option<Self> {
        let m = Self {
            r: self.r - 1,
            c: self.c,
        };
        if m.valid(g) {
            Option::Some(m)
        } else {
            None
        }
    }
    fn down(&self, g: &Grid) -> Option<Self> {
        let m = Self {
            r: self.r + 1,
            c: self.c,
        };
        if m.valid(g) {
            Option::Some(m)
        } else {
            None
        }
    }
}

struct Grid {
    w: i32,
    h: i32,
    x: Vec<u8>,
}

impl Debug for Grid {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "Grid {}\n  w: {},\n  h: {},\n  x: [\n", "{", self.w, self.h).unwrap();
        for row in self.x.chunks(self.w as usize) {
            write!(f, "    {:?}\n", row).unwrap();
        }
        write!(f, "{}\n", "  ]\n}").unwrap();
        Ok(())
    }
}

struct Solution {
    c: Vec<i32>,
    p: Vec<Position>,
}

struct DebugSolution<'a> {
    g: &'a Grid,
    s: &'a Solution,
}

impl<'a> Debug for DebugSolution<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "Costs\n").unwrap();
        for r in 0..self.g.h {
            for c in 0..self.g.w {
                let p = Position { r, c };
                let p_pred = *ipred_at(self.g, self.s, p);
                write!(
                    f,
                    "(0, 0) -> ({}, {}) [{}], cost is {}, pred ({}, {})\n",
                    r,
                    c,
                    grid_at(self.g, p),
                    *icost_at(self.g, self.s, p),
                    p_pred.r,
                    p_pred.c
                )
                .unwrap();
            }
        }
        Ok(())
    }
}

fn grid_at(g: &Grid, p: Position) -> u8 {
    g.x[(p.r as usize) * (g.w as usize) + (p.c as usize)]
}

fn cost_at<'a>(g: &Grid, s: &'a mut Solution, p: Position) -> &'a mut i32 {
    &mut s.c[(p.r as usize) * (g.w as usize) + (p.c as usize)]
}

fn icost_at<'a>(g: &Grid, s: &'a Solution, p: Position) -> &'a i32 {
    &s.c[(p.r as usize) * (g.w as usize) + (p.c as usize)]
}

fn pred_at<'a>(g: &Grid, s: &'a mut Solution, p: Position) -> &'a mut Position {
    &mut s.p[(p.r as usize) * (g.w as usize) + (p.c as usize)]
}

fn ipred_at<'a>(g: &Grid, s: &'a Solution, p: Position) -> &'a Position {
    &s.p[(p.r as usize) * (g.w as usize) + (p.c as usize)]
}

fn bellman_ford(g: &Grid) -> Solution {
    let n_verts = (g.w as usize) * (g.h as usize);

    let mut s = Solution {
        c: vec![i32::MAX; n_verts],
        p: vec![INVALID; n_verts],
    };
    *cost_at(g, &mut s, Position { r: 0, c: 0 }) = 0;

    for _rep in 0..n_verts {
        let mut n_updates = 0;
        for r in 0..g.h {
            for c in 0..g.w {
                let p = Position { r, c };
                let cost_here: i32 = *cost_at(g, &mut s, p);
                for _neigh in [p.left(g), p.right(g), p.up(g), p.down(g)].iter() {
                    if let Some(neigh) = _neigh {
                        let update: i32 = grid_at(g, *neigh) as i32;
                        let cost_there: &mut i32 = cost_at(g, &mut s, *neigh);
                        let new_cost: i32 = cost_here.saturating_add(update);
                        if new_cost < *cost_there {
                            n_updates += 1;
                            *cost_there = new_cost;
                            *pred_at(g, &mut s, *neigh) = p;
                        }
                    }
                }
            }
            // dbgprint!("After round {}\n", _rep);
            // dbgprint!("{:?}", DebugSolution { g, s: &mut s });
        }
        if n_updates == 0 {
            dbgprint!("Skip\n");
            break;
        }
    }

    return s;
}

fn special_truncate(mut x: u8) -> u8 {
    while x / 10 > 0 {
        x -= 9;
    }
    return x;
}

fn expand_row(v: &[u8]) -> Vec<u8> {
    let v = vec![v].repeat(5);
    let v = v
        .iter()
        .enumerate()
        .map(|(i, s)| {
            s.iter()
                .map(|c| special_truncate(c + i as u8))
                .collect::<Vec<u8>>()
        })
        .flatten()
        .collect::<Vec<u8>>();
    return v;
}

fn read_problem() -> Grid {
    let lines = BufReader::new(stdin()).lines();
    let mut grid = Grid {
        w: 0,
        h: 0,
        x: vec![],
    };
    for line in lines.filter_map(|l| l.ok()) {
        for c in line.as_bytes().iter().map(|c| *c - ('0' as u8)) {
            grid.x.push(c);
        }
        if grid.w == 0 {
            grid.w = grid.x.len() as i32;
        }
        grid.h += 1;
    }

    // Part 2 stuff. Tiling
    let row = grid
        .x
        .chunks(grid.w as usize)
        .map(|chunk| expand_row(chunk))
        .flatten()
        .collect::<Vec<u8>>();
    let new_grid_content: Vec<u8> = expand_row(&row);
    let grid = Grid {
        w: grid.w * 5,
        h: grid.h * 5,
        x: new_grid_content,
    };
    // dbgprint!("New Grid:\n{:?}\n", grid);
    return grid;
}

fn main() {
    let mut grid = read_problem();
    let mut solution = bellman_ford(&mut grid);

    println!(
        "Cost {}",
        *cost_at(
            &grid,
            &mut solution,
            Position {
                r: grid.h - 1,
                c: grid.w - 1
            }
        )
    )
}
