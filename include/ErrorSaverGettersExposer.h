/*
 * ErrorSaverGettersExposer.h
 *
 *  Created on: Feb 7, 2024
 *      Author: ntadesse
 */

#ifndef UTILITIES_COMMON_ERRORSAVERGETTERSEXPOSER_H_
#define UTILITIES_COMMON_ERRORSAVERGETTERSEXPOSER_H_

/**
 * @brief A template class that exposes event-driven data.
 *
 * This class is designed to be a base class for other classes that need to expose
 * event-driven data in a multi-threaded environment. It provides several pure virtual
 * functions for getting new and recent data.
 *
 * @tparam T The type of data that the class exposes.
 */
template <typename ErrorType>
class ErrorSaverGettersExposer {
public:
	/**
	 * @brief Destructor.
	 *
	 * This destructor is called when an instance of the ErrorSaverGettersExposer class is destroyed.
	 */
	virtual ~ErrorSaverGettersExposer() noexcept = default;

    /**
     * @brief Check if error is set.
     * @param error The error to check.
     * @return True if the error is set, false otherwise.
     */
    virtual bool IsErrorSet(ErrorType error) noexcept = 0;

    /**
     * @brief Check if an error is set.
     * @param error The error to check.
     * @return True if the error is set, false otherwise.
     */
    virtual bool IsAnyErrorSet() noexcept = 0;

    /**
     * @brief Check if an error is ignored.
     * @param error The error to check.
     * @return True if the error is ignored, false otherwise.
     */
    virtual bool IsErrorIgnored(ErrorType error) noexcept = 0;

    /**
     * @brief Waits for a new error setting/clearing activities within timeout.
     * @param[in] waitTime The maximum time to wait for new error activity.
     * @return True of new error activity happened in specified timeout, false otherwise.
     */
    virtual bool GetNewErrorActivity(ULONG waitTime = TX_WAIT_FOREVER) noexcept = 0;
};

#endif /* UTILITIES_COMMON_ERRORSAVERGETTERSEXPOSER_H_ */
