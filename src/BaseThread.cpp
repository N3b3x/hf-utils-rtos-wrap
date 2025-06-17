/**
 * @file BaseThread.cpp
 * @brief Implementation of the BaseThread class.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * This source file implements the BaseThread abstraction which provides the
 * common infrastructure for service and clinical threads. Refer to
 * BaseThread.h for a detailed description of the class design and usage.
 */

#include "BaseThread.h"
#include "OsUtility.h"
#include "Utility.h"
#include "ConsolePort.h"

//==============================================================//
/// VERBOSE??
//==============================================================//

static bool verbose = false;

//==============================================================//
/// BASE THREAD ITEMS BASE NAMES
//==============================================================//

/// Base thread start semaphore base osThreadName that will be pre-fixed on the thread Name

//==============================================================//
/// BASE THREAD
//==============================================================//

/**
  * @brief  The constructor initializes the thread and prepares it to start
  *  @return n/a
   */
BaseThread::BaseThread( const char* threadName) :
    initialized( false ),
	osThread{},
    osThreadName( threadName ),   // TODO:  This should be a strncpy into a buffer
	osThreadCreated( false),
	signalSemaphore( baseThreadStartSemaphoreBaseName, threadName),

    consolePort(ConsolePort::GetInstance()),

	waitBeforeStep(0),
	minTimestampBeforeNextStep(0),
	threadRunning(false),
	threadStepInDelay(false),
	setupComplete(false),
	cleanupComplete(false),
	stopThreadRequested(false)
{
	/// No code at this moment.
}

/**
  * @brief  The destructor de-initializes the thread. It is not likely to be called in released products,
  * but will  support unit testing
  * @return n/a
  */
BaseThread::~BaseThread()
{
	if( osThreadCreated )
	{
		ConsolePort::WriteConditional(verbose,"BaseThread::~BaseThread()",  "- Deleting %s.", osThreadName);

		os_thread_delete_ex( &osThread);
	}
}

/**
 * @brief Returns the Base thread osThreadName.
 *
 * @return Returns the base thread osThreadName.
 */
const char* BaseThread::GetThreadName() noexcept
{

	return osThreadName;
}

/**
 * @brief Function to Create TX thread.
 * @param osThreadName Name of thread
 * @param stack - statically allocted stack
 * @param stackSizeBytes Numer of bytes allocated
 * @param priority - thread priority
 * @param preempt_threshold - thread preemption priority
 * @param timeSliceAllowed -
 *  @param auto_start -
 */
bool BaseThread::CreateBaseThread( uint8_t* stack, OS_Ulong stackSizeBytes,
                                   OS_Uint priority, OS_Uint preempt_threshold,
                                   OS_Ulong timeSliceAllowed, OS_Uint auto_start ) noexcept
{
	/// Then, if the thread is not already created, create it
	if(signalSemaphore.EnsureInitialized())
	{
	  /// Create the OS Thread
           bool status = os_thread_create_ex(&osThread, const_cast<char*>(osThreadName), ThreadEntry,
                                        reinterpret_cast<OS_Ulong>(this), stack,
                                        stackSizeBytes, priority, preempt_threshold,
                                       timeSliceAllowed, auto_start );
	   /// If successfully created
	   if (status)
	   {
		   /// Mark the base thread as created.
		   osThreadCreated = true;

		   ConsolePort::WriteConditional(verbose,"BaseThread::CreateBaseThread() - Successfully created %s.", osThreadName);
		return true;
	   }
	   else
	   {
		   ConsolePort::WriteConditional(true,"BaseThread::CreateBaseThread() - Failed to create %s.", osThreadName);
		   os_delay_msec( 5 );
		   return false;
	   }
	   return osThreadCreated;
	}

	return false;
}

/**
  * @brief Suspend the underlying thread OS task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::Suspend() noexcept
{
	if( EnsureInitialized() )
	{
		return os_thread_suspend_ex( &osThread);
	}
	return false;
}

/**
  * @brief Resume the underlying thread OS task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::Resume() noexcept
{
	if( EnsureInitialized() )
	{
		return os_thread_resume_ex(&osThread);
	}
	return false;
}



/**
  * @brief Check the status of the underlying thread OS task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::IsSuspended() noexcept
{
        if( initialized )
        {
                OS_Uint state = 0;

	    // Get the current state of the thread
            if( OS_SUCCESS == os_thread_info_get(&osThread, &state) )
	    {
	    	// Check if the thread is suspended
                return (state == eSuspended) || (state == eBlocked);
	    }

	    return false;
	}
	else  // Not initialized?  consider it suspended
	{
		return true;
	}
}

bool BaseThread::Start() noexcept
{
	ConsolePort::WriteConditional(verbose, "BaseThread::Start() - invoked on thread: %s.", osThreadName);

	if(IsThreadRunning()) {
		ConsolePort::WriteConditional(verbose, "BaseThread::Start() - thread: %s - already running.", osThreadName);
		return true;
	}

	ConsolePort::WriteConditional(verbose, "BaseThread::Start() - thread: %s - running StartAction().", osThreadName);

	/// Run start action
	if(StartAction()) {
		ConsolePort::WriteConditional(verbose, "BaseThread::Start() - thread: %s - StartAction() successful, signaling start.", osThreadName);

	    /// If successful, signal the thread via the designated semaphore to start the process
	    return signalSemaphore.Signal();
	}
	else {
		ConsolePort::WriteConditional(verbose, "BaseThread::Start() - thread: %s - StartAction() FAILED, CANNOT SIGNAL START.", osThreadName);
	}

	/// Otherwise, indicate that we cannot start thread.
	return false;
}

bool BaseThread::StartAction() noexcept
{
	return true;
}


bool BaseThread::Stop() noexcept
{
    MarkThreadStopRequested();

    /// In case we are somehow suspended in our step, let's resume.
    os_thread_resume_if_suspended( &osThread );

    return true;
}

void BaseThread::ThreadEntry( OS_Ulong threadEntry)
{
    BaseThread* thread = reinterpret_cast<BaseThread*>(threadEntry);

    ConsolePort::WriteConditional( verbose,  "BaseThread::ThreadEntry() for %s.",  thread->osThreadName );

    while ( true )
    {
        thread->ClearThreadRunning();   /// Clear the thread running flag so that we can Start() properly.

        thread->WaitForStart();         /// Wait for a signal to start ( it occurs in BaseThread Start() )

        thread->ResetVariables();		/// Reset all necessary variables

        thread->MarkThreadRunning();		/**< Thread should be marked as running immediately. */

        thread->ClearThreadStopRequested(); /**< Clear this flag in case CALLER didn't wait to send START command until the mode was completely stopped
                                                 and then sent another STOP flag because CALLER doesn't see the mode starting, this should not be the case
                                                 in the final product but necessary during development. */
        thread->ClearCleanupComplete();		/**< Clear the cleanup complete flag. */

        ConsolePort::WriteConditional( verbose,  "%s() - Start initiated on semaphore: %s.",
                 thread->osThreadName, thread->signalSemaphore.GetName());

        if( !thread->IsSetupComplete() )
        {
            thread->Setup();
            thread->MarkSetupComplete();
        }

        while ( !thread->IsThreadStopRequested() )
        {
        	thread->waitBeforeStep = thread->Step(); 											/// Main control loop sequence
        	thread->minTimestampBeforeNextStep = os_get_elapsed_time_msec() + thread->waitBeforeStep; /// Calculate next timestamp to Step() again

			thread->MarkThreadStepInDelay();							/// Mark that we are going to a delay
			os_delay_msec(static_cast<uint16_t>(thread->waitBeforeStep));	/// Delay requested time
			thread->ClearThreadStepInDelay();							/// Clear that we are in a delay
        }

        ConsolePort::WriteConditional( verbose,  "%s() - Stop initiated on semaphore: %s.",
           thread->osThreadName, thread->signalSemaphore.GetName());


        /// Make sure to mark
        if( !thread->IsCleanupComplete() )
        {
            thread->Cleanup();              /// Clean up before exiting the thread.
            thread->MarkCleanupComplete();	/// Let's set that the cleanup flag so that we can make that a cleanup is complete
        }

        thread->ClearSetupComplete();   /// Let's clear the setup complete flag so that a setup is ran before re-entering loop.

    }
}

/**
 * @brief Puts the caller thread in a waiting state until the service_start() is called.
 *
 */
void BaseThread::WaitForStart() noexcept
{
     ConsolePort::WriteConditional( verbose,  "%s() - Waiting indefinitely for start on semaphore: %s.",
         osThreadName, signalSemaphore.GetName());

     signalSemaphore.WaitUntilSignalled();

     ConsolePort::WriteConditional( verbose,  "%s() - received start on semaphore: %s.",
             osThreadName, signalSemaphore.GetName());
}

bool BaseThread::StartThreadAndWaitToVerify(uint32_t startTimeoutMsec) {
    /// Start the thread
    if (Start()) {
       ConsolePort::WriteConditional(verbose,"BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has been requested to start.", osThreadName);

        /// Function that will test if the thread has been started
        std::function<bool()> CheckIfThreadIsStarted = [this]() {
            return IsThreadRunning();
        };

        const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

        /// Expecting true within the timeout specified
        bool result = TestLogicWithTimeout(CheckIfThreadIsStarted, true, startTimeoutMsec, 10);

        const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;


        /// Print info to user and do other things if necessary.
        if (result) {
            ConsolePort::WriteConditional(verbose, "BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has started after %u msec.", osThreadName, elapsedStartTimeMsec);
        }
        else {
            ConsolePort::WriteConditional(verbose, "BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has NOT started within [%u] Msec!!!!!.", osThreadName, startTimeoutMsec);
        }

        return result; /// Return result
    }

    return false; /// Return false indicating Start() call failure
}

bool BaseThread::StopThreadAndWaitToVerify(uint32_t stopTimeoutMsec) {
    /// Stop the thread
    if (Stop())
    {
        ConsolePort::WriteConditional(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has been requested to stop.", osThreadName);

        /// Function that will test if the thread has been stopped
        std::function<bool()> CheckIfThreadIsStopped = [this]() {
            return IsThreadStopped();
        };

        const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

        /// Expecting true within the timeout specified
        bool result = TestLogicWithTimeout(CheckIfThreadIsStopped, true, stopTimeoutMsec, 10);


        const uint32_t elapsedStopTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

        /// Print info to user and do other things if necessary.
        if (result) {
        	ConsolePort::WriteConditional(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has stopped after %u msec", osThreadName, elapsedStopTimeMsec );
        }
        else {
        	ConsolePort::WriteConditional(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has NOT stopped within [%u] Msec!!!!!.", osThreadName, stopTimeoutMsec);
        }

        return result; /// Return result
    }

    return false; /// Return false indicating Stop() call failure
}



uint32_t BaseThread::GetStackHighWaterMark() const noexcept
{
    return uxTaskGetStackHighWaterMark(osThread);
}

bool BaseThread::ChangePriority(uint32_t newPriority) noexcept
{
    if (osThread) {
        vTaskPrioritySet(osThread, newPriority);
        return true;
    }
    return false;
}
