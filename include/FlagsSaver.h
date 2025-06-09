/**
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_FLAGSSAVER_H_
#define UTILITIES_COMMON_FLAGSSAVER_H_

#include <bitset>
#include <functional>
#include "UTILITIES/common/RtosCompat.h"

#include <HAL/component_handlers/ConsolePort.h>

#include "UTILITIES/common/FlagsSaverGettersExposer.h"
#include "UTILITIES/common/FlagsSaverSettersExposer.h"
#include <UTILITIES/common/EnumeratedSetStatus.h>
#include <UTILITIES/common/ThingsToString.h>
#include <UTILITIES/common/MutexGuard.h>

//==============================================================//
/// CLASS
//==============================================================//

/**
 * @class FlagsSaver
 * @brief This class is used to save and manage flags.  Flags status is indexed via an enum s the index.   Skipped enum values are ignored,
 *          but they do take up space.   Each flags is in one of the following states:
 *       Unknown,   // Initial state
 *       Ignored,   //
 *       Set,       // The flag is determined to exist
 *       Clear      // The flag is determined not to exist
 *
 * @tparam FlagsType The type of the flag.
 * @tparam N The size of the bitset.
 */
template <typename FlagsType, size_t N>
class FlagsSaver : public FlagsSaverGettersExposer<FlagsType>, public FlagsSaverSettersExposer<FlagsType>
{
public:

	//====================================//
	/// CONSTRUCTOR
	//====================================//

	FlagsSaver(const char* (*enumToStringConverter)( FlagsType ) = nullptr);
	virtual ~FlagsSaver();

	FlagsSaver( const FlagsSaver& copy ) noexcept = delete;
	FlagsSaver& operator = ( const FlagsSaver& copy ) noexcept = delete;

	//====================================//
	/// FLAGS SETTERS
	//====================================//

	/**
	 * @brief Set an flag. Can only be set by the variable owner thread.
	 * @param flag The flag to set.
     * @return True if the actions successful, false otherwise.
	 */
    bool SetFlag(FlagsType flag) noexcept override;

    /**
     * @brief Clear an flag.
     * @param flag The flag to clear.
     * @return True if the actions successful, false otherwise.
     */
    bool ClearFlag(FlagsType flag) noexcept override;

    /**
     * @brief Sets flag status as unknown.
     * @param flag The flag to ignore.
     * @return True if the actions successful, false otherwise.
     */
    bool SetUnknown(FlagsType flag) noexcept override;

    /**
     * @brief Sets all flag status as unknown.
     * @return True if the actions successful, false otherwise.
     */
    bool SetAllUnknown() noexcept;

	//====================================//
	/// FLAGS CHECKERS
	//====================================//

    /**
     * @brief Check if flag is set.
     * @param flag The flag to check.
     * @return True if the flag is set, false otherwise.
     */
     bool IsFlagSet(FlagsType flag) noexcept override;

    /**
     * @brief Check if an flag is set.
     * @param flag The flag to check.
     * @return True if the flag is set, false otherwise.
     */
     bool IsAnyFlagsSet() noexcept override;

    /**
     * @brief Check if an flag is unknown.
     * @param flag The flag to check.
     * @return True if the flag is ignored, false otherwise.
     */
     bool IsFlagUnknown(FlagsType flag) noexcept override;

 	//====================================//
 	/// NEW FLAG ACTIVITY GETTERS
 	//====================================//

     /**
      * @brief Waits for a new flag setting/clearing activities within timeout.
      * @param[in] waitTime The maximum time to wait for new flag activity.
      * @return True of new flag activity happened in specified timeout, false otherwise.
      */
     bool GetNewFlagsActivity(ULONG waitTime = TX_WAIT_FOREVER) noexcept;

     /**
      * @brief Clears the New Data Event generated.
      * @return True if actions are true, false otherwise.
      */
     bool ClearNewDataEvent() noexcept;

  	//====================================//
  	/// PRIVILEDGE SETTERS
  	//====================================//

     /**
      * @brief Sets the specified thread as the owner of the data getters operation.
      *
      * @param dataGettersOwnerThreadArg Pointer to thread wanting to set as owner.
      *
      * @return True if successful in setting the owner, false otherwise.
      */
     bool SetGettersOwnerThreadTo(TX_THREAD* dataGetterOwnerThreadArg) noexcept;

     /**
      * @brief Sets the specified thread as the owner of the data setter operation.
      *
      * @param dataSetterOwnerThreadArg Pointer to thread wanting to set as owner.
      *
      * @return True if successful in setting the owner, false otherwise.
      */
     bool SetSetterOwnerThreadTo(TX_THREAD* dataSetterOwnerThreadArg) noexcept;

     /**
      * @brief Gets the flag getter owner thread.
      * @return Flags getter owner thread pointer.
      */
     TX_THREAD* GetFlagsGetterOwnerThread() noexcept;

     /**
      * @brief Gets the flag setter owner thread.
      * @return Flags setter owner thread pointer.
      */
     TX_THREAD* GetFlagsSetterOwnerThread() noexcept;

   	//====================================//
   	/// DATA PRINTERS
   	//====================================//

     /**
      * @brief Prints what the flag is currently set to.
      * @param flag FlagsType wanting to print info of.
      */
     void PrintFlags( FlagsType flag) noexcept;

private:

     /**
      * @brief Helper function to support lazy initialization.
      *
      * @return true if initialized, false otherwise.
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
	 * @brief Function to initialize class.
	 * Generally, the initialize function should return false rather than causing a low-level fault.
	 *
	 * @return true if able to initialize, false otherwise.
	 */
	bool Initialize() noexcept;

    TX_THREAD* dataSetterOwnerThread;
    TX_THREAD* dataGetterOwnerThread;

    TX_EVENT_FLAGS_GROUP dataAvailableFlagGroup;  	/**< Event flag to signal data availability. */
    bool eventFlagGroupCreated;						/**< Flag indicating that event flag group has been created. */
    static const ULONG DATA_AVAILABLE_FLAG = 0x01;  /**< Named constant for the event flag value */
    static const char dataAvailableFlagGroupName[];

    Mutex mutex;       ///< Mutex for class and ownership.
    static const char mutexName[];

    bool initialized;		   ///< To make sure class is properly initialized.
    static const bool verbose; ///< Verbosity flag.

    EnumeratedSetStatus<FlagsType, FlagsStatus, 2, N> flagsStatus;    ///< Bitset to store flag status

};

//==============================================================//
/// VERBOSE??
//==============================================================//

template <typename FlagsType, size_t N>
const bool FlagsSaver<FlagsType,N>::verbose = false;

//==============================================================//
//==============================================================//

template <typename FlagsType, size_t N>
const char FlagsSaver<FlagsType,N>::mutexName[] = "FlagsSaver-Mutex";

template <typename FlagsType, size_t N>
const char FlagsSaver<FlagsType,N>::dataAvailableFlagGroupName[] = "FlagsSaver-EventFlagGroup";

/**
 * @brief Constructor for FlagsSaver.
 * @param flagToStringFunction Function to convert flag to string.
 */
template <typename FlagsType, size_t N>
FlagsSaver<FlagsType,N>::FlagsSaver(const char* (*enumToStringConverter)( FlagsType )) :
	dataSetterOwnerThread(nullptr),
	dataGetterOwnerThread(nullptr),
	eventFlagGroupCreated(false),
	mutex( mutexName),
	initialized(false),
	flagsStatus( FlagsStatus::Unknown, enumToStringConverter,  &FlagsStatusToString )  // Each flag starts with an unknown status
{
	/// No code at this time.
}

/**
 * @brief Destructor for FlagsSaver.
 */
template <typename FlagsType, size_t N>
FlagsSaver<FlagsType,N>::~FlagsSaver()
{
  /// No code at this time
}

/**
 * @brief Function to initialize class.
 * Generally, the initialize function should return false rather than causing a low-level fault.
 *
 * @return true if able to initialize, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::Initialize() noexcept
{
	bool success = false;

	/// Create the mutex and lock it, check for success
	MutexGuard guard(mutex, MutexGuard::MaxInitializationTimeMsec, &success);

	if( success)
	{
		if(!eventFlagGroupCreated)
		{
			eventFlagGroupCreated = CreateTxEventFlags(dataAvailableFlagGroup, dataAvailableFlagGroupName);
		}
		return eventFlagGroupCreated;
	}
	return false;
}

/**
 * @brief Sets the calling thread as the owner of the retrieval operation.
 *
 * If no other thread is currently waiting for data, this function sets the calling thread as the owner.
 *
 * @return True if successful in setting the owner, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::SetSetterOwnerThreadTo(TX_THREAD* dataSetterOwnerThreadArg) noexcept {
    if(dataSetterOwnerThreadArg != nullptr) {
    	/// If no thread is currently waiting for data, set the current thread as the owner
    	dataSetterOwnerThread = dataSetterOwnerThreadArg;
        return true;
    }
    return false;
}

/**
 * @brief Sets the specified thread as the owner of the data getters operation.
 *
 * @param dataGettersOwnerThreadArg Pointer to thread wanting to set as owner.
 *
 * @return True if successful in setting the owner, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::SetGettersOwnerThreadTo(TX_THREAD* dataGetterOwnerThreadArg) noexcept {
    if(dataGetterOwnerThreadArg != nullptr) {
    	/// If no thread is currently waiting for data, set the current thread as the owner
    	dataGetterOwnerThread = dataGetterOwnerThreadArg;
        return true;
    }
    return false;
}

/**
 * @brief Returns owner thread.
 *
 * @return Owner thread pointer.
 */
template <typename FlagsType, size_t N>
TX_THREAD* FlagsSaver<FlagsType,N>::GetFlagsGetterOwnerThread() noexcept{
	return dataGetterOwnerThread;
}

/**
 * @brief Returns owner thread.
 *
 * @return Owner thread pointer.
 */
template <typename FlagsType, size_t N>
TX_THREAD* FlagsSaver<FlagsType,N>::GetFlagsSetterOwnerThread() noexcept {
	return dataSetterOwnerThread;
}

/**
 * @brief Waits for a new flag setting/clearing activities within timeout.
 * @param[in] waitTime The maximum time to wait for new flag activity.
 * @return True of new flag activity happened in specified timeout, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::GetNewFlagsActivity(ULONG waitTime) noexcept {
    ULONG actual_flags;

    UINT status = tx_event_flags_get(&dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, TX_OR_CLEAR, &actual_flags, waitTime);

    if (status == TX_SUCCESS && (actual_flags & DATA_AVAILABLE_FLAG)) {
    	return true;
    }

	/// If all failed
    return false;
}

/**
 * @brief Set an flag. Can only be set by the variable owner thread.
 * @param flag The flag to set.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::SetFlag(FlagsType flag) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already set, Set the flag in a mutex protected manner
    		if( !flagsStatus.IsStatus(flag, FlagsStatus::Set) ) {
				/// Set the flag in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				flagsStatus.Set(flag, FlagsStatus::Set);
				PrintFlags(flag);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

/**
 * @brief Clear an flag.
 * @param flag The flag to clear.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::ClearFlag(FlagsType flag) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already cleared, clear the flag in a mutex protected manner
    		if( !flagsStatus.IsStatus(flag, FlagsStatus::Cleared) ) {
				/// Set the flag in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				flagsStatus.Set(flag, FlagsStatus::Cleared );
				PrintFlags(flag);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::SetUnknown(FlagsType flag) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already cleared, clear the flag in a mutex protected manner
    		if( !flagsStatus.IsStatus(flag, FlagsStatus::Unknown) ) {
				/// Set the flag in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				flagsStatus.Set(flag, FlagsStatus::Unknown );
				PrintFlags(flag);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

template <typename FlagsType, size_t N> bool FlagsSaver<FlagsType,N>::SetAllUnknown() noexcept
{
    if( EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify())
    	{
    		MutexGuard guard(mutex);

    		/// if not already cleared, clear the flag in a mutex protected manner
    		flagsStatus.SetAll( FlagsStatus::Unknown);
    		return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    	}
    }

    return false;
}

template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::IsFlagSet(FlagsType flag) noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

		    return flagsStatus.IsStatus(flag, FlagsStatus::Set);
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Check if an flag is set.
 * @param flag The flag to check.
 * @return True if the flag is set, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::IsAnyFlagsSet() noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

			 return flagsStatus.IsAny(FlagsStatus::Set);
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Check if an flag is ignored.
 * @param flag The flag to check.
 * @return True if the flag is ignored, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::IsFlagUnknown(FlagsType flag) noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

		    return flagsStatus.IsStatus(flag, FlagsStatus::Unknown);
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Clears the new internal new data flags.
 *
 * @return True if successful, false otherwise.
 */
template <typename FlagsType, size_t N>
bool FlagsSaver<FlagsType,N>::ClearNewDataEvent() noexcept {
    return ClearTxEventFlags(dataAvailableFlagGroup, ~DATA_AVAILABLE_FLAG, true);
}

/**
 * @brief Print an flag.
 * @param flag The flag to print.
 */
template <typename FlagsType, size_t N>
void FlagsSaver<FlagsType,N>::PrintFlags( FlagsType flag) noexcept
{
	if( verbose )
	{
		FlagsStatus status = flagsStatus.Get( flag );
		ConsolePort::Write( "FlagsSaver::PrintFlags() - Entry %s(%d) set to %s(%d).",
		  flagsStatus.ToEnumerationString(flag), std::to_underlying(flag), flagsStatus.ToStatusString(status), std::to_underlying(status) );
		TxDelayMsec ( 2 );
	}
}

#endif /* UTILITIES_COMMON_FLAGSSAVER_H_ */
