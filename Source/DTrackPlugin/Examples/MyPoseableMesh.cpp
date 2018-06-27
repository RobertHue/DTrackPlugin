// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPoseableMesh.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Helper/CoordinateConverter.h" // for RAD_TO_DEG


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
	m_pCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	m_pCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	m_pCameraComponent->bUsePawnControlRotation = true;

	//////////////////////
	// Poseable Mesh
	//////////////////////
	m_pPoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("UPoseableMeshComponent"));
	m_pPoseableMeshComponent->bWantsInitializeComponent = true;
	m_pPoseableMeshComponent->RelativeLocation = FVector(0, 64.f, 64.f); // Position the hand
	m_pPoseableMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//////////////////////
	// DTrack Component
	//////////////////////
	m_dtrack_component = CreateDefaultSubobject<UDTrackComponent>(TEXT("DTrackComponent"));
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
	RootComponent = m_pCameraComponent;//  m_pCapsuleComponent;
	m_pPoseableMeshComponent->SetupAttachment(RootComponent);	// only for USceneComponents
	m_pCapsuleComponent->SetupAttachment(RootComponent);
	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor end: AMyPoseableMesh"));
} 

void AMyPoseableMesh::PostLoad()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostLoad()"));
	Super::PostLoad();
	// here the SkinnedMeshComp has a SkeletalMesh ("SK_Mannequin")

	//if (BoneQuats.Num() == 0) {

	//	const TArray<FMeshBoneInfo>& rawRefBoneInfos = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBoneInfo();
	//	const TArray<FTransform>& rawRefBonePoses = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBonePose();

	//	for (int i = 0; i < rawRefBonePoses.Num(); ++i) {
	//		BoneQuats.Add(rawRefBoneInfos[i].Name, rawRefBonePoses[i].GetRotation()); 
	//		//m_BoneNameToPoseMap.Add(FBoneNameToTransform{ rawRefBoneInfos[i].Name, rawRefBonePoses[i] });
	//	} 
	//}

	//if (m_BoneToDTrackIDMap.Num() <= 0) {	// only assign skeleton bone names if map is empty
	//	TArray <FName> BoneNames;
	//	m_pPoseableMeshComponent->GetBoneNames(BoneNames);
	//	for (int i = 0; i < BoneNames.Num(); ++i) {
	//		FName NameOfBone = BoneNames[i];
	//		m_BoneToDTrackIDMap.Add(NameOfBone, i);
	//	}
	//}


	// only assign once AND only assign if PoseableMeshComp has Bones...
	//if (m_hand_r_bones.Num() <= 0 && m_pPoseableMeshComponent->GetNumBones() > 0) {
	//	for (int32 id = 29; id <= 44; ++id) {
	//		FName boneName = m_pPoseableMeshComponent->GetBoneName(id);
	//		FBoneNameToBoneRotation test(id,
	//			boneName,
	//			m_pPoseableMeshComponent->GetBoneQuaternion(boneName, EBoneSpaces::WorldSpace)
	//		);
	//		m_hand_r_bones.Add(FBoneNameToBoneRotation(id,
	//			boneName, 
	//			m_pPoseableMeshComponent->GetBoneQuaternion(boneName, EBoneSpaces::WorldSpace)
	//		));
	//	}
	//}

	/*if (m_hand_l_bones.Num() <= 0) {
		for (int32 i = 8; i <= 23; ++i) {
			int32 id = i;
			m_hand_l_bones.Add(FBoneNameToBoneRotation(id,
				m_pPoseableMeshComponent->GetBoneName(id),
				m_pPoseableMeshComponent->GetBoneQuaternion(m_pPoseableMeshComponent->GetBoneName(id), EBoneSpaces::WorldSpace)
			)); 
		}
	}*/
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
	//int32 index = e.GetArrayIndex(TEXT("m_hand_data_r"));
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyPoseableMesh, BoneQuats)) {

	//	// use GetArrayIndex() on the FPropertyChangedChainEvent parameter to get the element index whose property is being changed
	//	int32 index = e.GetArrayIndex(TEXT("BoneQuats"));

	//	if (index >= 0 && index <= (BoneQuats.Num() - 1)) {
	//		FName boneName = m_pPoseableMeshComponent->GetBoneName(index); 
	//		m_pPoseableMeshComponent->SetBoneRotationByName(boneName, BoneQuats[boneName].Rotator(), EBoneSpaces::WorldSpace);
	//		// m_pPoseableMeshComponent->SetBoneTransformByName(boneName, m_pRawRefBonePoseMap[index], EBoneSpaces::WorldSpace);
	//	}
	//}
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

	AActor* actorThatOwnsm_pPoseableMeshComponentComponent = m_pPoseableMeshComponent->GetOwner();
	if (actorThatOwnsm_pPoseableMeshComponentComponent) {
		FString name = actorThatOwnsm_pPoseableMeshComponentComponent->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of Actor that owns m_pPoseableMeshComponent component(UMyPoseableMeshComp) : %s"), *name);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
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
	/*FName boneName = m_BoneToDTrackIDMap[BodyID];
	m_pPoseableMeshComponent->SetBoneRotationByName(boneName, Rotation, EBoneSpaces::WorldSpace);*/	
	 
	/*
	if (BodyID == 1) {
		this->SetActorLocation(
			Position + m_PlayerStartPos, 
			true,			// bSweep - stop if the target is collided with something
			nullptr, 
			ETeleportType::None		// collisions along the way will occur
		);
		// only set location everything else is unnatural
	}



	//const FName* pBoneName = m_BoneToDTrackIDMap.FindKey(BodyID);	// takes O(N)
	const FName& rBoneName = m_DTrackIDToBoneMap[BodyID];	// takes O(1)
	m_pPoseableMeshComponent->SetBoneRotationByName(
		rBoneName,
		FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll), 
		EBoneSpaces::WorldSpace
	);*/
	 
	//if (BodyID == 2) {
	//	const FRotator defaultPose2 = Rotation;
	//	FName boneName("upperarm_r");
	//	const TArray<FTransform>& boneTransforms = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBonePose();
	//	int32 boneIndex = m_pPoseableMeshComponent->GetBoneIndex(boneName);
	//	FRotator rawRotation = boneTransforms[boneIndex].Rotator();
	//	m_pPoseableMeshComponent->SetBoneRotationByName(boneName, (Rotation - defaultPose2) + rawRotation, EBoneSpaces::WorldSpace);
	//}
	//if (BodyID == 1) { 
	//	const FRotator defaultPose1 = Rotation;
	//	FName boneName("lowerarm_r");
	//	const TArray<FTransform>& boneTransforms = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBonePose();
	//	int32 boneIndex = m_pPoseableMeshComponent->GetBoneIndex(boneName);
	//	FRotator rawRotation = boneTransforms[boneIndex].Rotator();
	//	m_pPoseableMeshComponent->SetBoneRotationByName(boneName, (Rotation - defaultPose1) + rawRotation, EBoneSpaces::WorldSpace);
	//}

	//if (BodyID == 2) { 
	//	//m_pPoseableMeshComponent->SetBoneRotationByName("hand_r", Rotation, EBoneSpaces::WorldSpace);
	//	//m_pPoseableMeshComponent->SetBoneLocationByName("hand_r", Position + m_cachedHandRLocation, EBoneSpaces::WorldSpace);
	//}

	//if (BodyID == 1) {

	//	IKSegment *pSeg = nullptr;
	//	 
	//	if(m_ik_solver == nullptr) { UE_LOG(LogTemp, Warning, TEXT("m_ik_solver is nullptr")); }
	//	else {
	//		FVector handBonePos = m_pPoseableMeshComponent->GetBoneLocation("hand_r", EBoneSpaces::WorldSpace);
	//		FVector newPos = Position + handBonePos;
	//		UE_LOG(LogTemp, Warning, TEXT("Position : %s"), *(Position).ToString());
	//		UE_LOG(LogTemp, Warning, TEXT("TargetPos : newPos : %s"), *(newPos).ToString());
	//		m_ik_solver->Solve(newPos);	// newPos as TargetPos of Kinematic Chain
	//		pSeg = m_ik_solver->GetRootSegment();
	//		pSeg = pSeg->Child();	// advance pSeg pointer
	//	} 

	//	if (pSeg==nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg is nullptr")); }
	//	else {
	//		UE_LOG(LogTemp, Warning, TEXT("pSeg : %f"), pSeg->GetTheta());
	//	} 


	//	if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child() is nullptr")); }
	//	else {
	//		double thetaRad1 = pSeg->GetTheta(); 
	//		double thetaDeg1 = thetaRad1 * RAD_TO_DEG;
	//		UE_LOG(LogTemp, Warning, TEXT("r1 : thetaDeg1 : %f"), thetaDeg1);
	//		FQuat q1 = FQuat(FRotator(0, 0, thetaDeg1)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("hand_r"), EBoneSpaces::WorldSpace);
	//		m_pPoseableMeshComponent->SetBoneRotationByName("index_01_r", q1.Rotator(), EBoneSpaces::WorldSpace);
	//		pSeg = pSeg->Child();	// advance pSeg pointer
	//	} 

	//	if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child()->Child() is nullptr")); }
	//	else {
	//		double thetaRad2 = pSeg->GetTheta();
	//		double thetaDeg2 = thetaRad2 * RAD_TO_DEG;
	//		UE_LOG(LogTemp, Warning, TEXT("r2 : thetaDeg2 : %f"), thetaDeg2);
	//		FQuat q2 =  FQuat(FRotator(0, 0, thetaDeg2)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("index_01_r"), EBoneSpaces::WorldSpace);
	//		m_pPoseableMeshComponent->SetBoneRotationByName("index_02_r", q2.Rotator(), EBoneSpaces::WorldSpace);
	//		pSeg = pSeg->Child();	// advance pSeg pointer
	//	}

	//	if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child()->Child()->Child() is nullptr")); }
	//	else {
	//		double thetaRad3 = pSeg->GetTheta();
	//		double thetaDeg3 = thetaRad3 * RAD_TO_DEG;
	//		UE_LOG(LogTemp, Warning, TEXT("r3 : thetaDeg3 : %f"), thetaDeg3);
	//		FQuat q3 = FQuat(FRotator(0, 0, thetaDeg3)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("index_02_r"), EBoneSpaces::WorldSpace);
	//		m_pPoseableMeshComponent->SetBoneRotationByName("index_03_r", q3.Rotator(), EBoneSpaces::WorldSpace);
	//		pSeg = pSeg->Child();	// advance pSeg pointer 
	//	}
	//}

	/*
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

	// m_pPoseableMeshComponent->SetBoneRotationByName("upperarm_l", (defaultUpperArmLRot + Rotation).Quaternion().Rotator(), EBoneSpaces::WorldSpace);
	m_pPoseableMeshComponent->SetBoneRotationByName("lowerarm_l", (Rotation).Quaternion().Rotator(), EBoneSpaces::WorldSpace);
	}
	*/
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

void AMyPoseableMesh::OnHandTracking_Implementation(const int32 HandID, const bool Right, const FVector & Translation, const FRotator & Rotation, const TArray<FDTrackFinger>& Fingers)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHandTracking_Implementation"));


	if (Right == true) {
		FName boneNameHand = "hand_r";
		FQuat NewRotHand = FQuat(Rotation) * FQuat(m_rotatorPatchForHand);
		m_pPoseableMeshComponent->SetBoneLocationByName(boneNameHand, Translation + m_DefaultHandLocation + m_PlayerStartPos, EBoneSpaces::WorldSpace);
		m_pPoseableMeshComponent->SetBoneRotationByName(boneNameHand, (NewRotHand).Rotator(), EBoneSpaces::WorldSpace);
		//UE_LOG(LogTemp, Warning, TEXT("Hand-FINGER_DEG : %s"), *Rotation.ToString()); 

		int32 idxOffset;
		for (int i=0; i<5; ++i)
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
			/*FName indexBoneRName01 = "index_01_r";  
			FName indexBoneRName02 = "index_02_r"; 
			FName indexBoneRName03 = "index_03_r";*/
			//FName boneNameParent = m_pPoseableMeshComponent->GetBoneName(index-1);
			//FName boneName = m_pPoseableMeshComponent->GetBoneName(index);


			FQuat RawRot;

			// Possiblities:		Roll (X)		Pitch (Y)		Yaw (Z)
			//FName propName = "Yaw";		// UE-specific:		is rotation around the vector pointing to the side of the finger

			////////////////////////////////////////////////////////////
			// bring DTrack-Data in suitable format: (postprocessing)



			//if (propName == "Roll") {
			//	localBoneAxis = RawRot.GetAxisX();
			//	//angle = m_SkeletonRotations[index].boneRotator.Roll * DEG_TO_RAD;
			//}
			//else if (propName == "Pitch") {
			//	localBoneAxis = RawRot.GetAxisY();
			//	//angle = m_SkeletonRotations[index].boneRotator.Pitch * DEG_TO_RAD;
			//}
			//else if (propName == "Yaw") {
			//	localBoneAxis = RawRot.GetAxisZ();
			//	//angle = m_SkeletonRotations[index].boneRotator.Yaw * DEG_TO_RAD;
			//}
			//else {
			//	// do nothing   
			//	return;
			//}

			///////////////////////////////////
			// assign to UE4-Skeleton:
			const EBoneSpaces::Type SpaceType = EBoneSpaces::WorldSpace;

			RawRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName03)].boneRotation;
			FVector localBoneAxis3 = RawRot.GetAxisZ();
			float angle3 = indexFinger03Angle * DEG_TO_RAD;
			FQuat newRot3(localBoneAxis3, angle3);
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName03, (newRot3 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName02, SpaceType)).Rotator(), SpaceType);

			RawRot = m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName02)].boneRotation;
			FVector localBoneAxis2 = RawRot.GetAxisZ();
			float angle2 = indexFinger02Angle * DEG_TO_RAD;
			FQuat newRot2(localBoneAxis2, angle2);
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName02, (newRot2 * m_pPoseableMeshComponent->GetBoneQuaternion(indexBoneRName01, SpaceType)).Rotator(), SpaceType);

			RawRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			FVector localBoneAxis0 = RawRot.GetAxisY();
			float angle0 = indexFinger00Angle * DEG_TO_RAD;
			FQuat newRot0(localBoneAxis0, angle0);

			RawRot = m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType); // m_RawSkeletonRotations[m_pPoseableMeshComponent->GetBoneIndex(indexBoneRName01)].boneRotation;
			FVector localBoneAxis1 = RawRot.GetAxisZ();
			float angle1 = indexFinger01Angle * DEG_TO_RAD;
			FQuat newRot1(localBoneAxis1, angle1);
			m_pPoseableMeshComponent->SetBoneRotationByName(indexBoneRName01, ((newRot1) * m_pPoseableMeshComponent->GetBoneQuaternion(handBoneRName, SpaceType)).Rotator(), SpaceType);


			//UE_LOG(LogTemp, Warning, TEXT("FINGER_ANGLE_DEG : %f"), indexFinger01Angle);
		}

	}  
}

void AMyPoseableMesh::OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint>& Joints)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHumanModel_Implementation"));
}
 