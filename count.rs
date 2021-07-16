// Modified from:
// https://rust-lang.github.io/async-book/02_execution/01_chapter.html

mod exec;

use {
    exec::{new_executor_and_spawner, report, TimerFuture},
    std::time::Duration,
};

async fn count(n: usize, interval: f64) {
    report("before loop");
    for _ in 0..n {
        TimerFuture::new(Duration::from_secs_f64(interval)).await;
        report(&format!("{} seconds", interval));
    }
}

async fn run() {
    // Get starting time then run counters.
    report("begin");
    let frames = vec![
        count(2, 1.0),
        count(3, 0.6),
    ];
    println!("frame size: {}", std::mem::size_of_val(&frames[0]));
    futures::future::join_all(frames).await;
    report("end");
}

fn main() {
    report("begin all");
    let (executor, spawner) = new_executor_and_spawner();

    spawner.spawn(run());
    report("spawned");

    // Drop the spawner so that our executor knows it is finished and won't
    // receive more incoming tasks to run.
    drop(spawner);
    report("dropped");

    // Run the executor until the task queue is empty.
    // This will print "howdy!", pause, and then print "done!".
    executor.run();
    report("end all");
}
