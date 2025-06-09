[⬅️ Previous](Queues.md) | [🗂️ Index](index.md) | [➡️ Next](RTOSAbstraction.md)

# Console Port 📣

The `ConsolePort` class is a tiny wrapper around the ESP‑IDF logging macros. It allows formatted printing without directly including the ESP headers in every file.

## Highlights
- Static methods mirror `ESP_LOGx` functions.
- Keeps your application code independent of ESP headers.

## Quick Example
```cpp
ConsolePort::Info("Hello %d", 123);
```

[⬅️ Previous](Queues.md) | [🗂️ Index](index.md) | [➡️ Next](RTOSAbstraction.md)
