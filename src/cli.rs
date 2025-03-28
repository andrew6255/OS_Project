use tui::{backend::CrosstermBackend, Terminal};
use crossterm::{event::{self, KeyCode}, execute, terminal::{enable_raw_mode, disable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen}};
use std::{io, time::Duration};
use crate::process::{get_process_list, kill_process, suspend_process, resume_process}; // Import functions

pub fn run_cli() {
    enable_raw_mode().unwrap();
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen).unwrap();
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend).unwrap();

    loop {
        let processes = get_process_list(); // Get the process list for display
        terminal.draw(|f| {
            let size = f.size();
            let block = tui::widgets::Block::default().title("Processes").borders(tui::widgets::Borders::ALL);
            f.render_widget(block, size);
        }).unwrap();

        if event::poll(Duration::from_millis(500)).unwrap() {
            if let event::Event::Key(key) = event::read().unwrap() {
                match key.code {
                    KeyCode::Char('q') => break, // Quit the app
                    KeyCode::Char('k') => {
                        println!("Enter the PID of the process to kill:");
                        let pid = get_input_pid(); // You'd implement this function to read the PID from the user
                        kill_process(pid); // Call the kill process function
                    }
                    KeyCode::Char('s') => {
                        println!("Enter the PID of the process to suspend:");
                        let pid = get_input_pid();
                        suspend_process(pid); // Suspend process by PID
                    }
                    KeyCode::Char('r') => {
                        println!("Enter the PID of the process to resume:");
                        let pid = get_input_pid();
                        resume_process(pid); // Resume the process by PID
                    }
                    _ => {}
                }
            }
        }
    }

    disable_raw_mode().unwrap();
    execute!(io::stdout(), LeaveAlternateScreen).unwrap();
}

// A helper function to read PID from user input
fn get_input_pid() -> i32 {
    // Logic to get PID from user input (e.g., using `std::io::stdin()` to capture input)
    // For now, just returning a mock PID (replace this with actual input logic)
    1234
}
