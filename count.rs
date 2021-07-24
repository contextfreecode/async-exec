// #![feature(type_name_of_val)]

pub mod exec;

use std::time::Duration;

pub fn report(message: &str) {
    use {chrono::Utc, std::thread};
    println!("{:?} {:?} {}", thread::current().id(), Utc::now(), message);
}

async fn count(n: usize, interval: f64) {
    use async_std::task::sleep;
    // use exec::sleep;
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

// #[async_std::main]
// async fn main() {
//     run().await;
// }

fn main() {
    // use async_std::task::block_on;
    // use exec::block_on;
    exec::block_on(run());
    println!("--------------");
    async_std::task::block_on(run());
}
