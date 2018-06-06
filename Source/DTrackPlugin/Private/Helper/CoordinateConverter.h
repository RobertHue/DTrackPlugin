#pragma once

#include "CoreMinimal.h"
#include "DTrackInterface.h"

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