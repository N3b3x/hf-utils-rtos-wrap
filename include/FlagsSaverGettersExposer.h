/**
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_FLAGSSAVERGETTERSEXPOSER_H_
#define UTILITIES_COMMON_FLAGSSAVERGETTERSEXPOSER_H_

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
template <typename FlagsType>
class FlagsSaverGettersExposer {
public:
	/**
	 * @brief Destructor.
	 *
	 * This destructor is called when an instance of the FlagsSaverGettersExposer class is destroyed.
	 */
	virtual ~FlagsSaverGettersExposer() noexcept = default;

    /**
     * @brief Check if flag is set.
     * @param flag The flag to check.
     * @return True if the flag is set, false otherwise.
     */
    virtual bool IsFlagSet(FlagsType flag) noexcept = 0;

    /**
     * @brief Check if an flag is set.
     * @param flag The flag to check.
     * @return True if the flag is set, false otherwise.
     */
    virtual bool IsAnyFlagsSet() noexcept = 0;

    /**
     * @brief Check if an flag is ignored.
     * @param flag The flag to check.
     * @return True if the flag is ignored, false otherwise.
     */
    virtual bool IsFlagUnknown(FlagsType flag) noexcept = 0;

    /**
     * @brief Waits for a new flag setting/clearing activities within timeout.
     * @param[in] waitTime The maximum time to wait for new flag activity.
     * @return True of new flag activity happened in specified timeout, false otherwise.
     */
    virtual bool GetNewFlagsActivity(ULONG waitTime = TX_WAIT_FOREVER) noexcept = 0;
};

#endif /* UTILITIES_COMMON_FLAGSSAVERGETTERSEXPOSER_H_ */
