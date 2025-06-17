#pragma once
/**
 * @file OsAbstraction.h
 * @brief Minimal generic RTOS abstraction used by the component.
 *
 * This header wraps the underlying RTOS (FreeRTOS on ESP-IDF) with a set of
 * generic types and helper functions.  None of the exported names expose the
 * specific RTOS being used so that a different implementation can be provided
 * in the future.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/stream_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Generic handle types used by the wrapper. */
typedef TaskHandle_t            OS_Thread;
typedef SemaphoreHandle_t       OS_Mutex;
typedef SemaphoreHandle_t       OS_Semaphore;
typedef QueueHandle_t           OS_Queue;
typedef EventGroupHandle_t      OS_EventGroup;
typedef TimerHandle_t           OS_Timer;
typedef StreamBufferHandle_t    OS_StreamBuffer;

/** Common integral aliases. */
typedef unsigned long           OS_Ulong;
typedef unsigned int            OS_Uint;

enum {
    OS_WAIT_FOREVER = portMAX_DELAY,
    OS_NO_WAIT      = 0,
    OS_SUCCESS      = 0,
    OS_AND          = 1,
    OS_OR           = 2,
    OS_AUTO_START   = 1,
    OS_DONT_START   = 0,
    OS_INHERIT      = 0
};

static inline OS_Ulong os_time_get(void) { return xTaskGetTickCount(); }

/* Thread wrappers ---------------------------------------------------------*/
typedef struct {
    void (*entry)(OS_Ulong);
    OS_Ulong arg;
} os_thread_start_t;

static inline void os_thread_start_trampoline(void *p)
{
    os_thread_start_t params = *(os_thread_start_t *)p;
    vPortFree(p);
    params.entry(params.arg);
}

static inline OS_Uint os_thread_create(OS_Thread *t, const char *name,
                                       void (*entry)(OS_Ulong),
                                       OS_Ulong input, uint8_t *stack,
                                       OS_Ulong stack_size, OS_Uint priority,
                                       OS_Uint /*preempt*/, OS_Ulong /*slice*/,
                                       OS_Uint auto_start)
{
    os_thread_start_t *params =
        (os_thread_start_t *)pvPortMalloc(sizeof(os_thread_start_t));
    if (!params) return 1;
    params->entry = entry;
    params->arg = input;
    BaseType_t res = xTaskCreate(os_thread_start_trampoline, name,
                                 stack_size / sizeof(StackType_t), params,
                                 priority, t);
    if (res != pdPASS) {
        vPortFree(params);
        return 1;
    }
    if (!auto_start)
        vTaskSuspend(*t);
    return OS_SUCCESS;
}

static inline OS_Uint os_thread_resume(OS_Thread *t)   { vTaskResume(*t); return OS_SUCCESS; }
static inline OS_Uint os_thread_suspend(OS_Thread *t)  { vTaskSuspend(*t); return OS_SUCCESS; }
static inline OS_Uint os_thread_delete(OS_Thread *t)   { vTaskDelete(*t);  return OS_SUCCESS; }
static inline OS_Uint os_thread_terminate(OS_Thread *t){ vTaskDelete(*t);  return OS_SUCCESS; }
static inline OS_Uint os_thread_info_get(OS_Thread *t, OS_Uint *state)
{
    if (state)
        *state = eTaskGetState(*t);
    return OS_SUCCESS;
}
static inline OS_Uint os_thread_sleep(OS_Ulong ticks)  { vTaskDelay(ticks); return OS_SUCCESS; }

/* Mutex wrappers ---------------------------------------------------------*/
static inline OS_Uint os_mutex_create(OS_Mutex *m, const char * /*name*/, OS_Uint /*inherit*/)
{
    *m = xSemaphoreCreateMutex();
    return (*m) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1;
}
static inline OS_Uint os_mutex_get(OS_Mutex *m, OS_Ulong wait)   { return xSemaphoreTake(*m, wait) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_mutex_put(OS_Mutex *m)                  { return xSemaphoreGive(*m) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_mutex_delete(OS_Mutex *m)               { vSemaphoreDelete(*m); return OS_SUCCESS; }

/* Semaphore wrappers -----------------------------------------------------*/
static inline OS_Uint os_semaphore_create(OS_Semaphore *s, const char* /*name*/, OS_Uint initial)
{
    *s = xSemaphoreCreateCounting(UINT16_MAX, initial);
    return (*s) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1;
}
static inline OS_Uint os_semaphore_delete(OS_Semaphore *s)   { vSemaphoreDelete(*s); return OS_SUCCESS; }
static inline OS_Uint os_semaphore_put(OS_Semaphore *s)      { return xSemaphoreGive(*s) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_semaphore_get(OS_Semaphore *s, OS_Ulong wait) { return xSemaphoreTake(*s, wait) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_semaphore_info_get(OS_Semaphore *s, OS_Ulong *count)
{
    if (count)
        *count = uxSemaphoreGetCount(*s);
    return OS_SUCCESS;
}

/* Queue wrappers ---------------------------------------------------------*/
static inline OS_Uint os_queue_create(OS_Queue *q, const char* /*name*/, OS_Uint item_words,
                                      void* /*store*/, OS_Ulong length)
{
    *q = xQueueCreate(length, item_words * sizeof(uint32_t));
    return (*q) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1;
}
static inline OS_Uint os_queue_delete(OS_Queue *q)    { vQueueDelete(*q); return OS_SUCCESS; }
static inline OS_Uint os_queue_send(OS_Queue *q, void *msg, OS_Ulong wait)    { return xQueueSend(*q, msg, wait) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_queue_receive(OS_Queue *q, void *msg, OS_Ulong wait) { return xQueueReceive(*q, msg, wait) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }

/* Event group wrappers ---------------------------------------------------*/
static inline OS_Uint os_event_group_create(OS_EventGroup *g, const char* /*name*/)
{
    *g = xEventGroupCreate();
    return (*g) ? (OS_Uint)OS_SUCCESS : (OS_Uint)1;
}
static inline OS_Uint os_event_group_delete(OS_EventGroup *g) { vEventGroupDelete(*g); return OS_SUCCESS; }
static inline OS_Uint os_event_group_set(OS_EventGroup *g, OS_Ulong flags)
{ xEventGroupSetBits(*g, flags); return OS_SUCCESS; }
static inline OS_Uint os_event_group_clear(OS_EventGroup *g, OS_Ulong flags)
{ xEventGroupClearBits(*g, flags); return OS_SUCCESS; }
static inline OS_Uint os_event_group_get(OS_EventGroup *g, OS_Ulong flags, OS_Uint option,
                                         OS_Ulong *actual, OS_Ulong wait)
{
    EventBits_t bits = xEventGroupWaitBits(*g, flags, option == OS_AND,
                                           option == OS_AND, wait);
    if (actual)
        *actual = bits;
    return OS_SUCCESS;
}

/* Timer wrappers ---------------------------------------------------------*/
typedef struct {
    void (*cb)(OS_Ulong);
    OS_Ulong id;
} os_timer_cb_t;

static inline void os_timer_trampoline(TimerHandle_t timer)
{
    os_timer_cb_t *params = (os_timer_cb_t *)pvTimerGetTimerID(timer);
    params->cb(params->id);
}

static inline OS_Uint os_timer_create(OS_Timer *t, const char *name,
                                      void (*cb)(OS_Ulong), OS_Ulong id,
                                      OS_Ulong initial, OS_Ulong reload,
                                      OS_Uint auto_act)
{
    os_timer_cb_t *params = (os_timer_cb_t *)pvPortMalloc(sizeof(os_timer_cb_t));
    if (!params) return 1;
    params->cb = cb;
    params->id = id;

    *t = xTimerCreate(name, initial, reload != 0, (void *)params,
                      os_timer_trampoline);
    if (!*t) {
        vPortFree(params);
        return 1;
    }
    if (auto_act)
        xTimerStart(*t, 0);
    return OS_SUCCESS;
}
static inline OS_Uint os_timer_delete(OS_Timer *t)
{
    os_timer_cb_t *params = (os_timer_cb_t *)pvTimerGetTimerID(*t);
    vPortFree(params);
    xTimerDelete(*t, 0);
    return OS_SUCCESS;
}
static inline OS_Uint os_timer_activate(OS_Timer *t)   { return xTimerStart(*t,0)==pdPASS ? OS_SUCCESS:(OS_Uint)1; }
static inline OS_Uint os_timer_deactivate(OS_Timer *t) { return xTimerStop(*t,0)==pdPASS ? OS_SUCCESS:(OS_Uint)1; }

/* Stream buffer wrappers ------------------------------------------------*/
static inline OS_Uint os_stream_buffer_create(OS_StreamBuffer *b, size_t capacity, size_t trigger)
{
    *b = xStreamBufferCreate(capacity, trigger);
    return (*b) ? OS_SUCCESS : (OS_Uint)1;
}
static inline OS_Uint os_stream_buffer_delete(OS_StreamBuffer *b) { vStreamBufferDelete(*b); return OS_SUCCESS; }
static inline OS_Uint os_stream_buffer_send(OS_StreamBuffer *b, const void *data, size_t len, OS_Ulong wait)
{ return xStreamBufferSend(*b, data, len, wait) == len ? (OS_Uint)OS_SUCCESS : (OS_Uint)1; }
static inline OS_Uint os_stream_buffer_receive(OS_StreamBuffer *b, void *data, size_t len, OS_Ulong wait)
{ return xStreamBufferReceive(*b, data, len, wait) > 0 ? OS_SUCCESS : (OS_Uint)1; }

/* Critical section wrappers ----------------------------------------------*/
typedef portMUX_TYPE OS_Critical;
extern OS_Critical os_critical_mux;
static inline void os_critical_enter(void)  { portENTER_CRITICAL(&os_critical_mux); }
static inline void os_critical_exit(void)   { portEXIT_CRITICAL(&os_critical_mux); }

#ifdef __cplusplus
}
#endif

