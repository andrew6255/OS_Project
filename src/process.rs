use sysinfo::{System, Process, Signal, Pid};
use sysinfo::*;  // Corrected import for SystemExt

use std::sync::{Arc, Mutex};

lazy_static! {
    static ref SYSTEM: Arc<Mutex<System>> = Arc::new(Mutex::new(System::new_all()));
}

pub fn get_process_list() -> Vec<(i32, String, f32)> {
    let system = SYSTEM.lock().unwrap();
    let mut processes: Vec<(i32, String, f32)> = system.processes()
        .iter()
        .map(|(&pid, proc)| (pid.as_u32() as i32, proc.name().to_string(), proc.cpu_usage()))
        .collect();
    
    processes.sort_by(|a, b| b.2.partial_cmp(&a.2).unwrap());
    processes
}

pub fn get_cpu_usage() -> f32 {
    let mut system = System::new_all();
    system.refresh_cpu();
    system.global_cpu_info().cpu_usage()
}

pub fn get_memory_usage() -> f32 {
    let mut system = System::new_all();
    system.refresh_memory();
    let total = system.total_memory() as f32;
    let used = (system.total_memory() - system.available_memory()) as f32;
    (used / total) * 100.0
}

pub fn kill_process(pid: i32) {
    let mut system = System::new_all();
    system.refresh_processes();
    if let Some(process) = system.process(Pid::from(pid as usize)) {
        process.kill();
    }
}

pub fn suspend_process(pid: i32) {
    let mut system = System::new_all();
    system.refresh_processes();
    if let Some(process) = system.process(Pid::from(pid as usize)) {
        process.kill_with(sysinfo::Signal::Stop);
    }
}

pub fn resume_process(pid: i32) {
    let mut system = System::new_all();
    system.refresh_processes();
    if let Some(process) = system.process(Pid::from(pid as usize)) {
        process.kill_with(sysinfo::Signal::Continue);
    }
}