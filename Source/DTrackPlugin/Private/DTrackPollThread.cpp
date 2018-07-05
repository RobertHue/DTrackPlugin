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

#include "DTrackPollThread.h"

// this makes UBT allow windows types as (probably) used by the SDK header
// @todo check if this is indeed the case and remove if not needed
#include "AllowWindowsPlatformTypes.h" 

#include "DTrackSDK.hpp"
#include "Engine/Engine.h"  // for GEngine->AddOnScreenDebugMessage

// revert above allowed types to UBT default
#include "HideWindowsPlatformTypes.h"
#include "Async.h" 
#include "GenericPlatform/GenericPlatformMath.h"

#define LOCTEXT_NAMESPACE "DTrackPlugin"
 
FDTrackPollThread *FDTrackPollThread::m_runnable = nullptr;

FThreadSafeCounter FDTrackPollThread::m_UniqueNameCounter; /// Default constructor. Initializes the counter to 0.

// for returns
// enum class Return { error=0, success=1 };
#define RETURN_SUCCESS 1
#define RETURN_ERROR   0

// n_client can be accessed indirectly through the singleton...
// plugin can be accessed anyways because its a singleton...
FDTrackPollThread::FDTrackPollThread(const UDTrackComponent *n_client, FDTrackPlugin *n_plugin) 
		: m_plugin(n_plugin)
		, m_dtrack2(n_client->m_dtrack_2)
		, m_dtrack_server_ip(TCHAR_TO_UTF8(*n_client->m_dtrack_server_ip))
		, m_dtrack_client_port(n_client->m_dtrack_client_port)
		, m_stop_counter(0)
		, m_coord_converter(n_client->m_coordinate_system)
{
	UE_LOG(DTrackPollThreadLog, Display, TEXT("Constructor of TDTrackPollThread (the factory class for DTrack threads)"));


	// LogStats:Warning: MetaData mismatch.
	// This warning occurs when you have two threads with the same name
	// 
	// m_thread = FRunnableThread::Create(this, TEXT("FDTrackPollThread"), 0, TPri_Normal);
	// better solution to upper (with FThreadSafeCounter WorkerCounter):
	// 
	// Increment the counter and create an unique name.
	FString ThreadName(FString::Printf(TEXT("MyThreadName%i"), FDTrackPollThread::m_UniqueNameCounter.Increment()));

	// Create the actual thread
	UE_LOG(DTrackPollThreadLog, Display, TEXT("Create new Thread: %s"), *ThreadName);
	m_thread = FRunnableThread::Create(this, *ThreadName);


	UE_LOG(DTrackPollThreadLog, Display, TEXT("Created: %s (%d)"), *m_thread->GetThreadName(), m_thread->GetThreadID());
	// only one thread is created in this class to run the worker FRunnable on; 
	// due to the class design it's not possible that more than one thread is created
}

FDTrackPollThread::~FDTrackPollThread() {
	m_dtrack.reset(nullptr);	// release and delete the DTrackSDK-object, since this uptr is scoped its not really needed

	if (m_thread) {
		delete m_thread;
		m_thread = nullptr;
	}

	m_runnable = nullptr;
	UE_LOG(DTrackPollThreadLog, Display, TEXT("Destruct of TDTrackPollThread (the factory class for DTrack threads)"));
}

FDTrackPollThread *FDTrackPollThread::start(const UDTrackComponent *n_client, FDTrackPlugin *n_plugin) {
	
	// Create new instance of thread if it does not exist and the platform supports multi threading
	if (!m_runnable && FPlatformProcess::SupportsMultithreading()) {
		m_runnable = new FDTrackPollThread(n_client, n_plugin);
	}
	return m_runnable;
}

void FDTrackPollThread::interrupt() {
	UE_LOG(DTrackPollThreadLog, Display, TEXT("Stop: %s (%d)"), *m_thread->GetThreadName(), m_thread->GetThreadID());
	Stop();
}

void FDTrackPollThread::join() {

	UE_LOG(DTrackPollThreadLog, Display, TEXT("WaitForCompletion: %s (%d)"), *m_thread->GetThreadName(), m_thread->GetThreadID());
	m_thread->WaitForCompletion();
}  

bool FDTrackPollThread::Init() {

	// I don't wanna initialize the SDK in here as both SDK and (surprisingly) this Runnable interface 
	// have undocumented threading behavior.
	// The only thing I know for sure to run in the actual thread is Run() 
	return true;
}

// Run only gets processed once
// 1 is success
// 0 is failure
uint32 FDTrackPollThread::Run() {

	// Initial wait before starting (in case not everything in Unreal is setup yet)
	FPlatformProcess::Sleep(0.1);

	if (m_dtrack2) {
		UE_LOG(DTrackPollThreadLog, Display, 
			TEXT("Using DTrack2 TCP Connection (server-IP: %s , client-port: %d )"),
			*FString(m_dtrack_server_ip.c_str()), m_dtrack_client_port
		);

		m_dtrack.reset(new DTrackSDK(
			m_dtrack_server_ip,		// server_host (server_port is always 50105)
			m_dtrack_client_port	// data_port
		));
	} else {
		UE_LOG(DTrackPollThreadLog, Display, 
			TEXT("Using DTrack2 UDP Connection (server-IP: %d , client-port: %d )"),
			*FString(m_dtrack_server_ip.c_str()), m_dtrack_client_port
		);
		m_dtrack.reset(new DTrackSDK(
			m_dtrack_client_port	// data_port
		));
	}

	// I don't know when this can occur but I guess it's client 
	// port collision with fixed UDP ports
	if (!m_dtrack->isLocalDataPortValid()) {
		UE_LOG(DTrackPollThreadLog, Display, TEXT("Local Data port is not valid (port collision with fixed UDP ports)"));
		m_dtrack.reset();
		return RETURN_ERROR;
	}

	// start the tracking via tcp route if applicable
	if (m_dtrack2) {
		UE_LOG(DTrackPollThreadLog, Display, TEXT("starting tcp measurement"));
		if (!m_dtrack->startMeasurement()) {
			if (m_dtrack->getLastServerError() == DTrackSDK::ERR_TIMEOUT) {
				UE_LOG(DTrackPollThreadLog, Error, TEXT("Could not start tracking, timeout"));
			} else if (m_dtrack->getLastServerError() == DTrackSDK::ERR_NET) {
				UE_LOG(DTrackPollThreadLog, Error, TEXT("Could not start tracking, network error"));
			} else {
				UE_LOG(DTrackPollThreadLog, Error, TEXT("Could not start tracking"));
			}
			return RETURN_ERROR;
		} 
	} 

	// now go looping until Stop() increases the stop condition
	while (!m_stop_counter.GetValue()) {
		// receive as much as we can
		if (m_dtrack->receive()) {		// busy waiting
			UE_LOG(DTrackPollThreadLog, Display, TEXT("m_dtrack->receive()"));

			m_plugin->begin_injection();	// set some timestamp of this measurement

			// treat body info and cache results into plug-in
			handle_bodies();
			handle_flysticks();
			handle_hands();
			handle_human_model();
		
			// collect data here in this reactive system and forward them with end_injection()
			// TODO Nice to have:    Forward nothing if there is no new data! (quality==0)
			m_plugin->end_injection();
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "no DTRACK data received!!!!");
		}
	}

	if (m_dtrack2) {
		if (m_dtrack->isCommandInterfaceValid()) {
			UE_LOG(DTrackPollThreadLog, Display, TEXT("TCP connection for DTrack2 commands is active."));
		} else {
			UE_LOG(DTrackPollThreadLog, Display, TEXT("TCP connection for DTrack2 commands is inactive."));
		} 

		UE_LOG(DTrackPollThreadLog, Display, TEXT("Stopping DTrack2 measurement."));
		if (m_dtrack->stopMeasurement()) {// Is command successful ? If measurement is not running return value is true.
			UE_LOG(DTrackPollThreadLog, Display, TEXT("Stop command was successful! :D"));
		}
		else {
			UE_LOG(DTrackPollThreadLog, Display, TEXT("Stop command was unsuccessful"));
		}

		std::string answer;
		int retVal;

		retVal = m_dtrack->sendDTrack2Command("dtrack2 get status active", &answer);
		UE_LOG(DTrackPollThreadLog, Display, TEXT("retVal : %d"), retVal);
		UE_LOG(DTrackPollThreadLog, Display, TEXT("answer : %s"), *FString(answer.c_str()));

		retVal = m_dtrack->sendDTrack2Command("dtrack2 tracking stop", &answer);
		UE_LOG(DTrackPollThreadLog, Display, TEXT("retVal : %d"), retVal);
		UE_LOG(DTrackPollThreadLog, Display, TEXT("answer : %s"), *FString(answer.c_str()));


		if (m_dtrack->isCommandInterfaceValid()) {
			UE_LOG(DTrackPollThreadLog, Display, TEXT("TCP connection for DTrack2 commands is active."));
		} else {
			UE_LOG(DTrackPollThreadLog, Display, TEXT("TCP connection for DTrack2 commands is inactive."));
		}

	}

	UE_LOG(DTrackPollThreadLog, Display, TEXT("Successful finish of: %s (%d)"), *m_thread->GetThreadName(), m_thread->GetThreadID());
	return RETURN_SUCCESS;
} 

void FDTrackPollThread::Stop() {

	m_stop_counter.Set(1);
}


void FDTrackPollThread::Exit() {
	// Here's where we can do any cleanup we want to 
}


void FDTrackPollThread::handle_bodies() {
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FDTrackPollThread::handle_bodies()"));

	const DTrack_Body_Type_d *body = nullptr;
	//UE_LOG(DTrackPollThreadLog, Display, TEXT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	//UE_LOG(DTrackPollThreadLog, Display, TEXT("m_dtrack->getFrameCounter() : %d"), m_dtrack->getFrameCounter());
	//UE_LOG(DTrackPollThreadLog, Display, TEXT("m_dtrack->getNumBody() : %d"), m_dtrack->getNumBody());

	for (int i = 0; i < m_dtrack->getNumBody(); i++) {  // why do we still use int for those counters?
		body = m_dtrack->getBody(i);
		checkf(body, TEXT("DTrack API error, body address null"));
		 
		//UE_LOG(DTrackPollThreadLog, Display, TEXT("body->id : %d"), body->id);
		//UE_LOG(DTrackPollThreadLog, Display, TEXT("body->quality : %f"), body->quality);
		//UE_LOG(DTrackPollThreadLog, Display, TEXT("body->loc : < %f %f %f >"), body->loc[0], body->loc[1], body->loc[2]);
		//UE_LOG(DTrackPollThreadLog, Display, TEXT("body->rot : \n %f %f %f \n %f %f %f \n %f %f %f"), body->rot[0], body->rot[1], body->rot[2], body->rot[3], body->rot[4], body->rot[5], body->rot[6], body->rot[7], body->rot[8]);

		if (body->quality > 0) {	// with quality you can check whether the registered target is being tracked!
			// Quality below zero means the body is not visible to the system right now. I won't call the interface

			FVector translation = m_coord_converter.from_dtrack_location(body->loc);
			FRotator rotation = m_coord_converter.from_dtrack_rotation(body->rot);

			m_plugin->inject_body_data(body->id, true, translation, rotation);
		}
		else {
			// here: the registered target is not visible to the system anymore...
			m_plugin->inject_body_data(body->id, false);
		}
	}
}

void FDTrackPollThread::handle_flysticks() {
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FDTrackPollThread::handle_flysticks()"));

	const DTrack_FlyStick_Type_d *flystick = nullptr;
	for (int i = 0; i < m_dtrack->getNumFlyStick(); i++) {
		flystick = m_dtrack->getFlyStick(i);
		checkf(flystick, TEXT("DTrack API error, flystick address null"));

		if (flystick->quality > 0) {
			// Quality below zero means the body is not visible to the system right now. I won't call the interface
			FVector translation = m_coord_converter.from_dtrack_location(flystick->loc);
			FRotator rotation = m_coord_converter.from_dtrack_rotation(flystick->rot);

			// create a state vector for the button states
			TArray<int> buttons;
			buttons.SetNumZeroed(flystick->num_button);
			for (int idx = 0; idx < flystick->num_button; idx++) {
				buttons[idx] = flystick->button[idx];
			}

			// create a state vector for the joystick states
			TArray<float> joysticks;  // have to use float as blueprints don't support TArray<double>
			joysticks.SetNumZeroed(flystick->num_joystick);
			for (int idx = 0; idx < flystick->num_joystick; idx++) {
				joysticks[idx] = static_cast<float>(flystick->joystick[idx]);
			}

			m_plugin->inject_flystick_data(flystick->id, true, translation, rotation, buttons, joysticks);
		}
		else {
			// here: the registered target is not visible to the system anymore...
			m_plugin->inject_flystick_data(flystick->id, false);
		}
	}
}



void FDTrackPollThread::handle_hands() {
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FDTrackPollThread::handle_hands()"));

	const DTrack_Hand_Type_d *hand = nullptr;
	for (int i = 0; i < m_dtrack->getNumHand(); i++) {
		hand = m_dtrack->getHand(i);
		checkf(hand, TEXT("DTrack API error, hand address is null"));

		if (hand->quality > 0) {
			FVector translation = m_coord_converter.from_dtrack_location(hand->loc);
			FRotator rotation	= m_coord_converter.from_dtrack_rotation(hand->rot);
			TArray<FDTrackFinger> fingers;	// an array of fingers

			// number of the tracked fingers (3 or 5)
			for (int j = 0; j < hand->nfinger; j++) {
				FDTrackFinger finger;
				switch (j) {     // this is mostly to allow for the blueprint to be a 
								 // little more expressive than using assumptions about the index' meaning
					case 0: finger.m_type = EDTrackFingerType::FT_Thumb;	break;
					case 1: finger.m_type = EDTrackFingerType::FT_Index;	break;
					case 2: finger.m_type = EDTrackFingerType::FT_Middle;	break;
					case 3: finger.m_type = EDTrackFingerType::FT_Ring;		break;
					case 4: finger.m_type = EDTrackFingerType::FT_Pinky;	break;
				}
				///////////////////////////////////////////////////////////////////////////////////////////
				// unused data:		(also not relevant for the UE4-Skeleton)
				finger.m_relLocation = m_coord_converter.from_dtrack_location(hand->finger[j].loc); // local backOfHand relative location
				finger.m_location = finger.m_relLocation + translation;	// global room location
				finger.m_tip_radius = hand->finger[j].radiustip;	// the radius of the finger tip to identify its position and orientation
				finger.m_inner_phalanx_length  = hand->finger[j].lengthphalanx[2];
				finger.m_middle_phalanx_length = hand->finger[j].lengthphalanx[1];
				finger.m_outer_phalanx_length  = hand->finger[j].lengthphalanx[0];
				///////////////////////////////////////////////////////////////////////////////////////////
				// here: only the phalanx angles are of importance and not the Rotations...
				FRotator rotatorFingerTip = m_coord_converter.from_dtrack_rotation(hand->finger[j].rot);
				finger.m_rotation = FRotator(rotatorFingerTip);
				FQuat fingerTipOrientation(finger.m_rotation);

				finger.m_middle_outer_phalanx_angle = hand->finger[j].anglephalanx[0];  // gamma
				finger.m_inner_middle_phalanx_angle = hand->finger[j].anglephalanx[1];	// beta

				// calucalte backwards (analitically, with quaternions):
				//FRotator patchRotator(0, 0, 0);  //  + patchRotator 
				//FQuat fingerTipOrientation(finger.m_rotation); // TODO: find out:  why is roll not being zero???! or staying at a value at least when finger is just using pitch
				FQuat rotationGammaQuat(FVector::ForwardVector, -finger.m_middle_outer_phalanx_angle * DEG_TO_RAD); 
				FQuat rotationBetaQuat(FVector::ForwardVector,  -finger.m_inner_middle_phalanx_angle * DEG_TO_RAD);

				FQuat quatOuter = fingerTipOrientation;
				FQuat quatMiddle = quatOuter  * rotationGammaQuat;		// first apply rotationGammaQuat and then quatOuter
				FQuat quatInner  = quatMiddle * rotationBetaQuat;		// first apply rotationBetaQuat  and then quatMiddle
				


				//FQuat quatOuterGlobally = fingerTipOrientation * rotation.Quaternion(); 
				//FQuat quatInnerGlobally = quatOuterGlobally * rotationGammaQuat * rotationBetaQuat;
				////////////////////////////
				/* FQuat
					.Rotator():					--------->		.Euler(): reorders to	X Y Z
						Pitch = around Y-axis	(to side of finger)						
						Yaw   = around Z-axis	(up the finger)
						Roll  = around X-axis	(along the finger)
				*/ 

				// some calculations for checking whether what Rotator has is correct:
				float outerAnglePitch	= FGenericPlatformMath::Acos(FVector::DotProduct(quatOuter.GetForwardVector().GetSafeNormal(), FVector::ForwardVector));	// X Pitch is the angle between forward vectors along the finger
				float outerAngleYaw		= FGenericPlatformMath::Acos(FVector::DotProduct(quatOuter.GetUpVector().GetSafeNormal(), FVector::UpVector));				// Z Yaw
				float outerAngleRoll	= FGenericPlatformMath::Acos(FVector::DotProduct(quatOuter.GetRightVector().GetSafeNormal(), FVector::RightVector));		// Y Roll
				 
				float innerAnglePitch	= FGenericPlatformMath::Acos(FVector::DotProduct(quatInner.GetForwardVector().GetSafeNormal(), FVector::ForwardVector));	// X Pitch is the angle between forward vectors along the finger
				float innerAngleYaw		= FGenericPlatformMath::Acos(FVector::DotProduct(quatInner.GetUpVector().GetSafeNormal(), FVector::UpVector));				// Z Yaw
				float innerAngleRoll	= FGenericPlatformMath::Acos(FVector::DotProduct(quatInner.GetRightVector().GetSafeNormal(), FVector::RightVector));		// Y Roll

				// add to proper UE4-rotation part: (is needed because forward is pointing to the negative side upwards the finger bones)
				FRotator rotatorInner = quatInner.Rotator();
				finger.m_hand_inner_phalanx_angle_pitch = -rotatorInner.Roll - 180.f;
				finger.m_hand_inner_phalanx_angle_yaw = rotatorInner.Yaw - 180.f;
				finger.m_hand_inner_phalanx_angle_roll = rotatorInner.Pitch; // should be zero (unchanged), because its not possible to do a finger twist.
				

				if(j == 0)	// == index_finger
				{
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: |+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: rotatorFingerTip : %s"), *(rotatorFingerTip.ToString()));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: fingerTipOrientation : %s"), *(fingerTipOrientation.Rotator().ToString()));
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: r2 : %s"), *(r2.ToString()));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: ----------------------------------"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: m_middle_outer_phalanx_angle : %f"), (finger.m_middle_outer_phalanx_angle));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: m_inner_middle_phalanx_angle : %f"), (finger.m_inner_middle_phalanx_angle));

					// rotation Gamma and Beta are the positive rotation values around the local Y-axis (pointing to the side of the finger)
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: rotationGammaQuat    : %s"), *(rotationGammaQuat.Rotator().ToString()));	
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: rotationBetaQuat     : %s"), *(rotationBetaQuat.Rotator().ToString()));

					// Angular Dist is the sum of the above angles... Fabs()
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: AngularDistance: %f"), quatOuter.AngularDistance(quatInner) * RAD_TO_DEG);

					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: ----------------"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: quatOuter :  %s"), *(quatOuter.Rotator().ToString())); // == fingerTipOrientation (local Orientation)
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: quatMiddle:  %s"), *(quatMiddle.Rotator().ToString()));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: quatInner :  %s"), *(quatInner.Rotator().ToString())); 
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: ------------"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: outerAnglePitch :  %f | outerAngleYaw :  %f | outerAngleRoll :  %f"), outerAnglePitch * RAD_TO_DEG, outerAngleYaw * RAD_TO_DEG, outerAngleRoll * RAD_TO_DEG);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: innerAnglePitch :  %f | innerAngleYaw :  %f | innerAngleRoll :  %f"), innerAnglePitch * RAD_TO_DEG, innerAngleYaw * RAD_TO_DEG, innerAngleRoll * RAD_TO_DEG);
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: ------"));
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: quatOuterGlobally :  %s"), *(quatOuterGlobally.Rotator().ToString()));
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: quatInnerGlobally :  %s"), *(quatInnerGlobally.Rotator().ToString()));
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: AngularDistance between tip and inner (globally): %f"), quatOuterGlobally.AngularDistance(quatInnerGlobally) * RAD_TO_DEG);
					//UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: AngularDistance To Back Of Hand: %f"), quatInnerGlobally.AngularDistance(rotation.Quaternion()) * RAD_TO_DEG);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: ---"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: finger.m_hand_inner_phalanx_angle_pitch : %f"), finger.m_hand_inner_phalanx_angle_pitch);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: finger.m_hand_inner_phalanx_angle_yaw   : %f"), finger.m_hand_inner_phalanx_angle_yaw);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: finger.m_hand_inner_phalanx_angle_roll  : %f (should be close to Zero (0)"), finger.m_hand_inner_phalanx_angle_roll);

				}
				if (j == 0) {
					UE_LOG(DTrackPollThreadLog, Display, TEXT("FINGER: |+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|"));
					UE_LOG(DTrackPollThreadLog, Display, TEXT("THUMB: finger.m_hand_inner_phalanx_angle_pitch : %f"), finger.m_hand_inner_phalanx_angle_pitch);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("THUMB: finger.m_hand_inner_phalanx_angle_yaw   : %f"), finger.m_hand_inner_phalanx_angle_yaw);
					UE_LOG(DTrackPollThreadLog, Display, TEXT("THUMB: finger.m_hand_inner_phalanx_angle_roll  : %f (should be close to Zero (0)"), finger.m_hand_inner_phalanx_angle_roll);

				}
				// calucalte backwards (analitically, with vectors):  // experimental (do not use this commented code)
				//FVector fingerTipPosition(finger.m_location);
				//FQuat fingerTipOrientation(finger.m_rotation);
				//FVector v_outer = fingerTipPosition - (fingerTipPosition + finger.m_outer_phalanx_length * fingerTipOrientation.GetAxisZ());
				//FVector v_middle = -(cos(finger.m_middle_outer_phalanx_angle) * finger.m_outer_phalanx_length * finger.m_middle_phalanx_length * (-v_outer));
				//FVector v_inner = -(cos(finger.m_inner_middle_phalanx_angle) * finger.m_middle_phalanx_length * finger.m_inner_phalanx_length * (-v_middle));
				//finger.m_hand_inner_phalanx_angle = 180.0 - acos( FVector::DotProduct(v_inner, FVector::ForwardVector)/(finger.m_inner_phalanx_length));

				// angles between the single phalanxes as well as their respective lengths.
				fingers.Add(std::move(finger)); 
			}

			// a value to distinguish between right and left hand,
			m_plugin->inject_hand_data(
				hand->id,	// hand-id
				true,  // indicate whether new data has arrived (is_BeingTracked)
				(hand->lr == 1), translation, rotation, // hand related (left or right hand, pos, rot)
				fingers	// also pass the array of fingers (fingers do not have an id)
			);
		}
		else {
			// here: the registered target is not visible to the system anymore...
			m_plugin->inject_hand_data(hand->id, false);
		}
	}
}

/// TODO: still needs to be tested
void FDTrackPollThread::handle_human_model() {
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FDTrackPollThread::handle_human_model()"));
	
	const DTrack_Human_Type_d *human = nullptr;
	for (int i = 0; i < m_dtrack->getNumHuman(); i++) {
		human = m_dtrack->getHuman(i);
		checkf(human, TEXT("DTrack API error, human address is null"));

		TArray<FDTrackJoint> joints;

		for (int j = 0; j < human->num_joints; j++) {
			FDTrackJoint joint;
			// I'm not sure if I should check for quality as I don't know if the caller
			// would expect number and order of joints to be relevant/constant.
			// They do carry an ID though so I suppose the caller must be aware of that.
			if (human->joint[j].quality > 0.1) {
				joint.m_id = human->joint[j].id;
				joint.m_location = m_coord_converter.from_dtrack_location(human->joint[j].loc);
				joint.m_rotation = m_coord_converter.from_dtrack_rotation(human->joint[j].rot);
				joint.m_angles.Add(human->joint[j].ang[0]);   // well, are they Euler angles of the same rot as above or not?
				joint.m_angles.Add(human->joint[j].ang[1]);
				joint.m_angles.Add(human->joint[j].ang[2]);
				joints.Add(std::move(joint));
			}
		}

		m_plugin->inject_human_model_data(human->id, true, joints);
	}
}

#undef LOCTEXT_NAMESPACE
