[⬅️ Previous](BaseThread.md) | [🗂️ Index](index.md) | [➡️ Next](Queues.md)

# Synchronization Primitives 🔒

HF-RTOSW-ESPIDF provides several helpers to coordinate tasks safely.

## Highlights
- `Mutex` and `MutexGuard` provide RAII locking.
- `SignalSemaphore` is a lightweight binary semaphore for event notifications.
- `OsEventFlags` wraps an event group for broadcasting bit flags to multiple listeners.

## Mutex
`Mutex` wraps the underlying RTOS mutex. It is created on demand and deleted automatically when the object goes out of scope.

## MutexGuard
A convenience guard that locks a `Mutex` in its constructor and unlocks it when destroyed. Perfect for scoped locking.

## SignalSemaphore
A lightweight binary semaphore used to notify or wait for events between threads.

## OsEventFlags
Wrapper around an RTOS event group for broadcasting bit flags to multiple listeners.

## CriticalGuard
`CriticalGuard` disables interrupts while in scope using `portENTER_CRITICAL`. Use it for protecting short sections of code that must not be interrupted.

## Quick Example
```cpp
Mutex mtx;
{
    MutexGuard lock(mtx);       // protects shared data
    CriticalGuard irqLock;      // disable interrupts
    // critical section
}
```

[⬅️ Previous](BaseThread.md) | [🗂️ Index](index.md) | [➡️ Next](Queues.md)
