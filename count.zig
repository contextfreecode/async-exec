// From std/event/loop.zig

pub const io_mode = .evented;

const std = @import("std");
const Loop = std.event.Loop;

fn count(n: usize, interval: f64) void {
    var i: usize = 0;
    const wait_ns = @floatToInt(u64, interval * std.time.ns_per_s);
    while (i < n) : (i += 1) {
        Loop.instance.?.sleep(wait_ns);
        const thread_id = std.Thread.getCurrentId();
        std.debug.print(
            "{} {}: {} seconds\n", .{thread_id, time_s(), interval}
        );
    }
}

fn time_s() f64 {
    return
        @intToFloat(f64, std.time.nanoTimestamp()) /
        @intToFloat(f64, std.time.ns_per_s);
}

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    const thread_id = std.Thread.getCurrentId();
    std.debug.print("{} {}: begin\n", .{thread_id, time_s()});

    const frames = try allocator.alloc(@Frame(count), 2);
    // defer allocator.free(frames);
    frames[0] = async count(2, 1.0);
    frames[1] = async count(3, 0.6);

    for (frames) |*frame| {
        await frame;
    }
}
