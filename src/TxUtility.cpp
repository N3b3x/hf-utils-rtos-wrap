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

#include <cmath>
#include "UTILITIES/common/RtosCompat.h"

#include <component_handlers/ConsolePort.h>

#include "UTILITIES/common/TxUtility.h"
#include <UTILITIES/common/ThingsToString.h>

//==============================================================//
// FUNCTIONS
//==============================================================//

void TxDelayMsec( uint16_t msec )
{
	uint32_t ticks = ConvertMsecToDelayTicks( msec );

	uint32_t result = tx_thread_sleep (ticks);
	if( result != TX_SUCCESS )
	{
		ConsolePort::GetInstance().Write ( "TxDelayMsec() - Failed to sleep for %u msec, reason: %s.", msec, ThreadxRetToString(result) );
	}
}

void TxDelayTime(uint32_t timeBetweenSamples, time_unit_t timeUnit)
{
    if(timeBetweenSamples == 0){
        return;
    }

    switch(timeUnit)
    {
		case(TIME_UNIT_US):
		{
			TxDelayMsec( static_cast<uint16_t>(timeBetweenSamples/1000000U ));
			break;
		}
        case(TIME_UNIT_MS):
        {
            TxDelayMsec((uint16_t)timeBetweenSamples);
            break;
        }
        case(TIME_UNIT_S):
        {
            TxDelayMsec(static_cast<uint16_t>(timeBetweenSamples*1000));
            break;
        }
        default: break;
    }
}

uint32_t GetElapsedTimeMsec() {
	float elapsedTime = static_cast<float>(tx_time_get()) * 1000.0F / threadxTickRateHz;
    return static_cast<uint32_t>(std::ceil(elapsedTime));
}

uint32_t GetElapsedProcessorCycleCount(uint32_t startCycleCount, time_unit_t unit )
{
    uint32_t elapsed_cycles;

    static const float time_unit_us_divider = (1000000.0);
    static const float time_unit_ms_divider = (1000.0);
	static const float time_unit_s_divider = (1.0);
	float divider;

    switch(unit){
    	case(TIME_UNIT_US):
		{
    		divider = time_unit_us_divider;
    		break;
		}
    	case(TIME_UNIT_MS):
		{
    		divider = time_unit_ms_divider;
			break;
		}
    	case(TIME_UNIT_S):
		{
    		divider = time_unit_s_divider;
			break;
		}
    	default:
		{
    		divider = time_unit_ms_divider;
			break;
		}
    }

    if(startCycleCount > DWT->CYCCNT)
    {
        elapsed_cycles = ~(startCycleCount - DWT->CYCCNT);
    }
    else
    {
        elapsed_cycles = DWT->CYCCNT - startCycleCount;
    }

    return (uint32_t)(elapsed_cycles / (UTIL_SYSTEM_CLOCK / divider));
}

uint32_t GetProcessorCycleCount(){
	return DWT->CYCCNT;
}

//constexpr uint32_t ConvertMsecToDelayTicks( uint32_t milliseconds )
//{
//
//	//Calculate the delay in terms of the threadx tick rate */
//	return milliseconds * threadxTickRateHz / 1000U;
//}
//
//constexpr uint32_t ConvertDelayTicksToMsec(uint32_t delayTicks)
//{
//    // Calculate the delay in terms of milliseconds
//    return (delayTicks * 1000U) / threadxTickRateHz;
//}
//
//constexpr uint32_t ConvertHzToDelayTicks( uint32_t frequency )
//{
//	// Calculate the delay in terms of the threadx tick rate
//	return threadxTickRateHz/frequency;
//}

//============================================================================================//
// MUTEX
//============================================================================================//

/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The ThreadX mutex to create
 * @param name - The name fo rthe mutex
 * @param inherit - The inherit property for the mutex
 */
bool CreateTxMutex(TX_MUTEX& mutex, const char* mutexName, UINT inherit, bool suppressVerbose) noexcept
{
	if( mutexName != nullptr )
	{
		UINT status = tx_mutex_create(&mutex, const_cast<char*>(mutexName), inherit);
		if (status == TX_SUCCESS)
		{
			WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Successful for %s, reason: %s(%d).",
					mutexName, ThreadxRetToString(status), status);
			return true;
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Failed for %s, reason: %s(%d).",
					mutexName, ThreadxRetToString(status), status);
		}
	}
	else
	{
		WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Null pointer passed for mutex name.");
	}

	// TODO: Error Handling
	return false;
}

bool CreateTxMutexP(TX_MUTEX* mutex, const char* mutexName, UINT inherit, bool suppressVerbose) noexcept
{
	if( mutexName != nullptr )
	{
		UINT status = tx_mutex_create(mutex, const_cast<char*>(mutexName), inherit);
		if (status == TX_SUCCESS)
		{
			WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Successful for %s, reason: %s(%d).",
					mutexName, ThreadxRetToString(status), status);
			return true;
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Failed for %s, reason: %s(%d).",
					mutexName, ThreadxRetToString(status), status);
		}
	}
	else
	{
		WRITE_CONDITIONAL( !suppressVerbose, "CreateMutex() - Null pointer passed for mutex name.");
	}

	// TODO: Error Handling
	return false;
}

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool GetTxMutex(TX_MUTEX& mutex, ULONG wait_option, bool suppressVerbose) noexcept
{
    UINT err = tx_mutex_get(&mutex, wait_option);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxMutex() - Successfully acquired mutex - %s.", mutex.tx_mutex_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxMutex() - Failed to acquire mutex - %s - Error: %s(%d).", mutex.tx_mutex_name, ThreadxRetToString(err), err);
        // TBD: Log appropriate error here
        return false;
    }
}

/**
 * @brief Acquire (wait for) a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param wait_option Specifies the maximum time to wait for the mutex.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully acquired, false otherwise.
 */
bool GetTxMutexP(TX_MUTEX* mutex, ULONG wait_option, bool suppressVerbose) noexcept
{
    UINT err = tx_mutex_get(mutex, wait_option);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxMutex() - Successfully acquired mutex - %s.", mutex->tx_mutex_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxMutex() - Failed to acquire mutex - %s - Error: %s(%d).", mutex->tx_mutex_name, ThreadxRetToString(err), err);
        // TBD: Log appropriate error here
        return false;
    }
}

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool PutTxMutex(TX_MUTEX& mutex, bool suppressVerbose) noexcept
{
    UINT err = tx_mutex_put(&mutex);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxMutex() - Successfully released mutex - %s.", mutex.tx_mutex_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxMutex() - Failed to release mutex - %s - Error: %s(%d).", mutex.tx_mutex_name, ThreadxRetToString(err), err);
        // TBD: Log appropriate error here
        return false;
    }
}

/**
 * @brief Release a TX mutex.
 * @param mutex_ptr Pointer to the mutex control block.
 * @param suppressVerbose Flag to suppress verbose output.
 * @return true if the mutex was successfully released, false otherwise.
 */
bool PutTxMutexP(TX_MUTEX* mutex, bool suppressVerbose) noexcept
{
    UINT err = tx_mutex_put(mutex);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxMutex() - Successfully released mutex - %s.", mutex->tx_mutex_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxMutex() - Failed to release mutex - %s - Error: %s(%d).", mutex->tx_mutex_name, ThreadxRetToString(err), err);
        // TBD: Log appropriate error here
        return false;
    }
}

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The ThreadX mutex to delete
 **/
bool DeleteTxMutex(TX_MUTEX& mutex, bool suppressVerbose) noexcept
{
	auto status = tx_mutex_delete( &mutex );
	if( status != TX_SUCCESS)
	{
		if( mutex.tx_mutex_name )
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxMutex() - Failed to delete mutex: %s, reason: %s(%d).",  mutex.tx_mutex_name, ThreadxRetToString(status), status);
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxMutex() - Failed to delete mutex, reason: %s(%d)", ThreadxRetToString(status), status);
		}
		return false;
	}

	return true;
}

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The ThreadX mutex to delete
 **/
bool DeleteTxMutexP(TX_MUTEX* mutex, bool suppressVerbose) noexcept
{
	auto status = tx_mutex_delete( mutex );
	if( status != TX_SUCCESS)
	{
		if( mutex->tx_mutex_name )
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxMutex() - Failed to delete mutex: %s, reason: %s(%d).",  mutex->tx_mutex_name, ThreadxRetToString(status), status);
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxMutex() - Failed to delete mutex, reason: %s(%d)", ThreadxRetToString(status), status);
		}
		return false;
	}

	return true;
}

//============================================================================================//
// THREAD
//============================================================================================//

//================//
// THREAD COUNTS
//================//

extern uint32_t g_ssp_common_thread_count;      /**< External variable representing the common thread count in SSP. */

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
						ULONG timeSliceAllowed, UINT auto_start, bool suppressVerbose ) noexcept
{
    /// Create the ThreadX Thread
    UINT err = tx_thread_create(txThread, const_cast<char*>(name), entry_function, entry_input, stack, stackSizeBytes, priority, preempt_threshold, timeSliceAllowed, auto_start);

    if (err == TX_SUCCESS)
    {
        /// Add to the overall system thread count
        ++g_ssp_common_thread_count;

        WRITE_CONDITIONAL(!suppressVerbose, "CreateTxThread() - Successfully created %s.", name);
        return true;
    }
    else
    {
    //    WRITE_CONDITIONAL(!suppressVerbose, "CreateTxThread() - Failed to create %s. - Failure: %s(%d).", name, ThreadxRetToString(err), err);
    	 ConsolePort::Write( "CreateTxThread() - Failed to create %s. - Failure: %s(%d).", name, ThreadxRetToString(err), err);
    	 TxDelayMsec( 5 );
        /// TODO: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to Delete TX thread.
 * @param txThread Pointer to the ThreadX thread control block to be deleted.
 * @return true if the thread was successfully deleted, false otherwise.
 */
bool DeleteTxThread(TX_THREAD *txThread, bool suppressVerbose) noexcept
{

	UINT err = tx_thread_terminate(txThread);
	if (err == TX_SUCCESS)
	{
		err = tx_thread_delete(txThread);

		if (err == TX_SUCCESS)
		{
			WRITE_CONDITIONAL(!suppressVerbose, "DeleteTxThread() - Successfully deleted %s.", txThread->tx_thread_name);
			--g_ssp_common_thread_count; /// Decrement the overall system thread count
			return true;
		}
	}
	WRITE_CONDITIONAL(!suppressVerbose, "DeleteTxThread() - Failed to delete %s. - Failure: %s(%d).",
        								txThread->tx_thread_name, ThreadxRetToString(err), err);
        /// TODO: log appropriate error here
    return false;
}

bool ResumeTxThread( TX_THREAD* thread, bool suppressVerbose )
{
	bool success = false;
	if( thread != nullptr )
	{
		if( thread->tx_thread_id != 0 )
		{

			WRITE_CONDITIONAL( !suppressVerbose, "ResumeTxThread() - Resuming %s.",  thread->tx_thread_name);
			uint32_t result = tx_thread_resume(thread);

			TxDelayMsec( 10 ); // Give the thread a chance to start


			if ((result != TX_SUCCESS) && (result != TX_RESUME_ERROR ) && (result != TX_SUSPEND_LIFTED  ))
			{
				WRITE_CONDITIONAL( !suppressVerbose,  "ResumeTxThread() - Failed to resume %s, reason: %s.",
				   thread->tx_thread_name,  ThreadxRetToString( result));
			}
			else
			{
				success = true;
			}
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose,  "ResumeTxThread() - Attempted to resume a thread that wasn't created.");
		}
	}
	else
	{
		WRITE_CONDITIONAL( !suppressVerbose,  "ResumeTxThread() - Null pointer passed.");
	}
	return success;
}

bool ResumeTxThreadIfSuspended(TX_THREAD *thread, bool suppressVerbose) noexcept
{
    UINT state = TX_READY;

    // Get the current state of the thread
    if( TX_SUCCESS == tx_thread_info_get(thread, nullptr, &state, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) )
    {
		// Check if the thread is suspended
		if (state == TX_SUSPENDED)
		{
			// If the thread is suspended, attempt to resume it
			UINT err = tx_thread_resume(thread);
			if (err == TX_SUCCESS)
			{
				TxDelayMsec( 1 );   // Give chance for thread to execute
				WRITE_CONDITIONAL(!suppressVerbose, "ResumeTxThreadIfSuspended() - Successfully resumed thread - %s.",
												thread->tx_thread_name);
				return true;
			}
			else
			{
				WRITE_CONDITIONAL(!suppressVerbose, "ResumeTxThreadIfSuspended() - Failed to resume thread - %s - Failure: %s(%d).",
												thread->tx_thread_name, ThreadxRetToString(err), err);
				// TBD: log appropriate error here
				return false;
			}
		}
		else
		{
			// If the thread is not suspended, there's nothing to do
			WRITE_CONDITIONAL(!suppressVerbose, "Thread is not suspended, no action taken.");
			return true;
		}
    }
	else  // Unable to get state of thread
	{
		WRITE_CONDITIONAL(!suppressVerbose, "Unable to determien state of thread, reason:Thread is not suspended, no action taken.");
		return false;
	}
}


bool SuspendTxThread( TX_THREAD* thread, bool suppressVerbose )
{
    if( thread != nullptr )
    {
        if( thread->tx_thread_id != 0 )
        {

            WRITE_CONDITIONAL( !suppressVerbose, "SuspendTxThread() - Suspending %s.",  thread->tx_thread_name);
            uint32_t result = tx_thread_suspend(thread);

            TxDelayMsec( 10 ); // Give the thread a chance to start

            if ( result == TX_SUCCESS)
            {
                return true;
            }
            else
            {
                WRITE_CONDITIONAL( !suppressVerbose,  "SuspendTxThread() - Failed to suspend %s, reason: %s.",
                                   thread->tx_thread_name,  ThreadxRetToString( result));
            }
        }
        else
        {
            WRITE_CONDITIONAL( !suppressVerbose,  "SuspendTxThread() - Attempted to suspend a thread that wasn't created.");
        }
    }
    else
    {
        WRITE_CONDITIONAL( !suppressVerbose,  "SuspendTxThread() - Null pointer passed.");
    }
    return false;
}

//============================================================================================//
// QUEUE
//============================================================================================//

/**
 * @brief Creates a queue with the specified parameters.
 *
 * @param queue - The ThreadX queue to create
 * @param queueName - The name for the queue
 * @param messageSizeInWords - The size of the messages that will be stored in the queue
 */
bool CreateTxQueue(TX_QUEUE& queue, const char* queueName, UINT messageSizeInWords, void* queueStorage, ULONG queueSize, bool suppressVerbose) noexcept
{
    if(queueName != nullptr)
    {
        UINT status = tx_queue_create(&queue, const_cast<char*>(queueName), messageSizeInWords, queueStorage, queueSize);
        if (status == TX_SUCCESS)
        {
            WRITE_CONDITIONAL( !suppressVerbose, "CreateTxQueue() - Queue %s successfully created.", queueName);
            return true;
        }
        else
        {
            WRITE_CONDITIONAL( !suppressVerbose, "CreateTxQueue() - Failed for %s, reason: %s(%d).",
                    queueName, ThreadxRetToString(status), status);
        }
    }
    else
    {
        WRITE_CONDITIONAL( !suppressVerbose, "CreateTxQueue() - Null pointer passed for queue name.");
    }

    // TODO: Error Handling
    return false;
}

/**
 * @brief Deletes the specified queue and logs any error.
 *
 * @param queue - The ThreadX queue to delete
 **/
bool DeleteTxQueue(TX_QUEUE& queue, bool suppressVerbose) noexcept
{
    auto status = tx_queue_delete(&queue);
    if(status != TX_SUCCESS)
    {
        if(queue.tx_queue_name)
        {
            WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxQueue() - Failed to delete queue: %s, reason: %s(%d).",  queue.tx_queue_name, ThreadxRetToString(status), status);
        }
        else
        {
            WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxQueue() - Failed to delete queue, reason: %s(%d)", ThreadxRetToString(status), status);
        }
        return false;
    }

    return true;
}

/**
 * @brief Send a message to the queue.
 *
 * @param queue - The ThreadX queue to send the message to
 * @param message - The message to send
 * @param wait_option - The option on how to wait if the queue is full
 */
bool SendToTxQueue(TX_QUEUE& queue, void* message, ULONG wait_option, bool suppressVerbose) noexcept {
    UINT status = tx_queue_send(&queue, message, wait_option);
    if (status == TX_SUCCESS) {
        WRITE_CONDITIONAL( !suppressVerbose, "SendToTxQueue() - Successfully sent message to queue: %s.", queue.tx_queue_name);
        return true;
    } else {
        WRITE_CONDITIONAL( !suppressVerbose, "SendToTxQueue() - Failed to send message to queue: %s, error code: %u .", queue.tx_queue_name, status);
        return false;
    }
}

/**
 * @brief Receive a message from the queue.
 *
 * @param queue - The ThreadX queue to receive the message from
 * @param message - The received message
 * @param wait_option - The option on how to wait if the queue is empty
 */
bool ReceiveFromTxQueue(TX_QUEUE& queue, void* message, ULONG wait_option, bool suppressVerbose) noexcept {
    UINT status = tx_queue_receive(&queue, message, wait_option);
    if (status == TX_SUCCESS) {
        WRITE_CONDITIONAL( !suppressVerbose, "ReceiveFromTxQueue() - Successfully received message from queue: %s.", queue.tx_queue_name);
        return true;
    } else {
        WRITE_CONDITIONAL( !suppressVerbose, "ReceiveFromTxQueue() - Failed to receive message from queue: %s, error code: %u .", queue.tx_queue_name, status);
        return false;
    }
}

//============================================================================================//
// TIMER
//============================================================================================//
#include <EventDrivenDataMultiThread.h>

/**
  * @brief Function to create a ThreadX timer and report errors associated with the creation
  *
  * @param timer Reference to timer control block.
  * @param name Pointer to timer name.
  * @param callback timer expiration callback function.
  * @param callbackExpirationInput A uint32_t data point that can be passed to expiration callback function.
  * @param initialTimeoutTicks Initial Expiration ticks.
  * @param rescheduleTimeoutTicks Reschedule ticks.
  * @param autoActivate The data value to set.
 */
bool CreateTxTimer ( TX_TIMER& timer, const char* name, void (*callback)(uint32_t), uint32_t callbackExpirationInput,
		  uint32_t initialTimeoutTicks, uint32_t rescheduleTimeoutTicks, UINT autoActivate, bool suppressVerbose ) noexcept
{
	if( name != nullptr )
	{
		auto status = tx_timer_create(&timer, const_cast<char*>(name), callback, callbackExpirationInput, initialTimeoutTicks,
				rescheduleTimeoutTicks, autoActivate);
		if (status == TX_SUCCESS)
		{
			WRITE_CONDITIONAL( !suppressVerbose,  "CreateTxTimer() - Successfully created timer: %s (%s), address: %p, Expiration Input: %u.",
				name, timer.tx_timer_name, &timer, callbackExpirationInput);
			return true;
		}
		else
		{
			ConsolePort::Write(  "CreateTxTimer() - Failed to create timer: %s, address: %p, reason: %s(%d)", name,
					&timer,ThreadxRetToString(status), status);
			TxDelayMsec( 5 );
		}
	}
	else
	{
		ConsolePort::Write( "CreateTxTimer() - Null pointer passed for timer name.");
	}

	return false;
}



/**
  * @brief Function to delete a ThreadX timer and report errors associated with the deletion
  *
  * @param timer The timer to delete.
  **/
bool DeactivateAndDeleteTxTimer( TX_TIMER& timer, bool suppressVerbose) noexcept
{
	// Stop the timer first
	auto status = tx_timer_deactivate(&timer);
	if (status == TX_SUCCESS)
	{
		status = tx_timer_delete(&timer);
		if (status != TX_SUCCESS)
		{
			if( timer.tx_timer_name )
			{
				WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxTimer() - Failed to delete timer: %s, reason: %s(%d).", timer.tx_timer_name, ThreadxRetToString(status), status);
			}
			else
			{
				WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxTimer() - Failed to delete timer, reason: %s(%d)", ThreadxRetToString(status), status);
			}
		}
	}
	else  // Failed to stop timer, usually due to back timer
	{
		// Time might have already been fired. No need to do any logging here
		WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxTimer() - Failed to stop the timer prior to deleting, reason : %s(%d).",  ThreadxRetToString(status), status);
	}
	return status == TX_SUCCESS;
}

bool ActivateTxTimer( TX_TIMER& timer, bool suppressVerbose) noexcept
{
   auto status = tx_timer_activate(&timer);
   if (status != TX_SUCCESS)
   {
		if( timer.tx_timer_name )
		{
			WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Failed to activate timer: %s, reason: %s(%d).",
					timer.tx_timer_name, ThreadxRetToString(status), status);
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Failed to activate un-named timer, reason: %s(%d).",
					ThreadxRetToString(status), status);
		}
   }
   else if( !suppressVerbose )
   {
	   if( timer.tx_timer_name )
	   {
		   WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Activated timer: %s.",
				timer.tx_timer_name);
	   }
	   else
	   {
		   WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Activated un-named timer.");
	   }
   }
   return (status == TX_SUCCESS);
}

bool DeactivateTxTimer( TX_TIMER& timer, bool suppressVerbose) noexcept
{
	auto status = tx_timer_deactivate(&timer);
	if (status != TX_SUCCESS)
	{
		if( timer.tx_timer_name )
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeactivateTxTimer() - Failed to deactivate timer: %s, reason: %s(%d).", timer.tx_timer_name,
					ThreadxRetToString(status), status);
		}
		else
		{
			WRITE_CONDITIONAL( !suppressVerbose, "DeactivateTxTimer() - Failed to deactivate timer, reason: %s(%d).",
					ThreadxRetToString(status), status);
		}
	}
	else if( !suppressVerbose )
	{
		if( timer.tx_timer_name )
		{
		   WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Deactivated timer: %s.",
				timer.tx_timer_name);
		}
		else
		{
		   WRITE_CONDITIONAL( !suppressVerbose, "ActivateTxTimer() - Deactivated un-named timer.");
		}
	}

	return (status == TX_SUCCESS);
}

//============================================================================================//
// SEMAPHORES
//============================================================================================//

/**
 * @brief Function to Create TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @param name Name of the semaphore.
 * @param initial_count Initial count of the semaphore.
 * @return true if the semaphore was successfully created, false otherwise.
 */
bool CreateTxSemaphore(TX_SEMAPHORE* txSemaphore, const char* name, UINT initial_count, bool suppressVerbose) noexcept
{
    UINT err = tx_semaphore_create(txSemaphore, const_cast<char*>(name), initial_count);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "CreateTxSemaphore() - Successfully created semaphore %s.", name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "CreateTxSemaphore() - Failed to create semaphore - %s - Failure: %s(%d).", name, ThreadxRetToString(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to Delete TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block to be deleted.
 * @return true if the semaphore was successfully deleted, false otherwise.
 */
bool DeleteTxSemaphore(TX_SEMAPHORE *txSemaphore, bool suppressVerbose) noexcept
{
    UINT err = tx_semaphore_delete(txSemaphore);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "DeleteTxSemaphore() - Successfully deleted semaphore - %s.",
        								txSemaphore->tx_semaphore_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "DeleteTxSemaphore() - Failed to delete semaphore - %s - Failure: %s(%d).",
        								txSemaphore->tx_semaphore_name , ThreadxRetToString(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to put (release) a TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @return true if the semaphore was successfully released, false otherwise.
 */
bool PutTxSemaphore(TX_SEMAPHORE* txSemaphore, bool suppressVerbose) noexcept
{
    UINT err = tx_semaphore_put(txSemaphore);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxSemaphore() - Successfully put semaphore - %s.", txSemaphore->tx_semaphore_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "PutTxSemaphore() - Failed to put semaphore - %s - Failure: %s(%d).", txSemaphore->tx_semaphore_name, ThreadxRetToString(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to get (wait for) a TX semaphore.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @param wait_option Specifies the maximum time to wait for the semaphore.
 * @return true if the semaphore was successfully acquired, false otherwise.
 */
bool GetTxSemaphore(TX_SEMAPHORE* txSemaphore, ULONG wait_option, bool suppressVerbose) noexcept
{
    UINT err = tx_semaphore_get(txSemaphore, wait_option);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxSemaphore() - Successfully got semaphore - %s.", txSemaphore->tx_semaphore_name);
        return true;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxSemaphore() - Failed to get semaphore - %s - Failure: %s(%d).", txSemaphore->tx_semaphore_name, ThreadxRetToString(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to get TX semaphore count.
 * @param txSemaphore Pointer to the ThreadX semaphore control block.
 * @return the count of the semaphore.
 */
ULONG GetTxSemaphoreCount(TX_SEMAPHORE *txSemaphore, bool suppressVerbose) noexcept
{
    ULONG count;
    UINT err = tx_semaphore_info_get(txSemaphore, nullptr, &count, nullptr, nullptr, nullptr);

    if (err == TX_SUCCESS)
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxSemaphoreCount() - Successfully got semaphore count - &s - %lu.",
										txSemaphore->tx_semaphore_name , count);
        return count;
    }
    else
    {
        WRITE_CONDITIONAL(!suppressVerbose, "GetTxSemaphoreCount() - Failed to get semaphore count - &s - Failure: %s(%d).",
										txSemaphore->tx_semaphore_name , ThreadxRetToString(err), err);
        // TBD: log appropriate error here
        return 0;
    }
}


//============================================================================================//
// EVENTS
//============================================================================================//

/**
 * @brief Creates an event flag group with the specified parameters.
 *
 * @param eventFlags - The ThreadX event flags to create
 * @param name - The name for the event flag group
 */
bool CreateTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, const char* name, bool suppressVerbose) noexcept
{
    if(name != nullptr)
    {
        UINT status = tx_event_flags_create(&eventFlags, const_cast<char*>(name));
        if (status == TX_SUCCESS)
        {
            return true;
        }
        else
        {
       //     WRITE_CONDITIONAL( !suppressVerbose, "CreateTxEventFlags() - Failed for %s, reason: %s(%d).",
       //             name, ThreadxRetToString(status), status);

        	ConsolePort::Write( "CreateTxEventFlags() - Failed for %s, reason: %s(%d).",
        	  name, ThreadxRetToString(status), status);
        }
    }
    else
    {
        WRITE_CONDITIONAL( !suppressVerbose, "CreateTxEventFlags() - Null pointer passed for event flag group name.");
    }

    // TODO: Error Handling
    return false;
}

/**
 * @brief Deletes the specified event flag group and logs any error.
 *
 * @param eventFlags - The ThreadX event flags to delete
 **/
bool DeleteTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, bool suppressVerbose) noexcept
{
    auto status = tx_event_flags_delete(&eventFlags);
    if(status != TX_SUCCESS)
    {
        if(eventFlags.tx_event_flags_group_name)
        {
            WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxEventFlags() - Failed to delete event flag group: %s, reason: %s(%d).",  eventFlags.tx_event_flags_group_name, ThreadxRetToString(status), status);
        }
        else
        {
            WRITE_CONDITIONAL( !suppressVerbose, "DeleteTxEventFlags() - Failed to delete event flag group, reason: %s(%d)", ThreadxRetToString(status), status);
        }
        return false;
    }

    return true;
}

/**
 * @brief Set event flags in the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to set
 * @param flagsToSet - The flags to set
 */
bool SetTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToSet, bool suppressVerbose) noexcept {
    UINT status = tx_event_flags_set(&eventFlags, flagsToSet, TX_OR);
    if (status == TX_SUCCESS) {
        WRITE_CONDITIONAL( !suppressVerbose, "SetTxEventFlags() - Successfully set event flags in group: %s.", eventFlags.tx_event_flags_group_name);
        return true;
    } else {
        WRITE_CONDITIONAL( !suppressVerbose, "SetTxEventFlags() - Failed to set event flags in group: %s, error code: %u .", eventFlags.tx_event_flags_group_name, status);
        return false;
    }
}

/**
 * @brief Clears event flags in the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to set
 * @param flagsToClear - The flags to clear
 */
bool ClearTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToClear, bool suppressVerbose) noexcept {
    UINT status = tx_event_flags_set(&eventFlags, flagsToClear, TX_AND);
    if (status == TX_SUCCESS) {
        WRITE_CONDITIONAL( !suppressVerbose, "SetTxEventFlags() - Successfully cleared event flags in group: %s.", eventFlags.tx_event_flags_group_name);
        return true;
    } else {
        WRITE_CONDITIONAL( !suppressVerbose, "SetTxEventFlags() - Failed to clear event flags in group: %s, error code: %u .", eventFlags.tx_event_flags_group_name, status);
        return false;
    }
}

/**
 * @brief Get event flags from the event flag group.
 *
 * @param eventFlags - The ThreadX event flags to get
 * @param flagsToGet - The flags to get
 * @param getOption - The option on how to get the flags
 * @param actualFlags - The actual flags gotten
 * @param wait_option - The option on how to wait if the flags are not available
 */
bool GetTxEventFlags(TX_EVENT_FLAGS_GROUP& eventFlags, ULONG flagsToGet, UINT getOption, ULONG& actualFlags, ULONG wait_option, bool suppressVerbose) noexcept {
    UINT status = tx_event_flags_get(&eventFlags, flagsToGet, getOption, &actualFlags, wait_option);
    if (status == TX_SUCCESS) {
        WRITE_CONDITIONAL( !suppressVerbose, "GetTxEventFlags() - Successfully got event flags from group: %s.", eventFlags.tx_event_flags_group_name);
        return true;
    } else {
        WRITE_CONDITIONAL( !suppressVerbose, "GetTxEventFlags() - Failed to get event flags from group: %s, error code: %u .", eventFlags.tx_event_flags_group_name, status);
        return false;
    }
}

//============================================================================================//
// STACK FAULT HANDLER
//============================================================================================//

void StackFaultHandler(TX_THREAD* thread )
{
	ConsolePort::Write( "==||***********************************************||==");
	ConsolePort::Write( "==||************  THREAD STACK FAULT   ************||==");
	ConsolePort::Write( "==||***********************************************||==");
	ConsolePort::Write( "==||StackFaultHandler() - Thread: %s.", thread->tx_thread_name);
	ConsolePort::Write( "==||***********************************************||==");
}

