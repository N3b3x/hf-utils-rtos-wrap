/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *
  *  Contains the declaration and definition of the utility functions related to
  *     the generic OS..
  *
  *   Note:  These functions are not thread or interrupt-safe and should be called
  *          called with appropriate guards if used within an ISR or shared between tasks.
*/
#ifndef OS_UTILITY_H_
#define OS_UTILITY_H_

#include "OsAbstraction.h"

#include "CommonIDs.h"
#include "Utility.h"

#define UTIL_SYSTEM_CLOCK (240000000.0)	//240MHz

static constexpr uint32_t osTickRateHz = configTICK_RATE_HZ; // 1000 ticks per second

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Delays execution for the specified number of milliseconds.
 *
 * @param msec The delay duration in milliseconds.
 */
void os_delay_msec(uint16_t msec);

/**
 * @brief Delays execution for the specified number of time.
 *
 * Currently only supports MSEC AND S, everything else just returns
 *
 * @param msec The delay duration in milliseconds.
 */
void os_delay_time(uint32_t timeBetweenSamples, time_unit_t timeUnit);

/**
 * @brief Retrieves the elapsed time in milliseconds.
 *
 * @return The elapsed time in milliseconds.
 */
uint32_t os_get_elapsed_time_msec();

/**
 * @brief Function to get the elapsed time from a specified processor cycle count.
 * 		  The cycle count can be acquired through os_get_processor_cycle_count() function.
 *
 * @param startCycleCount :	value of the cycle count DWT->CYCCNT at first position of the program.
 * @param unit 			  :	UNIT_MS,  if the result is requested in milliseconds.
 *							UNIT_US,  if the result is requested in microseconds.
 *							UNIT_SEC, if the result is requested in seconds.
 *
 * @return Elapsed time in the unit specified
 */
uint32_t os_get_elapsed_processor_cycle_count(uint32_t startCycleCount, time_unit_t unit);

/**
 * @brief   Gets the processor's current cycle count
 * @return	Processor's current cycle count (DWT->CYCCNT)
 */
uint32_t os_get_processor_cycle_count();

/**
 * @brief Converts milliseconds to delay ticks.
 *
 * @param milliseconds The duration in milliseconds.
 * @return The corresponding delay ticks.
 */
constexpr uint32_t os_convert_msec_to_delay_ticks( uint32_t milliseconds )
{
	//Calculate the delay in terms of the threadx tick rate */
	return milliseconds * osTickRateHz / 1000U;
}


/**
 * @brief Converts delay ticks to milliseconds.
 *
 * @param delayTicks The duration in delay ticks.
 * @return The corresponding milliseconds.
 */
constexpr uint32_t os_convert_delay_ticks_to_msec(uint32_t delayTicks)
{
    // Calculate the delay in terms of milliseconds
    return (delayTicks * 1000U) / osTickRateHz;
}

/**
 * @brief Converts frequency to delay ticks.
 *
 * @param frequency The frequency value.
 * @return The corresponding delay ticks.
 */
constexpr uint32_t os_convert_hz_to_delay_ticks( uint32_t frequency )
{
	// Calculate the delay in terms of the threadx tick rate
	return osTickRateHz/frequency;
}

/**
 * @brief Handler for stack faults in threads.
 *
 * @param thread The thread where the stack fault occurred.
 */
void os_stack_fault_handler(OS_Thread* thread);

//=============//
// MUTEX
//=============//

/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The OS mutex to create
 * @param name - The name fo rthe mutex
 * @param name - The priority for the mutex
 */
bool os_mutex_create_ex(OS_Mutex& mutex, const char* mutexName,
                   OS_Uint priority = OS_INHERIT,
                   bool suppressVerbose=true ) noexcept;

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool os_mutex_get_ex(OS_Mutex& mutex_ptr, OS_Ulong wait_option, bool suppressVerbose=true) noexcept;

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool os_mutex_put_ex(OS_Mutex& mutex_ptr, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The OS mutex to delete
 */
bool os_mutex_delete_ex(OS_Mutex& mutex, bool suppressVerbose=true) noexcept;


/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The OS mutex to create
 * @param name - The name fo rthe mutex
 * @param name - The priority for the mutex
 */
bool os_mutex_create_p(OS_Mutex* mutex, const char* mutexName,
                    OS_Uint priority = OS_INHERIT,
                    bool suppressVerbose=true ) noexcept;

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool os_mutex_get_p(OS_Mutex* mutex_ptr, OS_Ulong wait_option, bool suppressVerbose=true) noexcept;

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool os_mutex_put_p(OS_Mutex* mutex_ptr, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The OS mutex to delete
 */
bool os_mutex_delete_p(OS_Mutex* mutex, bool suppressVerbose=true) noexcept;

//=============//
// THREAD
//=============//

/**
 * @brief Function to Create TX thread.
 * @param txThread Pointer to the OS thread control block.
 * @param name Name of the thread.
 * @param entry_function Pointer to the thread's entry function.
 * @param entry_input Input value for the thread's entry function.
 * @param stack Pointer to the thread's stack.
 * @param stackSizeBytes Size of the thread's stack in bytes.
 * @param priority Priority of the thread.
 * @param preempt_threshold Preemption threshold of the thread.
 * @param timeSliceAllowed Maximum length of time (in ticks) that the thread can execute before OS will reschedule.
 * @param auto_start OS_AUTO_START if the thread should start immediately, or OS_DONT_START if it should not start until explicitly resumed.
 * @return true if the thread was successfully created, false otherwise.
 */
bool os_thread_create_ex(OS_Thread* txThread, const char* name,
                    void (*entry_function)(OS_Ulong id), OS_Ulong entry_input,
                    uint8_t* stack, OS_Ulong stackSizeBytes, OS_Uint priority,
                    OS_Uint preempt_threshold, OS_Ulong timeSliceAllowed,
                    OS_Uint auto_start, bool suppressVerbose=true) noexcept;

/**
 * @brief Resumes the specified OS thread.
 *
 * @param thread The OS thread to resume.
 */
bool os_thread_resume_ex(OS_Thread* thread, bool suppressVerbose=true);

/**
 * @brief Function to Resume TX thread only if it is suspended.
 * @param txThread Pointer to the OS thread control block to be resumed.
 * @return true if the thread was successfully resumed or if it was not suspended, false otherwise.
 */
bool os_thread_resume_if_suspended(OS_Thread *txThread, bool suppressVerbose=true) noexcept;

/**
 * @brief Suspends the specified OS thread.
 *
 * @param thread The OS thread to resume.
 */
bool os_thread_suspend_ex( OS_Thread* thread, bool suppressVerbose=true);

/**
 * @brief Function to Delete TX thread.
 * @param txThread Pointer to the OS thread control block to be deleted.
 * @return true if the thread was successfully deleted, false otherwise.
 */
bool os_thread_delete_ex(OS_Thread *txThread, bool suppressVerbose=true) noexcept;

//=============//
// QUEUE
//=============//

/**
 * @brief Creates a queue with the specified parameters.
 *
 * @param queue - The OS queue to create
 * @param queueName - The name for the queue
 * @param messageSizeInWords - The size of the messages that will be stored in the queue
 */
bool os_queue_create_ex(OS_Queue& queue, const char* queueName, OS_Uint messageSizeInWords,
                   void* queueStorage, OS_Ulong queueSize, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified queue and logs any error.
 *
 * @param queue - The OS queue to delete
 **/
bool os_queue_delete_ex(OS_Queue& queue, bool suppressVerbose=true) noexcept;

/**
 * @brief Send a message to the queue.
 *
 * @param queue - The OS queue to send the message to
 * @param message - The message to send
 * @param wait_option - The option on how to wait if the queue is full
 */
bool os_queue_send_ex(OS_Queue& queue, void* message, OS_Ulong wait_option = OS_WAIT_FOREVER,
                   bool suppressVerbose=true) noexcept;

/**
 * @brief Receive a message from the queue.
 *
 * @param queue - The OS queue to receive the message from
 * @param message - The received message
 * @param wait_option - The option on how to wait if the queue is empty
 */
bool os_queue_receive_ex(OS_Queue& queue, void* message, OS_Ulong wait_option = OS_WAIT_FOREVER,
                        bool suppressVerbose=true) noexcept;

//=============//
// TIMER
//=============//
/**
  * @brief Creates a OS timer and reports any errors associated with the creation.
  *
  * @param timer Reference to the timer control block.
  * @param name Pointer to the name of the timer.
  * @param callback Function pointer to the timer expiration callback function.
  * @param callbackExpirationInput A uint32_t data point that can be passed to the expiration callback function.
  * @param initialTimeoutTicks The initial expiration time in ticks.
  * @param rescheduleTimeoutTicks The reschedule time in ticks.
  * @param autoActivate Flag to indicate whether the timer should be automatically activated upon creation.
  * @return Returns true if the timer was successfully created, false otherwise.
 */
bool os_timer_create_ex ( OS_Timer& timer, const char* name, void (*callback)(uint32_t),
                     uint32_t callbackExpirationInput, uint32_t initialTimeoutTicks,
                     uint32_t rescheduleTimeoutTicks, OS_Uint autoActivate,
                     bool suppressVerbose=true ) noexcept;

/**
  * @brief Stops and deletes a OS timer, reporting any errors associated with the operation.
  *
  * @param timer Reference to the timer to be stopped and deleted.
  * @return Returns true if the timer was successfully stopped and deleted, false otherwise.
  **/
bool os_timer_deactivate_and_delete_ex( OS_Timer& timer, bool suppressVerbose=true) noexcept;

/**
  * @brief Activates a OS timer.
  *
  * @param timer Reference to the timer to be activated.
  * @return Returns true if the timer was successfully activated, false otherwise.
  */
bool os_timer_activate_ex( OS_Timer& timer, bool suppressVerbose=true) noexcept;

/**
  * @brief Deactivates a OS timer.
  *
  * @param timer Reference to the timer to be deactivated.
  * @return Returns true if the timer was successfully deactivated, false otherwise.
  */
bool os_timer_deactivate_ex( OS_Timer& timer, bool suppressVerbose=true) noexcept;

//=============//
// SEMAPHORES
//=============//

/**
 * @brief Function to Create TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @param name Name of the semaphore.
 * @param initial_count Initial count of the semaphore.
 * @return true if the semaphore was successfully created, false otherwise.
 */
bool os_semaphore_create_ex(OS_Semaphore* txSemaphore, const char* name, OS_Uint initial_count,
                       bool suppressVerbose=true) noexcept;

/**
 * @brief Function to Delete TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block to be deleted.
 * @return true if the semaphore was successfully deleted, false otherwise.
 */
bool os_semaphore_delete_ex(OS_Semaphore *txSemaphore, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to put (release) a TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @return true if the semaphore was successfully released, false otherwise.
 */
bool os_semaphore_put_ex(OS_Semaphore* txSemaphore, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to get (wait for) a TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @param wait_option Specifies the maximum time to wait for the semaphore.
 * @return true if the semaphore was successfully acquired, false otherwise.
 */
bool os_semaphore_get_ex(OS_Semaphore* txSemaphore, OS_Ulong wait_option,
                    bool suppressVerbose=true) noexcept;

/**
 * @brief Function to get TX semaphore count.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @return the count of the semaphore.
 */
OS_Ulong os_semaphore_get_count_ex(OS_Semaphore *txSemaphore, bool suppressVerbose=true) noexcept;

//=============//
// EVENTS
//=============//

/**
 * @brief Creates an event flag group with the specified parameters.
 *
 * @param eventFlags - The OS event flags to create
 * @param name - The name for the event flag group
 */
bool os_event_flags_create_ex(OS_EventGroup& eventFlags, const char* name, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified event flag group and logs any error.
 *
 * @param eventFlags - The OS event flags to delete
 **/
bool os_event_flags_delete_ex(OS_EventGroup& eventFlags, bool suppressVerbose=true) noexcept;

/**
 * @brief Set event flags in the event flag group.
 *
 * @param eventFlags - The OS event flags to set
 * @param flagsToSet - The flags to set
 */
bool os_event_flags_set_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToSet,
                     bool suppressVerbose=true) noexcept;

/**
 * @brief Clears event flags in the event flag group.
 *
 * @param eventFlags - The OS event flags to set
 * @param flagsToClear - The flags to clear
 */
bool os_event_flags_clear_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToClear,
                       bool suppressVerbose=true) noexcept;

/**
 * @brief Get event flags from the event flag group.
 *
 * @param eventFlags - The OS event flags to get
 * @param flagsToGet - The flags to get
 * @param getOption - The option on how to get the flags
 * @param actualFlags - The actual flags gotten
 * @param wait_option - The option on how to wait if the flags are not available
 */
bool os_event_flags_get_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToGet,
                     OS_Uint getOption, OS_Ulong& actualFlags,
                     OS_Ulong wait_option, bool suppressVerbose=true) noexcept;

//=============//
//=============//

#ifdef __cplusplus
}
#endif

#endif // OS_UTILITY_H_
