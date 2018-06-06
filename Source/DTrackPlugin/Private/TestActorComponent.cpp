// Fill out your copyright notice in the Description page of Project Settings.

#include "TestActorComponent.h"

#include "FDTrackPlugin.h"

 
// Sets default values for this component's properties
UTestActorComponent::UTestActorComponent(const FObjectInitializer &n_initializer)
	: Super(n_initializer) 
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true; // see above
	bWantsInitializeComponent = true;	// If true, we call the virtual InitializeComponent
	bAutoActivate = true;	// component is activated at creation 
	// All Components in the level will be Initialized on load before 
	// any Actor/Component gets BeginPlay Requires component to be registered

	// ...
	UE_LOG(LogTemp, Warning, TEXT("UTestActorComponent 0000 8 "));
} 

 
// Called when the game starts
void UTestActorComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay of UDTrackComponent"));
	UE_LOG(LogTemp, Warning, TEXT("Members of UDTrackComponent"));
	UE_LOG(LogTemp, Warning, TEXT("m_dtrack_server_ip : %s"), *m_dtrack_server_ip);
	UE_LOG(LogTemp, Warning, TEXT("m_dtrack_client_port : %d"), m_dtrack_client_port);
	UE_LOG(LogTemp, Warning, TEXT("m_dtrack_2 : %s"), (m_dtrack_2 ? TEXT("True") : TEXT("False")));


	// ... 
	// Attach the delegate pointer automatically to the owner of the component

	if (FDTrackPlugin::IsAvailable()) {
		IDTrackPlugin &plugin = FDTrackPlugin::Get();

		m_plugin = &plugin;

		// tell the plugin we exist and want to be called as data come in
		// m_plugin->start_up(this);

	}
	else {
		UE_LOG(DTrackPluginLog, Warning, TEXT("DTrack Plugin not available, cannot track this object"));
	}
	
}

void UTestActorComponent::EndPlay(const EEndPlayReason::Type n_reason)
{
	Super::EndPlay(n_reason);

	UE_LOG(DTrackPluginLog, Display, TEXT("DTrack EndPlay() called"));

	if (m_plugin) {
		// m_plugin->remove(this);
		m_plugin = nullptr; 
	}
	else {
		UE_LOG(DTrackPluginLog, Warning, TEXT("DTrack Plugin not available, cannot stop tracking of this object"));
	}
}


// Called every frame
void UTestActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

