/**
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 */

#ifndef UTILITIES_COMMON_ERRORSAVERSETTERSEXPOSER_H_
#define UTILITIES_COMMON_ERRORSAVERSETTERSEXPOSER_H_


/**
 * @brief A template class that exposes event-driven data.
 *
 * This class is designed to be a base class for other classes that need to expose
 * event-driven data in a multi-threaded environment. It provides several pure virtual
 * functions for getting new and recent data.
 *
 * @tparam ErrorType The type of data that the class exposes.
 */
template <typename ErrorType>
class ErrorSaverSettersExposer {
public:
	/**
	 * @brief Destructor.
	 *
	 * This destructor is called when an instance of the ErrorSaverSettersExposer class is destroyed.
	 */
	virtual ~ErrorSaverSettersExposer() noexcept = default;

	/**
	 * @brief Set an error. Can only be set by the variable owner thread.
	 * @param error The error to set.
	 */
    virtual bool SetError(ErrorType error) noexcept = 0;

    /**
     * @brief Clear an error.
     * @param error The error to clear.
     */
    virtual bool ClearError(ErrorType error) noexcept = 0;

    /**
     * @brief Sets error status as unknown.
     * @param error The error to ignore.
     */
    virtual bool SetUnknown(ErrorType error) noexcept = 0;

    /**
     * @brief Ignore an error.
     * @param error The error to ignore.
     */
    virtual bool IgnoreError(ErrorType error) noexcept = 0;

};

#endif /* UTILITIES_COMMON_ERRORSAVERSETTERSEXPOSER_H_ */
