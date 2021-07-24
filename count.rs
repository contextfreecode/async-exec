// #![feature(type_name_of_val)]

pub mod exec;

use std::time::{Duration, Instant};

pub fn report(message: &str) {
    use {chrono::Utc, std::thread};
    println!("{:?} {:?} {}", thread::current().id(), Utc::now(), message);
}

async fn count(n: usize, interval: f64) -> f64 {
    let now = Instant::now();
    use async_std::task::sleep;
    // use exec::sleep;
    report("before loop");
    for _ in 0..n {
        sleep(Duration::from_secs_f64(interval)).await;
        report(&format!("{} seconds", interval));
    }
    return now.elapsed().as_secs_f64();
}

async fn run() -> f64 {
    report("begin");
    let frames = vec![count(2, 1.0), count(3, 0.6)];
    println!("frame size: {}", std::mem::size_of_val(&frames[0]));
    let result = futures::future::join_all(frames).await.iter().sum::<f64>();
    report("end");
    result
}

// #[async_std::main]
// async fn main() {
//     run().await;
// }

fn main() {
    // use async_std::task::block_on;
    // use exec::block_on;
    let value = exec::block_on(run());
    println!("value: {}", value);
    println!("--------------");
    let value = async_std::task::block_on(run());
    println!("value: {}", value);
}
