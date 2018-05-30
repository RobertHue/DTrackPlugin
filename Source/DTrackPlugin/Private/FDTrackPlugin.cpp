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

#include "FDTrackPlugin.h"
#include "DTrackPollThread.h"
#include "Math/UnrealMathUtility.h"
#include "DTrackSDK.hpp"

IMPLEMENT_MODULE(FDTrackPlugin, DTrackPlugin)

DEFINE_LOG_CATEGORY(DTrackPluginLog);
DEFINE_LOG_CATEGORY(DTrackPollThreadLog);

#define LOCTEXT_NAMESPACE "DTrackPlugin"

#define PLUGIN_VERSION "1"


void FDTrackPlugin::StartupModule() {
	
	UE_LOG(DTrackPluginLog, Log, TEXT("Using DTrack Plugin, threaded version %s"), TEXT(PLUGIN_VERSION));

	m_front.reset(new DataBuffer);
	m_back.reset(new DataBuffer);
	m_injected.reset(new DataBuffer);
}

void FDTrackPlugin::ShutdownModule() {
	UE_LOG(LogTemp, Warning, TEXT("start ShutdownModule"));

	// Another wait for potential asyncs. 
	// Should be able to catch them but don't know how
	FPlatformProcess::Sleep(0.1);

	// we should have been stopped but what can you do?
	if (m_polling_thread) {
		UE_LOG(LogTemp, Warning, TEXT("polling thread interrupt"));
		m_polling_thread->interrupt();
		UE_LOG(LogTemp, Warning, TEXT("polling thread join"));
		m_polling_thread->join();
		UE_LOG(LogTemp, Warning, TEXT("cleanup pointers and containers"));
		delete m_polling_thread;
		m_polling_thread = nullptr;
	}

	m_front.reset();
	m_back.reset();
	m_injected.reset();
	UE_LOG(LogTemp, Warning, TEXT("finished ShutdownModule"));
}

void FDTrackPlugin::start_up(UDTrackComponent *n_client) {

	UE_LOG(LogTemp, Warning, TEXT("startup of FDTrackPlugin"));
	if (!m_polling_thread) {
		UE_LOG(LogTemp, Warning, TEXT("Create new Thread"));
		m_polling_thread = FDTrackPollThread::start(n_client, this);
	}

	if (n_client) {
		UE_LOG(LogTemp, Display, TEXT("Add Client: %s"), *n_client->GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Client is NULL :(")); 
	}
	// on error, the object is created but an error condition set within
	m_clients.Add(n_client);
}

void FDTrackPlugin::remove(class UDTrackComponent *n_client) {
	UE_LOG(LogTemp, Warning, TEXT("start FDTrackPlugin::remove"));

	UE_LOG(LogTemp, Warning, TEXT("remove all all added clients (aka the UDTrackComponents)"));
	m_clients.RemoveAll([&](const TWeakObjectPtr<UDTrackComponent> p) {
		return p.Get() == n_client;
	});

	// we have no reason to run anymore
	if (m_polling_thread && (m_clients.Num() == 0)) {
		UE_LOG(LogTemp, Warning, TEXT("polling thread interrupt"));
		m_polling_thread->interrupt();
		UE_LOG(LogTemp, Warning, TEXT("polling thread join"));
		m_polling_thread->join();
		UE_LOG(LogTemp, Warning, TEXT("cleanup pointers and containers"));
		delete m_polling_thread;
		m_polling_thread = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("end FDTrackPlugin::remove"));
}

FCriticalSection *FDTrackPlugin::swapping_mutex() {

	return &m_swapping_mutex;
}

void FDTrackPlugin::tick(const float n_delta_time, const UDTrackComponent *n_component) {

	if (!m_polling_thread) {
		return;
	}
	
	// only one component may cause an actual tick to save performance.
	// which one doesn't matter though. So I take any and store it
	if (!m_ticker.IsValid()) {
		for (TWeakObjectPtr<UDTrackComponent> c : m_clients) {
			if (c.IsValid()) {
				m_ticker = c;
				break;
			}
		}
	}
	
	// if this is the case, who calls us then? 
	checkf(m_ticker.IsValid(), TEXT("unknown ticker"));
	if (m_ticker.Get() != n_component) {
		return;
	}

	// iterate all registered components and call the interface methods upon them
	for (TWeakObjectPtr<UDTrackComponent> c : m_clients) {
		// components might get killed and created along the way.
		// I only operate those which seem to live OK
		UDTrackComponent *component = c.Get();
		if (component) {
			// now handle the different tracking types by calling the component
			
			UE_LOG(LogTemp, Display, TEXT("start handling of %s"), *component->GetName());
			handle_bodies(component);
			handle_flysticks(component);
			handle_hands(component);
			handle_human_model(component);
		}
	}
}

/************************************************************************/
/* Injection routines                                                   */
/* Called as lambdas from polling thread, executed in game thread       */
/************************************************************************/

void FDTrackPlugin::inject_body_data(const int n_body_id, const bool n_is_being_tracked, const FVector &n_translation, const FRotator &n_rotation) {

	check(m_injected);
	TArray<FDTrackBody> &body_inject = m_injected->m_body_data;

	// resize the TArrray if its currentSize is smaller than the received (ID + 1)
	if (body_inject.Num() < (n_body_id + 1)) {	
		body_inject.SetNumZeroed(n_body_id + 1, false);
	}

	body_inject[n_body_id].m_isBeingTracked = n_is_being_tracked;
	body_inject[n_body_id].m_location = n_translation;
	body_inject[n_body_id].m_rotation = n_rotation;
}

void FDTrackPlugin::inject_flystick_data(const int n_flystick_id, const bool n_is_being_tracked, const FVector &n_translation, const FRotator &n_rotation, const TArray<int> &n_button_state, const TArray<float> &n_joystick_state) {

	check(m_injected);
	TArray<FDTrackFlystick> &flystick_inject = m_injected->m_flystick_data;

	if (flystick_inject.Num() < (n_flystick_id + 1)) {
		flystick_inject.SetNumZeroed(n_flystick_id + 1, false);
	}

	flystick_inject[n_flystick_id].m_location = n_translation;
	flystick_inject[n_flystick_id].m_rotation = n_rotation;
	flystick_inject[n_flystick_id].m_button_states = n_button_state;
	flystick_inject[n_flystick_id].m_joystick_states = n_joystick_state;
}

void FDTrackPlugin::inject_hand_data(const int n_hand_id, const bool n_is_being_tracked, const bool &n_right, const FVector &n_translation, const FRotator &n_rotation, const TArray<FDTrackFinger> &n_fingers) {

	check(m_injected);
	TArray<FDTrackHand> &hand_inject = m_injected->m_hand_data;

	if (hand_inject.Num() < (n_hand_id + 1)) {
		hand_inject.SetNumZeroed(n_hand_id + 1, false);
	}

	hand_inject[n_hand_id].m_right = n_right;
	hand_inject[n_hand_id].m_location = n_translation;
	hand_inject[n_hand_id].m_rotation = n_rotation;
	hand_inject[n_hand_id].m_fingers = n_fingers;
}

void FDTrackPlugin::inject_human_model_data(const int n_human_id, const bool n_is_being_tracked, const TArray<FDTrackJoint> &n_joints) {
	
	check(m_injected);
	TArray<FDTrackHuman> &human_inject = m_injected->m_human_model_data;

	if (human_inject.Num() < (n_human_id + 1)) {
		human_inject.SetNumZeroed(n_human_id + 1, false);
	}

	human_inject[n_human_id].m_joints = n_joints;
}

void FDTrackPlugin::begin_injection() {

	m_last_injection_time = m_current_injection_time;
	m_current_injection_time = FPlatformTime::Cycles64();
}

void FDTrackPlugin::end_injection() {

	// so this thread cannot be accessed by another one at the same time (called by Polling Thread)
	FScopeLock lock(swapping_mutex()); 


	// injection vector becomes front now, front becomes back and back becomes 
	// the new injection data storage... (its actually a ring buffer with 3 slots here)
	// here lies the error:
	// std::swap(m_front, m_back);
	// std::swap(m_front, m_injected);
	// the old data (back) becomes the injected data... but when the old data is not overwritten
	// which is the case when body->quality==0 then the old data inside m_injected gets transfered over to m_front anyways
	// solution: 
	// 1) either just swap when there is really new data from the producer thread (polling thread)
	// 2) or make the ringbuffer into a queue
	// 3) or pass along the quality value so that its clear whether the target is being tracked

	std::swap(m_front, m_back);
	std::swap(m_front, m_injected);
}
 


/************************************************************************/
/* Handler methods. Called in game thread tick                          */
/* to relay information to components                                   */
/************************************************************************/

void FDTrackPlugin::handle_bodies(UDTrackComponent *n_component) {
	UE_LOG(LogTemp, Display, TEXT("FDTrackPlugin::handle_bodies"));
	
	check(m_front);
	check(m_back);

	FScopeLock lock(swapping_mutex());
	for (int32 i = 0; i < m_front->m_body_data.Num(); i++) {

		const FDTrackBody &current_body = m_front->m_body_data[i];

		UE_LOG(LogTemp, Error, TEXT("current_body loc : %s rot : %s"), 
			*(current_body.m_location.ToString()), 
			*(current_body.m_rotation.ToString())
		);
		
		if (current_body.m_isBeingTracked) {
			n_component->body_tracking(i, current_body.m_location, current_body.m_rotation);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("COMPONENT NOT AVAILABLE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
		}
		// else do nothing ( do not change loc nor rot )
	}
}

void FDTrackPlugin::handle_flysticks(UDTrackComponent *n_component) {
	UE_LOG(LogTemp, Display, TEXT("FDTrackPlugin::handle_flysticks"));

	// treat all flysticks
	FScopeLock lock(swapping_mutex());
	for (int32 i = 0; i < m_front->m_flystick_data.Num(); i++) {

		FDTrackFlystick &current_flystick = m_front->m_flystick_data[i];

		// tracking first, it's always called
		n_component->flystick_tracking(i, current_flystick.m_location, current_flystick.m_rotation);

		if (current_flystick.m_button_states.Num()) {
			// compare button states with the last seen state, calling button handlers if appropriate

			// See if we have to resize our actual state vector to accommodate this.
			if (m_last_button_states.size() < (i + 1)) {
				// vector too small, insert new empties to pad
				TArray<int> new_stick;
				new_stick.SetNumZeroed(DTRACKSDK_FLYSTICK_MAX_BUTTON);
				m_last_button_states.resize(i + 1, new_stick);
			}
			
			const TArray<int> &current_states = current_flystick.m_button_states;
			TArray<int> &last_states = m_last_button_states[i];

			// have to go through all the button states now to figure out which ones have differed
			for (int32 b = 0; b < current_states.Num(); b++) {
				if (current_states[b] != last_states[b]) {
					last_states[b] = current_states[b];
					n_component->flystick_button(i, b, (current_states[b] == 1));
				}
			}
		}

		// Call joysticks if we have 'em
		if (current_flystick.m_joystick_states.Num()) {
			n_component->flystick_joystick(i, current_flystick.m_joystick_states);
		}
	}

	// that's it. Flystick all done.
}

void FDTrackPlugin::handle_hands(UDTrackComponent *n_component) {
	UE_LOG(LogTemp, Display, TEXT("FDTrackPlugin::handle_hands"));

	// treat all tracked hands
	FScopeLock lock(swapping_mutex());
	for (int32 i = 0; i < m_front->m_hand_data.Num(); i++) {
		const FDTrackHand &hand = m_front->m_hand_data[i];
		n_component->hand_tracking(i, hand.m_right, hand.m_location, hand.m_rotation, hand.m_fingers);
	}
}

void FDTrackPlugin::handle_human_model(UDTrackComponent *n_component) {
	UE_LOG(LogTemp, Display, TEXT("FDTrackPlugin::handle_human_model"));
	
	FScopeLock lock(swapping_mutex());
	// treat all tracked hands
	for (int32 i = 0; i < m_front->m_human_model_data.Num(); i++) {
		const FDTrackHuman &human = m_front->m_human_model_data[i];
		n_component->human_model(i, human.m_joints);
	}
}

#undef LOCTEXT_NAMESPACE
