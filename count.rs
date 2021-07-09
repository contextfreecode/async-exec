// From: https://rust-lang.github.io/async-book/02_execution/01_chapter.html

mod exec;

use {
    exec::{new_executor_and_spawner, report, TimerFuture},
    std::time::Duration,
};

async fn count(n: i32, interval: f64) {
    for _ in 0..n {
        TimerFuture::new(Duration::from_secs_f64(interval)).await;
        report(&format!("{} seconds", interval));
    }
}

fn main() {
    let (executor, spawner) = new_executor_and_spawner();

    // Get starting time then run counters.
    report("begin");
    spawner.spawn(count(2, 1.0));
    spawner.spawn(count(3, 0.6));

    // Drop the spawner so that our executor knows it is finished and won't
    // receive more incoming tasks to run.
    drop(spawner);

    // Run the executor until the task queue is empty.
    // This will print "howdy!", pause, and then print "done!".
    executor.run();
}
