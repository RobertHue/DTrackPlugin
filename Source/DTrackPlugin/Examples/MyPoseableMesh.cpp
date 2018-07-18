// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPoseableMesh.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

//#include "Helper/SpaceConverter.h" // for RAD_TO_DEG


AMyPoseableMesh::AMyPoseableMesh()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("-------------------------------------------------"));
	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor start: AMyPoseableMesh"));
	this->SetActorEnableCollision(true); 

	//// Character rotation only changes in Yaw, to prevent the capsule from changing orientation.
	//// Ask the Controller for the full rotation if desired (ie for aiming).
	//bUseControllerRotationPitch = false;
	//bUseControllerRotationRoll = false;
	//bUseControllerRotationYaw = true;


	//////////////////////
	// collision capsule
	//////////////////////
	m_pCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CAPSULE NAME 01"));
	m_pCapsuleComponent->InitCapsuleSize(42.f, 96.0f);
	m_pCapsuleComponent->SetCollisionProfileName(TEXT("PAWN PROFILE NAME 01"));
	//----------
	m_pCapsuleComponent->CanCharacterStepUpOn = ECB_No;
	m_pCapsuleComponent->bShouldUpdatePhysicsVolume = true;
	m_pCapsuleComponent->bCheckAsyncSceneOnMove = false;
	m_pCapsuleComponent->SetCanEverAffectNavigation(false);
	m_pCapsuleComponent->bDynamicObstacle = true;

	/////////////////////////////
	// Create a CameraComponent	
	/////////////////////////////
	m_pCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("m_pCameraComponent"));
	m_pCameraComponent->bUsePawnControlRotation = true;
	m_pCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Rel. Position of the camera

	//////////////////////
	// Poseable Mesh (right hand)
	//////////////////////
	m_pPoseableMeshComponentLeftHand = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("m_pPoseableMeshComponent_LeftHand"));
	m_pPoseableMeshComponentLeftHand->bWantsInitializeComponent = true;
	////////////////////////////////
	m_pPoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("m_pPoseableMeshComponent_RightHand"));
	m_pPoseableMeshComponent->bWantsInitializeComponent = true;
	//m_pPoseableMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//m_pPoseableMeshComponent->RelativeLocation = FVector(30.f, 20.f, 0.f); // Rel. Position of the camera // is not being used if the BP using the CPP class is changing the rel. loc

	//////////////////////
	// DTrack Component
	//////////////////////
	m_dtrack_component = CreateDefaultSubobject<UDTrackComponent>(TEXT("m_dtrack_component"));
	if (!m_dtrack_component) {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could not be created"));  // m_pPoseableMeshComponent shouldn't happen but you may treat it.
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could be created :D"));  // m_pPoseableMeshComponent shouldn't happen but you may treat it.
	}

	//////////////////////
	// Enable Physics
	//////////////////////
	//m_pCapsuleComponent->SetSimulatePhysics(true);

	// m_pPoseableMeshComponent->SetupAttachment(m_pPoseableMeshComponent);  
	// Initializes desired Attach Parent and SocketName to be attached to when the component is registered.
	// Generally intended to be called from its Owning Actor's constructor and should be preferred over AttachToComponent when
	// a component is not registered.
	//
	// m_pPoseableMeshComponent->AttachTo(...); // DEPRECATED(4.12, "m_pPoseableMeshComponent function is deprecated, please use AttachToComponent instead.")
	// Attach m_pPoseableMeshComponent component to another scene component, optionally at a named socket. 
	// It is valid to call m_pPoseableMeshComponent on components whether or not they have been Registered. 
	// (calls AttachToComponent)
	//
	// m_pPoseableMeshComponent->AttachToComponent(...);	
	// It is valid to call m_pPoseableMeshComponent on components whether or not they have been Registered, 
	// however from constructor or when not registered it is preferable to use SetupAttachment.

	///////////////////////////////////////
	// SetupAttachment only for USceneComponents (DTrack-Component is not one of them...)
	RootComponent = m_pCapsuleComponent;	// RootComponent is the Collision Primitive (which defines the transform (location, rotation, scale) of this Actor)
	m_pCameraComponent->SetupAttachment(RootComponent);
	m_pPoseableMeshComponent->SetupAttachment(RootComponent);
	m_pPoseableMeshComponentLeftHand->SetupAttachment(RootComponent);


	// other inits:
	m_HandREndEffectorLocations.Init(FVector::ZeroVector, 5);
	m_HandREndEffectorRotators.Init(FRotator::ZeroRotator, 5);
	m_FingerData.Init(FDTrackFinger(), 5);
	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor end: AMyPoseableMesh"));
} 

void AMyPoseableMesh::PostLoad()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostLoad()"));
	Super::PostLoad();
	// here the SkinnedMeshComp has a SkeletalMesh ("SK_Mannequin") but without preset rotations
}

// If you want object to react on invidual property chages in editor use PostEditChangeProperty
void AMyPoseableMesh::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	// m_pPoseableMeshComponent gets registered after  PostEditChangeProperty (calls OnConstruction)
	//UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostEditChangeProperty()"));
	Super::PostEditChangeProperty(e);
	int32 index = e.GetArrayIndex(TEXT("m_SkeletonRotations"));
	// Property : boneRotation	MemberProperty : m_HandR
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostEditChangeProperty()"));
} 
//for TArrays, TMap, etc...:
void AMyPoseableMesh::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e)
{ 
	//UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostEditChangeChainProperty()"));
	Super::PostEditChangeChainProperty(e);

	// only change what has been changed and dont adapt everything...
	FName PropertyName = e.GetPropertyName();	// is m_pRawRefBonePoseMap...
	UProperty* up = e.Property;	// the actual property that changed... 

	//	use GetArrayIndex() on the FPropertyChangedChainEvent parameter to get the element index whose property is being changed
	int32 index = e.GetArrayIndex(TEXT("m_SkeletonRotations"));
	if (index >= 0 && index < m_pPoseableMeshComponent->GetNumBones()) {
		FName boneName = m_pPoseableMeshComponent->GetBoneName(index); 
		FQuat RawRot = m_RawSkeletonRotations[index].boneRotation;

		FName propName = e.GetPropertyName();	// Possiblities: Roll Pitch Yaw

		FVector localBoneAxis;
		float angle;
		if (propName == "Roll") {
			localBoneAxis = RawRot.GetAxisX();
			angle = m_SkeletonRotations[index].boneRotator.Roll * DEG_TO_RAD;
		}
		else if (propName == "Pitch") {
			localBoneAxis = RawRot.GetAxisY();
			angle = m_SkeletonRotations[index].boneRotator.Pitch * DEG_TO_RAD;
		}
		else if (propName == "Yaw") {
			localBoneAxis = RawRot.GetAxisZ();
			angle = m_SkeletonRotations[index].boneRotator.Yaw * DEG_TO_RAD;
		}
		else {
			// do nothing 
			return;
		}
		FQuat newRot = FQuat(localBoneAxis, angle); // additive quat mult
		m_pPoseableMeshComponent->SetBoneRotationByName(boneName, (newRot * RawRot).Rotator(), EBoneSpaces::WorldSpace);
	}
}



void AMyPoseableMesh::PostActorCreated()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostActorCreated()"));
	Super::PostActorCreated();
}

//You use RegisterComponent() when you dynamically create a component at runtime.
//Typically, I would use it when adding component in the Construction script, 
//for example.Otherwise, your component will not show up and/or problems could occur.
void AMyPoseableMesh::OnConstruction(const FTransform & Transform)
{
	//UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::OnConstruction()"));
	Super::OnConstruction(Transform);

	// to avoid null-pointer exceptions:
	if (m_pPoseableMeshComponent == nullptr) { return; }

	// only assign once ((m_SkeletonRotations.Num() <= 0)) &&  AND only assign if PoseableMeshComp has Bones...
	if (m_SkeletonRotations.Num() <= 0 && m_pPoseableMeshComponent->GetNumBones() > 0) {
		TArray <FName> BoneNames;
		m_pPoseableMeshComponent->GetBoneNames(BoneNames);
		for (int32 i = 0; i < BoneNames.Num(); ++i) {
			FName NameOfBone = BoneNames[i];
			FQuat RotOfBone = m_pPoseableMeshComponent->GetBoneQuaternion(NameOfBone, EBoneSpaces::WorldSpace);
			FRotator RotatorOfBone = m_pPoseableMeshComponent->GetBoneRotationByName(NameOfBone, EBoneSpaces::WorldSpace);
			//m_DTrackIDToBoneMap.Add(-1, NameOfBone); 
			m_SkeletonRotations.Add(FBoneNameToBoneRotation(i, NameOfBone, FQuat(0,0,0,1), FRotator(0,0,0)));
			m_RawSkeletonRotations.Add(FBoneNameToBoneRotation(i, NameOfBone, RotOfBone, RotatorOfBone));
		}
	} 
}

void AMyPoseableMesh::PreInitializeComponents()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PreInitializeComponents()"));
	Super::PreInitializeComponents();
}

void AMyPoseableMesh::PostInitializeComponents()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostInitializeComponents()"));
	Super::PostInitializeComponents();
}

void AMyPoseableMesh::BeginPlay()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::BeginPlay()"));
	Super::BeginPlay();

	if (m_pPoseableMeshComponent != nullptr) {
		AActor* actorThatOwnsm_pPoseableMeshComponentComponent = m_pPoseableMeshComponent->GetOwner();
		if (actorThatOwnsm_pPoseableMeshComponentComponent) {
			FString name = actorThatOwnsm_pPoseableMeshComponentComponent->GetName();
			UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns m_pPoseableMeshComponent component(UMyPoseableMeshComp) : %s"), *name);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////// IK-Solver Tests
	// setup the segments
	//int def_DOF = 1;

	//IKSegment *hand_r_Segment = new IKSegment(def_DOF);
	//hand_r_Segment->SetDoFId(0);
	//hand_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	//hand_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("hand_r"));

	//IKSegment *index_01_r_Segment = new IKSegment(def_DOF);
	//index_01_r_Segment->SetDoFId(1);
	//index_01_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	//index_01_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_01_r"));

	//IKSegment *index_02_r_Segment = new IKSegment(def_DOF);
	//index_02_r_Segment->SetDoFId(2);
	//index_02_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	//index_02_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_02_r"));

	//IKSegment *index_03_r_Segment = new IKSegment(def_DOF);
	//index_03_r_Segment->SetDoFId(3);
	//index_03_r_Segment->SetRotationAxis(FVector(0,0,1));
	//index_03_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_03_r"));

	//// setup the kinematic chain
	//index_03_r_Segment->SetParent(index_02_r_Segment);
	//index_02_r_Segment->SetParent(index_01_r_Segment);
	//index_01_r_Segment->SetParent(hand_r_Segment);
	//m_ik_solver = new IKSolver(hand_r_Segment);	// hand as root of kinematic chain


	// cache the start pos of the player:
	m_PlayerStartPos = this->GetActorLocation();
}

void AMyPoseableMesh::OnBodyData_Implementation(const int32 BodyID, const FVector & Position, const FRotator & Rotation)
{ 
	UE_LOG(LogTemp, Warning, TEXT("OnBodyData_Implementation"));


	if (BodyID == 2) {
		this->SetActorLocation(
			Position + m_PlayerStartPos,
			true,			// bSweep - stop if the target is collided with something
			nullptr,
			ETeleportType::None		// collisions along the way will occur
		);
		// only set location everything else is unnatural
		//m_pCameraComponent->SetWorldRotation(FQuat(Rotation));
		//m_pCameraComponent->RelativeRotation = Rotation;
		//this->SetActorRotation(FQuat(Rotation));
	}
}

void AMyPoseableMesh::OnFlystickData_Implementation(const int32 FlystickID, const FVector & Position, const FRotator & Rotation)
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

void AMyPoseableMesh::OnFlystickButton_Implementation(const int32 FlystickID, const int32 & ButtonIndex, const bool Pressed)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickButton_Implementation"));
}

void AMyPoseableMesh::OnFlystickJoystick_Implementation(const int32 FlystickID, const TArray<float>& JoystickValues)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFlystickJoystick_Implementation"));
}

void AMyPoseableMesh::OnHandTracking_Implementation(const int32 HandID, const bool Right, const FVector & Translation/*TODO: rather the Location*/, const FRotator & Rotation, const TArray<FDTrackFinger>& Fingers)
{
	const EBoneSpaces::Type SpaceType = EBoneSpaces::WorldSpace;
	UE_LOG(LogTemp, Warning, TEXT("OnHandTracking_Implementation"));

	if (Right == true) {
		FName boneNameHand = "hand_r";
		FQuat NewHandRot = FQuat(Rotation); //  * FQuat(m_rotatorPatchForHand);

		FVector locationIndexFingerBase = m_pPoseableMeshComponent->GetBoneLocationByName("index_01_r", SpaceType);		// world location
		FVector locationBackOfHand = m_pPoseableMeshComponent->GetBoneLocationByName("hand_r", SpaceType);				// world location
		FVector relativeDBackOfHand = locationIndexFingerBase - locationBackOfHand;

		FVector NewHandPos = Translation + m_PlayerStartPos - relativeDBackOfHand;
		m_pPoseableMeshComponent->SetBoneLocationByName(boneNameHand, NewHandPos, SpaceType);
		m_pPoseableMeshComponent->SetBoneRotationByName(boneNameHand, (NewHandRot).Rotator(), SpaceType);

		int32 idxOffset;
		for (int i=0; i<Fingers.Num(); ++i)
		{
			switch (Fingers[i].m_type) {
				case EDTrackFingerType::FT_Thumb:  idxOffset = 13; break;
				case EDTrackFingerType::FT_Index:  idxOffset =  1; break;
				case EDTrackFingerType::FT_Middle: idxOffset =  4; break;
				case EDTrackFingerType::FT_Ring:   idxOffset =  7; break;
				case EDTrackFingerType::FT_Pinky:  idxOffset = 10; break;
			}
			FRotator indexFingerRotation = Fingers[i].m_rotation;
			float indexFinger00Angle = Fingers[i].m_hand_inner_phalanx_angle_yaw;
			float indexFinger01Angle = Fingers[i].m_hand_inner_phalanx_angle_pitch;
			float indexFinger02Angle = Fingers[i].m_inner_middle_phalanx_angle;
			float indexFinger03Angle = Fingers[i].m_middle_outer_phalanx_angle;
			UE_LOG(LogTemp, Warning, TEXT("FINGER_DEG : hand: %s - fingerTip: %s - a:%f b:%f c:%f"), *Rotation.ToString(), *(Fingers[1].m_rotation).ToString(), indexFinger01Angle, indexFinger02Angle, indexFinger03Angle);
			 
			//int32 index = 30; // do not do it via indices and rather use the name itself!
			FName handBoneRName = "hand_r";
			int32 idxHandBone = m_pPoseableMeshComponent->GetBoneIndex(handBoneRName);
			FName indexBoneRName01 = m_pPoseableMeshComponent->GetBoneName(idxHandBone + idxOffset + 0);
			FName indexBoneRName02 = m_pPoseableMeshComponent->GetBoneName(idxHandBone + idxOffset + 1);
			FName indexBoneRName03 = m_pPoseableMeshComponent->GetBoneName(idxHandBone + idxOffset + 2);

			FQuat ParentRot;

			///////////////////////////////////
			// assign to UE4-Skeleton: (in WorldSpace) - dunno why UE4 removed bone local space... But that cant be easily calculated by qChild * qParent

			/// newRot3 = Finger Tip					- PitchAngle
			/// newRot2 = Finger Middle Phalanx			- PitchAngle
			/// newRot1 = Finger Inner Phalanx (Base)	- PitchAngle
			/// newRot0 = Finger Inner Phalanx (Base)	- YawAngle

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			//FVector localBoneAxis0 = ParentRot.GetAxisY();
			//float angle0 = indexFinger00Angle * DEG_TO_RAD;
			//FQuat newRot0(localBoneAxis0, angle0);
			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			//FVector localBoneAxis1 = ParentRot.GetAxisZ();
			//float angle1 = indexFinger01Angle * DEG_TO_RAD;
			//FQuat newRot1(localBoneAxis1, angle1);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName01, ((newRot1 * newRot0) * m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType)).Rotator(), SpaceType);

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName02)].boneRotation;
			//FVector localBoneAxis2 = ParentRot.GetAxisZ();
			//float angle2 = indexFinger02Angle * DEG_TO_RAD;
			//FQuat newRot2(localBoneAxis2, angle2);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName02, (newRot2 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType)).Rotator(), SpaceType);

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName03)].boneRotation;
			//FVector localBoneAxis3 = ParentRot.GetAxisZ();
			//float angle3 = indexFinger03Angle * DEG_TO_RAD; 
			//FQuat newRot3(localBoneAxis3, angle3);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName03, (newRot3 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType)).Rotator(), SpaceType);

			//////////////////////////////

			FRotator indexFingerInnerRotator = (Fingers[i].m_hand_inner_phalanx_quater).Rotator();
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName01, indexFingerInnerRotator, SpaceType);

			FRotator indexFingerMiddleRotator = (Fingers[i].m_inner_middle_phalanx_quater).Rotator();
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName02, indexFingerMiddleRotator, SpaceType);

			FRotator indexFingerTipRotator = (Fingers[i].m_middle_outer_phalanx_quater).Rotator();
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName03, indexFingerTipRotator, SpaceType);



			//////////////////////////////


			// some IK testing here:
			FQuat   quaterOfHand = m_pPoseableMeshComponent->GetBoneQuaternion("hand_r", SpaceType);
			FVector relLocationOfFingerTip = Fingers[i].m_relLocation;	// doesn't change when you rotate just the back of your hand without changing your finger angles

			FVector NewEndEffectorLocation1 = Fingers[i].m_location + m_PlayerStartPos;			// DTrackGlobalRoomCoords + UE4GlobalWorldCoords (geht auch :D)
			FVector NewEndEffectorLocation2 = relLocationOfFingerTip + locationIndexFingerBase;	// DTrackLokalHandCoords  + UE4GlobalWorldIndexFingerBaseCoords (geht auch :D)

																								//m_pPoseableMeshComponent->SetBoneLocationByName(indexBoneRName03, NewEndEffectorLocation, SpaceType);
			m_HandREndEffectorLocations[i] = NewEndEffectorLocation1;
			m_HandREndEffectorRotators[i] = indexFingerTipRotator;

			m_HandLocation = m_pPoseableMeshComponent->GetBoneLocationByName("hand_r", SpaceType);
			m_HandRotator = Rotation; // m_pPoseableMeshComponent->GetBoneRotationByName("hand_r", SpaceType);

			m_FingerData[i] = Fingers[i];	// copy by value

			if (i == 1) // finger == index 
			{
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: ----------------------------------"));
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: NewEndEffectorLocation1 : %s"), *NewEndEffectorLocation1.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: NewEndEffectorLocation2 : %s"), *NewEndEffectorLocation2.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: worldLocationFingerTip : %s"), *Fingers[i].m_location.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: relLocationOfFingerTip : %s"), *relLocationOfFingerTip.ToString());
			}
			 
			///////////////////////////////////////////////////////////////////////////////////////////////////
			//UE_LOG(LogTemp, Warning, TEXT("FINGER_ANGLE_DEG : %f"), indexFinger01Angle);
		}
	}  
	//////////////////////////////////////////////////////////////////
	// following is a copy of above TODO: avoid code duplicates...
	else if (Right == false) {
		FName boneNameHand = "hand_r";
		FQuat NewHandRot = FQuat(Rotation); //  * FQuat(m_rotatorPatchForHand);

		FVector locationIndexFingerBase = m_pPoseableMeshComponentLeftHand->GetBoneLocationByName("index_01_r", SpaceType);		// world location
		FVector locationBackOfHand = m_pPoseableMeshComponentLeftHand->GetBoneLocationByName("hand_r", SpaceType);				// world location
		FVector relativeDBackOfHand = locationIndexFingerBase - locationBackOfHand;

		FVector NewHandPos = Translation + m_PlayerStartPos - relativeDBackOfHand;
		m_pPoseableMeshComponentLeftHand->SetBoneLocationByName(boneNameHand, NewHandPos, SpaceType);
		m_pPoseableMeshComponentLeftHand->SetBoneRotationByName(boneNameHand, (NewHandRot).Rotator(), SpaceType);

		int32 idxOffset;
		for (int i = 0; i<Fingers.Num(); ++i)
		{
			switch (Fingers[i].m_type) {
			case EDTrackFingerType::FT_Thumb:  idxOffset = 13; break;
			case EDTrackFingerType::FT_Index:  idxOffset = 1; break;
			case EDTrackFingerType::FT_Middle: idxOffset = 4; break;
			case EDTrackFingerType::FT_Ring:   idxOffset = 7; break;
			case EDTrackFingerType::FT_Pinky:  idxOffset = 10; break;
			}
			FRotator indexFingerRotation = Fingers[i].m_rotation;
			float indexFinger00Angle = Fingers[i].m_hand_inner_phalanx_angle_yaw;
			float indexFinger01Angle = Fingers[i].m_hand_inner_phalanx_angle_pitch;
			float indexFinger02Angle = Fingers[i].m_inner_middle_phalanx_angle;
			float indexFinger03Angle = Fingers[i].m_middle_outer_phalanx_angle;
			UE_LOG(LogTemp, Warning, TEXT("FINGER_DEG : hand: %s - fingerTip: %s - a:%f b:%f c:%f"), *Rotation.ToString(), *(Fingers[1].m_rotation).ToString(), indexFinger01Angle, indexFinger02Angle, indexFinger03Angle);

			//int32 index = 30; // do not do it via indices and rather use the name itself!
			FName handBoneRName = "hand_r";
			int32 idxHandBone = m_pPoseableMeshComponentLeftHand->GetBoneIndex(handBoneRName);
			FName indexBoneRName01 = m_pPoseableMeshComponentLeftHand->GetBoneName(idxHandBone + idxOffset + 0);
			FName indexBoneRName02 = m_pPoseableMeshComponentLeftHand->GetBoneName(idxHandBone + idxOffset + 1);
			FName indexBoneRName03 = m_pPoseableMeshComponentLeftHand->GetBoneName(idxHandBone + idxOffset + 2);

			FQuat ParentRot;

			///////////////////////////////////
			// assign to UE4-Skeleton: (in WorldSpace) - dunno why UE4 removed bone local space... But that cant be easily calculated by qChild * qParent

			/// newRot3 = Finger Tip					- PitchAngle
			/// newRot2 = Finger Middle Phalanx			- PitchAngle
			/// newRot1 = Finger Inner Phalanx (Base)	- PitchAngle
			/// newRot0 = Finger Inner Phalanx (Base)	- YawAngle

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			//FVector localBoneAxis0 = ParentRot.GetAxisY();
			//float angle0 = indexFinger00Angle * DEG_TO_RAD;
			//FQuat newRot0(localBoneAxis0, angle0);
			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			//FVector localBoneAxis1 = ParentRot.GetAxisZ();
			//float angle1 = indexFinger01Angle * DEG_TO_RAD;
			//FQuat newRot1(localBoneAxis1, angle1);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName01, ((newRot1 * newRot0) * m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType)).Rotator(), SpaceType);

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName02)].boneRotation;
			//FVector localBoneAxis2 = ParentRot.GetAxisZ();
			//float angle2 = indexFinger02Angle * DEG_TO_RAD;
			//FQuat newRot2(localBoneAxis2, angle2);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName02, (newRot2 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType)).Rotator(), SpaceType);

			//ParentRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName03)].boneRotation;
			//FVector localBoneAxis3 = ParentRot.GetAxisZ();
			//float angle3 = indexFinger03Angle * DEG_TO_RAD; 
			//FQuat newRot3(localBoneAxis3, angle3);
			//m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName03, (newRot3 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType)).Rotator(), SpaceType);

			//////////////////////////////

			FRotator indexFingerInnerRotator = (Fingers[i].m_hand_inner_phalanx_quater).Rotator();
			m_pPoseableMeshComponentLeftHand->SetBoneRotationByName(indexBoneRName01, indexFingerInnerRotator, SpaceType);

			FRotator indexFingerMiddleRotator = (Fingers[i].m_inner_middle_phalanx_quater).Rotator();
			m_pPoseableMeshComponentLeftHand->SetBoneRotationByName(indexBoneRName02, indexFingerMiddleRotator, SpaceType);

			FRotator indexFingerTipRotator = (Fingers[i].m_middle_outer_phalanx_quater).Rotator();
			m_pPoseableMeshComponentLeftHand->SetBoneRotationByName(indexBoneRName03, indexFingerTipRotator, SpaceType);



			//////////////////////////////


			// some IK testing here:
			FQuat   quaterOfHand = m_pPoseableMeshComponentLeftHand->GetBoneQuaternion("hand_r", SpaceType);
			FVector relLocationOfFingerTip = Fingers[i].m_relLocation;	// doesn't change when you rotate just the back of your hand without changing your finger angles

			FVector NewEndEffectorLocation1 = Fingers[i].m_location + m_PlayerStartPos;			// DTrackGlobalRoomCoords + UE4GlobalWorldCoords (geht auch :D)
			FVector NewEndEffectorLocation2 = relLocationOfFingerTip + locationIndexFingerBase;	// DTrackLokalHandCoords  + UE4GlobalWorldIndexFingerBaseCoords (geht auch :D)

																								//m_pPoseableMeshComponentLeftHand->SetBoneLocationByName(indexBoneRName03, NewEndEffectorLocation, SpaceType);
			m_HandREndEffectorLocations[i] = NewEndEffectorLocation1;
			m_HandREndEffectorRotators[i] = indexFingerTipRotator;

			m_HandLocation = m_pPoseableMeshComponentLeftHand->GetBoneLocationByName("hand_r", SpaceType);
			m_HandRotator = Rotation; // m_pPoseableMeshComponentLeftHand->GetBoneRotationByName("hand_r", SpaceType);

			m_FingerData[i] = Fingers[i];	// copy by value

			if (i == 1) // finger == index 
			{
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: ----------------------------------"));
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: NewEndEffectorLocation1 : %s"), *NewEndEffectorLocation1.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: NewEndEffectorLocation2 : %s"), *NewEndEffectorLocation2.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: worldLocationFingerTip : %s"), *Fingers[i].m_location.ToString());
				UE_LOG(LogTemp, Warning, TEXT("IK_TEST: relLocationOfFingerTip : %s"), *relLocationOfFingerTip.ToString());
			}

			///////////////////////////////////////////////////////////////////////////////////////////////////
			//UE_LOG(LogTemp, Warning, TEXT("FINGER_ANGLE_DEG : %f"), indexFinger01Angle);
		}
	}
}

void AMyPoseableMesh::OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint>& Joints)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHumanModel_Implementation"));
}