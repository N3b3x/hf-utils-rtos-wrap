/**
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_FLAGSSAVERSETTERSEXPOSER_H_
#define UTILITIES_COMMON_FLAGSSAVERSETTERSEXPOSER_H_

#include "UTILITIES/common/RtosCompat.h"

/**
 * @brief A template class that exposes event-driven data.
 *
 * This class is designed to be a base class for other classes that need to expose
 * event-driven data in a multi-threaded environment. It provides several pure virtual
 * functions for setting, clearing, and marking flags as unknown.
 *
 * @tparam FlagsType The type of data that the class exposes.
 */
template <typename FlagsType>
class FlagsSaverSettersExposer {
public:
    /**
     * @brief Destructor.
     *
     * This destructor is called when an instance of the FlagsSaverSettersExposer class is destroyed.
     */
    virtual ~FlagsSaverSettersExposer() noexcept = default;

    /**
     * @brief Set a flag. Can only be set by the variable owner thread.
     * @param flag The flag to set.
     */
    virtual bool SetFlag(FlagsType flag) noexcept = 0;

    /**
     * @brief Clear a flag.
     * @param flag The flag to clear.
     */
    virtual bool ClearFlag(FlagsType flag) noexcept = 0;

    /**
     * @brief Mark a flag as unknown.
     * @param flag The flag to mark as unknown.
     */
    virtual bool SetUnknown(FlagsType flag) noexcept = 0;
};

#endif /* UTILITIES_COMMON_FLAGSSAVERSETTERSEXPOSER_H_ */

