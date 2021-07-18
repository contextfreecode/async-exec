const std = @import("std");

pub fn exec(frame: anyframe->void) void {
    // std.debug.print("Hi: {}\n", .{@sizeOf(@TypeOf(frame))});
    // resume frame;
}

pub fn sleep(ns: u64) void {
    suspend {
        var frame = @frame();
        // resume frame;
    }
}
