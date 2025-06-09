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

#ifndef MUTEXGUARD_H_
#define MUTEXGUARD_H_

#include "Mutex.h"
#include "variant"

class MutexGuard
{
public:

	/**
	  * @brief The constructor acquires the mutex, expected to wait forever.
	  * @param mutex Mutex to be acquired
	  * @param[out] success A pointer to store success of mutex acquiring.
	  */
    MutexGuard(TX_MUTEX* mutex, bool* pSuccess);

	/**
	  * @brief The constructor acquires the mutex, expected to wait forever.
	  * @param mutex Mutex to be acquired
	  * @param[out] success A pointer to store success of mutex acquiring.
	  */
    MutexGuard(TX_MUTEX& mutex, bool* pSuccess);

    /**
  	  * @brief The constructor acquires the mutex
  	  * @param mutex Mutex to be acquired
	  * @param[out] success A pointer to store success of mutex acquiring.
  	  */
     MutexGuard( Mutex& mutex, bool* pSuccess);

	/**
	  * @brief The constructor acquires the mutex
	  * @param mutex Mutex to be acquired
	  * @param maxMsecToWait Max time to wait before timing out
	  * @param[out] success A pointer to store success of mutex acquiring.
	  */
    MutexGuard(TX_MUTEX* mutex, uint32_t maxMsecToWait = MaxWaitTimeMsec, bool* pSuccess = nullptr);

	/**
	  * @brief The constructor acquires the mutex
	  * @param mutex Mutex to be acquired
	  * @param maxMsecToWait Max time to wait before timing out
	  * @param[out] success A pointer to store success of mutex acquiring.
	  */
    MutexGuard(TX_MUTEX& mutex, uint32_t maxMsecToWait = MaxWaitTimeMsec, bool* pSuccess = nullptr);

    /**
  	  * @brief The constructor acquires the mutex
  	  * @param mutex Mutex to be acquired
  	  * @param maxMsecToWait Max time to wait before timing out
	  * @param[out] success A pointer to store success of mutex acquiring.
  	  */
     MutexGuard( Mutex& mutex, uint32_t maxMsecToWait = MaxWaitTimeMsec, bool* pSuccess = nullptr);

    /**
	  * @brief The destructor releases the lock on the mutex
	  */
    ~MutexGuard();


    static constexpr uint32_t MaxWaitTimeMsec = 250U;  /// TODO:  This seems unreasonably long.. lets work down to about 100 msec
    static constexpr uint32_t MaxInitializationTimeMsec = 10U;  /// Used for initial creation/lock


private:
    mutable std::variant<TX_MUTEX*, Mutex*> mutex;        //  Address of existing threadX mutex or Mutex instance

};


#endif /* MUTEXGUARD_H_ */
