/**
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_ERRORSAVER_H_
#define UTILITIES_COMMON_ERRORSAVER_H_

#include <bitset>
#include <functional>
#include "UTILITIES/common/RtosCompat.h"

#include <HAL/component_handlers/ConsolePort.h>

#include <UTILITIES/common/ThingsToString.h>
#include <UTILITIES/common/EnumeratedSetStatus.h>
#include <UTILITIES/common/ErrorSaverGettersExposer.h>
#include <UTILITIES/common/ErrorSaverSettersExposer.h>
#include <UTILITIES/common/MutexGuard.h>

//==============================================================//
// CLASS
//==============================================================//
/**
 * @class ErrorSaver
 * @brief This class is used to save and manage errors.  Error status is indexed via an enum s the index.   Skipped enum values are ignored,
 *          but they do take up space.   Each errors is in one of the following states:
 *       Unknown,   // Initial state
 *       Ignored,   //
 *       Set,       // The error is determined to exist
 *       Clear      // The error is determined not to exist
 *
 * @tparam ErrorType The type of the error.
 * @tparam N The size of the bitset.
 */
template <typename ErrorType, size_t N>
class ErrorSaver : public ErrorSaverGettersExposer<ErrorType>, public ErrorSaverSettersExposer<ErrorType>
{
public:

	ErrorSaver(const char* (*enumToStringConverter)( ErrorType ) = nullptr);
	virtual ~ErrorSaver();

	ErrorSaver( const ErrorSaver& copy ) noexcept = delete;
	ErrorSaver& operator = ( const ErrorSaver& copy ) noexcept = delete;

	/**
	 * @brief Set an error. Can only be set by the variable owner thread.
	 * @param error The error to set.
     * @return True if the actions successful, false otherwise.
	 */
    bool SetError(ErrorType error) noexcept override;

    /**
     * @brief Clear an error.
     * @param error The error to clear.
     * @return True if the actions successful, false otherwise.
     */
    bool ClearError(ErrorType error) noexcept override;

    /**
     * @brief Sets error status as unknown.
     * @param error The error to ignore.
     * @return True if the actions successful, false otherwise.
     */
    bool SetUnknown(ErrorType error) noexcept override;

    bool SetAllUnknown() noexcept;

    /**
     * @brief Ignore an error.
     * @param error The error to ignore.
     * @return True if the actions successful, false otherwise.
     */
    bool IgnoreError(ErrorType error) noexcept override;

    /**
     * @brief Check if error is set.
     * @param error The error to check.
     * @return True if the error is set, false otherwise.
     */
     bool IsErrorSet(ErrorType error) noexcept override;

    /**
     * @brief Check if an error is set.
     * @param error The error to check.
     * @return True if the error is set, false otherwise.
     */
     bool IsAnyErrorSet() noexcept override;

    /**
     * @brief Check if an error is ignored.
     * @param error The error to check.
     * @return True if the error is ignored, false otherwise.
     */
     bool IsErrorIgnored(ErrorType error) noexcept override;

     /**
      * @brief Waits for a new error setting/clearing activities within timeout.
      * @param[in] waitTime The maximum time to wait for new error activity.
      * @return True of new error activity happened in specified timeout, false otherwise.
      */
     bool GetNewErrorActivity(ULONG waitTime = TX_WAIT_FOREVER) noexcept;


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
      * @brief Clears the New Data Event generated.
      * @return True if actions are true, false otherwise.
      */
     bool ClearNewDataEvent() noexcept;

     /**
      * @brief Gets the error getter owner thread.
      * @return Error getter owner thread pointer.
      */
     TX_THREAD* GetErrorGetterOwnerThread() noexcept;


     /**
      * @brief Gets the error setter owner thread.
      * @return Error setter owner thread pointer.
      */
     TX_THREAD* GetErrorSetterOwnerThread() noexcept;

     /**
      * @brief Prints what the error is currently set to.
      * @param error ErrorType wanting to print info of.
      */
     void PrintError( ErrorType error) noexcept;

     /**
      * @brief Prints all errors in a well-formatted manner.
      *
      * This function iterates through all possible errors, checks their status, and prints them to the console.
      * It ensures thread safety by using a mutex guard and only prints errors that are not in the Unknown state.
      *
	  * @param reason String indicating the reason or location of the call
      */
     void PrintAllErrors(const char* reason) noexcept;

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

    EnumeratedSetStatus<ErrorType, ErrorStatus, 2, N> errorStatus;    ///< Bitset to store error status

};

//==============================================================//
// VERBOSE??
//==============================================================//

template <typename ErrorType, size_t N>
const bool ErrorSaver<ErrorType,N>::verbose = false;

//==============================================================//
//==============================================================//

template <typename ErrorType, size_t N>
const char ErrorSaver<ErrorType,N>::mutexName[] = "ErrorSaver-Mutex";

template <typename ErrorType, size_t N>
const char ErrorSaver<ErrorType,N>::dataAvailableFlagGroupName[] = "ErrorSaver-EventFlagGroup";

/**
 * @brief Constructor for ErrorSaver.
 * @param errorToStringFunction Function to convert error to string.
 */
template <typename ErrorType, size_t N>
ErrorSaver<ErrorType,N>::ErrorSaver(const char* (*enumToStringConverter)( ErrorType )) :
	dataSetterOwnerThread(nullptr),
	dataGetterOwnerThread(nullptr),
	eventFlagGroupCreated(false),
	mutex( mutexName),
	initialized(false),
	errorStatus( ErrorStatus::Unknown, enumToStringConverter,  &ErrorStatusToString )  // Each error starts with an unknown status
{
	/// No code at this time.
}

/**
 * @brief Destructor for ErrorSaver.
 */
template <typename ErrorType, size_t N>
ErrorSaver<ErrorType,N>::~ErrorSaver()
{
  // No code at this time
}

/**
 * @brief Function to initialize class.
 * Generally, the initialize function should return false rather than causing a low-level fault.
 *
 * @return true if able to initialize, false otherwise.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::Initialize() noexcept
{
	bool success = false;

	MutexGuard guard(mutex, MutexGuard::MaxInitializationTimeMsec, &success);  // Create the mutex and lock it, check for success

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
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::SetSetterOwnerThreadTo(TX_THREAD* dataSetterOwnerThreadArg) noexcept {
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
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::SetGettersOwnerThreadTo(TX_THREAD* dataGetterOwnerThreadArg) noexcept {
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
template <typename ErrorType, size_t N>
TX_THREAD* ErrorSaver<ErrorType,N>::GetErrorGetterOwnerThread() noexcept{
	return dataGetterOwnerThread;
}

/**
 * @brief Returns owner thread.
 *
 * @return Owner thread pointer.
 */
template <typename ErrorType, size_t N>
TX_THREAD* ErrorSaver<ErrorType,N>::GetErrorSetterOwnerThread() noexcept {
	return dataSetterOwnerThread;
}

/**
 * @brief Waits for a new error setting/clearing activities within timeout.
 * @param[in] waitTime The maximum time to wait for new error activity.
 * @return True of new error activity happened in specified timeout, false otherwise.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::GetNewErrorActivity(ULONG waitTime) noexcept {
    ULONG actual_flags;

    UINT status = tx_event_flags_get(&dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, TX_OR_CLEAR, &actual_flags, waitTime);

    if (status == TX_SUCCESS && (actual_flags & DATA_AVAILABLE_FLAG)) {
    	return true;
    }

	/// If all failed
    return false;
}

/**
 * @brief Set an error. Can only be set by the variable owner thread.
 * @param error The error to set.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::SetError(ErrorType error) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already set, Set the error in a mutex protected manner
    		if( !errorStatus.IsStatus(error, ErrorStatus::Set) ) {
				/// Set the error in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				errorStatus.Set(error, ErrorStatus::Set);
				PrintError(error);

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
 * @brief Clear an error.
 * @param error The error to clear.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::ClearError(ErrorType error) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already cleared, clear the error in a mutex protected manner
    		if( !errorStatus.IsStatus(error, ErrorStatus::Cleared) ) {
				/// Set the error in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				errorStatus.Set(error, ErrorStatus::Cleared );
				PrintError(error);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::SetUnknown(ErrorType error) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already cleared, clear the error in a mutex protected manner
    		if( !errorStatus.IsStatus(error, ErrorStatus::Unknown) ) {
				/// Set the error in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				errorStatus.Set(error, ErrorStatus::Unknown );
				PrintError(error);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

template <typename ErrorType, size_t N> bool ErrorSaver<ErrorType,N>::SetAllUnknown() noexcept
{
    if( EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify())
    	{
    		MutexGuard guard(mutex);

    		/// if not already cleared, clear the error in a mutex protected manner
    		errorStatus.SetAll( ErrorStatus::Unknown);
    		return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    	}
    }

    return false;
}

/**
 * @brief Ignore an error.
 * @param error The error to ignore.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::IgnoreError(ErrorType error) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// if not already ignored, ignore the error in a mutex protected manner
    		if( !errorStatus.IsStatus(error, ErrorStatus::Ignored) ) {
				/// Set the error in a mutex protected manner
				MutexGuard guard((TX_MUTEX*)&mutex);
				errorStatus.Set(error, ErrorStatus::Ignored);
				PrintError(error);

				/// Signal that new data is available
				return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, true);
    		}

			/// If not a new event, just continue
			return (true);
    	}
    }

    return false;
}

template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::IsErrorSet(ErrorType error) noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

		    return errorStatus.IsStatus(error, ErrorStatus::Set);
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Check if an error is set.
 * @param error The error to check.
 * @return True if the error is set, false otherwise.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::IsAnyErrorSet() noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

			 return errorStatus.IsAny(ErrorStatus::Set);
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Check if an error is ignored.
 * @param error The error to check.
 * @return True if the error is ignored, false otherwise.
 */
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::IsErrorIgnored(ErrorType error) noexcept
{
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

		    return errorStatus.IsStatus(error, ErrorStatus::Ignored);
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
template <typename ErrorType, size_t N>
bool ErrorSaver<ErrorType,N>::ClearNewDataEvent() noexcept {
    return ClearTxEventFlags(dataAvailableFlagGroup, ~DATA_AVAILABLE_FLAG, true);
}

/**
 * @brief Print an error.
 * @param error The error to print.
 */
template <typename ErrorType, size_t N>
void ErrorSaver<ErrorType,N>::PrintError( ErrorType error) noexcept
{
	if( verbose )
	{
		ErrorStatus status = errorStatus.Get( error );
		ConsolePort::Write( "ErrorSaver::PrintError() - Entry %s(%d) set to %s(%d).",
		  errorStatus.ToEnumerationString(error), std::to_underlying(error), errorStatus.ToStatusString(status), std::to_underlying(status) );
		TxDelayMsec ( 2 );
	}
}

template <typename ErrorType, size_t N>
void ErrorSaver<ErrorType, N>::PrintAllErrors(const char* reason) noexcept {
    if (EnsureInitialized()) {
		MutexGuard guard(mutex);

		ConsolePort::GetInstance().NewLine();
		ConsolePort::GetInstance().Write( "==||=======================================================||==");
		ConsolePort::GetInstance().Write( "==||***  ErrorSaver DATA: ErrorSaver::PrintAllErrors()  ***||==");
		ConsolePort::GetInstance().Write( "==||=======================================================||==");
		ConsolePort::GetInstance().Write( "ErrorSaver::PrintAllErrors() - %s", reason );

        for (size_t i = 0; i < N; ++i) {
            ErrorType error = static_cast<ErrorType>(i);
            ErrorStatus status = errorStatus.Get(error);

            // Print the error only if it is not in the Unknown state
            if (/*status != ErrorStatus::Unknown*/ true) {
                ConsolePort::Write(
                	"	- Error: %-55s (%2d) - Status: %-10s (%d)",
                    errorStatus.ToEnumerationString(error),
                    static_cast<int>(error),
                    errorStatus.ToStatusString(status),
                    static_cast<int>(status)
                );
            }
        }

		ConsolePort::GetInstance().Write( "==||=======================================================||==");
		ConsolePort::GetInstance().NewLine();

    }
}

#endif /* UTILITIES_COMMON_ERRORSAVER_H_ */
