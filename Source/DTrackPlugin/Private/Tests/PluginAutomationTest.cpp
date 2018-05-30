#pragma once

#include "PluginAutomationTest.h"
#include "AutomationTest.h"
#include "CoreMinimal.h"
#include "DTrackPollThread.h"
#include "DTrackComponent.h"
 
// the class that you want to test:
#include "Helper/CoordinateConverter.h"

// Definition von Log-Ausgaben
DEFINE_LOG_CATEGORY_STATIC(LogEditorAutomationTests, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogEngineAutomationTests, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogAnalytics, Log, All);

/////////////////////////////////////////////////////////////////////

inline bool FDTrackPollThreadTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Inside FPlaceholderTest::RunTest"));

	/*
	USphereComponent* Component = NewObject<USphereComponent>(this);
	FObjectInitializer* ObjectInitializer = new FObjectInitializer();
	*/
	const double n_mat[9] = {
		90.0, 90.0, 90.0,
		90.0, 0.0, 0.0,
		0.0, 0, 90.0
	}; 
	FCoordinateConverter coordConverter;
	FRotator resultRotator = coordConverter.from_dtrack_rotation(n_mat);

	UE_LOG(LogEngineAutomationTests, Log, TEXT("Input : \n %f %f %f \n %f %f %f \n %f %f %f"), n_mat[0], n_mat[1], n_mat[2], n_mat[3], n_mat[4], n_mat[5], n_mat[6], n_mat[7], n_mat[8]);
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Result : %s"), *resultRotator.ToString());


	// Make the test pass by returning true, or fail by returning false.
	return true;
}
