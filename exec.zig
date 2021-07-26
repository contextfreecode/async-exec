const std = @import("std");

const Task = struct { frame: anyframe, time: i128 };
var task_list = [1]?Task{null} ** 10;
var task_mutex = std.Thread.Mutex{};

pub fn runTasks() void {
    while (true) {
        const lock = task_mutex.acquire();
        const now = std.time.nanoTimestamp();
        var any = false;
        var frame = for (task_list) |*task| {
            if (task.* != null) {
                any = true;
                if (task.*.?.time <= now) {
                    var frame = task.*.?.frame;
                    task.* = null;
                    break frame;
                }
            }
        } else null;
        lock.release();
        if (frame != null) resume frame.?;
        if (!any) break;
    }
}

pub fn sleep(ns: u64) void {
    suspend {
        const lock = task_mutex.acquire();
        defer lock.release();
        const slot = findSlot();
        const time = std.time.nanoTimestamp() + ns;
        slot.* = Task{ .frame = @frame(), .time = time };
    }
}

fn findSlot() *?Task {
    return for (task_list) |*task| {
        if (task.* == null) break task;
    } else unreachable;
}
