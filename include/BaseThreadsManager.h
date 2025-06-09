/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *
  * Contains the declaration of "singleton" class MwSubThreadsManager that can access and control
  * all HAL sub-threads.
  */

#ifndef UTILITIES_COMMON_BASETHREADSMANAGER_H_
#define UTILITIES_COMMON_BASETHREADSMANAGER_H_

#include <map>
#include <bitset>
#include <functional>

#include "OsAbstraction.h"

#include "ConsolePort.h"

#include "OsUtility.h"
#include "Utility.h"
#include "BaseThread.h"
#include "Mutex.h"
#include "MutexGuard.h"

//==============================================================//
// CLASS
//==============================================================//
template <typename EnumType, EnumType MaxCount> class BaseThreadsManager
{
	public:
		BaseThreadsManager(const std::map<EnumType, BaseThread*>& threads, std::function<const char*(EnumType)> enumToStringFunc) ;

		/**
		  * @brief The Copy constructor is deleted to avoid copying instances.
	  	  * @return n/a
		  */

		BaseThreadsManager(const BaseThreadsManager&) = delete;


	    /**
	      * @brief  The assignment operator constructor is deleted to avoid copying instances.
	      * @return n/a
		  */

		BaseThreadsManager& operator = (const BaseThreadsManager&) = delete ;

		virtual ~BaseThreadsManager();

		bool EnsureInitialized() noexcept;

		bool ResumeAll() noexcept;

		bool ResumeSelected(const std::vector<EnumType>& selectedEnums) noexcept;

		bool StartAll() noexcept;

		bool StartSelected(const std::vector<EnumType>& selectedEnums) noexcept;

		bool StartAllExceptSelected(const std::vector<EnumType>& selectedEnums) noexcept;

		bool StartAllAndWaitToVerify(uint32_t waitToVerifyTimeoutMsec) noexcept;

		bool StartSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept;

		bool StartAllExceptSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept;

		bool StopAll() noexcept;

		bool StopSelected(const std::vector<EnumType>& selectedEnums) noexcept;

		bool StopAllExceptSelected(const std::vector<EnumType>& selectedEnums) noexcept;

		bool StopAllAndWaitToVerify(uint32_t waitToVerifyTimeoutMsec) noexcept;

		bool StopSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept;

		bool StopAllExceptSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept;

		virtual bool PreThreadInitializationActions() noexcept;

		virtual bool PostThreadInitializationActions() noexcept;

	private:

		/**
		 * @brief Initializes the BaseThreadsManager.
		 * @return True if the initialization is successful, false otherwise.
		 */
		bool Initialize() noexcept;

		//==============================================================//
		// VERBOSE??
		//==============================================================//
		static constexpr bool verbose = false;
		static constexpr bool timeBetweenChecksMsec = 10;

		bool initialized; /**< Flag indicating if the MwSubThreadsManager is initialized. */

		Mutex mutex;
		static const char mutexName[];

		std::map<EnumType, BaseThread*> threadsManaged;
		std::function<const char*(EnumType)> enumToString;

		std::bitset<MaxCount> threadsInitializedTracker;
		std::bitset<MaxCount> threadsStartedTracker;
		std::bitset<MaxCount> threadsStoppedTracker;
};


template <typename EnumType, EnumType MaxCount>
BaseThreadsManager<EnumType, MaxCount>::BaseThreadsManager(const std::map<EnumType, BaseThread*>& threads, std::function<const char*(EnumType)> enumToStringFunc) :
	initialized(false),
	mutex(mutexName),
	threadsManaged(threads),
	enumToString(enumToStringFunc),
	threadsInitializedTracker(),
	threadsStartedTracker(),
	threadsStoppedTracker()
{
	/// No code at this time.
}

template <typename EnumType, EnumType MaxCount>
BaseThreadsManager<EnumType, MaxCount>::~BaseThreadsManager() {
	}

/**
 * @brief Ensures that the BaseThreadsManager is initialized.
 *
 * This function checks if the BaseThreadsManager has been initialized and initializes it if not.
 *
 * @return true if the BaseThreadsManager is initialized, false otherwise.
 */
template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::EnsureInitialized() noexcept
{
	if (!initialized)
	{
		initialized = Initialize();
	}
	return initialized;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::ResumeAll() noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::ResumeAll() - BaseThreads have been requested to Resume.");

		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::ResumeAll() - Resuming: %s", enumToString(threadMap.first));
			threadMap.second->Resume();
		}

		return true;
	}

	return false;
}
template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::ResumeSelected(const std::vector<EnumType>& selectedEnums) noexcept
{
	bool resumed = false;
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::ResumeSelected() - Selected BaseThreads have been requested to Resume.");

		for(const auto& enumValue : selectedEnums)
		{
			auto threadMap = threadsManaged.find(enumValue);
			if(threadMap != threadsManaged.end())
			{
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::ResumeSelected() - Resuming: %s", enumToString(threadMap->first));
				resumed = threadMap->second->Resume();
			}
		}
	}

	return resumed;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartAll() noexcept
{
	 if (EnsureInitialized() )
	 {
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAll() - BaseThreads have been requested to start.");

		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAll() - Starting: %s", enumToString(threadMap.first));
			threadsStartedTracker[threadMap.first] = threadMap.second->Start();
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAll() - All starts commanded, NOT indicated to wait to verify if they all started.");

		return threadsStartedTracker.all();
	}

	return false;
}


template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartSelected(const std::vector<EnumType>& selectedEnums) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartSelected() - Selected BaseThreads have been requested to start.");

		bool allSelectedStarted = true;
		for(const auto& enumValue : selectedEnums) {
			auto threadMap = threadsManaged.find(enumValue);
			if(threadMap != threadsManaged.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelected() - Starting: %s", enumToString(threadMap->first));
				bool started = threadMap->second->Start();
				allSelectedStarted = allSelectedStarted && started;
				threadsStartedTracker[threadMap->first] = started;
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartSelected() - All selected starts commanded, NOT indicated to wait to verify if they all started.");

		return allSelectedStarted;
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartAllExceptSelected(const std::vector<EnumType>& selectedEnums) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllExceptSelected() - All BaseThreads except selected have been requested to start.");

		bool allExceptSelectedStarted = true;
		/// Go through all threads managed
		for(const auto& threadMap : threadsManaged) {
			/// If it's not in the selected threads to not start
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				/// Start Thread
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelected() - Starting: %s", enumToString(threadMap.first));
				bool started = threadMap.second->Start();
				allExceptSelectedStarted = allExceptSelectedStarted && started;
				threadsStartedTracker[threadMap.first] = started;
			} else {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelected() - Not Starting (Selected): %s", enumToString(threadMap.first));
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllExceptSelected() - All except selected starts commanded, NOT indicated to wait to verify if they all started.");

		return allExceptSelectedStarted;
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartAllAndWaitToVerify(uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllAndWaitToVerify() - BaseThreads have been requested to start.");

		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "Starting: %s", enumToString(threadMap.first));
			threadMap.second->Start();
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllAndWaitToVerify() - All starts commanded, waiting to verify all started within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the thread has been started
		std::function<bool()> CheckIfAllHalSubThreadsAreStarted = [this]() {
			for(auto& threadMap : threadsManaged) {
				threadsStartedTracker[threadMap.first] = threadMap.second->IsThreadRunning();
			}
			return threadsStartedTracker.all();
		};

		const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllHalSubThreadsAreStarted, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - BaseThreads Manager SUCCEEDED to start all subthreads within Timeout: [%u] Msec.", elapsedStartTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to start all subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - //============================================//");
		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - Subthread - %s - %s STARTED.", enumToString(threadMap.first), (threadsStartedTracker[threadMap.first] ? "":"NOT"));
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartSelectedAndWaitToVerify() - Selected BaseThreads have been requested to start.");

		for(const auto& enumValue : selectedEnums) {
			auto threadMap = threadsManaged.find(enumValue);
			if(threadMap != threadsManaged.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - Starting: %s", enumToString(threadMap->first));
				threadMap->second->Start();
			}
		}


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartSelectedAndWaitToVerify() - All starts commanded, waiting to verify all started within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the specified threads has been started
		std::function<bool()> CheckIfAllSelectedThreadsAreStarted = [this, selectedEnums] {
			/// Get if the selected threads are running
			for(const auto& enumValue : selectedEnums) {
				threadsStartedTracker[enumValue] = threadsManaged[enumValue]->IsThreadRunning();
			}
			/// Check if all specified subthreads have started
			for(const auto& enumValue : selectedEnums) {
				if(!threadsStartedTracker[enumValue]) {
					return false;
				}
			}
			return true;
		};

		const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllSelectedThreadsAreStarted, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - BaseThreads Manager SUCCEEDED to start all selected subthreads within Timeout: [%u] Msec.", elapsedStartTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to start all selected subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - //============================================//");
		for(const auto& enumValue : selectedEnums) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - Subthread - %s - %s STARTED.", enumToString(enumValue), (threadsStartedTracker[enumValue] ? "":"NOT"));
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartSelectedAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StartAllExceptSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - All BaseThreads except selected have been requested to start.");

		/// For all threads managed
		for(const auto& threadMap : threadsManaged) {
			/// If they're not in the selected threads list indicated not to start
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				/// Start threads
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - Starting: %s", enumToString(threadMap.first));
				threadMap.second->Start();
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - All except selected starts commanded, waiting to verify all started within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the specified threads has been started
		std::function<bool()> CheckIfAllExceptSelectedThreadsAreStarted = [this, selectedEnums] {
			/// Get if the threads not selected are running
			for(const auto& threadMap : threadsManaged) {
				if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
					threadsStartedTracker[threadMap.first] = threadMap.second->IsThreadRunning();
				}
			}
			/// Check if all threads not selected have started
			for(const auto& threadMap : threadsManaged) {
				if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
					if(!threadsStartedTracker[threadMap.first]) {
						return false;
					}
				}
			}
			return true;
		};

		const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllExceptSelectedThreadsAreStarted, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - BaseThreads Manager SUCCEEDED to start all except selected subthreads within Timeout: [%u] Msec.", elapsedStartTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to start all except selected subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - //============================================//");
		for(const auto& threadMap : threadsManaged) {
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - Subthread - %s - %s STARTED.", enumToString(threadMap.first), (threadsStartedTracker[threadMap.first] ? "":"NOT"));
			}
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StartAllExceptSelectedAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopAll() noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAll() - BaseThreads have been requested to stop.");

		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAll() - Stopping: %s", enumToString(threadMap.first));
			threadsStoppedTracker[threadMap.first] = threadMap.second->Stop();
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAll() - All stops commanded, NOT indicated to wait in order to verify if they all stopped.");

		return threadsStoppedTracker.all();
	}

	return false;
}
template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopSelected(const std::vector<EnumType>& selectedEnums) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopSelected() - Selected BaseThreads have been requested to stop.");

		bool allSelectedStopped = true;
		for(const auto& enumValue : selectedEnums) {
			auto threadMap = threadsManaged.find(enumValue);
			if(threadMap != threadsManaged.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelected() - Stopping: %s", enumToString(threadMap->first));
				bool stopped = threadMap->second->Stop();
				allSelectedStopped = allSelectedStopped && stopped;
				threadsStoppedTracker[threadMap->first] = stopped;
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopSelected() - All selected stops commanded, NOT indicated to wait in order to verify if they all stopped.");

		return allSelectedStopped;
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopAllExceptSelected(const std::vector<EnumType>& selectedEnums) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);


		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllExceptSelected() - All BaseThreads except selected have been requested to stop.");

		bool allExceptSelectedStopped = true;
		for(const auto& threadMap : threadsManaged) {
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelected() - Stopping: %s", enumToString(threadMap.first));
				bool stopped = threadMap.second->Stop();
				allExceptSelectedStopped = allExceptSelectedStopped && stopped;
				threadsStoppedTracker[threadMap.first] = stopped;
			} else {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelected() - Not Stopping (Selected): %s", enumToString(threadMap.first));
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllExceptSelected() - All except selected stops commanded, NOT indicated to wait in order to verify if they all stopped.");

		return allExceptSelectedStopped;
	}

	return false;
}
template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopAllAndWaitToVerify(uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllAndWaitToVerify() - BaseThreads has been requested to stop.");

		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - Stopping: %s", enumToString(threadMap.first));
			threadsStoppedTracker[threadMap.first] = threadMap.second->Stop();
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllAndWaitToVerify() - All stops commanded, waiting to verify all stopped within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the thread has been started
		std::function<bool()> CheckIfAllHalSubThreadsAreStopped = [this]() {
			for(auto& threadMap : threadsManaged) {
				threadsStoppedTracker[threadMap.first] = threadMap.second->IsThreadStopped();
			}
			return threadsStoppedTracker.all();
		};

		/// Get the time the waiting is starting
		const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllHalSubThreadsAreStopped, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		/// Get total elapsed time in logic test function
		const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - //============================================//");
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - BaseThreads Manager SUCCEEDED to stop all subthreads within Timeout: [%u] Msec.", elapsedStartTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to stop all subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - //============================================//");
		for(auto& threadMap : threadsManaged) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - Subthread - %s - %s STOPPED.", enumToString(threadMap.first), (threadsStoppedTracker[threadMap.first] ? "":"NOT"));
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);
;

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopSelectedAndWaitToVerify() - Selected BaseThreads have been requested to stop.");

		for(const auto& enumValue : selectedEnums) {
			auto threadMap = threadsManaged.find(enumValue);
			if(threadMap != threadsManaged.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - Stopping: %s", enumToString(threadMap->first));
				threadsStoppedTracker[threadMap->first] = threadMap->second->Stop();
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopSelectedAndWaitToVerify() - All stops commanded, waiting to verify all stopped within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the specified threads has been stopped
		std::function<bool()> CheckIfAllSelectedThreadsAreStopped = [this, selectedEnums] {
			for(const auto& enumValue : selectedEnums) {
				threadsStoppedTracker[enumValue] = threadsManaged[enumValue]->IsThreadStopped();
			}
			for(const auto& enumValue : selectedEnums) {
				if(!threadsStoppedTracker[enumValue]) {
					return false;
				}
			}
			return true;
		};

		const uint32_t waitStartTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllSelectedThreadsAreStopped, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		const uint32_t elapsedStartTimeMsec = os_get_elapsed_time_msec() - waitStartTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - BaseThreads Manager SUCCEEDED to stop all selected subthreads within Timeout: [%u] Msec.", elapsedStartTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to stop all selected subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - //============================================//");
		for(const auto& enumValue : selectedEnums) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - Subthread - %s - %s STOPPED.", enumToString(enumValue), (threadsStoppedTracker[enumValue] ? "":"NOT"));
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopSelectedAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}


template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::StopAllExceptSelectedAndWaitToVerify(const std::vector<EnumType>& selectedEnums, uint32_t waitToVerifyTimeoutMsec) noexcept
{
	if (EnsureInitialized() )
	{
		MutexGuard guard(mutex);

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - All BaseThreads except selected have been requested to stop.");

		/// For all threads managed
		for(const auto& threadMap : threadsManaged) {
			/// If they're not in the selected threads list indicated not to start
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				/// Start threads
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - Stopping: %s", enumToString(threadMap.first));
				threadMap.second->Stop();
			} else {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - Not Stopping (Selected): %s", enumToString(threadMap.first));
			}
		}

		ConsolePort::WriteConditional(verbose,"BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - All except selected stop commanded, waiting to verify all stopped within %u.", waitToVerifyTimeoutMsec);

		/// Function that will test if the specified threads has been stopped
		std::function<bool()> CheckIfAllExceptSelectedThreadsAreStopped = [this, selectedEnums] {
			/// Get if the threads not selected are stopped
			for(const auto& threadMap : threadsManaged) {
				if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
					threadsStoppedTracker[threadMap.first] = threadMap.second->IsThreadStopped();
				}
			}
			/// Check if all threads not selected have started
			for(const auto& threadMap : threadsManaged) {
				if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
					if(!threadsStoppedTracker[threadMap.first]) {
						return false;
					}
				}
			}
			return true;
		};

		const uint32_t waitStopTimeMsec = os_get_elapsed_time_msec();

		/// Expecting true within the timeout specified
		bool result = TestLogicWithTimeout(CheckIfAllExceptSelectedThreadsAreStopped, true, waitToVerifyTimeoutMsec, timeBetweenChecksMsec);

		const uint32_t elapsedStopTimeMsec = os_get_elapsed_time_msec() - waitStopTimeMsec;

		/// Print info to user.
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - //============================================//");

		if (result) {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - BaseThreads Manager SUCCEEDED to stop all except selected subthreads within Timeout: [%u] Msec.", elapsedStopTimeMsec);
		}
		else {
			ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - BaseThreads Manager !!!!! FAILED !!!!! to stop all except selected subthreads within Timeout: [%u] Msec.",  waitToVerifyTimeoutMsec);
		}

		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - //============================================//");
		for(const auto& threadMap : threadsManaged) {
			if(std::find(selectedEnums.begin(), selectedEnums.end(), threadMap.first) == selectedEnums.end()) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - Subthread - %s - %s STOPPED.", enumToString(threadMap.first), (threadsStoppedTracker[threadMap.first] ? "":"NOT"));
			}
		}
		ConsolePort::WriteConditional(verbose, "BaseThreadsManager::StopAllExceptSelectedAndWaitToVerify() - //============================================//");

		return result; /// Return result
	}

	return false;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::PreThreadInitializationActions() noexcept {
	return true;
}

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::PostThreadInitializationActions() noexcept {
	return true;
}

/**
 * @brief Initializes the BaseThreadsManager.
 * @return True if the initialization is successful, false otherwise.
 */

template <typename EnumType, EnumType MaxCount>
bool BaseThreadsManager<EnumType, MaxCount>::Initialize() noexcept
{
	bool success = false;
	MutexGuard guard(mutex, &success);

	if( success )
	{
		/// If pre thread initialization actions fail, return false
		if( !PreThreadInitializationActions() ) { return false; }

		for(auto& threadMap : threadsManaged) {
			bool threadInitialized = threadMap.second->EnsureInitialized();
			threadsInitializedTracker[threadMap.first] = threadInitialized;

			if(threadInitialized) {
				ConsolePort::WriteConditional(verbose, "BaseThreadsManager::Initialize() - Initialized: %s", enumToString(threadMap.first));
			}
			else {
			//	ConsolePort::WriteConditional(verbose, "BaseThreadsManager::Initialize() - Failed to initialize: %s", enumToString(threadMap.first));
				ConsolePort::Write("BaseThreadsManager::Initialize() - Failed to initialize: %s", enumToString(threadMap.first));
				os_delay_msec( 5 );
			}
		}

		/// If post thread initialization actions fail, return false
		if( !PostThreadInitializationActions() ) { return false; }

		/// Otherwise, see if mutex and all threads managed are initialized.
	}
	return success && threadsInitializedTracker.all();
}



template <typename EnumType, EnumType MaxCount>
const char BaseThreadsManager<EnumType, MaxCount>::mutexName[] = "BaseThreadManager-Mutex";

#endif /* UTILITIES_COMMON_BASETHREADSMANAGER_H_ */
