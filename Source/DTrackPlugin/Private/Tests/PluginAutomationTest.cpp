#pragma once
#include "PluginAutomationTest.h"	// must be the first header included!

// #define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#ifndef M_PI
	#define M_PI	3.14159265358979323846   // pi
#endif

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


constexpr double DEG_TO_RAD = (double)(M_PI) / 180.0f;
constexpr double RAD_TO_DEG = (double)(180.0f) / M_PI;

/////////////////////////////////////////////////////////////////////

/*
 * @brief: creates a DTrack2 Rotation matrix (3x3) with the given Euler Angles in degree
 * R = Rx(eta) * Ry(theta) * Rz(phi)
 * R = Rx(rx) * Ry(ry) * Rz(rz) (where rx, ry, rz are the Euler Angles in degree)
 *
 * @notes: 
 * Note that per definitionem the angles can only have the values:
 *		-180° <= rz <= 180°, -90° <= ry <= 90°, -180° <= rx <= 180°
 *	rotation angles can show strange behaviour at certain orientations.
 *	In particular, for orientations close to theta (ry) = +- 90 degrees
 *	the other two angles can experience large odd-looking changes 
 *	(so called "Gimbal Lock").
 * @note: the columns of the matrix are the axes (X,Y,Z) of the body coordinate system (column-wise)
 * @see: DTrack2_UserManual.pdf page 253 B.1.3 "6DOF Results" "Description by Rotation Angles"
 */
void CreateRotationMatrix(double(&n_matrix)[9], double rx, double ry, double rz) {
	// convert angles from degrees to rad:
	rx *= DEG_TO_RAD; ry *= DEG_TO_RAD; rz *= DEG_TO_RAD;

	n_matrix[0] = cos(rz) * cos(ry);
	n_matrix[1] = -sin(rz) * cos(ry);
	n_matrix[2] = sin(ry); 
	n_matrix[3] = sin(rz) * cos(rx) + cos(rz) * sin(ry) * sin(rx);
	n_matrix[4] = cos(rz) * cos(rx) - sin(rz) * sin(ry) * sin(rx);
	n_matrix[5] = -cos(ry) * sin(rx);
	n_matrix[6] = sin(rz) * cos(rx) - cos(rz) * sin(ry) * cos(rx);
	n_matrix[7] = cos(rz) * sin(rx) + sin(rz) * sin(ry) * cos(rx);
	n_matrix[8] = cos(ry) * cos(rx);
}

/// @brief	pretty print a matrix n_mat with a prefixed message msg
/// @example: 
///		double matrix[9]{ 0 };
///		LogMatrix(std::string("Inhalt"), matrix);
void LogMatrix(const std::string& msg, double(&n_mat)[9]) {
	UE_LOG(LogEngineAutomationTests, Log, TEXT("%s : \n %f %f %f \n %f %f %f \n %f %f %f"), *FString(msg.c_str()),
		n_mat[0], n_mat[1], n_mat[2], 
		n_mat[3], n_mat[4], n_mat[5], 
		n_mat[6], n_mat[7], n_mat[8]
	); 
}



inline bool FDTrackPollThreadTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Inside FPlaceholderTest::RunTest"));

	FCoordinateConverter coordConverter;
	TestEqual(
		"Is the EDTrackCoordSysType the CST_Normal one?",	// What the test is about
		coordConverter.getCoordinateSystemType(),			// Actual
		EDTrackCoordinateSystemType::CST_Normal				// Expected
	);

	double matrix[9]{ 0 };
	LogMatrix("After Construction of Matrix", matrix);
	double rx = 0.0, ry = 0.0, rz = 0.0;

	rx = 30.0, ry = 20.0, rz = 0.0;
	CreateRotationMatrix(matrix, rx, ry, rz);
	LogMatrix(std::string("Inhalt"), matrix);
	FRotator resultRotator1 = coordConverter.from_dtrack_rotation(matrix);
	TestEqual("Unreal Pitch ry == DTrack rx", resultRotator1.Pitch, rx );	// Pitch == Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down)
	TestEqual("Unreal Roll rx == DTrack ry", resultRotator1.Roll, ry);	// Roll == Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW.
	TestEqual("Unreal Yaw rz == DTrack rz", resultRotator1.Yaw, rz);	// Yaw == Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South.
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Result : %s"), *resultRotator1.ToString());
	FRotationMatrix testRotMatrix(FRotator(rx, rz, ry));
	UE_LOG(LogEngineAutomationTests, Log, TEXT("testRotMatrix : %s"), *testRotMatrix.ToString());
	UE_LOG(LogEngineAutomationTests, Log, TEXT("testRotMatrix -> Rotator() : %s"), *testRotMatrix.Rotator().ToString());


	rx = 0.0, ry = 45.0, rz = 0.0; 
	CreateRotationMatrix(matrix, rx, ry, rz);
	LogMatrix("Inhalt", matrix);
	FRotator resultRotator2 = coordConverter.from_dtrack_rotation(matrix);
	TestEqual("Unreal Pitch ry == DTrack rx", resultRotator2.Pitch, rx);	// Pitch == Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down)
	TestEqual("Unreal Roll rx == DTrack ry", resultRotator2.Roll, ry);	// Roll == Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW.
	TestEqual("Unreal Yaw rz == DTrack rz", resultRotator2.Yaw, rz);	// Yaw == Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South.
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Result : %s"), *resultRotator2.ToString()); 
	 
	 
	rx = 5.0, ry = 15.0, rz = 45.0;
	CreateRotationMatrix(matrix, rx, ry, rz);
	LogMatrix("Inhalt", matrix);
	FRotator resultRotator3 = coordConverter.from_dtrack_rotation(matrix);
	TestEqual("Unreal Pitch ry == DTrack rx", resultRotator3.Pitch, rx);	// Pitch == Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down)
	TestEqual("Unreal Roll rx == DTrack ry", resultRotator3.Roll, ry);	// Roll == Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW.
	TestEqual("Unreal Yaw rz == DTrack rz", resultRotator3.Yaw, rz);	// Yaw == Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South.
	UE_LOG(LogEngineAutomationTests, Log, TEXT("Result : %s"), *resultRotator3.ToString());


	// Make the test pass by returning true, or fail by returning false.
	return true;
} 
