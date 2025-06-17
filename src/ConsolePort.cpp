/**
 * @file ConsolePort.c
 * @brief Implementation of C-compatible logging functions.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 */

#include "ConsolePort.h"

void console_set_level(const char* tag, esp_log_level_t level) {
    esp_log_level_set(tag, level);
}

void console_info(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
    va_end(ap);
}

void console_warn(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_WARN, tag, fmt, ap);
    va_end(ap);
}

void console_error(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_ERROR, tag, fmt, ap);
    va_end(ap);
}

void console_debug(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_DEBUG, tag, fmt, ap);
    va_end(ap);
}

void console_verbose(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_VERBOSE, tag, fmt, ap);
    va_end(ap);
}

void console_write_conditional(bool cond, const char* tag, const char* fmt, ...) {
    if (!cond) return;
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
    va_end(ap);
}

void console_write(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
    va_end(ap);
}
