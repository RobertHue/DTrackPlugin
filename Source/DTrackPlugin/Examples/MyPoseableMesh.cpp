// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPoseableMesh.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"

#include "Helper/CoordinateConverter.h" // for RAD_TO_DEG


AMyPoseableMesh::AMyPoseableMesh()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("-------------------------------------------------"));
	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor start: AMyPoseableMesh"));

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling m_pPoseableMeshComponent pawn)
	m_pPoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("UPoseableMeshComponent"));
	// m_pPoseableMeshComponent->SetupAttachment(FirstPersonCameraComponent);	// Attach mesh to FirstPersonCameraComponent
	m_pPoseableMeshComponent->bWantsInitializeComponent = true;

	// create DTrackComponent instance
	m_dtrack_component = CreateDefaultSubobject<UDTrackComponent>(TEXT("DTrackComponent"));
	if (!m_dtrack_component) {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could not be created"));  // m_pPoseableMeshComponent shouldn't happen but you may treat it.
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("DTrack Component could be created :D"));  // m_pPoseableMeshComponent shouldn't happen but you may treat it.
	}


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

	UE_LOG(LogSkeletalMesh, Warning, TEXT("Constructor end: AMyPoseableMesh"));
} 

void AMyPoseableMesh::PostLoad()
{
	UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostLoad()"));
	Super::PostLoad();
	// here the SkinnedMeshComp has a SkeletalMesh ("SK_Mannequin")

	// assign default values to the vars inside the editor
	const TArray<FTransform>& rawRefBonePose = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBonePose();

	m_pRawRefBonePose = m_pPoseableMeshComponent->SkeletalMesh->RefSkeleton.GetRawRefBonePose();

	if (m_BoneToDTrackIDMap.Num() <= 0) {	// only assign skeleton bone names if map is empty
		TArray <FName> BoneNames;
		m_pPoseableMeshComponent->GetBoneNames(BoneNames);
		for (int i = 0; i < BoneNames.Num(); ++i) {
			FName NameOfBone = BoneNames[i];
			m_BoneToDTrackIDMap.Add(NameOfBone, i);
		}
	}
}

#if WITH_EDITOR
// If you want object to react on invidual property chages in editor use PostEditChangeProperty
void AMyPoseableMesh::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	// m_pPoseableMeshComponent gets registered after  PostEditChangeProperty (calls OnConstruction)
	//UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostEditChangeProperty()"));
	Super::PostEditChangeProperty(e);

	if (m_pPoseableMeshComponent->SkeletalMesh->Skeleton == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Skeleton not set from editor yet!!!"));
	}
	else {		// Skeleton available so do something with it

		//if (m_pPoseableMeshComponent->IsRegistered()) {
		//	// has to be registered to be able to get quaternions/rotations out of it...
		//	UE_LOG(LogTemp, Warning, TEXT("PoseableMeshComponent is registered"));
		//}
		//else {
		//	UE_LOG(LogTemp, Warning, TEXT("PoseableMeshComponent is NOT registered"));
		//}

		


		/*for (int i = 0; i < m_pRawRefBonePose.Num(); ++i) {
			FName boneName = m_pPoseableMeshComponent->GetBoneName(i);
			m_pPoseableMeshComponent->SetBoneTransformByName(boneName, m_pRawRefBonePose[i], EBoneSpaces::WorldSpace);
		}*/
	}

	//FName PropertyName = e.GetPropertyName();
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(this, m_Rot)) {
	//	/* Because you are inside the class, you should see the value already changed */
	//	m_Rot = ;
	//}
} 
//for TArrays:
void AMyPoseableMesh::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e)
{
	//UE_LOG(LogSkeletalMesh, Warning, TEXT("AMyPoseableMesh::PostEditChangeChainProperty()"));
	Super::PostEditChangeChainProperty(e);

	// only change what has been changed and dont adapt everything...
	FName PropertyName = e.GetPropertyName();	// is m_pRawRefBonePose...
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyPoseableMesh, m_pRawRefBonePose)) {
		int32 index = e.GetArrayIndex(PropertyName.ToString());
		if (index >= 0 && index <= (m_pRawRefBonePose.Num() - 1)) {
			FName boneName = m_pPoseableMeshComponent->GetBoneName(index);
			m_pPoseableMeshComponent->SetBoneTransformByName(boneName, m_pRawRefBonePose[index], EBoneSpaces::WorldSpace);
		}
	}
}
#endif



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
	int def_DOF = 1;

	IKSegment *hand_r_Segment = new IKSegment(def_DOF);
	hand_r_Segment->SetDoFId(0);
	hand_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	hand_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("hand_r"));

	IKSegment *index_01_r_Segment = new IKSegment(def_DOF);
	index_01_r_Segment->SetDoFId(1);
	index_01_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	index_01_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_01_r"));

	IKSegment *index_02_r_Segment = new IKSegment(def_DOF);
	index_02_r_Segment->SetDoFId(2);
	index_02_r_Segment->SetRotationAxis(FVector(0, 0, 1));
	index_02_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_02_r"));

	IKSegment *index_03_r_Segment = new IKSegment(def_DOF);
	index_03_r_Segment->SetDoFId(3);
	index_03_r_Segment->SetRotationAxis(FVector(0,0,1));
	index_03_r_Segment->SetPosition(m_pPoseableMeshComponent->GetBoneLocation("index_03_r"));

	// setup the kinematic chain
	index_03_r_Segment->SetParent(index_02_r_Segment);
	index_02_r_Segment->SetParent(index_01_r_Segment);
	index_01_r_Segment->SetParent(hand_r_Segment);
	m_ik_solver = new IKSolver(hand_r_Segment);	// hand as root of kinematic chain

}   

void AMyPoseableMesh::OnBodyData_Implementation(const int32 BodyID, const FVector & Position, const FRotator & Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBodyData_Implementation"));

	if (BodyID == 2) { 
		//m_pPoseableMeshComponent->SetBoneRotationByName("hand_r", Rotation, EBoneSpaces::WorldSpace);
		//m_pPoseableMeshComponent->SetBoneLocationByName("hand_r", Position + m_cachedHandRLocation, EBoneSpaces::WorldSpace);
	}

	if (BodyID == 1) {

		IKSegment *pSeg = nullptr;
		 
		if(m_ik_solver == nullptr) { UE_LOG(LogTemp, Warning, TEXT("m_ik_solver is nullptr")); }
		else {
			FVector handBonePos = m_pPoseableMeshComponent->GetBoneLocation("hand_r", EBoneSpaces::WorldSpace);
			FVector newPos = Position + handBonePos;
			UE_LOG(LogTemp, Warning, TEXT("Position : %s"), *(Position).ToString());
			UE_LOG(LogTemp, Warning, TEXT("TargetPos : newPos : %s"), *(newPos).ToString());
			m_ik_solver->Solve(newPos);	// newPos as TargetPos of Kinematic Chain
			pSeg = m_ik_solver->GetRootSegment();
			pSeg = pSeg->Child();	// advance pSeg pointer
		} 

		if (pSeg==nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg is nullptr")); }
		else {
			UE_LOG(LogTemp, Warning, TEXT("pSeg : %f"), pSeg->GetTheta());
		} 


		if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child() is nullptr")); }
		else {
			double thetaRad1 = pSeg->GetTheta(); 
			double thetaDeg1 = thetaRad1 * RAD_TO_DEG;
			UE_LOG(LogTemp, Warning, TEXT("r1 : thetaDeg1 : %f"), thetaDeg1);
			FQuat q1 = FQuat(FRotator(0, 0, thetaDeg1)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("hand_r"), EBoneSpaces::WorldSpace);
			m_pPoseableMeshComponent->SetBoneRotationByName("index_01_r", q1.Rotator(), EBoneSpaces::WorldSpace);
			pSeg = pSeg->Child();	// advance pSeg pointer
		} 

		if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child()->Child() is nullptr")); }
		else {
			double thetaRad2 = pSeg->GetTheta();
			double thetaDeg2 = thetaRad2 * RAD_TO_DEG;
			UE_LOG(LogTemp, Warning, TEXT("r2 : thetaDeg2 : %f"), thetaDeg2);
			FQuat q2 =  FQuat(FRotator(0, 0, thetaDeg2)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("index_01_r"), EBoneSpaces::WorldSpace);
			m_pPoseableMeshComponent->SetBoneRotationByName("index_02_r", q2.Rotator(), EBoneSpaces::WorldSpace);
			pSeg = pSeg->Child();	// advance pSeg pointer
		}

		if (pSeg == nullptr) { UE_LOG(LogTemp, Warning, TEXT("pSeg->Child()->Child()->Child() is nullptr")); }
		else {
			double thetaRad3 = pSeg->GetTheta();
			double thetaDeg3 = thetaRad3 * RAD_TO_DEG;
			UE_LOG(LogTemp, Warning, TEXT("r3 : thetaDeg3 : %f"), thetaDeg3);
			FQuat q3 = FQuat(FRotator(0, 0, thetaDeg3)) * m_pPoseableMeshComponent->GetBoneQuaternion(TEXT("index_02_r"), EBoneSpaces::WorldSpace);
			m_pPoseableMeshComponent->SetBoneRotationByName("index_03_r", q3.Rotator(), EBoneSpaces::WorldSpace);
			pSeg = pSeg->Child();	// advance pSeg pointer 
		}
	}

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

	/*
	TODO use the specified TARRAY which is the Mapping of DTrack IDs to Unreal Bone IDs (m_BoneToDTrackIDMap)
	// HandID is the ID of the tracked body. There can be many, depending on your setup.
	if (HandID == m_hand_r) { m_pPoseableMeshComponent->SetBoneRotationByName("hand_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_01_r) { m_pPoseableMeshComponent->SetBoneRotationByName("thumb_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_02_r) { m_pPoseableMeshComponent->SetBoneRotationByName("thumb_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_thumb_03_r) { m_pPoseableMeshComponent->SetBoneRotationByName("thumb_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_01_r) { m_pPoseableMeshComponent->SetBoneRotationByName("index_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_02_r) { m_pPoseableMeshComponent->SetBoneRotationByName("index_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_index_03_r) { m_pPoseableMeshComponent->SetBoneRotationByName("index_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_01_r) { m_pPoseableMeshComponent->SetBoneRotationByName("middle_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_02_r) { m_pPoseableMeshComponent->SetBoneRotationByName("middle_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_middle_03_r) { m_pPoseableMeshComponent->SetBoneRotationByName("middle_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_01_r) { m_pPoseableMeshComponent->SetBoneRotationByName("ring_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_02_r) { m_pPoseableMeshComponent->SetBoneRotationByName("ring_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_ring_03_r) { m_pPoseableMeshComponent->SetBoneRotationByName("ring_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_01_r) { m_pPoseableMeshComponent->SetBoneRotationByName("pinky_01_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_02_r) { m_pPoseableMeshComponent->SetBoneRotationByName("pinky_02_r", Rotation, EBoneSpaces::WorldSpace); }
	else if (HandID == m_pinky_03_r) { m_pPoseableMeshComponent->SetBoneRotationByName("pinky_03_r", Rotation, EBoneSpaces::WorldSpace); }
	else { UE_LOG(LogTemp, Warning, TEXT("HandID of %d is not assigned!"), HandID); } 
	*/
}

void AMyPoseableMesh::OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint>& Joints)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHumanModel_Implementation"));
}
 