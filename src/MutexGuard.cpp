/**
 * @file MutexGuard.h
 * @brief MutexGuard class definition.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 * The MutexGuard class provides an RAII-style mechanism for owning a
 * mutex for the duration of a scoped block. When a MutexGuard object
 * is created, it attempts to take ownership of the mutex. When the
 * object is destroyed, it releases the ownership of the mutex.
 *
 * Example:
 * \code{.cpp}
 * {
 *     MutexGuard lock(&myMutex);
 *     // critical section
 * }
 * // mutex is released here
 * \endcode
 *
 */

#include "OsAbstraction.h"
#include "ConsolePort.h"
#include "MutexGuard.h"
#include "OsUtility.h"

//==============================================================//
// VERBOSE??
//==============================================================//
static constexpr bool verbose = false;


//==============================================================//
// CLASS
//==============================================================//
MutexGuard::MutexGuard(OS_Mutex* mutexArg, bool* pSuccess) :
	mutex(mutexArg)
{
	uint32_t maxMsecToWait = MaxWaitTimeMsec;

	bool result = os_mutex_get_p(mutexArg, os_convert_msec_to_delay_ticks(maxMsecToWait), !verbose);  // tx_mutex_get checks for bad params
	if( !result )
	{
		if( mutexArg)
		{
			ConsolePort::WriteConditional(verbose, "MutexGuard", " Failed to lock mutex after %u msec", maxMsecToWait );
		}
		else
		{
			ConsolePort::WriteConditional(verbose, "MutexGuard", "%s", " Invalid mutex." );
		}
	}

	/// Return true in pointer if successful.
	if(pSuccess!=nullptr) { *pSuccess = (result); }
}

/*
 * @brief The constructor acquires the mutex
 * @param mutex Mutex to be acquired
 * @param maxMsecToWait Max time to wait before timing out
 * @param[out] success A pointer to store success of mutex acquiring.
 */

MutexGuard::MutexGuard(OS_Mutex& mutexArg, bool* pSuccess) :
		mutex( &mutexArg)
{
	uint32_t maxMsecToWait = MaxWaitTimeMsec;

        OS_Uint result = os_mutex_get_ex(mutexArg, os_convert_msec_to_delay_ticks(maxMsecToWait), !verbose);
	if( !result )
	{
		ConsolePort::WriteConditional(verbose, "MutexGuard", " Failed to lock mutex after %u msec", maxMsecToWait );
	}
	else
	{
		ConsolePort::WriteConditional(verbose, "MutexGuard", " Successfully locked mutex after %u msec", maxMsecToWait );
	}

	/// Return true in pointer if successful.
	if(pSuccess!=nullptr) { *pSuccess = (result); }
}

/**
 * @brief The constructor acquires the mutex
 * @param mutex Mutex to be acquired
 * @param maxMsecToWait Max time to wait before timing out
 */
MutexGuard::MutexGuard( Mutex& mutexArg, bool* pSuccess) :
	mutex(&mutexArg)
{
	bool success = mutexArg.Lock(MaxWaitTimeMsec );
	if(pSuccess!=nullptr) { *pSuccess = success; }
}


/**
 * @brief The constructor acquires the mutex
 * @param mutex Mutex to be acquired
 * @param maxMsecToWait Max time to wait before timing out
 * @param[out] success A pointer to store success of mutex acquiring.
 */
MutexGuard::MutexGuard(OS_Mutex* mutexArg, uint32_t maxMsecToWait, bool* pSuccess) :
	mutex(mutexArg)
{
        OS_Uint result = os_mutex_get_p(mutexArg, os_convert_msec_to_delay_ticks(maxMsecToWait), !verbose);  // tx_mutex_get checks for bad params
	if( !result )
	{
		if( mutexArg)
		{
			if( *mutexArg )
			{
				ConsolePort::WriteConditional(verbose, "MutexGuard", " Failed to lock mutex after %u msec", maxMsecToWait );
			}
			else
			{
				ConsolePort::WriteConditional(verbose, "MutexGuard", " Failed to lock un-named mutex, reasons." );
			}
		}
		else
		{
			ConsolePort::WriteConditional(verbose, "MutexGuard", " Invalid mutex." );
		}
	}
	else
	{
		ConsolePort::WriteConditional(verbose, "MutexGuard", " Successfully locked mutex after %u msec", maxMsecToWait );
	}

	/// Return true in pointer if successful.
	if(pSuccess!=nullptr) { *pSuccess = (result); }
}

/**
 * @brief The constructor acquires the mutex
 * @param mutex Mutex to be acquired
 * @param maxMsecToWait Max time to wait before timing out
 * @param[out] success A pointer to store success of mutex acquiring.
 */
MutexGuard::MutexGuard(OS_Mutex& mutexArg, uint32_t maxMsecToWait, bool* pSuccess) :
        mutex( &mutexArg)
{
        OS_Uint result = os_mutex_get_ex(mutexArg, os_convert_msec_to_delay_ticks(maxMsecToWait), !verbose);
	if( !result )
	{
		ConsolePort::WriteConditional(verbose, "MutexGuard", " Failed to lock mutex after %u msec", maxMsecToWait );
	}
	else
	{
		ConsolePort::WriteConditional(verbose, "MutexGuard", " Successfully locked mutex after %u msec", maxMsecToWait );
	}

	/// Return true in pointer if successful.
	if(pSuccess!=nullptr) { *pSuccess = (result); }
}

/**
 * @brief The constructor acquires the mutex
 * @param mutex Mutex to be acquired
 * @param maxMsecToWait Max time to wait before timing out
 */
MutexGuard::MutexGuard( Mutex& mutexArg, uint32_t maxMsecToWait, bool* pSuccess) :
	mutex(&mutexArg)
{
	bool success = mutexArg.Lock( maxMsecToWait );
	if(pSuccess!=nullptr) { *pSuccess = success; }
}

struct MutexUnlocker
{
    void operator()(OS_Mutex* mutexArg)
    {
        OS_Uint result = os_mutex_put(mutexArg);
		if( result != OS_SUCCESS )
		{
			if( mutexArg)
			{
				ConsolePort::Write("MutexUnlocker", " Failed to release mutex, reason: %u.", result );
			}
			else
			{
				ConsolePort::Write("MutexUnlocker", "%s", " Invalid mutex." );
			}
		}
	}
	void operator()(Mutex* mutex) const
	{
		mutex->Unlock();
	}
};

/**
  * @brief The destructor releases the lock on the mutex
  */


MutexGuard::~MutexGuard()
{
	std::visit( MutexUnlocker{}, mutex);
}

