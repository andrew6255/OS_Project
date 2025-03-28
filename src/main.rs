#[macro_use]
extern crate lazy_static;

mod cli;
mod process;
mod gui;
mod graphs;

use cli::run_cli;
use gui::run_gui;
use std::env;

fn main() {
    println!("Linux Process Manager (LPM) starting...");
    let args: Vec<String> = env::args().collect();
    if args.len() > 1 && args[1] == "gui" {
        run_gui();
    } else {
        run_cli();
    }
}