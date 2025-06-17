/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *
  *  Contains the declaration and definition of the utility functions related to
  *     the FreeRTOS RTOS.
  *
  *   Note:  These functions are not thread or interrupt-safe and should be called
  *          called with appropriate guards if used within an ISR or shared between tasks.
  */
#include <cmath>
#include "OsAbstraction.h"
#include "ConsolePort.h"
#include "FreeRTOSUtils.h"
#include "OsUtility.h"

//==============================================================//
// GLOBALS
//==============================================================//

// Critical section mutex required by OsAbstraction.h
OS_Critical os_critical_mux = portMUX_INITIALIZER_UNLOCKED;

//==============================================================//
// FUNCTIONS
//==============================================================//

void os_delay_msec( uint16_t msec )
{
	uint32_t ticks = os_convert_msec_to_delay_ticks( msec );

	uint32_t result = os_thread_sleep (ticks);
	if( result != OS_SUCCESS )
	{
		console_error("OsUtility", "os_delay_msec() - Failed to sleep for %u msec, reason: %s.", msec, freertos_ret_to_string(result));
	}
}

void os_delay_time(uint32_t timeBetweenSamples, time_unit_t timeUnit)
{
    if(timeBetweenSamples == 0){
        return;
    }

    switch(timeUnit)
    {
		case(TIME_UNIT_US):
		{
			os_delay_msec( static_cast<uint16_t>(timeBetweenSamples/1000000U ));
			break;
		}
        case(TIME_UNIT_MS):
        {
            os_delay_msec((uint16_t)timeBetweenSamples);
            break;
        }
        case(TIME_UNIT_S):
        {
            os_delay_msec(static_cast<uint16_t>(timeBetweenSamples*1000));
            break;
        }
        default: break;
    }
}

uint32_t os_get_elapsed_time_msec() {
	float elapsedTime = static_cast<float>(os_time_get()) * 1000.0F / osTickRateHz;
    return static_cast<uint32_t>(std::ceil(elapsedTime));
}

uint32_t os_get_elapsed_processor_cycle_count(uint32_t startCycleCount, time_unit_t unit )
{
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

    // ESP32 doesn't have DWT registers, use FreeRTOS tick count instead
    // Note: This provides lower resolution than ARM DWT but is portable
    uint32_t current_ticks = xTaskGetTickCount();
    uint32_t elapsed_ticks;
    
    if(startCycleCount > current_ticks)
    {
        elapsed_ticks = ~(startCycleCount - current_ticks);
    }
    else
    {
        elapsed_ticks = current_ticks - startCycleCount;
    }

    // Convert ticks to the requested time unit
    // Note: This is an approximation since we're using ticks instead of actual cycles
    return (uint32_t)(elapsed_ticks * (1000.0 / configTICK_RATE_HZ) / (1000.0 / divider));
}

uint32_t os_get_processor_cycle_count(){
    // Return FreeRTOS tick count instead of actual processor cycles
    // This provides consistent timing across different platforms
    return xTaskGetTickCount();
}

//constexpr uint32_t os_convert_msec_to_delay_ticks( uint32_t milliseconds )
//{
//
//	//Calculate the delay in terms of the threadx tick rate */
//	return milliseconds * osTickRateHz / 1000U;
//}
//
//constexpr uint32_t os_convert_delay_ticks_to_msec(uint32_t delayTicks)
//{
//    // Calculate the delay in terms of milliseconds
//    return (delayTicks * 1000U) / osTickRateHz;
//}
//
//constexpr uint32_t os_convert_hz_to_delay_ticks( uint32_t frequency )
//{
//	// Calculate the delay in terms of the threadx tick rate
//	return osTickRateHz/frequency;
//}

//============================================================================================//
// MUTEX
//============================================================================================//

/**
 * @brief Creates a mutex with the specified parameters.
 *
 * @param mutex - The OS mutex to create
 * @param name - The name fo rthe mutex
 * @param inherit - The inherit property for the mutex
 */
bool os_mutex_create_ex(OS_Mutex& mutex, const char* mutexName, OS_Uint inherit, bool suppressVerbose) noexcept
{
	if( mutexName != nullptr )
	{
                OS_Uint status = os_mutex_create(&mutex, const_cast<char*>(mutexName), inherit);
		if (status == OS_SUCCESS)
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Successful for %s, reason: %s(%d).",
					mutexName, freertos_ret_to_string(status), status);
			return true;
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Failed for %s, reason: %s(%d).",
					mutexName, freertos_ret_to_string(status), status);
		}
	}
	else
	{
		ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Null pointer passed for mutex name.");
	}

	// TODO: Error Handling
	return false;
}

bool os_mutex_create_p(OS_Mutex* mutex, const char* mutexName, OS_Uint inherit, bool suppressVerbose) noexcept
{
	if( mutexName != nullptr )
	{
                OS_Uint status = os_mutex_create(mutex, const_cast<char*>(mutexName), inherit);
		if (status == OS_SUCCESS)
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Successful for %s, reason: %s(%d).",
					mutexName, freertos_ret_to_string(status), status);
			return true;
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Failed for %s, reason: %s(%d).",
					mutexName, freertos_ret_to_string(status), status);
		}
	}
	else
	{
		ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "CreateMutex() - Null pointer passed for mutex name.");
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
bool os_mutex_get_ex(OS_Mutex& mutex, OS_Ulong wait_option, bool suppressVerbose) noexcept
{
    OS_Uint err = os_mutex_get(&mutex, wait_option);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_get_ex() - Successfully acquired mutex - %s.", mutex);
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_get_ex() - Failed to acquire mutex - %s - Error: %s(%d).", mutex, freertos_ret_to_string(err), err);
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
bool os_mutex_get_p(OS_Mutex* mutex, OS_Ulong wait_option, bool suppressVerbose) noexcept
{
    OS_Uint err = os_mutex_get(mutex, wait_option);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_get_ex() - Successfully acquired mutex - %s.", mutex);
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_get_ex() - Failed to acquire mutex - %s - Error: %s(%d).", mutex, freertos_ret_to_string(err), err);
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
bool os_mutex_put_ex(OS_Mutex& mutex, bool suppressVerbose) noexcept
{
    OS_Uint err = os_mutex_put(&mutex);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_put_ex() - Successfully released mutex - %s.", mutex);
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_put_ex() - Failed to release mutex - %s - Error: %s(%d).", mutex, freertos_ret_to_string(err), err);
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
bool os_mutex_put_p(OS_Mutex* mutex, bool suppressVerbose) noexcept
{
    OS_Uint err = os_mutex_put(mutex);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_put_ex() - Successfully released mutex - %s.", mutex);
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_put_ex() - Failed to release mutex - %s - Error: %s(%d).", mutex, freertos_ret_to_string(err), err);
        // TBD: Log appropriate error here
        return false;
    }
}

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The OS mutex to delete
 **/
bool os_mutex_delete_ex(OS_Mutex& mutex, bool suppressVerbose) noexcept
{
        OS_Uint status = os_mutex_delete( &mutex );
	if( status != OS_SUCCESS)
	{
		if( mutex )
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_delete_ex() - Failed to delete mutex: %s, reason: %s(%d).",  mutex, freertos_ret_to_string(status), status);
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_delete_ex() - Failed to delete mutex, reason: %s(%d)", freertos_ret_to_string(status), status);
		}
		return false;
	}

	return true;
}

/**
 * @brief Deletes the specified mutex and logs any error.
 *
 * @param mutex - The OS mutex to delete
 **/
bool os_mutex_delete_p(OS_Mutex* mutex, bool suppressVerbose) noexcept
{
        OS_Uint status = os_mutex_delete( mutex );
	if( status != OS_SUCCESS)
	{
		if( mutex )
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_delete_ex() - Failed to delete mutex: %s, reason: %s(%d).",  mutex, freertos_ret_to_string(status), status);
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_mutex_delete_ex() - Failed to delete mutex, reason: %s(%d)", freertos_ret_to_string(status), status);
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
                    OS_Uint auto_start, bool suppressVerbose ) noexcept
{
    /// Create the OS Thread
    OS_Uint err = os_thread_create(txThread, const_cast<char*>(name), entry_function, entry_input, stack, stackSizeBytes, priority, preempt_threshold, timeSliceAllowed, auto_start);

    if (err == OS_SUCCESS)
    {
        /// Add to the overall system thread count
        ++g_ssp_common_thread_count;

        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_create_ex() - Successfully created %s.", name);
        return true;
    }
    else
    {
    //    ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_create_ex() - Failed to create %s. - Failure: %s(%d).", name, freertos_ret_to_string(err), err);
    	 ConsolePort::Write("OsUtility",  "os_thread_create_ex() - Failed to create %s. - Failure: %s(%d).", name, freertos_ret_to_string(err), err);
    	 os_delay_msec( 5 );
        /// TODO: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to Delete TX thread.
 * @param txThread Pointer to the OS thread control block to be deleted.
 * @return true if the thread was successfully deleted, false otherwise.
 */
bool os_thread_delete_ex(OS_Thread *txThread, bool suppressVerbose) noexcept
{

        OS_Uint err = os_thread_terminate(txThread);
	if (err == OS_SUCCESS)
	{
		err = os_thread_delete(txThread);

		if (err == OS_SUCCESS)
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_delete_ex() - Successfully deleted %s.", "thread");
			--g_ssp_common_thread_count; /// Decrement the overall system thread count
			return true;
		}
	}
	ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_delete_ex() - Failed to delete %s. - Failure: %s(%d).",
        								"thread", freertos_ret_to_string(err), err);
        /// TODO: log appropriate error here
    return false;
}

bool os_thread_resume_ex( OS_Thread* thread, bool suppressVerbose )
{
	bool success = false;
	if( thread != nullptr )
	{
		if( *thread != NULL )
		{

			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_resume_ex() - Resuming %s.",  "thread");
			uint32_t result = os_thread_resume(thread);

			os_delay_msec( 10 ); // Give the thread a chance to start


			if (result != OS_SUCCESS)
			{
				ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_resume_ex() - Failed to resume %s, reason: %s.",
				   "thread",  freertos_ret_to_string( result));
			}
			else
			{
				success = true;
			}
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_resume_ex() - Attempted to resume a thread that wasn't created.");
		}
	}
	else
	{
		ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_resume_ex() - Null pointer passed.");
	}
	return success;
}

bool os_thread_resume_if_suspended(OS_Thread *thread, bool suppressVerbose) noexcept
{
    OS_Uint state = eReady;

    // Get the current state of the thread
    if( OS_SUCCESS == os_thread_info_get(thread, &state) )
    {
		// Check if the thread is suspended
                if (state == eSuspended)
		{
			// If the thread is suspended, attempt to resume it
                        OS_Uint err = os_thread_resume(thread);
			if (err == OS_SUCCESS)
			{
				os_delay_msec( 1 );   // Give chance for thread to execute
				ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_resume_if_suspended() - Successfully resumed thread - %s.",
												"thread");
				return true;
			}
			else
			{
				ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_resume_if_suspended() - Failed to resume thread - %s - Failure: %s(%d).",
												"thread", freertos_ret_to_string(err), err);
				// TBD: log appropriate error here
				return false;
			}
		}
		else
		{
			// If the thread is not suspended, there's nothing to do
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "Thread is not suspended, no action taken.");
			return true;
		}
    }
	else  // Unable to get state of thread
	{
		ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "Unable to determien state of thread, reason:Thread is not suspended, no action taken.");
		return false;
	}
}


bool os_thread_suspend_ex( OS_Thread* thread, bool suppressVerbose )
{
    if( thread != nullptr )
    {
        if( *thread != NULL )
        {

            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_thread_suspend_ex() - Suspending %s.",  "thread");
            uint32_t result = os_thread_suspend(thread);

            os_delay_msec( 10 ); // Give the thread a chance to start

            if ( result == OS_SUCCESS)
            {
                return true;
            }
            else
            {
                ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_suspend_ex() - Failed to suspend %s, reason: %s.",
                                   "thread",  freertos_ret_to_string( result));
            }
        }
        else
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_suspend_ex() - Attempted to suspend a thread that wasn't created.");
        }
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_thread_suspend_ex() - Null pointer passed.");
    }
    return false;
}

//============================================================================================//
// QUEUE
//============================================================================================//

/**
 * @brief Creates a queue with the specified parameters.
 *
 * @param queue - The OS queue to create
 * @param queueName - The name for the queue
 * @param messageSizeInWords - The size of the messages that will be stored in the queue
 */
bool os_queue_create_ex(OS_Queue& queue, const char* queueName, OS_Uint messageSizeInWords,
                   void* queueStorage, OS_Ulong queueSize, bool suppressVerbose) noexcept
{
    if(queueName != nullptr)
    {
        OS_Uint status = os_queue_create(&queue, const_cast<char*>(queueName), messageSizeInWords, queueStorage, queueSize);
        if (status == OS_SUCCESS)
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_create_ex() - Queue %s successfully created.", queueName);
            return true;
        }
        else
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_create_ex() - Failed for %s, reason: %s(%d).",
                    queueName, freertos_ret_to_string(status), status);
        }
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_create_ex() - Null pointer passed for queue name.");
    }

    // TODO: Error Handling
    return false;
}

/**
 * @brief Deletes the specified queue and logs any error.
 *
 * @param queue - The OS queue to delete
 **/
bool os_queue_delete_ex(OS_Queue& queue, bool suppressVerbose) noexcept
{
    auto status = os_queue_delete(&queue);
    if(status != OS_SUCCESS)
    {
        if("queue")
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_delete_ex() - Failed to delete queue: %s, reason: %s(%d).",  "queue", freertos_ret_to_string(status), status);
        }
        else
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_delete_ex() - Failed to delete queue, reason: %s(%d)", freertos_ret_to_string(status), status);
        }
        return false;
    }

    return true;
}

/**
 * @brief Send a message to the queue.
 *
 * @param queue - The OS queue to send the message to
 * @param message - The message to send
 * @param wait_option - The option on how to wait if the queue is full
 */
bool os_queue_send_ex(OS_Queue& queue, void* message, OS_Ulong wait_option, bool suppressVerbose) noexcept {
    OS_Uint status = os_queue_send(&queue, message, wait_option);
    if (status == OS_SUCCESS) {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_send_ex() - Successfully sent message to queue: %s.", "queue");
        return true;
    } else {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_send_ex() - Failed to send message to queue: %s, error code: %u .", "queue", status);
        return false;
    }
}

/**
 * @brief Receive a message from the queue.
 *
 * @param queue - The OS queue to receive the message from
 * @param message - The received message
 * @param wait_option - The option on how to wait if the queue is empty
 */
bool os_queue_receive_ex(OS_Queue& queue, void* message, OS_Ulong wait_option, bool suppressVerbose) noexcept {
    OS_Uint status = os_queue_receive(&queue, message, wait_option);
    if (status == OS_SUCCESS) {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_receive_ex() - Successfully received message from queue: %s.", "queue");
        return true;
    } else {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_queue_receive_ex() - Failed to receive message from queue: %s, error code: %u .", "queue", status);
        return false;
    }
}

//============================================================================================//
// TIMER
//============================================================================================//

/**
  * @brief Function to create a OS timer and report errors associated with the creation
  *
  * @param timer Reference to timer control block.
  * @param name Pointer to timer name.
  * @param callback timer expiration callback function.
  * @param callbackExpirationInput A uint32_t data point that can be passed to expiration callback function.
  * @param initialTimeoutTicks Initial Expiration ticks.
  * @param rescheduleTimeoutTicks Reschedule ticks.
  * @param autoActivate The data value to set.
 */
bool os_timer_create_ex ( OS_Timer& timer, const char* name, void (*callback)(uint32_t),
                     uint32_t callbackExpirationInput, uint32_t initialTimeoutTicks,
                     uint32_t rescheduleTimeoutTicks, OS_Uint autoActivate,
                     bool suppressVerbose ) noexcept
{
	if( name != nullptr )
	{
		auto status = os_timer_create(&timer, const_cast<char*>(name), callback, callbackExpirationInput, initialTimeoutTicks,
				rescheduleTimeoutTicks, autoActivate);
		if (status == OS_SUCCESS)
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility",  "os_timer_create_ex() - Successfully created timer: %s (%s), address: %p, Expiration Input: %u.",
				name, "timer", &timer, callbackExpirationInput);
			return true;
		}
		else
		{
			ConsolePort::Write("OsUtility",   "os_timer_create_ex() - Failed to create timer: %s, address: %p, reason: %s(%d)", name,
					&timer,freertos_ret_to_string(status), status);
			os_delay_msec( 5 );
		}
	}
	else
	{
		ConsolePort::Write("OsUtility",  "os_timer_create_ex() - Null pointer passed for timer name.");
	}

	return false;
}



/**
  * @brief Function to delete a OS timer and report errors associated with the deletion
  *
  * @param timer The timer to delete.
  **/
bool os_timer_deactivate_and_delete_ex( OS_Timer& timer, bool suppressVerbose) noexcept
{
	// Stop the timer first
	auto status = os_timer_deactivate(&timer);
	if (status == OS_SUCCESS)
	{
		status = os_timer_delete(&timer);
		if (status != OS_SUCCESS)
		{
			if( "timer" )
			{
				ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "DeleteOsTimer() - Failed to delete timer: %s, reason: %s(%d).", "timer", freertos_ret_to_string(status), status);
			}
			else
			{
				ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "DeleteOsTimer() - Failed to delete timer, reason: %s(%d)", freertos_ret_to_string(status), status);
			}
		}
	}
	else  // Failed to stop timer, usually due to back timer
	{
		// Time might have already been fired. No need to do any logging here
		ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "DeleteOsTimer() - Failed to stop the timer prior to deleting, reason : %s(%d).",  freertos_ret_to_string(status), status);
	}
	return status == OS_SUCCESS;
}

bool os_timer_activate_ex( OS_Timer& timer, bool suppressVerbose) noexcept
{
   auto status = os_timer_activate(&timer);
   if (status != OS_SUCCESS)
   {
		if( "timer" )
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Failed to activate timer: %s, reason: %s(%d).",
					"timer", freertos_ret_to_string(status), status);
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Failed to activate un-named timer, reason: %s(%d).",
					freertos_ret_to_string(status), status);
		}
   }
   else if( !suppressVerbose )
   {
	   if( "timer" )
	   {
		   ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Activated timer: %s.",
				"timer");
	   }
	   else
	   {
		   ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Activated un-named timer.");
	   }
   }
   return (status == OS_SUCCESS);
}

bool os_timer_deactivate_ex( OS_Timer& timer, bool suppressVerbose) noexcept
{
	auto status = os_timer_deactivate(&timer);
	if (status != OS_SUCCESS)
	{
		if( "timer" )
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_deactivate_ex() - Failed to deactivate timer: %s, reason: %s(%d).", "timer",
					freertos_ret_to_string(status), status);
		}
		else
		{
			ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_deactivate_ex() - Failed to deactivate timer, reason: %s(%d).",
					freertos_ret_to_string(status), status);
		}
	}
	else if( !suppressVerbose )
	{
		if( "timer" )
		{
		   ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Deactivated timer: %s.",
				"timer");
		}
		else
		{
		   ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_timer_activate_ex() - Deactivated un-named timer.");
		}
	}

	return (status == OS_SUCCESS);
}

//============================================================================================//
// SEMAPHORES
//============================================================================================//

/**
 * @brief Function to Create TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @param name Name of the semaphore.
 * @param initial_count Initial count of the semaphore.
 * @return true if the semaphore was successfully created, false otherwise.
 */
bool os_semaphore_create_ex(OS_Semaphore* txSemaphore, const char* name, OS_Uint initial_count, bool suppressVerbose) noexcept
{
    OS_Uint err = os_semaphore_create(txSemaphore, const_cast<char*>(name), initial_count);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_create_ex() - Successfully created semaphore %s.", name);
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_create_ex() - Failed to create semaphore - %s - Failure: %s(%d).", name, freertos_ret_to_string(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to Delete TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block to be deleted.
 * @return true if the semaphore was successfully deleted, false otherwise.
 */
bool os_semaphore_delete_ex(OS_Semaphore *txSemaphore, bool suppressVerbose) noexcept
{
    OS_Uint err = os_semaphore_delete(txSemaphore);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_delete_ex() - Successfully deleted semaphore - %s.",
        								"sem");
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_delete_ex() - Failed to delete semaphore - %s - Failure: %s(%d).",
        								"sem" , freertos_ret_to_string(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to put (release) a TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @return true if the semaphore was successfully released, false otherwise.
 */
bool os_semaphore_put_ex(OS_Semaphore* txSemaphore, bool suppressVerbose) noexcept
{
    OS_Uint err = os_semaphore_put(txSemaphore);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_put_ex() - Successfully put semaphore - %s.", "sem");
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_put_ex() - Failed to put semaphore - %s - Failure: %s(%d).", "sem", freertos_ret_to_string(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to get (wait for) a TX semaphore.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @param wait_option Specifies the maximum time to wait for the semaphore.
 * @return true if the semaphore was successfully acquired, false otherwise.
 */
bool os_semaphore_get_ex(OS_Semaphore* txSemaphore, OS_Ulong wait_option, bool suppressVerbose) noexcept
{
    OS_Uint err = os_semaphore_get(txSemaphore, wait_option);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_get_ex() - Successfully got semaphore - %s.", "sem");
        return true;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_get_ex() - Failed to get semaphore - %s - Failure: %s(%d).", "sem", freertos_ret_to_string(err), err);
        // TBD: log appropriate error here
        return false;
    }
}

/**
 * @brief Function to get TX semaphore count.
 * @param txSemaphore Pointer to the OS semaphore control block.
 * @return the count of the semaphore.
 */
OS_Ulong os_semaphore_get_count_ex(OS_Semaphore *txSemaphore, bool suppressVerbose) noexcept
{
    OS_Ulong count;
    OS_Uint err = os_semaphore_info_get(txSemaphore, &count);

    if (err == OS_SUCCESS)
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_get_count_ex() - Successfully got semaphore count - &s - %lu.",
										"sem" , count);
        return count;
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_semaphore_get_count_ex() - Failed to get semaphore count - &s - Failure: %s(%d).",
										"sem" , freertos_ret_to_string(err), err);
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
 * @param eventFlags - The OS event flags to create
 * @param name - The name for the event flag group
 */
bool os_event_flags_create_ex(OS_EventGroup& eventFlags, const char* name, bool suppressVerbose) noexcept
{
    if(name != nullptr)
    {
        OS_Uint status = os_event_group_create(&eventFlags, const_cast<char*>(name));
        if (status == OS_SUCCESS)
        {
            return true;
        }
        else
        {
       //     ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_create_ex() - Failed for %s, reason: %s(%d).",
       //             name, freertos_ret_to_string(status), status);

        	ConsolePort::Write("OsUtility",  "os_event_flags_create_ex() - Failed for %s, reason: %s(%d).",
        	  name, freertos_ret_to_string(status), status);
        }
    }
    else
    {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_create_ex() - Null pointer passed for event flag group name.");
    }

    // TODO: Error Handling
    return false;
}

/**
 * @brief Deletes the specified event flag group and logs any error.
 *
 * @param eventFlags - The OS event flags to delete
 **/
bool os_event_flags_delete_ex(OS_EventGroup& eventFlags, bool suppressVerbose) noexcept
{
    auto status = os_event_group_delete(&eventFlags);
    if(status != OS_SUCCESS)
    {
        if("evt")
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_delete_ex() - Failed to delete event flag group: %s, reason: %s(%d).",  "evt", freertos_ret_to_string(status), status);
        }
        else
        {
            ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_delete_ex() - Failed to delete event flag group, reason: %s(%d)", freertos_ret_to_string(status), status);
        }
        return false;
    }

    return true;
}

/**
 * @brief Set event flags in the event flag group.
 *
 * @param eventFlags - The OS event flags to set
 * @param flagsToSet - The flags to set
 */
bool os_event_flags_set_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToSet, bool suppressVerbose) noexcept {
    OS_Uint status = os_event_group_set(&eventFlags, flagsToSet);
    if (status == OS_SUCCESS) {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_set_ex() - Successfully set event flags in group: %s.", "evt");
        return true;
    } else {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_set_ex() - Failed to set event flags in group: %s, error code: %u .", "evt", status);
        return false;
    }
}

/**
 * @brief Clears event flags in the event flag group.
 *
 * @param eventFlags - The OS event flags to set
 * @param flagsToClear - The flags to clear
 */
bool os_event_flags_clear_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToClear, bool suppressVerbose) noexcept {
    OS_Uint status = os_event_group_clear(&eventFlags, flagsToClear);
    if (status == OS_SUCCESS) {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_clear_ex() - Successfully cleared event flags in group: %s.", "evt");
        return true;
    } else {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_clear_ex() - Failed to clear event flags in group: %s, error code: %u .", "evt", status);
        return false;
    }
}

/**
 * @brief Get event flags from the event flag group.
 *
 * @param eventFlags - The OS event flags to get
 * @param flagsToGet - The flags to get
 * @param getOption - The option on how to get the flags
 * @param actualFlags - The actual flags gotten
 * @param wait_option - The option on how to wait if the flags are not available
 */
bool os_event_flags_get_ex(OS_EventGroup& eventFlags, OS_Ulong flagsToGet, OS_Uint getOption,
                     OS_Ulong& actualFlags, OS_Ulong wait_option, bool suppressVerbose) noexcept {
    OS_Uint status = os_event_group_get(&eventFlags, flagsToGet, getOption, &actualFlags, wait_option);
    if (status == OS_SUCCESS) {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_get_ex() - Successfully got event flags from group: %s.", "evt");
        return true;
    } else {
        ConsolePort::WriteConditional(!suppressVerbose, "OsUtility", "os_event_flags_get_ex() - Failed to get event flags from group: %s, error code: %u .", "evt", status);
        return false;
    }
}

//============================================================================================//
// STACK FAULT HANDLER
//============================================================================================//

void os_stack_fault_handler(OS_Thread* thread )
{
	ConsolePort::Write("OsUtility",  "==||***********************************************||==");
	ConsolePort::Write("OsUtility",  "==||************  THREAD STACK FAULT   ************||==");
	ConsolePort::Write("OsUtility",  "==||***********************************************||==");
	ConsolePort::Write("OsUtility",  "==||os_stack_fault_handler() - Thread: %s.", "thread");
	ConsolePort::Write("OsUtility",  "==||***********************************************||==");
}

