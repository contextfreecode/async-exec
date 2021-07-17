// Modified from:
// https://rust-lang.github.io/async-book/02_execution/01_chapter.html

pub mod exec;

use std::time::Duration;

pub fn report(message: &str) {
    use {chrono::Utc, std::thread};
    println!("{:?} {:?} {}", thread::current().id(), Utc::now(), message);
}

async fn count(n: usize, interval: f64) {
    use {
        async_std::task::sleep,
        // exec::sleep,
    };
    report("before loop");
    for _ in 0..n {
        sleep(Duration::from_secs_f64(interval)).await;
        report(&format!("{} seconds", interval));
    }
}

async fn run() {
    report("begin");
    let frames = vec![count(2, 1.0), count(3, 0.6)];
    println!("frame size: {}", std::mem::size_of_val(&frames[0]));
    futures::future::join_all(frames).await;
    report("end");
}

#[async_std::main]
async fn main() {
    run().await;
}

// fn main() {
//     async_std::task::block_on(run());
// }

// fn main() {
//     report("begin all");
//     let (executor, spawner) = exec::new_executor_and_spawner();
//     spawner.spawn(run());
//     report("spawned");
//     // Drop the spawner so that our executor knows it has all the tasks.
//     drop(spawner);
//     report("dropped");
//     // Run the executor until the task queue is empty.
//     executor.run();
//     report("end all");
// }
