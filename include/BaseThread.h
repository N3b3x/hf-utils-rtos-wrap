/**
 * @file BaseThread.h
 * @brief Abstract base class for long running worker threads.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All rights reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * The BaseThread class provides a foundation for implementing service or
 * clinical threads.  Each thread runs continuously between calls to
 * `Start()` and `Stop()`, executing a setup routine once and then repeatedly
 * calling `Step()` until a stop is requested.  Derived classes implement the
 * virtual Setup, Step and Exit hooks to provide their behavior.
 */

#ifndef BaseThread_H
#define BaseThread_H

#include <memory>
#include <atomic>
#include "OsAbstraction.h"
#include "ConsolePort.h"
#include "OsUtility.h"
#include "SignalSemaphore.h"

/**
 * @class BaseThread
 * @brief Abstract class to assist in designing thread-based execution sequences.
 *
 * Threads are intended to run indefinitely but can be started and stopped multiple times.
 * On start, the `Setup()` function is executed followed by a continuous loop awaiting a stop
 * signal, repeatedly calling the `Step()` function. On a stop signal, the `Exit()` function
 * is executed for cleanup.
 */
class BaseThread
{
public:
    /**
     * @brief Construct a new Base Thread object.
     * @param threadName Name of the thread.
     * @param threadSemaphore Semaphore associated with the thread.
     */
    BaseThread(const char *threadName);

    /**
     * @brief The Copy constructor is deleted to avoid copying instances.
     * @return n/a
     */
    BaseThread(const BaseThread&) = delete;

    /**
     * @brief  The assignment operator constructor is deleted to avoid copying instances.
     * @return n/a
     */
    BaseThread& operator =(const BaseThread&) = delete;

    /**
     * @brief  The destructor de-initializes the singleton. It is not likely to be called in released products, but is
     * implemented to support unit testing
     * @return n/a
     */
    virtual ~BaseThread() noexcept;

    /**
     * @brief Get the pointer to the thread.
     *
     * @return OS_Thread* Pointer to the thread.
     */
    OS_Thread* GetThreadId() noexcept
    {
        return initialized ? &osThread : nullptr;
    }

    /**
	  * @brief  This function checks if the class is initialized;
	  * @return true if calibrated, false otherwise
	  */

	inline bool IsInitialized() const noexcept
	{
		return initialized;
	}

    /**
     * @brief Initialize the  thread.
     *
     * This function initializes and creates the thread, setting up
     * all necessary parameters. It must be called before the thread can be started.
     *
     * @return True if actions successful, false otherwise.
     */
    bool EnsureInitialized() noexcept
    {
        if (!initialized)
        {
            initialized = Initialize();
        }
        return initialized;
    }

    /**
      * @brief Suspend the underlying thread OS task.
      * @return True if action successful, false otherwise.
      */
    bool Suspend() noexcept;

    /**
      * @brief Resume the underlying thread OS task.
      * @return True if action successful, false otherwise.
      */

    bool Resume() noexcept;

    /**
	 * @brief Check the status of the underlying thread OS task.
	 * @return True if action successful, false otherwise.
	 */

    bool IsSuspended() noexcept;

    bool IsThreadCreated() noexcept { return osThreadCreated; }
    bool IsThreadStopRequested() noexcept { return stopThreadRequested; }
    bool IsSetupComplete() const noexcept { return setupComplete; }
    bool IsThreadRunning() const noexcept { return threadRunning; }
    bool IsThreadStopped() const noexcept { return !threadRunning; }
    bool IsCleanupComplete() const noexcept {return cleanupComplete; }
    bool IsThreadInStepDelay() const noexcept {return threadStepInDelay; }


    /**
     * @brief Starts the thread.
     *
     * This function starts the execution of the  thread if
     * not already running.  This results in executing Setup() if it has not already been
     * done.  The base class implementation simply signals the associated semaphore.
     *
     * The starting called may be ignored depending on case therefore a bool will be returned.
     *
     * @return True if the thread was signaled to start, false otherwise.
     *
     * @note The  thread must be waiting on a start call for this to take any effect.
     */
    bool Start() noexcept;

    /**
     * @brief Start Action that can be run to see if Start() qualifies to start thread.
     * @return True if start actions are successful and Start() can proceed to signal thread to start.
     * @return False otherwise.
     */
    virtual bool StartAction() noexcept;

    /**
     * @brief Stops the thread.
     *
     * This function stops the execution of the thread.
     * The thread can be resumed later by calling service_start().  The base class
     *  simple sets the variable indicating the thread should stop.
     *
     * @return True if the thread was signaled to stop, false otherwise.
     */
    bool Stop() noexcept;

    /**
     * @brief Setup the thread.
     *
     * This function setups up for execution of the thread whenever it is started or re-started.
     *     For example, this allows the valve manifold to be configured prior to starting thread thread.
     *     It can also be called prior to Start() e.g in response to settings Confirm.
     *
     * @return True if setup was actually ran, false otherwise which could indicate that mode is current active.
     */
    virtual bool Setup() noexcept = 0;

    /**
     * @brief Steps the thread through a single execution cycle .
     *
     * This function steps the execution of the service thread if
     * not already running. Then it returns how much it needs to wait
     * before next Step() is called.
     *
     * @return The time in milliseconds to wait before running calling next Step()
     */
    virtual uint32_t Step() noexcept = 0;

    /**
     * @brief Cleanup code that exists once the thread step sequence is stopped.
     *   The cleanup function should generally clear the setup flag to ensure the thread
     *   goes back through setup again.
     *
     * @return True if cleanup was actually ran, false otherwise which could indicate that mode is currently active.
     */
    virtual bool Cleanup() noexcept = 0;

    /**
     * @brief Returns the Base thread name.
     *
     * @return Returns the base thread name.
     */
    const char* GetThreadName() noexcept;

    /**
     * @brief [BLOCKING] Signals a BaseThread to start and waits to make sure it's started within the given timeout.
     *
     * @param threadToStart BaseThread to start
     * @param startTimeoutMsec	Timeout to make sure Thread has started in Milliseconds.
     *
     * @return True if thread is started, false otherwise
     */
    bool StartThreadAndWaitToVerify(uint32_t startTimeoutMsec = 1000);

    /**
     * @brief [BLOCKING] Signals a BaseThread to stop and waits to make sure it's started within the given timeout.
     *
     * @param threadToStop BaseThread to stop
     * @param stopTimeoutMsec	Timeout to make sure Thread has stopped in Milliseconds.
     *
     * @return True if thread is stopped, false otherwise
     */
    bool StopThreadAndWaitToVerify(uint32_t stopTimeoutMsec);

    /**
     * @brief Get the minimum remaining stack of the task.
     */
    uint32_t GetStackHighWaterMark() const noexcept;

    /**
     * @brief Change the running priority of the thread.
     */
    bool ChangePriority(uint32_t newPriority) noexcept;


protected:

    /**
     * @brief Starts the thread.
     *
     * This function waits for the thread to be signalled to start.
     *
     * @note The service thread must be waiting on a start call
     * for this to take any effect.
     */
    void WaitForStart() noexcept;

    /**
     * @brief Function to initialize thread.  Generally, the initialize function
     *  should return false rather than causing a low level fault.
     * @returns true if able to initialize flash interface, false otherwise
     */
    virtual bool Initialize() noexcept = 0;

    /**
     * @brief Resets all member variables to their initial states.
     *
     * This method is designed to reset all member variables to a known state,
     * either to their initial values as set in the constructor, or to zero
     * (or false, or other suitable default values). This can be useful in
     * situations where the object needs to be reused or reinitialized.
     *
     * @note This method is marked noexcept, indicating that it does not throw exceptions.
     */
    virtual bool ResetVariables() noexcept = 0;

    /**
     * @brief Thread entry that will be running.
     * @param instanceAddress
     */
    static void ThreadEntry(OS_Ulong instanceAddress);

    /**
     * @brief Function to Create TX thread.
     *
     * @param name Name of thread
     * @param stack - statically allocated stack
     * @param stackSizeBytes Number of bytes allocated
     * @param priority - thread priority
     * @param preempt_threshold - thread preemption priority
     * @param timeSliceAllowed - Time slice allowed to run before being preempted.
     * @param auto_start - Automatic start selection.
     */
    bool CreateBaseThread(uint8_t *stack, OS_Ulong stackSizeBytes, OS_Uint priority,
            OS_Uint preempt_threshold, OS_Ulong timeSliceAllowed, OS_Uint auto_start) noexcept;

    /**
     * @brief Mark the setup as complete.
     */
    void MarkSetupComplete() noexcept { setupComplete = true; }

    /**
     * @brief Clear the setup completion flag.
     */
    void ClearSetupComplete() noexcept { setupComplete = false; }

    /**
     * @brief Mark the cleanup as complete.
     */
    void MarkCleanupComplete() noexcept { cleanupComplete = true; }

    /**
     * @brief Clear the cleanup flag.
     */
    void ClearCleanupComplete() noexcept { cleanupComplete = false; }

    /**
     * @brief Mark the thread as running.
     */
    void MarkThreadRunning() noexcept { threadRunning = true; }

    /**
     * @brief Mark the thread as not running.
     */
    void ClearThreadRunning() noexcept { threadRunning = false; }

    /**
     * @brief Indicate a request to stop the thread.
     */
    void MarkThreadStopRequested() noexcept { stopThreadRequested = true; }

    /**
     * @brief Clear the stop request for the thread.
     */
    void ClearThreadStopRequested() noexcept { stopThreadRequested = false; }

    /**
     * @brief Indicate a request to stop the thread.
     */
    void MarkThreadStepInDelay() noexcept { threadStepInDelay = true; }

    /**
     * @brief Clear the stop request for the thread.
     */
    void ClearThreadStepInDelay() noexcept { threadStepInDelay = false; }

    bool initialized;
    OS_Thread osThread;
    const char *osThreadName;
    bool osThreadCreated;
    SignalSemaphore signalSemaphore;
    static constexpr char baseThreadStartSemaphoreBaseName[] = "BaseThreadStartSem-";

    ConsolePort &consolePort;

private:

    uint32_t waitBeforeStep;
    uint32_t minTimestampBeforeNextStep;

    std::atomic<bool> threadRunning;
    std::atomic<bool> threadStepInDelay;
	std::atomic<bool> setupComplete;
	std::atomic<bool> cleanupComplete;
	std::atomic<bool> stopThreadRequested;

};

#endif /* BaseThread */
