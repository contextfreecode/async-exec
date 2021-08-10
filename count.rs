pub mod exec;

use std::time::{Duration, Instant};

#[macro_export]
macro_rules! report {
    ($($arg:tt)*) => {
        print!("{:?} ", std::thread::current().id());
        println!($($arg)*);
    }
}

async fn count(n: usize, interval: f64) -> f64 {
    let start = Instant::now();
    use async_std::task::sleep;
    // use exec::sleep;
    report!("before loop {}", interval);
    for _ in 0..n {
        sleep(Duration::from_secs_f64(interval)).await;
        report!("slept {}", interval);
    }
    return start.elapsed().as_secs_f64();
}

async fn run() -> f64 {
    report!("begin");
    let futures = vec![count(2, 1.0), count(3, 0.6)];
    report!("count size: {}", std::mem::size_of_val(&futures[0]));
    let total = futures::future::join_all(futures).await.iter().sum::<f64>();
    report!("end");
    total
}

#[async_std::main]
async fn main() {
    let total = run().await;
    report!("total: {}", total);
}

// fn main() {
//     use async_std::task::block_on;
//     // use exec::block_on;
//     let future = run();
//     report!("run size: {}", std::mem::size_of_val(&future));
//     let total = block_on(future);
//     report!("total: {}", total);
//     // println!("--------------");
//     // let total = async_std::task::block_on(run());
//     // report!("total: {}", total);
// }
