const std = @import("std");
const expect = std.testing.expect;

var x: i32 = 1;

test "suspend with no resume" {
    var frame = async func();
    expect(x == 2);
}

fn func() void {
    x += 1;
    suspend;
    // This line is never reached because the suspend has no matching resume.
    x += 1;
}

test "resume from suspend" {
    var my_result: i32 = 1;
    _ = async testResumeFromSuspend(&my_result);
    expect(my_result == 2);
}

fn testResumeFromSuspend(my_result: *i32) void {
    suspend {
        resume @frame();
    }
    my_result.* += 1;
    suspend {}
    my_result.* += 1;
}
