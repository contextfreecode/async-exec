// From std/event/loop.zig

pub const io_mode = .evented;

const std = @import("std");
const exec = @import("./exec.zig");

fn count(n: usize, interval: f64) f64 {
    const start = time_s();
    const sleep = std.event.Loop.instance.?.sleep;
    // const sleep = exec.sleep;
    std.debug.print("{} {}: before loop\n", .{ std.Thread.getCurrentId(), time_s() });
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        sleep(wait_ns);
        const thread_id = std.Thread.getCurrentId();
        std.debug.print("{} {}: {} seconds\n", .{ thread_id, time_s(), interval });
    }
    return time_s() - start;
}

fn time_s() f64 {
    return @intToFloat(f64, std.time.nanoTimestamp()) /
        @intToFloat(f64, std.time.ns_per_s);
}

pub fn run() f64 {
    const thread_id = std.Thread.getCurrentId();
    std.debug.print("{} {}: begin\n", .{ thread_id, time_s() });
    var frames = [_]@Frame(count){
        async count(2, 1.0),
        async count(3, 0.6),
    };
    std.debug.print("frame size: {}\n", .{@sizeOf(@TypeOf(frames[0]))});
    var total = @as(f64, 0);
    for (frames) |*frame| {
        total += await frame;
    }
    std.debug.print("{} {}: done\n", .{ thread_id, time_s() });
    return total;
}

pub fn main() void {
    // var task = async run();
    // std.debug.print("run size: {}\n", .{@sizeOf(@TypeOf(task))});
    // exec.block();
    // var total = nosuspend await task;
    const total = run();
    std.debug.print("total: {}\n", .{total});
}
