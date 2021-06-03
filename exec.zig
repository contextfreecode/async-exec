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
