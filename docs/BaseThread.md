[⬅️ Previous](index.md) | [🗂️ Index](index.md) | [➡️ Next](Synchronization.md)

# BaseThread Class 🚦

The `BaseThread` class provides a consistent interface for creating long-running worker threads in the HardFOC ecosystem. This document explains **why** you might use the class, its key methods, and shows real-world examples.

## 📜 Table of Contents
1. [Why BaseThread? 🌟](#why-basethread-🌟)
2. [Overview ✨](#overview-✨)
3. [Typical Lifecycle 🚀](#typical-lifecycle-🚀)
4. [Quick Example 📝](#quick-example-📝)
5. [Usage Walkthrough 🎯](#usage-walkthrough-🎯)
6. [Helpful Methods 🛠️](#helpful-methods-🛠️)
7. [Notes](#notes)
8. [Conclusion 🎉](#conclusion-🎉)

## Why BaseThread? 🌟
- **Cut boilerplate** – avoid writing repetitive FreeRTOS code every time you need a service thread.
- **Predictable lifecycle** – the same setup, step and cleanup flow keeps your firmware organised.
- **Portability** – the abstraction can be reused on different RTOS implementations with minimal effort.
- **Safer start/stop** – helper calls verify that threads actually transition to the desired state.

## Overview ✨
`BaseThread` is an abstract C++ class that wraps a FreeRTOS task. It handles the boilerplate required to start, stop and synchronise threads. You derive from `BaseThread` and implement three virtual methods:
- `Setup()` – executed once when the thread is first started.
- `Step()` – called repeatedly while the thread is running. It returns a delay in milliseconds before the next call.
- `Cleanup()` – executed after a stop request to perform any shutdown logic.

Behind the scenes the class uses semaphores and FreeRTOS primitives defined in `OsAbstraction.h`.

## Typical Lifecycle 🚀
1. **Construction** – create your derived object with a thread name.
2. **CreateBaseThread** – allocate the stack and specify priority plus other RTOS parameters.
3. **Start()** – signals the thread to begin. Internally it runs `StartAction()` and then wakes the thread.
4. **Running Loop** – the thread calls `Setup()` once, then repeatedly executes `Step()` and delays for the returned time.
5. **Stop()** – signals the thread to exit its loop. `Cleanup()` is then called.

You can also use helper methods like `StartThreadAndWaitToVerify()` to block until the thread has actually started.

## Quick Example 📝
```cpp
class MyWorker : public BaseThread {
public:
    MyWorker() : BaseThread("MyWorker") {}

    bool Setup() override {
        // Initialization code here
        return true;
    }

    uint32_t Step() override {
        // Main work done here
        return 100; // run again in 100 ms
    }

    bool Cleanup() override {
        // Teardown logic here
        return true;
    }
};

MyWorker worker;
uint8_t stack[2048];

void app_main() {
    worker.CreateBaseThread(stack, sizeof(stack), 5, 0, 0, OS_AUTO_START);
    worker.StartThreadAndWaitToVerify();
}
```
This example demonstrates creating a thread that repeats an action every 100 ms.

## Usage Walkthrough 🎯
1. **Allocate a stack** for your worker instance.
2. **Call `CreateBaseThread()`** with stack size, priority and other runtime parameters.
3. **Start the thread** using `StartThreadAndWaitToVerify()` (or `Start()`) when you are ready.
4. Let the thread run its `Step()` loop.
5. **Stop it gracefully** by calling `StopThreadAndWaitToVerify()`.

The helper methods block until a state change is confirmed, giving you confidence that the thread is actually up or down.

## Helpful Methods 🛠️
- `Suspend()` / `Resume()` – pause or continue the underlying OS task.
- `IsThreadRunning()` – check if the loop is currently active.
- `ChangePriority(uint32_t newPriority)` – adjust runtime priority.
- `GetStackHighWaterMark()` – inspect minimum remaining stack usage.
- `StartThreadAndWaitToVerify()` – blocking start with confirmation.
- `StopThreadAndWaitToVerify()` – blocking stop with confirmation.

## Notes
- The class relies on `OsAbstraction` wrappers, so ensure the FreeRTOS environment is initialized before creating threads.
- All methods are `noexcept` where possible to avoid unexpected exceptions in embedded environments.
- Use `SignalSemaphore` internally for controlling start and stop events.

## Conclusion 🎉
`BaseThread` aims to simplify thread management in HardFOC-based applications. By deriving from it and implementing a few virtual methods, you gain a structured pattern for your worker threads while keeping your code portable across RTOS implementations.

Happy coding! 😄

[⬅️ Previous](index.md) | [🗂️ Index](index.md) | [➡️ Next](Synchronization.md)
