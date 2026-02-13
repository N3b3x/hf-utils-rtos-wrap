/**
 * @file ConsolePort.h
 * @brief Lightweight logging interface wrapping ESP-IDF log macros.
 *
 * Provides formatted printing without requiring ESP headers in every file.
 * This is a minimal implementation that forwards to ESP_LOG macros.
 *
 * Copyright © 2023-2025 Nebula Tech Corporation. All rights reserved.
 * Licensed under the GNU General Public License v3.0 or later.
 */

#ifndef CONSOLEPORT_H
#define CONSOLEPORT_H

#include <cstdarg>
#include <cstdio>
#include "esp_log.h"

class ConsolePort {
public:
    static ConsolePort& GetInstance() noexcept {
        static ConsolePort instance;
        return instance;
    }

    /// Printf-style write with tag
    static void Write(const char* tag, const char* format, ...) __attribute__((format(printf, 2, 3))) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, tag, format, args);
        va_end(args);
    }

    /// Printf-style write (instance)
    void WriteInstance(const char* format, ...) __attribute__((format(printf, 2, 3))) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, "ConsolePort", format, args);
        va_end(args);
    }

    /// Conditional write — only logs if condition is true
    static void WriteConditional(bool condition, const char* tag, const char* format, ...) __attribute__((format(printf, 3, 4))) {
        if (!condition) return;
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, tag, format, args);
        va_end(args);
    }

    /// Conditional write (2-arg overload: condition + format)
    static void WriteConditional(bool condition, const char* format, ...) __attribute__((format(printf, 2, 3))) {
        if (!condition) return;
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, "ConsolePort", format, args);
        va_end(args);
    }

    /// Log at Info level
    static void Info(const char* format, ...) __attribute__((format(printf, 1, 2))) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, "ConsolePort", format, args);
        va_end(args);
    }

    /// Print a newline
    void NewLine() noexcept {
        printf("\n");
    }

private:
    ConsolePort() = default;
};

#endif /* CONSOLEPORT_H */
