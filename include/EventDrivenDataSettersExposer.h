/**
 *
 * Nebula Tech Corporation
 *
 * Copyright Â© 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_EVENTDRIVENDATASETTERSEXPOSER_H_
#define UTILITIES_COMMON_EVENTDRIVENDATASETTERSEXPOSER_H_

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
class EventDrivenDataSettersExposer {
public:
	/**
	 * @brief Destructor.
	 *
	 * This destructor is called when an instance of the EventDrivenDataGettersExposer class is destroyed.
	 */
    virtual ~EventDrivenDataSettersExposer() noexcept = default;

    /**
     * @brief Sets the data to the specified value if the owner thread is the caller.
     *
     * If no setter owner thread has been set, whatever thread can mutex-protected access
     * data and set the value.
     *
     * @param value Value to be saved.
     * @return True if data was set to value, false otherwise.
     */
    virtual bool SetData(const DataType& value) noexcept = 0;
};

#endif /* UTILITIES_COMMON_EVENTDRIVENDATASETTERSEXPOSER_H_ */
