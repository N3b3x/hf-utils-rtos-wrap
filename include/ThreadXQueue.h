/**
 * @file ThreadXQueue.h
 * @brief Lightweight C++ wrapper for ThreadX/FreeRTOS queues.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * This header defines a templated queue class that lazily creates the
 * underlying RTOS queue and associated mutex the first time it is used. The
 * interface mimics a subset of the ThreadX queue API and is used across the
 * utility layer for inter-thread communication.
 */
#ifndef UTILITIES_COMMON_THREADXQUEUE_H_
#define UTILITIES_COMMON_THREADXQUEUE_H_

#include "UTILITIES/common/RtosCompat.h"

#include "HAL/component_handlers/ConsolePort.h"

#include "UTILITIES/common/ThingsToString.h"
#include "UTILITIES/common/Utility.h"
#include "UTILITIES/common/TxUtility.h"

/**
 * @class ThreadXQueue
 * @brief This class provides a C++ interface to a ThreadX queue.
 *
 * The ThreadX queue is lazily initialized the first time a message is sent or received.
 */
template <typename MessageType, size_t queueSizeBytes>
class ThreadXQueue {
public:
	/**
	 * @brief Construct a new ThreadXQueue object.
	 *
	 * The constructor does not initialize the ThreadX queue or the mutex.
	 * The queue and the mutex are initialized the first time a message is sent or received.
	 */
	ThreadXQueue(const char *queueName, uint32_t messageSizeInWordsArg) :
		initialized(false),
		queueCreated(false),
		mutexCreated(false),
		name(queueName),
		messageSizeInWords(messageSizeInWordsArg)
	{
		/// No code at this time.
	}

    /**
     * @brief Destroy the ThreadXQueue object.
     *
     * If the queue has been initialized, it is deleted.
     * If the mutex has been initialized, it is deleted.
     */
    ~ThreadXQueue() {
    	if (queueCreated) {
    		DeleteTxQueue(queue);
    	}
    	if (mutexCreated) {
    		DeleteTxMutex(mtx);
    	}
    }

	bool EnsureInitialized() noexcept
	{
		if (!initialized)
		{
			initialized = Initialize();
		}
		return initialized;
	}

    /**
     * @brief Send a message to the queue.
     *
     * If the queue has not been initialized, it is initialized before the message is sent.
     *
     * @param message The message to send.
     */
    bool Send(MessageType message, ULONG wait_option = TX_WAIT_FOREVER) noexcept {
        if (EnsureInitialized()) {
        	MutexGuard guard((TX_MUTEX*)&mtx);
        	return SendToTxQueue(queue, &message, wait_option);
        }
        ConsolePort::WriteConditional(verbose, "ThreadXQueue::Send() - [%s] Queue not initialized.", name);
        return false;
    }

    /**
     * @brief Receive a message from the queue.
     *
     * If the queue has not been initialized, it is initialized before the message is received.
     *
     * @return The received message.
     */
    bool Receive(MessageType& message, ULONG wait_option = TX_WAIT_FOREVER) noexcept {
        MessageType tmpMessage;
        if (EnsureInitialized()) {
        	MutexGuard guard((TX_MUTEX*)&mtx);
        	return ReceiveFromTxQueue(queue, &message, wait_option);
        }
        ConsolePort::WriteConditional(verbose, "ThreadXQueue::Receive() - [%s] Queue not initialized.", name);
        return false;
    }

private:
	bool Initialize() noexcept
	{
		if(!mutexCreated) {
			mutexCreated = CreateTxMutex(mtx, mutexName, TX_INHERIT);
		}

		if(!queueCreated) {
			queueCreated = CreateTxQueue(queue, name, messageSizeInWords, queue_storage, sizeof(queue_storage));
		}
		return mutexCreated && queueCreated;
	}

    bool initialized; ///< Whether the queue has been initialized

    TX_QUEUE queue; 	///< The ThreadX queue
    bool queueCreated;
    const char *name;

    TX_MUTEX mtx;
    static const char mutexName[];
    bool mutexCreated;

    MessageType queue_storage[queueSizeBytes]; ///< The storage for the queue

    uint32_t messageSizeInWords;
	//==============================================================//
	// VERBOSE??
	//==============================================================//
	static constexpr bool verbose = true;

};

template <typename MessageType, size_t queueSizeBytes>
const char ThreadXQueue<MessageType, queueSizeBytes>::mutexName[] = "ThreadXQueue-Mutex";

#endif /* UTILITIES_COMMON_THREADXQUEUE_H_ */
