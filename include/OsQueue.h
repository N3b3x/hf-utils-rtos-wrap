/**
 * @file OsQueue.h
 * @brief Lightweight C++ wrapper for Generic queues.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * This header defines a templated queue class that lazily creates the
 * underlying RTOS queue and associated mutex the first time it is used. The
 * interface mimics a subset of the OS queue API and is used across the
 * utility layer for inter-thread communication.
 */
#ifndef OS_QUEUE_H_
#define OS_QUEUE_H_

#include "OsAbstraction.h"
#include "Utility.h"
#include "OsUtility.h"

/**
 * @class OsQueue
 * @brief This class provides a C++ interface to a OS queue.
 *
 * The OS queue is lazily initialized the first time a message is sent or received.
 */
template <typename MessageType, size_t queueSizeBytes>
class OsQueue {
public:
	/**
	 * @brief Construct a new OsQueue object.
	 *
	 * The constructor does not initialize the OS queue or the mutex.
	 * The queue and the mutex are initialized the first time a message is sent or received.
	 */
	OsQueue(const char *queueName, uint32_t messageSizeInWordsArg) :
		initialized(false),
		queueCreated(false),
		mutexCreated(false),
		name(queueName),
		messageSizeInWords(messageSizeInWordsArg)
	{
		/// No code at this time.
	}

    /**
     * @brief Destroy the OsQueue object.
     *
     * If the queue has been initialized, it is deleted.
     * If the mutex has been initialized, it is deleted.
     */
    ~OsQueue() {
    	if (queueCreated) {
    		os_queue_delete_ex(queue);
    	}
    	if (mutexCreated) {
    		os_mutex_delete_ex(mtx);
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
    bool Send(MessageType message, OS_Ulong wait_option = OS_WAIT_FOREVER) noexcept {
        if (EnsureInitialized()) {
        	MutexGuard guard((OS_Mutex*)&mtx);
        	return os_queue_send_ex(queue, &message, wait_option);
        }
        return false;
    }

    /**
     * @brief Receive a message from the queue.
     *
     * If the queue has not been initialized, it is initialized before the message is received.
     *
     * @return The received message.
     */
    bool Receive(MessageType& message, OS_Ulong wait_option = OS_WAIT_FOREVER) noexcept {
        MessageType tmpMessage;
        if (EnsureInitialized()) {
        	MutexGuard guard((OS_Mutex*)&mtx);
        	return os_queue_receive_ex(queue, &message, wait_option);
        }
        return false;
    }

private:
	bool Initialize() noexcept
	{
		if(!mutexCreated) {
			mutexCreated = os_mutex_create_ex(mtx, mutexName, OS_INHERIT);
		}

		if(!queueCreated) {
			queueCreated = os_queue_create_ex(queue, name, messageSizeInWords, queue_storage, sizeof(queue_storage));
		}
		return mutexCreated && queueCreated;
	}

    bool initialized; ///< Whether the queue has been initialized

    OS_Queue queue; 	///< The OS queue
    bool queueCreated;
    const char *name;

    OS_Mutex mtx;
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
const char OsQueue<MessageType, queueSizeBytes>::mutexName[] = "OsQueue-Mutex";

#endif /* OS_QUEUE_H_ */
