[⬅️ Previous](Console.md) | [🗂️ Index](index.md) | [➡️ Next](Timers.md)

# RTOS Abstraction Layer 🎛️

`OsAbstraction` and `OsUtility` hide the direct FreeRTOS types behind a minimal interface. This makes it easier to port the component to other RTOSes in the future.

This wrapper targets FreeRTOS kernel version 11.1.x and is designed to match its APIs.
## Highlights
- Thin typedefs for tasks, queues and mutexes.
- New stream buffer helpers for variable-length data.
- Helper functions for delays and critical sections.
- `os_critical_enter()` and `os_critical_exit()` for interrupt masking.
- `CriticalGuard` RAII helper around those functions.
- Thin wrappers only; there is very little overhead.
- Designed for portability to other RTOSes.

[⬅️ Previous](Console.md) | [🗂️ Index](index.md) | [➡️ Next](Timers.md)
