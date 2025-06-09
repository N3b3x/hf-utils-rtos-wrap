/**
 * @file       EventDrivenDataMultiThread.h
 * @brief      Template class for event-driven data synchronization in a ThreadX environment.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_EVENTDRIVENDATAMULTITHREAD_H_
#define UTILITIES_COMMON_EVENTDRIVENDATAMULTITHREAD_H_

#include "UTILITIES/common/RtosCompat.h"

#include "UTILITIES/common/EventDrivenDataGettersExposer.h"
#include "UTILITIES/common/EventDrivenDataSettersExposer.h"
#include "UTILITIES/common/TxUtility.h"
#include "UTILITIES/common/MutexGuard.h"

/**
 * @class EventDrivenDataMultiThread
 * @brief A templatized class that provides event-driven data synchronization.
 *
 * This class allows one thread to set data and other threads to retrieve it in a synchronized manner.
 * The retrieval thread can wait for the data to be available, with an optional timeout.
 * There's also the capability for the retrieval operation to be owned by a specific thread.
 *
 * @tparam DataType The type of data to be synchronized.
 */
template <typename DataType>
class EventDrivenDataMultiThread : public EventDrivenDataGettersExposer<DataType>, public EventDrivenDataSettersExposer<DataType>
{
public:
    /**
     * @brief Default constructor. Initializes the event flag.
     */
    EventDrivenDataMultiThread() noexcept;

    /**
     * @brief Destructor. Deletes the event flag.
     */
    virtual ~EventDrivenDataMultiThread() noexcept;


    /**
     * @brief Sets the data to the specified value if the owner thread is the caller.
     *
     * If no setter owner thread has been set, whatever thread can mutex-protected access
     * data and set the value.
     *
     * @param value Value to be saved.
     * @return True if data was set to value, false otherwise.
     */
    bool SetData(const DataType& value) noexcept;


    /**
     * @brief Get new data without a timestamp.
     *
     * This is a pure function that must be overridden by derived classes.
     *
     * @param[out] outData The new data.
     * @param[in] waitTime The maximum time to wait for new data.
     * @return true if new data was successfully retrieved, false otherwise.
     */
    bool GetNewData(DataType& outData, ULONG waitTime = TX_WAIT_FOREVER) noexcept override;

    /**
     * @brief Get new data with a timestamp.
     *
     * This is a pure function that must be overridden by derived classes.
     *
     * @param[out] outData The new data.
     * @param[out] timestamp The timestamp of the new data.
     * @param[in] waitTime The maximum time to wait for new data.
     * @return true if new data was successfully retrieved, false otherwise.
     */
    bool GetNewDataWT(DataType& outData, uint32_t& timestamp, ULONG waitTime = TX_WAIT_FOREVER) noexcept override;

    /**
     * @brief Get the most recent data without a timestamp.
     *
     * This is a pure function that must be overridden by derived classes.
     *
     * @param[out] outData The most recent data.
     * @return true if recent data was successfully retrieved, false otherwise.
     */
    bool GetRecentData(DataType& outData) noexcept override;

    /**
     * @brief Get the most recent data with a timestamp.
     *
     * This is a pure function that must be overridden by derived classes.
     *
     * @param[out] outData The most recent data.
     * @param[out] timestamp The timestamp of the most recent data.
     *
     * @return true if recent data was successfully retrieved, false otherwise.
     */
    bool GetRecentDataWT(DataType& outData, uint32_t& timestamp) noexcept override;

    /**
     * @brief Gets the recent data if the data point is newer than timestampMsec specified
     * @param[in] timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
     * @param[out] outData Reference to variable of where data will be returned through.
     * @param[out] timestamp The timestamp of the most recent data.
     * @return True if data point newer than specifed timestamp and data has been stored in outData;
     */
    bool GetRecentDataIfNewerThan(uint32_t timestampMsec, DataType& outData) noexcept override;

    /**
     * @brief Gets the recent data if the data point is newer than timestampMsec specified
     * @param timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
     * @param outData Reference to variable of where data will be returned through.
     * @return True if data point newer than specifed timestamp and data has been stored in outData;
     */
    bool GetRecentDataIfNewerThanWT(uint32_t timestampMsec, DataType& outData, uint32_t& timestamp) noexcept override;


    /**
     * @brief Checking to see if most recent data is new than the specified ThreadX timestamp.
     * @param timestampMsec Timestamp acquired from GetElapsedTimeMsec()
     * @return True if recent data has a timestamp newer than timestamp specified, false otherwise.
     */
    bool IsRecentDataNewerThanMsec(uint32_t timestampMsec) noexcept override;

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

private:

    /**
     * @brief Initialize the event driven data by creating event flags used.
     *
     * @return ssp_err_t - Error status, with SSP_SUCCESS indicating successful thread creation.
     */
    bool EnsureInitialized() noexcept
    {
        if (!initialized)
        {
            initialized = Initialize ();
        }
        return initialized;
    }

    /**
     * @brief Initializes class by creating needed resources.
     * @return True if actions successful, false otherwise.
     */
    bool Initialize() noexcept;

    //================================================================//

	TX_MUTEX mutex;
	bool mutexCreated;
    static const char mutexName[];

    bool initialized;           /**< Flag indicating that Event Driven Var has been initialized. */
    bool waitingForNewData;     /**< Flag indicating if a thread is currently waiting for data. */

    TX_EVENT_FLAGS_GROUP dataAvailableFlagGroup;  	/**< Event flag to signal data availability. */
    bool eventFlagGroupCreated;						/**< Flag indicating that event flag group has been created. */
    static const ULONG DATA_AVAILABLE_FLAG = 0x01;  /**< Named constant for the event flag value */
    static const char dataAvailableFlagGroupName[];

    TX_THREAD* dataSetterOwnerThread;
    TX_THREAD* dataGetterOwnerThread;

    DataType data;				/**< The synchronized data. */
    uint32_t dataTimestamp;  	/**< The synchronized data last updated timestamp. */
};

template <typename DataType>
const char EventDrivenDataMultiThread<DataType>::mutexName[] = "EventDrivenData-Mutex";

template <typename DataType>
const char EventDrivenDataMultiThread<DataType>::dataAvailableFlagGroupName[] = "EventDrivenData-EventFlagGroup";

/**
 * @brief Default constructor. Initializes the event flag.
 */
template <typename DataType>
EventDrivenDataMultiThread<DataType>::EventDrivenDataMultiThread() noexcept
: mutexCreated(false),
  initialized(false),
  waitingForNewData(false),
  eventFlagGroupCreated(false),
  dataSetterOwnerThread(nullptr),
  dataGetterOwnerThread(nullptr),
  dataTimestamp(0)
{
    /// No code at this time.
}

/**
 * @brief Destructor. Deletes the event flag.
 */
template <typename DataType>
EventDrivenDataMultiThread<DataType>::~EventDrivenDataMultiThread() noexcept {
    if(eventFlagGroupCreated) {
    	DeleteTxEventFlags(dataAvailableFlagGroup);
    }

	if(mutexCreated) {
		DeleteTxMutex(mutex);
	}
}

template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::Initialize() noexcept {
    if(!eventFlagGroupCreated) {
    	eventFlagGroupCreated = CreateTxEventFlags(dataAvailableFlagGroup, dataAvailableFlagGroupName);
    }

    if(!mutexCreated) {
        mutexCreated = CreateTxMutex(mutex, mutexName, TX_INHERIT);
    }

    return eventFlagGroupCreated && mutexCreated;
}

/**
 * @brief Sets the calling thread as the owner of the retrieval operation.
 *
 * If no other thread is currently waiting for data, this function sets the calling thread as the owner.
 *
 * @return True if successful in setting the owner, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::SetSetterOwnerThreadTo(TX_THREAD* dataSetterOwnerThreadArg) noexcept {
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
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::SetGettersOwnerThreadTo(TX_THREAD* dataGetterOwnerThreadArg) noexcept {
    if(dataGetterOwnerThreadArg != nullptr) {
    	/// If no thread is currently waiting for data, set the current thread as the owner
    	dataGetterOwnerThread = dataGetterOwnerThreadArg;
        return true;
    }
    return false;
}

/**
 * @brief Sets the data and signals its availability to any waiting threads.
 *
 * @param value The data value to set.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::SetData(const DataType& value) noexcept {
    if(EnsureInitialized())
    {
    	/// If no thread has been marked as the data setter owner thread or caller is owner
    	if(dataSetterOwnerThread == nullptr || dataSetterOwnerThread == tx_thread_identify()) {
    		/// Set the data in a mutex protected manner
			MutexGuard guard((TX_MUTEX*)&mutex);
			data = value;
			dataTimestamp = GetElapsedTimeMsec();

			/// Signal that new data is available
			return SetTxEventFlags(dataAvailableFlagGroup, DATA_AVAILABLE_FLAG);
    	}
    }

    return false;
}

/**
 * @brief Retrieves new data if available in specified period.
 *
 * If no owner thread is set, the calling thread becomes the owner. If an owner is already set,
 * only the owner thread can pend and retrieve new data when available.
 *
 * This function waits for the data to be available based on the provided wait time.
 * If the data is available within the wait time, it retrieves the data and returns true.
 * Otherwise, it returns false.
 *
 * @param[out] outData Reference to store the retrieved data.
 * @param[in] waitTime Time to wait for the data (default is TX_WAIT_FOREVER).
 *
 * @return True if data was successfully retrieved, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetNewData(DataType& outData, ULONG waitTime) noexcept {
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data getter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
            ULONG actual_flags;

			/// Allow thread to pend on getting flag
			UINT status = tx_event_flags_get(&dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, TX_OR_CLEAR, &actual_flags, waitTime);

			/// First check that data getter thread is still not owned, and the flag was what we were waiting for.
			if ((status == TX_SUCCESS) && (actual_flags & DATA_AVAILABLE_FLAG)) {
				/// if so, get recent data and return it.
				return GetRecentData(outData);
			}
		}
    }

	/// If all failed
    return false;
}

/**
 * @brief Get new data with a timestamp.
 *
 * This is a pure function that must be overridden by derived classes.
 *
 * @param[out] outData The new data.
 * @param[out] timestamp The timestamp of the new data.
 * @param[in] waitTime The maximum time to wait for new data.
 * @return true if new data was successfully retrieved, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetNewDataWT(DataType& outData, uint32_t& timestamp, ULONG waitTime) noexcept {
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data getter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
			ULONG actual_flags;

			/// Allow thread to pend on getting flag
			UINT status = tx_event_flags_get(&dataAvailableFlagGroup, DATA_AVAILABLE_FLAG, TX_OR_CLEAR, &actual_flags, waitTime);

			if (status == TX_SUCCESS && (actual_flags & DATA_AVAILABLE_FLAG)) {
				return GetRecentDataWT(outData, timestamp);
			}
    	}
    }

	/// If all failed
    return false;
}

/**
 * @brief Retrieves most recent the data.
 *
 * This function only retrieves most recently updated data
 *
 * @param[out] outData Reference to store the retrieved data.
 * @return True if data was successfully retrieved, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetRecentData(DataType& outData) noexcept {
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data getter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

			outData = data;
			return true;
    	}
    }

    /// Otherwise, just return false.
    return false;
}

/**
 * @brief Retrieves most recent data and timestamp of when it was updated.
 *
 * This function only retrieves most recently updated data and data_timestamp
 *
 * @param[out] outData Reference to store the retrieved data.
 * @param[out] timestamp Timestamp of when data was last updated.
 *
 * @return True if data was successfully retrieved, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetRecentDataWT(DataType& outData, uint32_t& timestamp) noexcept {
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data getter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

	        outData = data;
	        timestamp = dataTimestamp;
			return true;
    	}
    }

    /// Otherwise, just return false.
    return false;
}


/**
 * @brief Gets the recent data if the data point is newer than timestampMsec specified.
 *
 * @param[in] timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
 * @param[out] outData Reference to variable of where data will be returned through.
 *
 * @return True if data point newer than specifed timestamp and data has been stored in outData;
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetRecentDataIfNewerThan(uint32_t timestampMsec, DataType& outData) noexcept {
	uint32_t tempDataTimestampMsec;

	return GetRecentDataIfNewerThanWT(timestampMsec, outData, tempDataTimestampMsec);
}

/**
 * @brief Gets the recent data if the data point is newer than timestampMsec specified.
 *
 * @param[in] timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
 * @param[out] outData Reference to variable of where data will be returned through.
 * @param[out] outDataTimestamp Timestamp of when data was last updated.
 *
 * @return True if data point newer than specifed timestamp and data has been stored in outData;
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::GetRecentDataIfNewerThanWT(uint32_t timestampMsec, DataType& outData, uint32_t& outDataTimestamp) noexcept {
	DataType tempData;
	uint32_t tempDataTimestampMsec;

	if(GetRecentDataWT(tempData, tempDataTimestampMsec)) {
		if(tempDataTimestampMsec > timestampMsec) {
			outData = tempData;
			outDataTimestamp = tempDataTimestampMsec;
			return true;
		}
	}

	return false;
}

/**
 * @brief Checking to see if most recent data is new than the specified ThreadX timestamp.
 * @param timestampMsec Timestamp acquired from GetElapsedTimeMsec()
 * @return True if recent data has a timestamp newer than timestamp specified, false otherwise.
 */
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::IsRecentDataNewerThanMsec(uint32_t timestampMsec) noexcept {
    if(EnsureInitialized()) {
    	/// If no thread has been marked as the data getter owner thread or caller is owner
    	if(dataGetterOwnerThread == nullptr || dataGetterOwnerThread == tx_thread_identify()) {
    		/// Give protected access to the data
			MutexGuard guard((TX_MUTEX*)&mutex);

			/// Compare current data timestamp to specified timestamp.
			return (dataTimestamp > timestampMsec);
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
template <typename DataType>
bool EventDrivenDataMultiThread<DataType>::ClearNewDataEvent() noexcept {
    return ClearTxEventFlags(dataAvailableFlagGroup, ~DATA_AVAILABLE_FLAG);
}

#endif /* UTILITIES_COMMON_EVENTDRIVENDATAMULTITHREAD_H_ */
