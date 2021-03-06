// Copyright (c) 2017, Advanced Realtime Tracking GmbH
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeCounter.h"
#include "Helper/SpaceConverter.h"

#include <memory> 
#include <string>

class FDTrackPollThread;
class DTrackSDK;
class DTrackComponent; 
class FDTrackPlugin;

/** @brief thread encapsulating all ART SDK interaction
 * A runnable object is an object that is "run" on an arbitrary thread. Which is created inside this constructor
 */
class FDTrackPollThread : public FRunnable {

public:
	FDTrackPollThread(const UDTrackComponent *n_client, FDTrackPlugin *n_plugin);
	~FDTrackPollThread();
		
	/** Singleton instance, can access the thread any time via static accessor
		*/
	static FDTrackPollThread *m_runnable;

	/**	Start the thread and the worker from static
		This function returns a handle to the newly started instance.
		*/
	static FDTrackPollThread* start(const UDTrackComponent *n_client, FDTrackPlugin *n_plugin);
	
	void interrupt();
	void join();

public:
	////////////////////
	// FRunnable
	////////////////////

	/// does nothing, SDK is initialized in run
	bool Init() override;

	/// Is called once when the Init was successful
	/// @return The exit code of the runnable object
	///			1 is success,  0 is failure
	uint32 Run() override;

	/// This is called if a thread is requested to terminate early.
	void Stop() override;

	/// empty teardown, Run() will cleanup
	void Exit() override;

private:

	/// after receive, treat body info and send it to the plug-in
	void handle_bodies();

	/// after receive, treat flystick info and send it to the plug-in
	void handle_flysticks();

	/// treat hand tracking info and send it to the plug-in
	void handle_hands();

	/// treat human model tracking info and send it to the plug-in
	void handle_human_model();

private:

	FRunnableThread   *m_thread;       //!< Thread to run the worker FRunnable on
	FThreadSafeCounter m_stop_counter; //!< atomic stop counter
	FDTrackPlugin     *m_plugin;       //!< during runtime, plugin gets data injected


	/// this is the DTrack SDK main object. I'll have one one owned here as I do not know if they can coexist
	std::unique_ptr< DTrackSDK > m_dtrack;

	/// parameters 
	const bool                   m_dtrack2;
	const std::string            m_dtrack_server_ip;
	const uint32                 m_dtrack_client_port;

private:

	FSpaceConverter m_space_converter;

public:

	/// a thread safe counter to use so unique names for the threads can be created
	static FThreadSafeCounter m_UniqueNameCounter;

public:
	/// some test function to kill the thread... or check whether its still active...
	FRunnableThread * GetRunnableThread() {	// todo do not use in production
		return m_thread;
	}
};