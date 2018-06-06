// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DTrackInterface.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"

#include "TestActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DTRACKPLUGIN_API UTestActorComponent 
	: 
		public USceneComponent
{
	GENERATED_UCLASS_BODY()


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type n_reason) override;

public:	 
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void test() { /**/ };
public:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "DTrack Server IP", ToolTip = "Enter the IP of your DTrack server host. Hostnames will not work."))
		FString m_dtrack_server_ip = "10.10.8.75dddd";

	UPROPERTY(EditAnywhere, meta = (DisplayName = "DTrack Server Port", ToolTip = "Enter the port your server uses"))
		uint32  m_dtrack_client_port = 5012;	// this is actually not the server port (its the client port on windows), whereas the SDK has the server port hardwired into its code...; so @TODO needs to be refactored (name change or something different...)

	UPROPERTY(EditAnywhere, meta = (DisplayName = "DTrack2 Protocol", ToolTip = "Use the TCP command channel based DTrack2 protocol; otherwhise use UDP"))
		bool    m_dtrack_2 = true;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "DTrack Room Calibration", ToolTip = "Set this according to your DTrack system's room calibration"))
		EDTrackCoordinateSystemType m_coordinate_system = EDTrackCoordinateSystemType::CST_Normal;

private:

	class IDTrackPlugin *m_plugin = nullptr;   //!< will cache that to avoid calling Module getter in every tick

	
};
