/**
 *
 * Nebula Tech Corporation
 *
 * Copyright Â© 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_EVENTDRIVENDATAGETTERSEXPOSER_H_
#define UTILITIES_COMMON_EVENTDRIVENDATAGETTERSEXPOSER_H_

#include "UTILITIES/common/RtosCompat.h"

/**
 * @brief A template class that exposes event-driven data.
 *
 * This class is designed to be a base class for other classes that need to expose
 * event-driven data in a multi-threaded environment. It provides several pure virtual
 * functions for getting new and recent data.
 *
 * @tparam T The type of data that the class exposes.
 */
template <typename DataType>
class EventDrivenDataGettersExposer {
public:
	/**
	 * @brief Destructor.
	 *
	 * This destructor is called when an instance of the EventDrivenDataGettersExposer class is destroyed.
	 */
    virtual ~EventDrivenDataGettersExposer() noexcept = default;

    /**
     * @brief Get new data without a timestamp.
     *
     * This is a pure virtual function that must be overridden by derived classes.
     *
     * @param[out] outData The new data.
     * @param[in] waitTime The maximum time to wait for new data.
     * @return true if new data was successfully retrieved, false otherwise.
     */
    virtual bool GetNewData(DataType& outData, ULONG waitTime = TX_WAIT_FOREVER) noexcept = 0;

    /**
     * @brief Get new data with a timestamp.
     *
     * This is a pure virtual function that must be overridden by derived classes.
     *
     * @param[out] outData The new data.
     * @param[out] timestamp The timestamp of the new data.
     * @param[in] waitTime The maximum time to wait for new data.
     * @return true if new data was successfully retrieved, false otherwise.
     */
    virtual bool GetNewDataWT(DataType& outData, uint32_t& timestamp, ULONG waitTime = TX_WAIT_FOREVER) noexcept = 0;

    /**
     * @brief Get the most recent data without a timestamp.
     *
     * This is a pure virtual function that must be overridden by derived classes.
     *
     * @param[out] outData The most recent data.
     * @return true if recent data was successfully retrieved, false otherwise.
     */
    virtual bool GetRecentData(DataType& outData) noexcept = 0;

    /**
     * @brief Get the most recent data with a timestamp.
     *
     * This is a pure virtual function that must be overridden by derived classes.
     *
     * @param[out] outData The most recent data.
     * @param[out] timestamp The timestamp of the most recent data.
     * @return true if recent data was successfully retrieved, false otherwise.
     */
    virtual bool GetRecentDataWT(DataType& outData, uint32_t& timestamp) noexcept = 0;

    /**
     * @brief Gets the recent data if the data point is newer than timestampMsec specified
     * @param timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
     * @param outData Reference to variable of where data will be returned through.
     * @return True if data point newer than specifed timestamp and data has been stored in outData;
     */
    virtual bool GetRecentDataIfNewerThan(uint32_t timestampMsec, DataType& outData) noexcept = 0;

    /**
     * @brief Gets the recent data if the data point is newer than timestampMsec specified
     * @param timestampMsec Timestamp either acquired from GetEllapsedTimeMsec() or from last data point.
     * @param outData Reference to variable of where data will be returned through.
     * @param timestamp Reference to variable of where data timestamp will be returned through.
     * @return True if data point newer than specifed timestamp and data has been stored in outData;
     */
    virtual bool GetRecentDataIfNewerThanWT(uint32_t timestampMsec, DataType& outData, uint32_t& timestamp) noexcept = 0;

    /**
     * @brief Checking to see if most recent data is new than the specified ThreadX timestamp.
     * @param timestampMsec Timestamp acquired from GetElapsedTimeMsec()
     * @return True if recent data has a timestamp newer than timestamp specified, false otherwise.
     */
    virtual bool IsRecentDataNewerThanMsec(uint32_t timestampMsec) noexcept = 0;
};

#endif /* UTILITIES_COMMON_EVENTDRIVENDATAGETTERSEXPOSER_H_ */
