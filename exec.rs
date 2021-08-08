// Mostly from:
// https://rust-lang.github.io/async-book/02_execution/01_chapter.html

use {
    crate::report,
    futures::{
        future::{BoxFuture, Future, FutureExt},
        task::{waker_ref, ArcWake},
    },
    std::{
        pin::Pin,
        sync::mpsc::{sync_channel, Receiver, SyncSender},
        sync::{Arc, Mutex},
        task::{Context, Poll, Waker},
        thread,
        time::Duration,
    },
};

pub fn block_on<F, Output>(future: F) -> Output
where
    F: Future<Output = Output> + 'static + Send,
{
    let (executor, spawner) = new_executor_and_spawner();
    spawner.spawn(future);
    // Drop the spawner so that our executor knows it has all the tasks.
    drop(spawner);
    // Run the executor until the task queue is empty.
    let value = executor.run();
    value.unwrap()
}

pub fn sleep(duration: Duration) -> SleepFuture {
    SleepFuture::new(duration)
}

pub struct SleepFuture {
    shared_state: Arc<Mutex<SharedState>>,
}

impl SleepFuture {
    pub fn new(duration: Duration) -> Self {
        let shared_state = Arc::new(Mutex::new(SharedState {
            completed: false,
            waker: None,
        }));
        let thread_shared_state = shared_state.clone();
        thread::spawn(move || {
            thread::sleep(duration);
            let mut shared_state = thread_shared_state.lock().unwrap();
            shared_state.completed = true;
            if let Some(waker) = shared_state.waker.take() {
                report!("sleep over y'all");
                waker.wake()
            }
        });
        SleepFuture { shared_state }
    }
}

struct SharedState {
    completed: bool,
    waker: Option<Waker>,
}

impl Future for SleepFuture {
    type Output = ();
    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut shared_state = self.shared_state.lock().unwrap();
        if shared_state.completed {
            // report!("poll ready {:p}", &shared_state.completed);
            Poll::Ready(())
        } else {
            // report!("poll pending {:p}", &shared_state.completed);
            shared_state.waker = Some(cx.waker().clone());
            Poll::Pending
        }
    }
}

pub struct Executor<Output> {
    ready_queue: Receiver<Arc<Task<Output>>>,
}

#[derive(Clone)]
pub struct Spawner<Output> {
    task_sender: SyncSender<Arc<Task<Output>>>,
}

struct Task<Output> {
    future: Mutex<Option<BoxFuture<'static, Output>>>,
    task_sender: SyncSender<Arc<Task<Output>>>,
}

pub fn new_executor_and_spawner<Output>() -> (Executor<Output>, Spawner<Output>) {
    // Maximum number of tasks to allow queueing in the channel at once.
    // This is just to make `sync_channel` happy, and wouldn't be present in
    // a real executor.
    const MAX_QUEUED_TASKS: usize = 10_000;
    let (task_sender, ready_queue) = sync_channel(MAX_QUEUED_TASKS);
    (Executor { ready_queue }, Spawner { task_sender })
}

impl<Output> Spawner<Output> {
    pub fn spawn(&self, future: impl Future<Output = Output> + 'static + Send) {
        let future = future.boxed();
        let task = Arc::new(Task {
            future: Mutex::new(Some(future)),
            task_sender: self.task_sender.clone(),
        });
        self.task_sender.send(task).expect("too many tasks queued");
    }
}

impl<Output> ArcWake for Task<Output> {
    fn wake_by_ref(arc_self: &Arc<Self>) {
        let cloned = arc_self.clone();
        arc_self
            .task_sender
            .send(cloned)
            .expect("too many tasks queued");
    }
}

impl<Output> Executor<Output> {
    pub fn run(&self) -> Option<Output> {
        report!("looping events y'all");
        while let Ok(task) = self.ready_queue.recv() {
            let mut future_slot = task.future.lock().unwrap();
            if let Some(mut future) = future_slot.take() {
                let waker = waker_ref(&task);
                let context = &mut Context::from_waker(&*waker);
                // `BoxFuture<T>` is a type alias for
                // `Pin<Box<dyn Future<Output = T> + Send + 'static>>`.
                // We can get a `Pin<&mut dyn Future + Send + 'static>`
                // from it by calling the `Pin::as_mut` method.
                match future.as_mut().poll(context) {
                    Poll::Pending => *future_slot = Some(future),
                    Poll::Ready(value) => return Some(value),
                }
            }
        }
        None
    }
}
