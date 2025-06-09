/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *
  *  Contains the declaration and definition of the utility functions related to
  *     the threadX RTOS..
  *
  *   Note:  These functions are not thread or interrupt-safe and should be called
  *          called with appropriate guards if used within an ISR or shared between tasks.

#ifndef TXUTILITY_H_
#define TXUTILITY_H_
#include "UTILITIES/common/RtosCompat.h"

#include <UTILITIES/common/CommonIDs.h>
#include <UTILITIES/common/Utility.h>

#define UTIL_SYSTEM_CLOCK (240000000.0)	//240MHz

static constexpr uint32_t threadxTickRateHz = TX_TIMER_TICKS_PER_SECOND; // 1000 ticks per second

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Delays execution for the specified number of milliseconds.
 *
 * @param msec The delay duration in milliseconds.
 */
void TxDelayMsec(uint16_t msec);

/**
 * @brief Delays execution for the specified number of time.
 *
 * Currently only supports MSEC AND S, everything else just returns
 *
 * @param msec The delay duration in milliseconds.
 */
void TxDelayTime(uint32_t timeBetweenSamples, time_unit_t timeUnit);

/**
 * @brief Retrieves the elapsed time in milliseconds.
 *
 * @return The elapsed time in milliseconds.
 */
uint32_t GetElapsedTimeMsec();

/**
 * @brief Function to get the elapsed time from a specified processor cycle count.
 * 		  The cycle count can be acquired through GetProcessorCycleCount() function.
 *
 * @param startCycleCount :	value of the cycle count DWT->CYCCNT at first position of the program.
 * @param unit 			  :	UNIT_MS,  if the result is requested in milliseconds.
 *							UNIT_US,  if the result is requested in microseconds.
 *							UNIT_SEC, if the result is requested in seconds.
 *
 * @return Elapsed time in the unit specified
 */
uint32_t GetElapsedProcessorCycleCount(uint32_t startCycleCount, time_unit_t unit);

/**
 * @brief   Gets the processor's current cycle count
 * @return	Processor's current cycle count (DWT->CYCCNT)
 */
uint32_t GetProcessorCycleCount();

/**
 * @brief Converts milliseconds to delay ticks.
 *
 * @param milliseconds The duration in milliseconds.
 * @return The corresponding delay ticks.
 */
constexpr uint32_t ConvertMsecToDelayTicks( uint32_t milliseconds )
{
	//Calculate the delay in terms of the threadx tick rate */
	return milliseconds * threadxTickRateHz / 1000U;
}


/**
 * @brief Converts delay ticks to milliseconds.
 *
 * @param delayTicks The duration in delay ticks.
 * @return The corresponding milliseconds.
 */
constexpr uint32_t ConvertDelayTicksToMsec(uint32_t delayTicks)
{
    // Calculate the delay in terms of milliseconds
    return (delayTicks * 1000U) / threadxTickRateHz;
}

/**
 * @brief Converts frequency to delay ticks.
 *
 * @param frequency The frequency value.
 * @return The corresponding delay ticks.
 */
constexpr uint32_t ConvertHzToDelayTicks( uint32_t frequency )
{
	// Calculate the delay in terms of the threadx tick rate
	return threadxTickRateHz/frequency;
}

/**
 * @brief Handler for stack faults in threads.
 *
 * @param thread The thread where the stack fault occurred.
 */
void StackFaultHandler(TX_THREAD* thread);

//=============//
// MUTEX
//=============//

/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The ThreadX mutex to create
 * @param name - The name fo rthe mutex
 * @param name - The priority for the mutex
 */
bool CreateTxMutex(TX_MUTEX& mutex, const char* mutexName, UINT priority = TX_INHERIT, bool suppressVerbose=true ) noexcept;

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool GetTxMutex(TX_MUTEX& mutex_ptr, ULONG wait_option, bool suppressVerbose=true) noexcept;

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool PutTxMutex(TX_MUTEX& mutex_ptr, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The ThreadX mutex to delete
 */
bool DeleteTxMutex(TX_MUTEX& mutex, bool suppressVerbose=true) noexcept;


/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The ThreadX mutex to create
 * @param name - The name fo rthe mutex
 * @param name - The priority for the mutex
 */
bool CreateTxMutexP(TX_MUTEX* mutex, const char* mutexName, UINT priority = TX_INHERIT, bool suppressVerbose=true ) noexcept;

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool GetTxMutexP(TX_MUTEX* mutex_ptr, ULONG wait_option, bool suppressVerbose=true) noexcept;

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool PutTxMutexP(TX_MUTEX* mutex_ptr, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The ThreadX mutex to delete
 */
bool DeleteTxMutexP(TX_MUTEX* mutex, bool suppressVerbose=true) noexcept;

//=============//
// THREAD
//=============//

/**
 * @brief Function to Create TX thread.
 * @param txThread Pointer to the ThreadX thread control block.
 * @param name Name of the thread.
 * @param entry_function Pointer to the thread's entry function.
 * @param entry_input Input value for the thread's entry function.
 * @param stack Pointer to the thread's stack.
 * @param stackSizeBytes Size of the thread's stack in bytes.
 * @param priority Priority of the thread.
 * @param preempt_threshold Preemption threshold of the thread.
 * @param timeSliceAllowed Maximum length of time (in ticks) that the thread can execute before ThreadX will reschedule.
 * @param auto_start TX_AUTO_START if the thread should start immediately, or TX_DONT_START if it should not start until explicitly resumed.
 * @return true if the thread was successfully created, false otherwise.
 */
bool CreateTxThread(TX_THREAD* txThread, const char* name, VOID (*entry_function)(ULONG id),ULONG entry_input,
						uint8_t* stack, ULONG stackSizeBytes, UINT priority,  UINT preempt_threshold,
						ULONG timeSliceAllowed, UINT auto_start, bool suppressVerbose=true) noexcept;

/**
 * @brief Resumes the specified ThreadX thread.
 *
 * @param thread The ThreadX thread to resume.
 */
bool ResumeTxThread(TX_THREAD* thread, bool suppressVerbose=true);

/**
 * @brief Function to Resume TX thread only if it is suspended.
 * @param txThread Pointer to the ThreadX thread control block to be resumed.
 * @return true if the thread was successfully resumed or if it was not suspended, false otherwise.
 */
bool ResumeTxThreadIfSuspended(TX_THREAD *txThread, bool suppressVerbose=true) noexcept;

/**
 * @brief Suspends the specified ThreadX thread.
 *
 * @param thread The ThreadX thread to resume.
 */
bool SuspendTxThread( TX_THREAD* thread, bool suppressVerbose=true);

/**
 * @brief Function to Delete TX thread.
 * @param txThread Pointer to the ThreadX thread control block to be deleted.
 * @return true if the thread was successfully deleted, false otherwise.
 */
bool DeleteTxThread(TX_THREAD *txThread, bool suppressVerbose=true) noexcept;

//=============//
// QUEUE
//=============//

/**
 * @brief Creates a queue with the specified parameters.
 *
 * @param queue - The ThreadX queue to create
 * @param queueName - The name for the queue
 * @param messageSizeInWords - The size of the messages that will be stored in the queue
 */
bool CreateTxQueue(TX_QUEUE& queue, const char* queueName, UINT messageSizeInWords, void* queueStorage, ULONG queueSize, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified queue and logs any error.
 *
 * @param queue - The ThreadX queue to delete
 **/
bool DeleteTxQueue(TX_QUEUE& queue, bool suppressVerbose=true) noexcept;

/**
 * @brief Send a message to the queue.
 *
 * @param queue - The ThreadX queue to send the message to
 * @param message - The message to send
 * @param wait_option - The option on how to wait if the queue is full
 */
bool SendToTxQueue(TX_QUEUE& queue, void* message, ULONG wait_option = TX_WAIT_FOREVER, bool suppressVerbose=true) noexcept;

/**
 * @brief Receive a message from the queue.
 *
 * @param queue - The ThreadX queue to receive the message from
 * @param message - The received message
 * @param wait_option - The option on how to wait if the queue is empty
 */
bool ReceiveFromTxQueue(TX_QUEUE& queue, void* message, ULONG wait_option = TX_WAIT_FOREVER, bool suppressVerbose=true) noexcept;

//=============//
// TIMER
//=============//
/**
  * @brief Creates a ThreadX timer and reports any errors associated with the creation.
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
bool CreateTxTimer ( TX_TIMER& timer, const char* name, void (*callback)(uint32_t), uint32_t callbackExpirationInput,
		  uint32_t initialTimeoutTicks, uint32_t rescheduleTimeoutTicks, UINT autoActivate, bool suppressVerbose=true ) noexcept;

/**
  * @brief Stops and deletes a ThreadX timer, reporting any errors associated with the operation.
  *
  * @param timer Reference to the timer to be stopped and deleted.
  * @return Returns true if the timer was successfully stopped and deleted, false otherwise.
  **/
bool DeactivateAndDeleteTxTimer( TX_TIMER& timer, bool suppressVerbose=true) noexcept;

/**
  * @brief Activates a ThreadX timer.
  *
  * @param timer Reference to the timer to be activated.
  * @return Returns true if the timer was successfully activated, false otherwise.
  */
bool ActivateTxTimer( TX_TIMER& timer, bool suppressVerbose=true) noexcept;

/**
  * @brief Deactivates a ThreadX timer.
  *
  * @param timer Reference to the timer to be deactivated.
  * @return Returns true if the timer was successfully deactivated, false otherwise.
  */
bool DeactivateTxTimer( TX_TIMER& timer, bool suppressVerbose=true) noexcept;

//=============//
// SEMAPHORES
//=============//

/**
 * @brief Function to Create TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @param name Name of the semaphore.
 * @param initial_count Initial count of the semaphore.
 * @return true if the semaphore was successfully created, false otherwise.
 */
bool CreateTxSemaphore(TX_SEMAPHORE* txSemaphore, const char* name, UINT initial_count, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to Delete TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block to be deleted.
 * @return true if the semaphore was successfully deleted, false otherwise.
 */
bool DeleteTxSemaphore(TX_SEMAPHORE *txSemaphore, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to put (release) a TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @return true if the semaphore was successfully released, false otherwise.
 */
bool PutTxSemaphore(TX_SEMAPHORE* txSemaphore, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to get (wait for) a TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @param wait_option Specifies the maximum time to wait for the semaphore.
 * @return true if the semaphore was successfully acquired, false otherwise.
 */
bool GetTxSemaphore(TX_SEMAPHORE* txSemaphore, ULONG wait_option, bool suppressVerbose=true) noexcept;

/**
 * @brief Function to get TX semaphore count.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @return the count of the semaphore.
 */
ULONG GetTxSemaphoreCount(TX_SEMAPHORE *txSemaphore, bool suppressVerbose=true) noexcept;

//=============//
// EVENTS
//=============//

/**
 * @brief Creates an event flag group with the specified parameters.
 *
 * @param eventFlags - The ThreadX event flags to create
 * @param name - The name for the event flag group
 */
bool CreateTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, const char* name, bool suppressVerbose=true) noexcept;

/**
 * @brief Deletes the specified event flag group and logs any error.
 *
 * @param eventFlags - The ThreadX event flags to delete
 **/
bool DeleteTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, bool suppressVerbose=true) noexcept;

/**
 * @brief Set event flags in the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to set
 * @param flagsToSet - The flags to set
 */
bool SetTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToSet, bool suppressVerbose=true) noexcept;

/**
 * @brief Clears event flags in the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to set
 * @param flagsToClear - The flags to clear
 */
bool ClearTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToClear, bool suppressVerbose=true) noexcept;

/**
 * @brief Get event flags from the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to get
 * @param flagsToGet - The flags to get
 * @param getOption - The option on how to get the flags
 * @param actualFlags - The actual flags gotten
 * @param wait_option - The option on how to wait if the flags are not available
 */
bool GetTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToGet, UINT getOption, ULONG& actualFlags, ULONG wait_option, bool suppressVerbose=true) noexcept;

//=============//
//=============//

#ifdef __cplusplus
}
#endif

#endif // TXUTILITY_H_
