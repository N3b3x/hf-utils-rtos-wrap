/**
 * @file Mutex.h
 * @brief Mutex class definition.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 * The Mutex class provides wrapper for a "named" mutex that is created dynamically
 *    created. When the Mutex object goes out of scope, the threadX mustex is deleted.
 *
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "UTILITIES/common/TxUtility.h"

class Mutex
{
public:

	/**
	  * @brief The constructor
	  * @param name of mutex( copied into private variable during construction)
	  * @param nameExtension of mutex( copied into private variable during construction).  Used for "uniquer"
	  */
    Mutex( const char* name, const char* nameExtension = 0 ) noexcept;

    /**
	  * @brief The Copy constructor is deleted to avoid copying instances.
	  * @return n/a
	  */
    Mutex( const Mutex& copy ) = delete;

    /**
      * @brief  The assignment operator constructor is deleted to avoid copying instances.
      * @return n/a
      */
    Mutex& operator = ( const Mutex& copy ) = delete;

    /**
	  * @brief The destructor deletes the mutex
	  *
	  */
    ~Mutex();

	/**
	  * @brief Accessor function to support testing
     */
	bool IsInitialized() const noexcept
	{
		return initialized;
	}


    /**
   	  * @brief This function locks the mutex
   	  * @param maxMsecToWait Max time to wait before timing out
   	  */
	bool Lock( uint32_t maxMsecToWait) noexcept;

	/**
	 * @brief This function unlocks the mutex
	 */
	bool Unlock() noexcept;



	uint32_t GetOwnershipCount() const noexcept
	{
		return mutex.tx_mutex_ownership_count;
	}

	const char* GetName() const noexcept
	{
		return mutexName;
	}

    bool EnsureInitialized() noexcept
    {
    	if ( !initialized )
    	{
    		initialized = CreateTxMutex( mutex, mutexName );
    	}
    	return initialized;
    }


  static constexpr uint32_t MaxNameLength = 39U;  // Not including null terminator
private:

    TX_MUTEX mutex;
    char mutexName[MaxNameLength + 1];
    bool initialized;

};


#endif /* MUTEX_H */
