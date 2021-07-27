// Small event loop by kprotty
// https://gist.github.com/kprotty/e4409f83f3e6ac87ae1c60f7aaaae95e

const std = @import("std");
const allocator = std.heap.page_allocator;

pub fn main() !void {
    const n = 10;
    const ret = try (try Task.run(fib, .{n}));
    std.debug.warn("fib({}) = {}\n", .{ n, ret });
}

fn fib(n: usize) std.mem.Allocator.Error!usize {
    Task.yield();

    if (n <= 1) {
        return n;
    }

    const l = try allocator.create(@Frame(fib));
    defer allocator.destroy(l);
    const r = try allocator.create(@Frame(fib));
    defer allocator.destroy(r);

    l.* = async fib(n - 1);
    r.* = async fib(n - 2);

    const lv = await l;
    const rv = await r;

    return (try lv) + (try rv);
}

pub const Task = struct {
    next: ?*Task = undefined,
    frame: anyframe,

    fn ReturnTypeOf(comptime func: anytype) type {
        return @typeInfo(@TypeOf(func)).Fn.return_type orelse unreachable;
    }

    pub fn run(comptime asyncFn: anytype, args: anytype) !ReturnTypeOf(asyncFn) {
        const Wrapper = struct {
            fn entry(fn_args: anytype, task: *Task, res: *?ReturnTypeOf(asyncFn)) void {
                Task.yield();
                const result = @call(.{}, asyncFn, fn_args);
                suspend res.* = result;
            }
        };

        var task: Task = undefined;
        var result: ?ReturnTypeOf(asyncFn) = null;
        var frame = async Wrapper.entry(args, &task, &result);

        while (Task.find()) |runnable_task| {
            resume runnable_task.frame;
        }

        return result orelse error.DeadLocked;
    }

    var head: ?*Task = null;
    var tail: ?*Task = null;

    pub fn yield() void {
        var task = Task{ .frame = @frame() };
        suspend task.schedule();
    }

    pub fn schedule(self: *Task) void {
        self.next = null;
        if (tail) |t|
            t.next = self;
        if (head == null)
            head = self;
        tail = self;
    }

    fn find() ?*Task {
        const task = head orelse return null;
        head = task.next;
        if (head == null)
            tail = null;
        return task;
    }
};
