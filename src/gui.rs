use gtk::prelude::*;
use gtk::{Application, ApplicationWindow, Box, Orientation, Label};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;
use crate::process::{get_cpu_usage, get_memory_usage}; // Import the necessary functions

use glib::idle_add; // Used for UI updates safely in the main thread
use glib::ControlFlow::Continue;

pub fn run_gui() {
    // Initialize the GTK application
    let app = Application::new(Some("com.lpm.gui"), Default::default());
    app.connect_activate(|app| {
        // Create the main application window
        let window = ApplicationWindow::new(app);
        window.set_title("Linux Process Manager");
        window.set_default_size(600, 400);

        // Create the layout box and labels for CPU and Memory usage
        let vbox = Box::new(Orientation::Vertical, 5);
        let cpu_label = Label::new(None);
        let memory_label = Label::new(None);

        vbox.pack_start(&cpu_label, true, true, 5);
        vbox.pack_start(&memory_label, true, true, 5);

        // Add the layout box to the window
        window.add(&vbox);

        // Start a new thread to update the CPU and Memory usage every 2 seconds
        thread::spawn(move || {
            loop {
                let cpu_usage = get_cpu_usage(); // Get CPU usage
                let memory_usage = get_memory_usage(); // Get memory usage

                // Use `idle_add` to safely update the UI in the main thread
                // Pass only the necessary data (not the labels themselves) to the closure
                idle_add(move || {
                    // Update the labels with new data in the main thread
                    cpu_label.set_text(&format!("CPU Usage: {:.2}%", cpu_usage));
                    memory_label.set_text(&format!("Memory Usage: {:.2}%", memory_usage));
                    Continue // Continue running this idle function
                });

                // Sleep for 2 seconds before updating again
                thread::sleep(Duration::from_secs(2));
            }
        });

        // Show all widgets in the window
        window.show_all();
    });

    // Start the GTK event loop
    app.run();
}
