// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"

#include "DTrackComponent.h"
#include "DTrackInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

#include "Animation/Skeleton.h"
#include "ReferenceSkeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"


#include "MyPoseableMesh.generated.h"

/// @struct		FBoneNameToBoneRotation:
///				used to store a mapping between boneID/boneName and the corresponding Rotation as FQuat/FRotator
/// @note		is not really a mapping because no map is used. It's possible to have the same boneName for another Rotation
USTRUCT(BlueprintType)
struct FBoneNameToBoneRotation
{
	GENERATED_BODY()

public:
	FBoneNameToBoneRotation() : boneID(0), boneName(FName("")), boneRotation(FQuat()), boneRotator(FRotator()) {}
	FBoneNameToBoneRotation(int32 id, FName bN, FQuat bQ, FRotator bR) : boneID(id), boneName(bN), boneRotation(bQ), boneRotator(bR) {}
	  
public:
	/////////////////////
	// UUID:
	UPROPERTY(BlueprintReadOnly, Category = "A DTrack")
	int32 boneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A DTrack")
	FName boneName;

	/////////////////////
	// Rotation
	UPROPERTY(BlueprintReadWrite, Category = "A DTrack")
	FQuat boneRotation;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A DTrack")
	FRotator boneRotator;
};


/**
 * @class	AMyPoseableMesh
 *			allows bone transforms to be driven by C++ (here: DTrack)
 * @note:  moved MyPoseableMeshComponent into a class deriving from AActor/APawn, because a standard component does not take over the values (always resets to default)
 *
 */
UCLASS( 
	ClassGroup = Rendering, // Unreal Editor's Actor Browser  
	Config = Engine			// this class is allowed to store data in a configuration file (.ini)
	//editinlinenew,  // Indicates that Objects of this class can be created from the Unreal Editor Property window, as opposed to being referenced from an existing Asset.
) 
class DTRACKPLUGIN_API AMyPoseableMesh
	:
		public APawn,	
		public IDTrackInterface			// because we want the data from DTrack controlling the mesh
{ 
	GENERATED_BODY()

public: 
	AMyPoseableMesh();
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/** The m_pCapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically aligned in simple collision check functions. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "DTrack Skeleton")
	UCapsuleComponent* m_pCapsuleComponent;

	/** First Person Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* m_pCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DTrack Skeleton")
	UPoseableMeshComponent* m_pPoseableMeshComponent;	// because we want a poseable mesh to be controlled by DTrack via C++
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DTrack Skeleton")
	UPoseableMeshComponent* m_pPoseableMeshComponentLeftHand;	// because we want a poseable mesh to be controlled by DTrack via C++

	 
public:
	/*************************/
	/** overrides of UObject */
	/*************************/
	/** Called after the C++ constructor and after the properties have been initialized, including those loaded from config. */
	// virtual void PostInitProperties() override;
	/** Do any object-specific cleanup required immediately after loading an object, and immediately after any undo/redo. */
	// virtual void PostLoad() override;
	/** Called when a property on this object has been modified externally */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostEditChangeChainProperty(FPropertyChangedChainEvent & e);	// for TArrays

public:
	/************************/
	/** overrides of AActor */
	/************************/
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void OnConstruction(const FTransform& Transform) override;	// AActor::OnConstruction - The construction of the actor, this is where Blueprint actors have their components created and blueprint variables are initialized
	virtual void PreInitializeComponents() override;	// AActor::PreInitializeComponents - Called before InitializeComponent is called on the actor's components
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	/// Called when this Actor hits (or is hit by) something solid. This could happen due to things like Character movement, using Set Location with 'sweep' enabled, or physics simulation.
	//virtual FActorHitSignature OnActorHit() override;

	// ::: following are only for components :::
	//virtual void InitializeComponent() override;	// Initializes the component.  Occurs at level startup. This is before BeginPlay (Actor or Component).  
	//virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


public: 
	/*********************************/
	/** overrides of DTrackInterface */
	/*********************************/
	UPROPERTY(VisibleAnywhere, Category = "DTrack")  // Choose your own property specifiers according to your needs.
	class UDTrackComponent* m_dtrack_component;


	/// This implements the IDTrackInterface event for body tracking data coming in
	virtual void OnBodyData_Implementation(const int32 BodyID, const FVector &Position, const FRotator &Rotation) override;

	/// This implements the IDTrackInterface event for flystick tracking data coming in
	virtual void OnFlystickData_Implementation(const int32 FlystickID, const FVector &Position, const FRotator &Rotation) override;

	/// This implements the IDTrackInterface event for flystick tracking data coming in
	virtual void OnFlystickButton_Implementation(const int32 FlystickID, const int32 &ButtonIndex, const bool Pressed) override;
	virtual void OnFlystickJoystick_Implementation(const int32 FlystickID, const TArray<float> &JoystickValues) override;

	/// This implements the IDTrackInterface event for @TODO insert comment
	virtual void OnHandTracking_Implementation(const int32 HandID, const bool Right, const FVector &Translation, const FRotator &Rotation, const TArray<FDTrackFinger> &Fingers) override;
	virtual void OnHumanModel_Implementation(const int32 ModelID, const TArray<FDTrackJoint> &Joints) override;

	 

public:
	//IKSolver * m_ik_solver;	// experimental ik-solver (use at your own risk)
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TMap<FName, int32> m_BoneToDTrackIDMap;*/
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FSkeletalHand m_HandR;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TArray<FBoneNameToBoneRotation> m_SkeletonRotations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DTrack Skeleton")
	TArray<FBoneNameToBoneRotation> m_RawSkeletonRotations;	// default values in UE4-TPose

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TMap<int32, FName> m_DTrackIDToBoneMap;

	/// cached start pos of the player (this is a copy assigned during BeginPlay - changing it will not do anything)
	FVector m_PlayerStartPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FVector m_DefaultHandLocation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	//FRotator m_rotatorPatchForHand = FRotator(0.0, -90.0, -90.0);  // for that skeleton hand <-90,0,-90> is suitable... (XYZ)


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FRotator m_ValueForBPRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FVector m_ValueForBPLocation;


	// for the indicators (crosshairs showing the location and rotations of bones)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TArray<FVector> m_HandREndEffectorLocations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TArray<FRotator> m_HandREndEffectorRotators;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FVector m_HandLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	FRotator m_HandRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack Skeleton")
	TArray<FDTrackFinger> m_FingerData;
};     