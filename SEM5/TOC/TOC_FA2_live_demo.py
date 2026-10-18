"""
TOC FA2 live demo: Runtime Call-Stack Verification vs Infinite Loop Detection
Run: python TOC_FA2_live_demo.py
The infinite-loop demonstration is bounded by a time monitor for safety.
"""
import time
import sys
import traceback


def helper(n):
    print("  helper frame active")
    return n * 2


def compute():
    print(" compute frame active")
    value = helper(5)
    return value + 1


def demo_normal_termination():
    print("\n=== DEMO 1: Normal call/return ===")
    print("Result:", compute())  # Expected: 11


def descend(n):
    print("enter", n)
    if n == 0:
        return "done"
    result = descend(n - 1)
    print("return", n)
    return result


def demo_bounded_recursion():
    print("\n=== DEMO 2: Bounded recursion / stack growth ===")
    print("Result:", descend(3))
    print("Notice: calls enter in descending order; returns unwind in reverse order.")


def demo_stack_trace():
    print("\n=== DEMO 3: Inspect a stack trace ===")
    def inner():
        print("Current Python stack (most recent call last):")
        traceback.print_stack(limit=5)
    def outer():
        inner()
    outer()


def monitored_loop(max_seconds=2.0):
    start = time.monotonic()
    steps = 0
    while True:
        steps += 1
        if time.monotonic() - start >= max_seconds:
            print(f"Monitor stopped the run after about {max_seconds:.1f}s; steps={steps}")
            return steps


def demo_monitored_infinite_loop():
    print("\n=== DEMO 4: Infinite loop with a runtime time limit ===")
    print("Loop condition is always true; the monitor ends this demo after 2 seconds.")
    monitored_loop(2.0)


if __name__ == "__main__":
    demo_normal_termination()
    demo_bounded_recursion()
    demo_stack_trace()
    demo_monitored_infinite_loop()
    print("\nAll demos finished. The monitor demonstrates a runtime policy, not a general proof of termination.")
