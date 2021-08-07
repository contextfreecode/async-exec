// See:
// https://github.com/ziglang/zig/blob/c6844072ce440f581787bf97909261084a9edc6c/lib/std/io.zig#L8
// https://github.com/ziglang/zig/blob/ade85471e2cdab466ba685a38c2c7949c9dd1632/lib/std/start.zig#L428
pub const io_mode = .evented;

const std = @import("std");
const exec = @import("./exec.zig");

pub fn report(comptime format: []const u8, args: anytype) void {
    std.debug.print("{} ", .{std.Thread.getCurrentId()});
    std.debug.print(format, args);
    std.debug.print("\n", .{});
}

fn timerSeconds(timer: std.time.Timer) f64 {
    return @intToFloat(f64, timer.read()) / @intToFloat(f64, std.time.ns_per_s);
}

fn count(n: usize, interval: f64) f64 {
    const timer = std.time.Timer.start() catch unreachable;
    // const sleep = std.event.Loop.instance.?.sleep;
    const sleep = std.time.sleep;
    // const sleep = exec.sleep;
    report("before loop {}", .{interval});
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        sleep(wait_ns);
        report("slept {}", .{interval});
    }
    return timerSeconds(timer);
}

fn run() f64 {
    report("begin", .{});
    var frames = [_]@Frame(count){
        async count(2, 1.0),
        async count(3, 0.6),
    };
    report("count size: {}", .{@sizeOf(@TypeOf(frames[0]))});
    var total = @as(f64, 0);
    for (frames) |*frame| {
        total += await frame;
    }
    report("end", .{});
    return total;
}

pub fn main() !void {
    // _ = await async hi();
    // var task = async run();
    // report("run size: {}", .{@sizeOf(@TypeOf(task))});
    // exec.runLoop(false);
    // const total = await task;
    // _ = try std.Thread.spawn(exec.runLoop, true);
    const total = run();
    report("total: {}", .{total});
}

// fn hi() f64 {
//     return 1.0;
// }
