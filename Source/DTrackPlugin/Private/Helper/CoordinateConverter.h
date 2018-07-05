#pragma once

#include "CoreMinimal.h"

// #define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI	3.14159265358979323846   // pi
#endif
constexpr double DEG_TO_RAD = (double)(M_PI) / 180.0f;
constexpr double RAD_TO_DEG = (double)(180.0f) / M_PI;

/**
* This represents different room calibration default settings as
* set in the DTrack system. Choose the one that corresponds with your setup
* and transformations will be translated into Unreal's standard coordinate
* system
*
* @note: CST stands for coordinate-system
*/
UENUM(BlueprintType, Category = DTrack)
enum class EDTrackCoordinateSystemType : uint8
{
	/// The normal setting:		Right handed (RHS) | Z is up | Y is front | X is side (default)
	CST_Normal			UMETA(DisplayName = "DTrack Normal (Z is up | Y is front | X is side)"),

	/// The unreal adapted setting:		Right handed (RHS) | Z is up | X as front | -Y is side
	CST_Unreal_Adapted	UMETA(DisplayName = "DTrack Unreal Adapted (Z is up | X as front | -Y is side)")

	// TODO: commented CST_Powerwall, because it needs some testing...
	///// Powerwall default setting with Y as up 
	//CST_Powerwall		UMETA(DisplayName = "DTrack Powerwall"),
};

/*
 * @class: 
 * this class uses the concept from https://www.geometrictools.com/Documentation/ConvertingBetweenCoordinateSystems.pdf
 * to convert from right-handed coordinate system (DTrack) to left-handed coordinate system (Unreal-Engine 4)
 * it does this by flipping one axis direction, namely the z-axis (-Z): this operation is represented by m_Sz
 * 
 * 
 */
class FCoordinateConverter 
{

public:
	FCoordinateConverter(EDTrackCoordinateSystemType coordinate_system = EDTrackCoordinateSystemType::CST_Normal);

public:

	/// translate dtrack translation to unreal space
	FVector from_dtrack_location(const double(&n_translation)[3]);

	/// translate dtrack rotation matrix to rotator according to selected room calibration
	FRotator from_dtrack_rotation(const double(&n_matrix)[9]);

	const EDTrackCoordinateSystemType getCoordinateSystemType() const;

private:
	/// room coordinate adoption matrix for "normal" setting
	const FMatrix  m_trafo_normal;

	/// transposed variant cached
	const FMatrix  m_trafo_normal_transposed;

	/// room coordinate adoption matrix for "power wall" setting
	const FMatrix  m_trafo_powerwall;

	/// transposed variant cached
	const FMatrix  m_trafo_powerwall_transposed;

	/// room coordinate adoption matrix for "unreal adapted" setting
	const FMatrix  m_trafo_unreal_adapted;

	/// transposed variant cached
	const FMatrix  m_trafo_unreal_adapted_transposed;



	const FMatrix  m_Sz;	// the Diagonal-Matrix Diag(1,1,-1) to flip the z-axis direction

	const EDTrackCoordinateSystemType  m_coordinate_system;

};