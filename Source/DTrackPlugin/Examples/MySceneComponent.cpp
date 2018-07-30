// Fill out your copyright notice in the Description page of Project Settings.

#include "MySceneComponent.h"


// Sets default values for this component's properties
UMySceneComponent::UMySceneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	// create DTrackComponent instance
	m_dtrack_component = CreateDefaultSubobject<UDTrackComponent>(TEXT("UDTrackComponent"));
	if (!m_dtrack_component) {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could not be created"));  // This shouldn't happen but you may treat it.
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could be created :D"));  // This shouldn't happen but you may treat it.
		//m_dtrack_component->SetupAttachment(this); 
	}
}  


// Called when the game starts
void UMySceneComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	StartPos = this->GetComponentLocation();
}


// Called every frame
void UMySceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMySceneComponent::OnBodyData_Implementation(const int32 BodyID, const FVector & Position, const FRotator & Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBodyData_Implementation"));


	if (BodyID == m_id) {        // The ID of the tracked body. There can be many, depending on your setup.
		// well, do whatever you like with the data. Much flexibility here really.
		//SetActorLocation(tmp, false, nullptr, ETeleportType::None);
		//SetActorRotation(Rotation.Quaternion(), ETeleportType::None);

		//UE_LOG(LogTemp, Warning, TEXT("DATA OF BODY-ID")); 
		 
		//UE_LOG(LogTemp, Warning, TEXT("X : %f"), Position.X);
		//UE_LOG(LogTemp, Warning, TEXT("Y : %f"), Position.Y);
		//UE_LOG(LogTemp, Warning, TEXT("Z : %f"), Position.Z);

		// position needs to be relative because the map is not guranteed to be at <0,0,0> 
		// also its a good use case when the starting player position is set by the game developer
		//this->SetWorldLocation(StartPos + Position, false);

		//UE_LOG(LogTemp, Warning, TEXT("Roll  : %f"), Rotation.Roll);
		//UE_LOG(LogTemp, Warning, TEXT("Pitch : %f"), Rotation.Pitch);
		//UE_LOG(LogTemp, Warning, TEXT("Yaw   : %f"), Rotation.Yaw);

		//FVector worlLocOfComp = this->GetComponentLocation(); 
		//UE_LOG(LogTemp, Warning, TEXT("worlLocOfComp  : %s"), *worlLocOfComp.ToString());
		 
		// rotation needs to be in world coords because DTrack offers only world coords
		//this->SetWorldRotation(Rotation, false);

		USceneComponent* pAttachedParent = this->GetAttachParent();
		USceneComponent* pAttachedRoot = this->GetAttachmentRoot();
		AActor* pAttachedRootActor = this->GetAttachmentRootActor();

		//UE_LOG(LogTemp, Warning, TEXT("pAttachedParent   : %s"), *pAttachedParent->GetName());
		//UE_LOG(LogTemp, Warning, TEXT("pAttachedRoot   : %s"), *pAttachedRoot->GetName());
		//UE_LOG(LogTemp, Warning, TEXT("pAttachedRootActor   : %s"), *pAttachedRootActor->GetName());

		// pAttachedParent->SetWorldLocation(StartPos + Position, false);
		FVector NewPosition = StartPos + Position;
		pAttachedParent->SetWorldRotation(Rotation, false); 
		pAttachedParent->SetWorldLocation(NewPosition);
		UE_LOG(LogTemp, Warning, TEXT("NewPosition  : %s"), *(NewPosition).ToString());
		UE_LOG(LogTemp, Warning, TEXT("Rotation  : %s"), *Rotation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("---> Currently Used for SetLocAndRot   : %s"), *pAttachedParent->GetName());
	} 
}

void UMySceneComponent::OnFlystickData_Implementation(const int32 FlystickID, const FVector & Position, const FRotator & Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickData_Implementation ID : %d"), FlystickID);

	/* 
	FString out;  // just output those values for info.
	for (int32 i = 0; i < JoystickValues.Num(); i++) {
	out.Append(FString::Printf(TEXT(" %i: %f"), i, JoystickValues[i]));
	}
	UE_LOG(LogTemp, Display, TEXT("%i joysticks - %s "), JoystickValues.Num(), *out);
	*/
	if (FlystickID == m_id) {        // The ID of the tracked body. There can be many, depending on your setup.
		UE_LOG(LogTemp, Warning, TEXT("Flystick sets rot to : %s"), *Rotation.ToString());
		USceneComponent* pAttachedParent = this->GetAttachParent();
		pAttachedParent->SetWorldRotation(Rotation, false);
		//pAttachedParent->SetWorldLocation(Position);
	}
}

void UMySceneComponent::OnFlystickButton_Implementation(const int32 FlystickID, const int32 & ButtonIndex, const bool Pressed)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickButton_Implementation"));
}

void UMySceneComponent::OnFlystickJoystick_Implementation(const int32 FlystickID, const TArray<float>& JoystickValues)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickJoystick_Implementation"));

}

void UMySceneComponent::OnHandTracking_Implementation(const int32 HandID, const bool Right, const FVector & Translation, const FRotator & Rotation, const TArray<FDTrackFinger>& Fingers)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHandTracking_Implementation"));
}

void UMySceneComponent::OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint>& Joints)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHumanModel_Implementation"));
}
