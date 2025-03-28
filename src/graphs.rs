use gtk::prelude::*;
use gtk::{DrawingArea, Window, Box, Orientation};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;
use crate::process::{get_cpu_usage, get_memory_usage};

pub fn create_graph_window() -> Window {
    let window = Window::new(gtk::WindowType::Toplevel);
    window.set_title("System Resource Usage");
    window.set_default_size(600, 400);
    
    let vbox = Box::new(Orientation::Vertical, 5);
    let cpu_area = DrawingArea::new();
    let memory_area = DrawingArea::new();

    let cpu_usage = Arc::new(Mutex::new(vec![0.0; 30]));
    let memory_usage = Arc::new(Mutex::new(vec![0.0; 30]));

    let cpu_usage_clone = Arc::clone(&cpu_usage);
    let memory_usage_clone = Arc::clone(&memory_usage);

    thread::spawn(move || {
        loop {
            {
                let mut cpu_data = cpu_usage_clone.lock().unwrap();
                cpu_data.remove(0);
                cpu_data.push(get_cpu_usage());

                let mut memory_data = memory_usage_clone.lock().unwrap();
                memory_data.remove(0);
                memory_data.push(get_memory_usage());
            }
            thread::sleep(Duration::from_secs(2));
        }
    });

    vbox.pack_start(&cpu_area, true, true, 5);
    vbox.pack_start(&memory_area, true, true, 5);
    window.set_child(Some(&vbox));
    window
}