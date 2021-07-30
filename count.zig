// From std/event/loop.zig

// https://github.com/ziglang/zig/blob/c6844072ce440f581787bf97909261084a9edc6c/lib/std/io.zig#L8
pub const io_mode = .evented;

const std = @import("std");
const exec = @import("./exec.zig");

fn count(n: usize, interval: f64) f64 {
    const start = timeSec();
    // const sleep = std.event.Loop.instance.?.sleep;
    const sleep = std.time.sleep;
    // const sleep = exec.sleep;
    std.debug.print("{} {}: before loop\n", .{ std.Thread.getCurrentId(), timeSec() });
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        sleep(wait_ns);
        const thread_id = std.Thread.getCurrentId();
        std.debug.print("{} {}: {} seconds\n", .{ thread_id, timeSec(), interval });
    }
    return timeSec() - start;
}

fn timeSec() f64 {
    return @intToFloat(f64, std.time.nanoTimestamp()) /
        @intToFloat(f64, std.time.ns_per_s);
}

pub fn run() f64 {
    const thread_id = std.Thread.getCurrentId();
    std.debug.print("{} {}: begin\n", .{ thread_id, timeSec() });
    var frames = [_]@Frame(count){
        async count(2, 1.0),
        async count(3, 0.6),
    };
    std.debug.print("frame size: {}\n", .{@sizeOf(@TypeOf(frames[0]))});
    var total = @as(f64, 0);
    for (frames) |*frame| {
        total += await frame;
    }
    std.debug.print("{} {}: done\n", .{ thread_id, timeSec() });
    return total;
}

pub fn main() !void {
    // var task = async run();
    // std.debug.print("run size: {}\n", .{@sizeOf(@TypeOf(task))});
    // exec.runLoop(false);
    // const total = await task;
    // _ = try std.Thread.spawn(exec.runLoop, true);
    const total = run();
    std.debug.print("total: {}\n", .{total});
}
