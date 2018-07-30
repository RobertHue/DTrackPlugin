// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "DTrackComponent.h"
#include "DTrackInterface.h"

#include "MySceneComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DTRACKPLUGIN_API UMySceneComponent
	:
		public USceneComponent,
		public IDTrackInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMySceneComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	 
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


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

	//////////////////////////////////////////
public:
	FVector StartPos;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "controlled by DTrack-ID", ToolTip = "Enter the target set controls this component (controlling mean setting its location and rotation"))
	int32  m_id = 1;	// can also be BodyID, FlystickID, HandID or ModelID

	
};
