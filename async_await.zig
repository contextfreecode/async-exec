// Async Await in 60 LOC by MasterQ32
// https://gist.github.com/MasterQ32/ff534f912c5faedbbb57974317e79778

const std = @import("std");

// usage:
fn asyncMain() !void {
    // Start two interleaving tasks
    var task_a = async waitUntilAndPrint(start + 1000, start + 1200, "task a");
    var task_b = async waitUntilAndPrint(start + 500, start + 1300, "task b");

    await task_a;
    await task_b;
}

fn waitUntilAndPrint(time1: i64, time2: i64, name: []const u8) void {
    waitForTime(time1);
    std.log.info("[{s}] it is now {} ms since start!", .{ name, std.time.milliTimestamp() - start });

    waitForTime(time2);
    std.log.info("[{s}] it is now {} ms since start!", .{ name, std.time.milliTimestamp() - start });
}

// impementation
const Task = struct { frame: anyframe, time: i64 };
var task_list = [1]?Task{null} ** 10;
var start: i64 = 0;

fn waitForTime(time: i64) void {
    suspend {
        // append the task into a list structure "tasks to be resumed later" and store the @frame()
        // as well as a condition (in this case: time)
        const slot = for (task_list) |*task| {
            if (task.* == null) break task;
        } else unreachable;
        slot.* = Task{ .frame = @frame(), .time = time };
    }
}

pub fn main() !void {
    start = std.time.milliTimestamp();

    var main_task = async asyncMain();

    while (true) // this is "the event loop"
    {
        const now = std.time.milliTimestamp();

        var any = false; // store if we have any tasks left
        for (task_list) |*task| {
            if (task.* != null) {
                any = true;
                if (task.*.?.time <= now) { // resume condition
                    var frame = task.*.?.frame; // resume location
                    task.* = null; // make task slot available again
                    resume frame;
                }
            }
        }
        if (!any) break; // all tasks done, we can now exit the program
    }

    // finalize the tasks properly (we know they finished as they are no more tasks left)
    // but main might have returned a error!
    try nosuspend await main_task;
}
