// From std/event/loop.zig

// pub const io_mode = .evented;

const std = @import("std");

fn count(n: usize, interval: f64) void {
    // See: https://github.com/ziglang/zig/blob/85755c51d529e7d9b406c6bdf69ce0a0f33f3353/lib/std/event/loop.zig#L765
    // const sleep = std.event.Loop.instance.?.sleep;
    const sleep = @import("./exec.zig").sleep;
    std.debug.print("{} {}: before loop\n", .{ std.Thread.getCurrentId(), time_s() });
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        sleep(wait_ns);
        const thread_id = std.Thread.getCurrentId();
        std.debug.print("{} {}: {} seconds\n", .{ thread_id, time_s(), interval });
    }
}

fn time_s() f64 {
    return @intToFloat(f64, std.time.nanoTimestamp()) /
        @intToFloat(f64, std.time.ns_per_s);
}

pub fn run() void {
    const thread_id = std.Thread.getCurrentId();
    std.debug.print("{} {}: begin\n", .{ thread_id, time_s() });
    var frames = [_]@Frame(count){
        async count(2, 1.0),
        async count(3, 0.6),
    };
    std.debug.print("frame size: {}\n", .{@sizeOf(@TypeOf(frames[0]))});
    for (frames) |*frame| {
        await frame;
    }
    std.debug.print("{} {}: done\n", .{ thread_id, time_s() });
}

pub fn main() void {
    const exec = @import("./exec.zig").exec;
    exec(&async run());
    // run();
}
