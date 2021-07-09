// From: https://rust-lang.github.io/async-book/02_execution/01_chapter.html

mod exec;

use {
    exec::{new_executor_and_spawner, report, TimerFuture},
    std::time::Duration,
};

fn main() {
    let (executor, spawner) = new_executor_and_spawner();

    // Spawn a task to print before and after waiting on a timer.
    report("begin");
    // TODO Make separate counter function.
    spawner.spawn(async {
        for _ in 0..2 {
            TimerFuture::new(Duration::from_secs_f64(1.0)).await;
            report("1 second");
        }
    });
    spawner.spawn(async {
        for _ in 0..3 {
            TimerFuture::new(Duration::from_secs_f64(0.6)).await;
            report("0.6 seconds");
        }
    });

    // Drop the spawner so that our executor knows it is finished and won't
    // receive more incoming tasks to run.
    drop(spawner);

    // Run the executor until the task queue is empty.
    // This will print "howdy!", pause, and then print "done!".
    executor.run();
}
