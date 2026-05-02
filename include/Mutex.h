/**
 * @file Mutex.h
 * @brief Named-mutex wrapper used by `BaseThread` infrastructure.
 *
 * The `Mutex` class wraps a dynamically created RTOS mutex with a name for
 * debugging. The handle is created in the constructor and destroyed when
 * the `Mutex` object goes out of scope.
 *
 * Thread-safety: lock / unlock are safe from any task context (FreeRTOS
 * mutex). Not ISR-safe — use `RtosMutex` or a critical section for ISRs.
 *
 * Allocation: one RTOS mutex handle is allocated on construction; no
 * allocation in `Lock` / `Unlock`.
 *
 * For `std::lock_guard` compatibility use `RtosMutex` (see `RtosMutex.h`);
 * this class is the named-handle variant intended for `BaseThread`
 * infrastructure that exposes the handle by name.
 *
 * @todo Add @copyright line once project copyright wording is finalised.
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "OsUtility.h"

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
		// FreeRTOS mutexes don't have ownership count like ThreadX
		// Return 0 to indicate not supported or implement alternative logic
		return 0;
	}

	const char* GetName() const noexcept
	{
		return mutexName;
	}

    bool EnsureInitialized() noexcept
    {
    	if ( !initialized )
    	{
    		initialized = os_mutex_create_ex( mutex, mutexName );
    	}
    	return initialized;
    }


  static constexpr uint32_t MaxNameLength = 39U;  // Not including null terminator
private:

    OS_Mutex mutex;
    char mutexName[MaxNameLength + 1];
    bool initialized;

};


#endif /* MUTEX_H */
