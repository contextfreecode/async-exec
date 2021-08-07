// From std/event/loop.zig

// https://github.com/ziglang/zig/blob/c6844072ce440f581787bf97909261084a9edc6c/lib/std/io.zig#L8
pub const io_mode = .evented;

const std = @import("std");
const exec = @import("./exec.zig");

fn threadId() i32 {
    return std.Thread.getCurrentId();
}

fn count(n: usize, interval: f64) !f64 {
    const timer = try std.time.Timer.start();
    // const sleep = std.event.Loop.instance.?.sleep;
    const sleep = std.time.sleep;
    // const sleep = exec.sleep;
    std.debug.print("{} before loop {}\n", .{ threadId(), interval });
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        sleep(wait_ns);
        std.debug.print("{} slept {}\n", .{ threadId(), interval });
    }
    return timerSeconds(timer);
}

fn timerSeconds(timer: std.time.Timer) f64 {
    return @intToFloat(f64, timer.read()) / @intToFloat(f64, std.time.ns_per_s);
}

fn run() !f64 {
    std.debug.print("{} begin\n", .{threadId()});
    var frames = [_]@Frame(count){
        async count(2, 1.0),
        async count(3, 0.6),
    };
    std.debug.print("{} count size: {}\n", .{ threadId(), @sizeOf(@TypeOf(frames[0])) });
    var total = @as(f64, 0);
    for (frames) |*frame| {
        total += try await frame;
    }
    std.debug.print("{} end\n", .{threadId()});
    return total;
}

pub fn main() !void {
    // var task = async run();
    // std.debug.print("{} run size: {}\n", .{threadId(), @sizeOf(@TypeOf(task))});
    // exec.runLoop(false);
    // const total = await task;
    // _ = try std.Thread.spawn(exec.runLoop, true);
    // _ = await async hi();
    const total = run();
    std.debug.print("{} total: {}\n", .{ threadId(), total });
}

// fn hi() f64 {
//     return 1.0;
// }
