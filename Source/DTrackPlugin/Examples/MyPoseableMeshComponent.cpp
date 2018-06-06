// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPoseableMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"

 
UMyPoseableMeshComponent::UMyPoseableMeshComponent()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("-------------------------------------------------"));
	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor start: UMyPoseableMeshComponent"));

	// m_pAltMeshAsset-Pointer is here NOT even set by the UE-Editor so its NULL
	// that UPROPERTY-pointer is just set when ...
	/* // Determining If a Given Actor implements The Interface
	if (this->GetClass()->ImplementsInterface(UDTrackInterface::StaticClass())) { 
		UE_LOG(LogSkeletalMesh, Warning, TEXT("UMyPoseableMeshComponent implements the interface UDTrackInterface"));
		// IDTrackInterface::Execute_OnBodyData(this, 0, FVector(0, 0, 0), FRotator(0, 0, 0));
	}
	else {
		UE_LOG(LogSkeletalMesh, Warning, TEXT("UMyPoseableMeshComponent does NOT implement the interface UDTrackInterface"));
	}*/
	AActor* actorThatOwnsThisComponent = this->GetOwner();
	if (actorThatOwnsThisComponent) {
		FString name = actorThatOwnsThisComponent->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns this component(UMyPoseableMeshComp) : %s"), *name);
	}
	  
	// create DTrackComponent instance
	m_dtrack_component = CreateDefaultSubobject<UDTrackComponent>(TEXT("DTrackComponent"));
	if (!m_dtrack_component) {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could not be created"));  // This shouldn't happen but you may treat it.
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could be created :D"));  // This shouldn't happen but you may treat it.
		m_dtrack_component->SetupAttachment(this);
	}
	 
	// this->SetupAttachment(this);  
	// Initializes desired Attach Parent and SocketName to be attached to when the component is registered.
	// Generally intended to be called from its Owning Actor's constructor and should be preferred over AttachToComponent when
	// a component is not registered.

	// this->AttachTo(...); // DEPRECATED(4.12, "This function is deprecated, please use AttachToComponent instead.")
	// Attach this component to another scene component, optionally at a named socket. 
	// It is valid to call this on components whether or not they have been Registered. 
	// (calls AttachToComponent)

	// this->AttachToComponent(...);	
	// It is valid to call this on components whether or not they have been Registered, 
	// however from constructor or when not registered it is preferable to use SetupAttachment.

	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor end: UMyPoseableMeshComponent"));
}

void UMyPoseableMeshComponent::PostInitProperties()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("PostInitProperties start: UMyPoseableMeshComponent (called after constructor)"));
	Super::PostInitProperties();
	// no UE_editor properties being assigned to this component yet
}

void UMyPoseableMeshComponent::PostLoad()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("UMyPoseableMeshComponent::PostLoad()"));
	Super::PostLoad();
	// no UE_editor properties being assigned to this component yet
}





void UMyPoseableMeshComponent::InitializeComponent()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("UMyPoseableMeshComponent::InitializeComponent()"));
	Super::InitializeComponent();
}

void UMyPoseableMeshComponent::BeginPlay()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("UMyPoseableMeshComponent::BeginPlay()"));
	Super::BeginPlay();

	/*
	USkeletalMesh* pSkeletalMesh = this->SkeletalMesh;
	m_pSkeletalMesh = pSkeletalMesh;
	if (pSkeletalMesh) {
		UE_LOG(LogSkeletalMesh, Warning, TEXT("pSkeletalMesh set :D"));

		USkeleton* pSkeleton = pSkeletalMesh->Skeleton;
		m_pSkeleton = pSkeleton;
		if (pSkeleton) {
			UE_LOG(LogSkeletalMesh, Warning, TEXT("pSkeleton set :D"));
		}
		else
		{
			UE_LOG(LogSkeletalMesh, Warning, TEXT("pSkeleton NOT set"));
		}
	}
	else
	{
		UE_LOG(LogSkeletalMesh, Warning, TEXT("pSkeletalMesh NOT set"));
	}


	this->SetBoneRotationByName("spine_01", FRotator(45, 45, 45), EBoneSpaces::WorldSpace);

	UE_LOG(LogSkeletalMesh, Warning, TEXT("-------------------------------------------------"));
	*/
	AActor* actorThatOwnsThisComponent = this->GetOwner();
	if (actorThatOwnsThisComponent) {
		FString name = actorThatOwnsThisComponent->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns this component(UMyPoseableMeshComp) : %s"), *name);
	}
}

void UMyPoseableMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMyPoseableMeshComponent::OnBodyData_Implementation(const int32 BodyID, const FVector & Position, const FRotator & Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBodyData_Implementation"));


	if (BodyID == 1) {        // The ID of the tracked body. There can be many, depending on your setup.
		FVector tmp = Position;
		tmp.X = tmp.X - 200;
		tmp.Z = tmp.Z + 200;
		// well, do whatever you like with the data. Much flexibility here really.
		//SetActorLocation(tmp, false, nullptr, ETeleportType::None);
		//SetActorRotation(Rotation.Quaternion(), ETeleportType::None);

		UE_LOG(LogTemp, Warning, TEXT("DATA OF BODY-ID"));
		UE_LOG(LogTemp, Warning, TEXT("Roll : %f"), Rotation.Roll);
		UE_LOG(LogTemp, Warning, TEXT("Pitch : %f"), Rotation.Pitch); 
		UE_LOG(LogTemp, Warning, TEXT("Yaw : %f"), Rotation.Yaw);

		// this->SetBoneRotationByName("upperarm_l", (defaultUpperArmLRot + Rotation).Quaternion().Rotator(), EBoneSpaces::WorldSpace);
		this->SetBoneRotationByName("lowerarm_l", (Rotation).Quaternion().Rotator(), EBoneSpaces::WorldSpace);
	}
}

void UMyPoseableMeshComponent::OnFlystickData_Implementation(const int32 FlystickID, const FVector & Position, const FRotator & Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickData_Implementation"));

	/*
	FString out;  // just output those values for info.
	for (int32 i = 0; i < JoystickValues.Num(); i++) {
		out.Append(FString::Printf(TEXT(" %i: %f"), i, JoystickValues[i]));
	}
	UE_LOG(LogTemp, Display, TEXT("%i joysticks - %s "), JoystickValues.Num(), *out); 
	*/
}

void UMyPoseableMeshComponent::OnFlystickButton_Implementation(const int32 FlystickID, const int32 & ButtonIndex, const bool Pressed)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickButton_Implementation"));
}

void UMyPoseableMeshComponent::OnFlystickJoystick_Implementation(const int32 FlystickID, const TArray<float>& JoystickValues)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickJoystick_Implementation"));
}

void UMyPoseableMeshComponent::OnHandTracking_Implementation(const int32 HandID, const bool Right, const FVector & Translation, const FRotator & Rotation, const TArray<FDTrackFinger>& Fingers)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHandTracking_Implementation"));


	// HandID is the ID of the tracked body. There can be many, depending on your setup.
	if (HandID == m_hand_r) { this->SetBoneRotationByName("hand_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_01_r) { this->SetBoneRotationByName("thumb_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_02_r) { this->SetBoneRotationByName("thumb_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_03_r) { this->SetBoneRotationByName("thumb_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_01_r) { this->SetBoneRotationByName("index_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_02_r) { this->SetBoneRotationByName("index_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_03_r) { this->SetBoneRotationByName("index_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_01_r) { this->SetBoneRotationByName("middle_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_02_r) { this->SetBoneRotationByName("middle_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_03_r) { this->SetBoneRotationByName("middle_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_01_r) { this->SetBoneRotationByName("ring_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_02_r) { this->SetBoneRotationByName("ring_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_03_r) { this->SetBoneRotationByName("ring_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_01_r) { this->SetBoneRotationByName("pinky_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_02_r) { this->SetBoneRotationByName("pinky_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_03_r) { this->SetBoneRotationByName("pinky_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else { UE_LOG(LogTemp, Warning, TEXT("HandID of %d is not assigned!"), HandID); } 
}

void UMyPoseableMeshComponent::OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint>& Joints)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHumanModel_Implementation"));
}
