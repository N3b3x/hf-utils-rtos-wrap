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


#include "Mutex.h"
#include <cstring>
#include <cstdio>
/**
  * @brief The constructor
  * @param name of mutex( copied into private variable during construction)
  * @param nameExtension of mutex( copied into private variable during construction).  Used for "uniquer"
  */
Mutex::Mutex( const char* name,  const char* nameExtension ) noexcept :
	mutex {},
	mutexName{},
	initialized( false )
{
	if( nameExtension != nullptr )
	{
		// Note:: snprintf ensures that there is a null termination character, so use the full buffer
		snprintf( mutexName, MaxNameLength + 1,"%s-%s", name, nameExtension );
	}
	else // strncpy does not guarantee there is a null termination character, so ensure there is one
	{
		strncpy ( mutexName, name, MaxNameLength);
		mutexName[MaxNameLength] = '\0';   // Add a  null character just in case the name was too long.
	}
}


/**
  * @brief The destructor deletes the mutex
  *
  */


Mutex::~Mutex()
{
	if( initialized )
	{
		os_mutex_delete_ex(mutex);
	}
}

/**
 * @brief This function locks the mutex
 * @param maxMsecToWait Max time to wait before timing out
 */
bool Mutex::Lock( uint32_t maxMsecToWait) noexcept
{
	if( EnsureInitialized() )
	{
                OS_Uint result = os_mutex_get( &mutex,  os_convert_msec_to_delay_ticks(maxMsecToWait));  // os_mutex_get checks for bad params
		if( result == OS_SUCCESS )
		{
			return true;
		}
		else
		{
			// Mutex lock failed (ConsolePort logging removed)
		}
	}
	else
	{
		// Mutex lock failed (ConsolePort logging removed)
	}
	return false;
}

/**
 * @brief This function unlocks the mutex
 */
bool Mutex::Unlock() noexcept
{
	if( EnsureInitialized() )
	{
                OS_Uint result = os_mutex_put(&mutex);
		if( result == OS_SUCCESS )
		{
			return true;
		}
		else
		{
			// Mutex unlock failed (ConsolePort logging removed)
		}
	}
	else
	{
		// Mutex unlock failed (ConsolePort logging removed)
	}

	return false;
}

