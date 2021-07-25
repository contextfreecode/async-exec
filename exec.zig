const std = @import("std");

const Loop = struct {
    frames: std.atomic.Queue(anyframe),
};

const Waiter = struct {
    node: std.atomic.Queue(anyframe).Node,
    ns: u64,
};

var loop = Loop {
    .frames = std.atomic.Queue(anyframe).init(),
};

pub fn exec(frame: anyframe->void) void {
    // std.debug.print("Hi: {}\n", .{@sizeOf(@TypeOf(frame))});
    // nosuspend await frame;
}

pub fn sleep(ns: u64) !void {
    suspend {
        var frame = @frame();
        std.debug.print("{}\n", .{@sizeOf(@TypeOf(frame))});
        var waiter = Waiter {.node = undefined, .ns = ns};
        waiter.node.data = frame;
        var thread = try std.Thread.spawn(sleepThread, &waiter);
    }
}

fn sleepThread(waiter: *Waiter) void {
    const ns = waiter.ns;
    std.os.nanosleep(ns / std.time.ns_per_s, ns % std.time.ns_per_s);
    std.debug.print("waited\n", .{});
    loop.frames.put(&waiter.node);
}

test "test spawn" {
    var waiter = Waiter {.node = undefined, .ns = std.time.ns_per_s / 2};
    var thread = try std.Thread.spawn(sleepThread, &waiter);
    // try sleep(std.time.ns_per_s / 2);
    std.os.nanosleep(1, 0);
    // std.Thread.wait(thread);
    std.debug.print("done\n", .{});
}
