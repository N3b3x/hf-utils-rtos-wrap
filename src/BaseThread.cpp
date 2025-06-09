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

#include "UTILITIES/common/BaseThread.h"
#include "UTILITIES/common/TxUtility.h"
#include "UTILITIES/common/Utility.h"

#include "HAL/component_handlers/ConsolePort.h"

//==============================================================//
/// VERBOSE??
//==============================================================//

static bool verbose = false;

//==============================================================//
/// BASE THREAD ITEMS BASE NAMES
//==============================================================//

/// Base thread start semaphore base txThreadName that will be pre-fixed on the thread Name

//==============================================================//
/// BASE THREAD
//==============================================================//

/**
  * @brief  The constructor initializes the thread and prepares it to start
  *  @return n/a
   */
BaseThread::BaseThread( const char* threadName) :
    initialized( false ),
	txThread{},
    txThreadName( threadName ),   // TODO:  This should be a strncpy into a buffer
	txThreadCreated( false),
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
	if( txThreadCreated )
	{
		WRITE_CONDITIONAL(verbose,"BaseThread::~BaseThread() - Deleting %s.", txThreadName);

		DeleteTxThread( &txThread);
	}
}

/**
 * @brief Returns the Base thread txThreadName.
 *
 * @return Returns the base thread txThreadName.
 */
const char* BaseThread::GetThreadName() noexcept
{

	return txThreadName;
}

/**
 * @brief Function to Create TX thread.
 * @param txThreadName Name of thread
 * @param stack - statically allocted stack
 * @param stackSizeBytes Numer of bytes allocated
 * @param priority - thread priority
 * @param preempt_threshold - thread preemption priority
 * @param timeSliceAllowed -
 *  @param auto_start -
 */
bool BaseThread::CreateBaseThread( uint8_t* stack, ULONG stackSizeBytes, UINT priority,  UINT preempt_threshold,  ULONG timeSliceAllowed, UINT auto_start  ) noexcept
{
	/// Then, if the thread is not already created, create it
	if(signalSemaphore.EnsureInitialized())
	{
	  /// Create the ThreadX Thread
	   bool status = CreateTxThread(&txThread, const_cast<char*>(txThreadName), ThreadEntry, reinterpret_cast<ULONG>(this), stack,  stackSizeBytes, priority, preempt_threshold,
			   	   	   timeSliceAllowed, auto_start );
	   /// If successfully created
	   if (status)
	   {
		   /// Mark the base thread as created.
		   txThreadCreated = true;

		   WRITE_CONDITIONAL(verbose,"BaseThread::CreateBaseThread() - Successfully created %s.", txThreadName);
		return true;
	   }
	   else
	   {
		   consolePort.Write("BaseThread::CreateBaseThread() - Failed to create %s.", txThreadName);
		   TxDelayMsec( 5 );
		   return false;
	   }
	   return txThreadCreated;
	}

	return false;
}

/**
  * @brief Suspend the underlying thread ThreadX task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::Suspend() noexcept
{
	if( EnsureInitialized() )
	{
		return SuspendTxThread( &txThread);
	}
	return false;
}

/**
  * @brief Resume the underlying thread ThreadX task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::Resume() noexcept
{
	if( EnsureInitialized() )
	{
		return ResumeTxThread(&txThread);
	}
	return false;
}



/**
  * @brief Check the status of the underlying thread ThreadX task.
  * @return True if action successful, false otherwise.
  */
bool BaseThread::IsSuspended() noexcept
{
	if( initialized )
	{
		UINT state = 0;

	    // Get the current state of the thread
	    if( TX_SUCCESS == tx_thread_info_get(&txThread, nullptr, &state, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) )
	    {
	    	// Check if the thread is suspended
	    	return (state == TX_SUSPENDED) || (state == TX_SEMAPHORE_SUSP);
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
	WRITE_CONDITIONAL(verbose, "BaseThread::Start() - invoked on thread: %s.", txThreadName);

	if(IsThreadRunning()) {
		WRITE_CONDITIONAL(verbose, "BaseThread::Start() - thread: %s - already running.", txThreadName);
		return true;
	}

	WRITE_CONDITIONAL(verbose, "BaseThread::Start() - thread: %s - running StartAction().", txThreadName);

	/// Run start action
	if(StartAction()) {
		WRITE_CONDITIONAL(verbose, "BaseThread::Start() - thread: %s - StartAction() successful, signaling start.", txThreadName);

	    /// If successful, signal the thread via the designated semaphore to start the process
	    return signalSemaphore.Signal();
	}
	else {
		WRITE_CONDITIONAL(verbose, "BaseThread::Start() - thread: %s - StartAction() FAILED, CANNOT SIGNAL START.", txThreadName);
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
    ResumeTxThreadIfSuspended( &txThread );

    return true;
}

void BaseThread::ThreadEntry( ULONG threadEntry)
{
    BaseThread* thread = reinterpret_cast<BaseThread*>(threadEntry);

    WRITE_CONDITIONAL( verbose,  "BaseThread::ThreadEntry() for %s.",  thread->txThreadName );

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

        WRITE_CONDITIONAL( verbose,  "%s() - Start initiated on semaphore: %s.",
                 thread->txThreadName, thread->signalSemaphore.GetName());

        if( !thread->IsSetupComplete() )
        {
            thread->Setup();
            thread->MarkSetupComplete();
        }

        while ( !thread->IsThreadStopRequested() )
        {
        	thread->waitBeforeStep = thread->Step(); 											/// Main control loop sequence
        	thread->minTimestampBeforeNextStep = GetElapsedTimeMsec() + thread->waitBeforeStep; /// Calculate next timestamp to Step() again

			thread->MarkThreadStepInDelay();							/// Mark that we are going to a delay
			TxDelayMsec(static_cast<uint16_t>(thread->waitBeforeStep));	/// Delay requested time
			thread->ClearThreadStepInDelay();							/// Clear that we are in a delay
        }

        WRITE_CONDITIONAL( verbose,  "%s() - Stop initiated on semaphore: %s.",
           thread->txThreadName, thread->signalSemaphore.GetName());


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
     WRITE_CONDITIONAL( verbose,  "%s() - Waiting indefinitely for start on semaphore: %s.",
         txThreadName, signalSemaphore.GetName());

     signalSemaphore.WaitUntilSignalled();

     WRITE_CONDITIONAL( verbose,  "%s() - received start on semaphore: %s.",
             txThreadName, signalSemaphore.GetName());
}

bool BaseThread::StartThreadAndWaitToVerify(uint32_t startTimeoutMsec) {
    /// Start the thread
    if (Start()) {
       WRITE_CONDITIONAL(verbose,"BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has been requested to start.", txThreadName);

        /// Function that will test if the thread has been started
        std::function<bool()> CheckIfThreadIsStarted = [this]() {
            return IsThreadRunning();
        };

        const uint32_t waitStartTimeMsec = GetElapsedTimeMsec();

        /// Expecting true within the timeout specified
        bool result = TestLogicWithTimeout(CheckIfThreadIsStarted, true, startTimeoutMsec, 10);

        const uint32_t elapsedStartTimeMsec = GetElapsedTimeMsec() - waitStartTimeMsec;


        /// Print info to user and do other things if necessary.
        if (result) {
            WRITE_CONDITIONAL(verbose, "BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has started after %u msec.", txThreadName, elapsedStartTimeMsec);
        }
        else {
            WRITE_CONDITIONAL(verbose, "BaseThread::StartThreadAndWaitToVerify() - Thread [%s] has NOT started within [%u] Msec!!!!!.", txThreadName, startTimeoutMsec);
        }

        return result; /// Return result
    }

    return false; /// Return false indicating Start() call failure
}

bool BaseThread::StopThreadAndWaitToVerify(uint32_t stopTimeoutMsec) {
    /// Stop the thread
    if (Stop())
    {
        WRITE_CONDITIONAL(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has been requested to stop.", txThreadName);

        /// Function that will test if the thread has been stopped
        std::function<bool()> CheckIfThreadIsStopped = [this]() {
            return IsThreadStopped();
        };

        const uint32_t waitStartTimeMsec = GetElapsedTimeMsec();

        /// Expecting true within the timeout specified
        bool result = TestLogicWithTimeout(CheckIfThreadIsStopped, true, stopTimeoutMsec, 10);


        const uint32_t elapsedStopTimeMsec = GetElapsedTimeMsec() - waitStartTimeMsec;

        /// Print info to user and do other things if necessary.
        if (result) {
        	WRITE_CONDITIONAL(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has stopped after %u msec", txThreadName, elapsedStopTimeMsec );
        }
        else {
        	WRITE_CONDITIONAL(verbose, "BaseThread::StopThreadAndWaitToVerify() - Thread [%s] has NOT stopped within [%u] Msec!!!!!.", txThreadName, stopTimeoutMsec);
        }

        return result; /// Return result
    }

    return false; /// Return false indicating Stop() call failure
}

