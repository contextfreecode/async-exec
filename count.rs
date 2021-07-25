// #![feature(type_name_of_val)]

pub mod exec;

use std::time::{Duration, Instant};

pub fn report(message: &str) {
    use {chrono::Utc, std::thread};
    println!("{:?} {:?} {}", thread::current().id(), Utc::now(), message);
}

async fn count(n: usize, interval: f64) -> f64 {
    let start = Instant::now();
    use async_std::task::sleep;
    // use exec::sleep;
    report("before loop");
    for _ in 0..n {
        sleep(Duration::from_secs_f64(interval)).await;
        report(&format!("{} seconds", interval));
    }
    return start.elapsed().as_secs_f64();
}

async fn run() -> f64 {
    report("begin");
    let futures = vec![count(2, 1.0), count(3, 0.6)];
    println!("frame size: {}", std::mem::size_of_val(&futures[0]));
    let total = futures::future::join_all(futures).await.iter().sum::<f64>();
    report("end");
    total
}

// #[async_std::main]
// async fn main() {
//     run().await;
// }

fn main() {
    // use async_std::task::block_on;
    // use exec::block_on;
    let total = exec::block_on(run());
    println!("total: {}", total);
    println!("--------------");
    let total = async_std::task::block_on(run());
    println!("total: {}", total);
}
