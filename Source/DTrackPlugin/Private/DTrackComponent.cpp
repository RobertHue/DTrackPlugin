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

#include "DTrackComponent.h"

#include "FDTrackPlugin.h"
#include "Engine/Engine.h"

UDTrackComponent::UDTrackComponent(const FObjectInitializer &n_initializer)
		: Super(n_initializer) {

 	bWantsInitializeComponent = true;
 	bAutoActivate = true; 
 	PrimaryComponentTick.bCanEverTick = true;

	this->RegisterComponent();

	AActor* actorThatOwnsThisComponent = this->GetOwner();
	if (actorThatOwnsThisComponent) {
		FString name = actorThatOwnsThisComponent->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns this component(UDTrackComponent) : %s"), *name);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has no Actor that owns it"));
	}

	USceneComponent* parentSceneComp = this->GetAttachParent();
	if(parentSceneComp) {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has parent scene comp : %s"), *parentSceneComp->GetName());
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has no parent scene comp"));
	}
}

void UDTrackComponent::BeginPlay() {

	Super::BeginPlay();

	UE_LOG(DTrackPluginLog, Display, TEXT("DTrack BeginPlay() called"));

	AActor* actorThatOwnsThisComponent = this->GetOwner();
	if (actorThatOwnsThisComponent) {
		FString name = actorThatOwnsThisComponent->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns this component(UDTrackComponent) : %s"), *name);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has no Actor that owns it"));
	}
	UE_LOG(DTrackPluginLog, Display, TEXT("-------------"));


	// Attach the delegate pointer automatically to the owner of the component

	if (FDTrackPlugin::IsAvailable()) {
		IDTrackPlugin &plugin = FDTrackPlugin::Get();

		m_plugin = &plugin;

		// tell the plugin we exist and want to be called as data come in
		m_plugin->start_up(this);

	} else {
		UE_LOG(DTrackPluginLog, Warning, TEXT("DTrack Plugin not available, cannot track this object"));
	}
}

void UDTrackComponent::EndPlay(const EEndPlayReason::Type n_reason) {

	Super::EndPlay(n_reason);

	UE_LOG(DTrackPluginLog, Display, TEXT("DTrack EndPlay() called"));

	if (m_plugin) {
		m_plugin->remove(this);
		m_plugin = nullptr;
	} else {
		UE_LOG(DTrackPluginLog, Warning, TEXT("DTrack Plugin not available, cannot stop tracking of this object"));
	}
}

void UDTrackComponent::TickComponent(float n_delta_time, enum ELevelTick n_tick_type,
	FActorComponentTickFunction *n_this_tick_function) {

	Super::TickComponent(n_delta_time, n_tick_type, n_this_tick_function);

	// Forward the component tick to the plugin
	if (m_plugin) {
		m_plugin->tick(n_delta_time, this);
	}
}


void UDTrackComponent::body_tracking(const int32 n_body_id, const FVector &n_translation, const FRotator &n_rotation) {
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::body_tracking");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::body_tracking"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
									  // skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return;
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnBodyData(GetOwner(), n_body_id, n_translation, n_rotation);
	}	
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {	
		IDTrackInterface::Execute_OnBodyData(this->GetAttachParent(), n_body_id, n_translation, n_rotation);
	} 
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnBodyData_Implementation)");
	}
}

void UDTrackComponent::flystick_tracking(const int32 n_flystick_id, const FVector &n_translation, const FRotator &n_rotation) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::flystick_tracking");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::flystick_tracking"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
		// skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return; 
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) { 
		IDTrackInterface::Execute_OnFlystickData(GetOwner(), n_flystick_id, n_translation, n_rotation);
	}
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */ 
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnFlystickData(this->GetAttachParent(), n_flystick_id, n_translation, n_rotation);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnFlystickData_Implementation)");
	}
}

void UDTrackComponent::flystick_button(const int32 n_flystick_id, const int32 n_button_number, const bool n_pressed) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::flystick_button");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::flystick_button"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
									  // skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return;
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnFlystickButton(GetOwner(), n_flystick_id, n_button_number, n_pressed);
	}
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnFlystickButton(this->GetAttachParent(), n_flystick_id, n_button_number, n_pressed);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnFlystickButton_Implementation)");
	}
}

void UDTrackComponent::flystick_joystick(const int32 n_flystick_id, const TArray<float> &n_joysticks) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::flystick_joystick");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::flystick_joystick"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
									  // skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return;
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnFlystickJoystick(GetOwner(), n_flystick_id, n_joysticks);
	}
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnFlystickJoystick(this->GetAttachParent(), n_flystick_id, n_joysticks);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnFlystickJoystick_Implementation)");
	}
}

void UDTrackComponent::hand_tracking(const int32 n_hand_id, const bool n_right, const FVector &n_translation, const FRotator &n_rotation, const TArray<FDTrackFinger> &n_fingers) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::hand_tracking");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::hand_tracking"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
									  // skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return;
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnHandTracking(GetOwner(), n_hand_id, n_right, n_translation, n_rotation, n_fingers);
	
	}
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnHandTracking(this->GetAttachParent(), n_hand_id, n_right, n_translation, n_rotation, n_fingers);
	}
	else {  // SceneComponent also has a method called GetAttachChildren() and GetAttachParent()
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnHandTracking_Implementation)");
	}
}

void UDTrackComponent::human_model(const int32 n_human_id, const TArray<FDTrackJoint> &n_joints) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "UDTrackComponent::human_model");
	UE_LOG(LogTemp, Display, TEXT("UDTrackComponent::human_model"));

	if (!(this->GetAttachParent())) { // if GetAttachParent == NULL (empty)
									  // skip and do nothing
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No attached parent...");
		return;
	}
	checkParentClass();

	/*
	if (GetOwner()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnHumanModel(GetOwner(), n_human_id, n_joints);
	}
	// if there is no Actor implementing the Interface and instead a USceneComponent is implementing it
	else */
		if (this->GetAttachParent()->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) {
		IDTrackInterface::Execute_OnHumanModel(this->GetAttachParent(), n_human_id, n_joints);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Owning Actor does not implement DTrack interface (IDTrackInterface::OnHumanModel_Implementation)");
	}
}

////////////////////////////
// private Helpers

void UDTrackComponent::checkParentClass()
{
	UE_LOG(LogTemp, Warning, TEXT("------------------------------------"));
	UE_LOG(LogTemp, Warning, TEXT("UDTrackComponent::checkParentClass"));

	USceneComponent* parentSceneComp = this->GetAttachParent();
	if (parentSceneComp) {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has parent scene comp : %s"), *parentSceneComp->GetName());
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("component(UDTrackComponent) has no parent scene comp"));
	} 

	bool bIsImplemented = parentSceneComp->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass()); 
	if (bIsImplemented) {
		UE_LOG(LogTemp, Warning, TEXT("%s does implement UDTrackInterface :D"), *parentSceneComp->GetName());
	} else { 
		UE_LOG(LogTemp, Warning, TEXT("%s does NOT implement UDTrackInterface"), *parentSceneComp->GetName());
	}

	IDTrackInterface* trackingObject = Cast<IDTrackInterface>(this->GetAttachParent());
	// ReactingObject will be non-null if OriginalObject implements UReactToTriggerInterface.
	if (trackingObject) {
		UE_LOG(LogTemp, Warning, TEXT("%s does implement UDTrackInterface :D"), *parentSceneComp->GetName());
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s does NOT implement UDTrackInterface"), *parentSceneComp->GetName());
	}
	UE_LOG(LogTemp, Warning, TEXT("------------------------------------"));
}
 