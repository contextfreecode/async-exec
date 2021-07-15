// From std/event/loop.zig

const std = @import("std");
const Loop = std.event.Loop;
const builtin = std.builtin;
const testing = std.testing;

pub const io_mode = .evented;

pub fn main() !void {
    // https://github.com/ziglang/zig/issues/1908
    if (!std.io.is_async) {
        std.debug.print("Not async\n", .{});
        return;
    }

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    const frames = try allocator.alloc(@Frame(testSleep), 10);
    // defer allocator.free(frames);

    const wait_time = 100 * std.time.ns_per_ms;
    var sleep_count: usize = 0;

    for (frames) |*frame| {
        frame.* = async testSleep(wait_time, &sleep_count);
    }
    for (frames) |*frame| {
        await frame;
    }

    std.debug.print("Expected {} ?= outcome {}\n", .{sleep_count, frames.len});
}

fn testSleep(wait_ns: u64, sleep_count: *usize) void {
    Loop.instance.?.sleep(wait_ns);
    _ = @atomicRmw(usize, sleep_count, .Add, 1, .SeqCst);
}
