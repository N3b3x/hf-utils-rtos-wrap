[⬅️ Previous](Synchronization.md) | [🗂️ Index](index.md) | [➡️ Next](Console.md)

# Message Queues 📬

The `OsQueue` template offers a simple way to pass messages between threads.

## Highlights
- Lazy initialization keeps memory usage low until the queue is used.
- Template parameters enforce type safety.
- Works smoothly with your `BaseThread` workers.

## Lazy Creation
Queues are created the first time you send or receive. This keeps memory usage minimal until the queue is actually needed.

## Usage
1. Declare an `OsQueue<T>` with the message type you want.
2. Send items with `Push()` and retrieve them with `Pop()`.
3. `Size()` returns the current number of messages in the queue.

## Quick Example
```cpp
OsQueue<int, 64> q("Numbers", sizeof(int));
q.Push(42);
int val;
if (q.Pop(val)) {
    // use val
}
```

[⬅️ Previous](Synchronization.md) | [🗂️ Index](index.md) | [➡️ Next](Console.md)
