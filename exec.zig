const std = @import("std");

const Task = struct { frame: anyframe, time: i128 };
var task_list = [1]?Task{null} ** 10;

pub fn runTasks() void {
    while (true) {
        const now = std.time.nanoTimestamp();
        var any = false;
        for (task_list) |*task| {
            if (task.* != null) {
                any = true;
                if (task.*.?.time <= now) {
                    var frame = task.*.?.frame;
                    task.* = null;
                    resume frame;
                }
            }
        }
        if (!any) break;
    }
}

pub fn sleep(ns: u64) void {
    suspend {
        const slot = for (task_list) |*task| {
            if (task.* == null) break task;
        } else unreachable;
        const time = std.time.nanoTimestamp() + ns;
        slot.* = Task{ .frame = @frame(), .time = time };
    }
}
