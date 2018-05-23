// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"

#include "DTrackComponent.h"
#include "DTrackInterface.h"

#include "MyPoseableMeshComponent.generated.h"

/**
 *
 */
UCLASS( 
	ClassGroup = Rendering, // Unreal Editor's Actor Browser  
	Config = Engine, // this class is allowed to store data in a configuration file (.ini)
	//editinlinenew,  // Indicates that Objects of this class can be created from the Unreal Editor Property window, as opposed to being referenced from an existing Asset.
	Meta = (BlueprintSpawnableComponent) // If present, the component class can be spawned by a Blueprint. 
) 
class DTRACKPLUGIN_API UMyPoseableMeshComponent
	:
		public UPoseableMeshComponent,
		public IDTrackInterface
{ 
	GENERATED_BODY()

public:
	UMyPoseableMeshComponent();


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/*************************/
	/** overrides of UObject */
	/*************************/
public:
	/** Called after the C++ constructor and after the properties have been initialized, including those loaded from config. */
	virtual void PostInitProperties() override;
	/** Do any object-specific cleanup required immediately after loading an object, and immediately after any undo/redo. */
	virtual void PostLoad() override;


	/** Called when a property on this object has been modified externally */
	//virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/********************************/
	/** overrides of ActorComponent */
	/********************************/
public:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATEST")
		class USkeletalMesh* m_pAltMeshAsset;

	UPROPERTY(VisibleAnywhere, Category = ATEST)
		class USkeletalMesh* m_pSkeletalMesh;

	UPROPERTY(VisibleAnywhere, Category = ATEST)
		class USkeleton* m_pSkeleton;


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/*********************************/
	/** overrides of DTrackInterface */
	/*********************************/
public:
	UPROPERTY(VisibleAnywhere, Category = DTrack)  // Choose your own property specifiers according to your needs.
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

	 
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	FRotator defaultUpperArmLRot;
	FRotator defaultLowerArmLRot;
};
