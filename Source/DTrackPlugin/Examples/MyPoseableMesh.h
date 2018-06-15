// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"

#include "DTrackComponent.h"
#include "DTrackInterface.h"
#include "IKSolver.h"
#include "GameFramework/Actor.h"

#include "Animation/Skeleton.h"
#include "ReferenceSkeleton.h"
#include "Components/SkeletalMeshComponent.h"


#include "MyPoseableMesh.generated.h"

/**
 *
 * allows bone transforms to be driven by C++ (here: DTrack)
 * @note:  moved MyPoseableMeshComponent into a class deriving from AActor, because a standard component does not take over the values (always resets to default)
 *
 */
UCLASS( 
	ClassGroup = Rendering, // Unreal Editor's Actor Browser  
	Config = Engine			// this class is allowed to store data in a configuration file (.ini)
	//editinlinenew,  // Indicates that Objects of this class can be created from the Unreal Editor Property window, as opposed to being referenced from an existing Asset.
) 
class DTRACKPLUGIN_API AMyPoseableMesh
	:
		public AActor,	
		public IDTrackInterface			// because we want the data from DTrack controlling the mesh
{ 
	GENERATED_BODY()

public:
	AMyPoseableMesh();
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	UPROPERTY(VisibleAnywhere, Category = "DTrack")
	UPoseableMeshComponent* m_pPoseableMeshComponent;	// because we want a poseable mesh to be controlled by DTrack via C++

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
	IKSolver * m_ik_solver;	// experimental ik-solver (use at your own risk)
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack")
	TMap<FName, int32> m_BoneToDTrackIDMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack")
	FVector m_PlayerStartPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack")
	FQuat m_Rot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack")
	FTransform m_TransformBone30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTrack")
	TArray<FTransform> m_pRawRefBonePose;
};
