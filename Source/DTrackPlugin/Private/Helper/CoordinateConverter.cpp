#include "CoordinateConverter.h"

// can also be used:
// constexpr double mm_2_cm () { return static_cast<double>(1 / 10.0); }
#define MM_2_CM 0.1

FCoordinateConverter::FCoordinateConverter(EDTrackCoordinateSystemType n_coord_system)
	:
	m_trafo_normal (
		FPlane( 0,  1,  0,  0 ),
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 )
	),
	m_trafo_normal_transposed (
		FPlane( 0,  1,  0,  0 ),
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 )
	),
	m_trafo_powerwall(
		FPlane( 0,  0, -1,  0 ),
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  1,  0,  0 ),
		FPlane( 0,  0,  0,  1 )
	),
	m_trafo_powerwall_transposed (
		FPlane(  0,  1,  0,  0 ),
		FPlane(  0,  0,  1,  0 ),
		FPlane( -1,  0,  0,  0 ),
		FPlane(  0,  0,  0,  1 )
	),
	m_trafo_unreal_adapted (
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0, -1,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 ) 
	),
	m_trafo_unreal_adapted_transposed (
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0, -1,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 ) 
	),
	m_coordinate_system(n_coord_system)
{
	// transposed is cached
	// also possible:
	// FMatrix &trafo_unreal_adapted = const_cast<FMatrix &>(m_trafo_unreal_adapted);
	// trafo_unreal_adapted.M[0][0] = 1; trafo_unreal_adapted.M[0][1] = 0; trafo_unreal_adapted.M[0][2] = 0; trafo_unreal_adapted.M[0][3] = 0;
	// trafo_unreal_adapted.M[1][0] = 0; trafo_unreal_adapted.M[1][1] = -1; trafo_unreal_adapted.M[1][2] = 0; trafo_unreal_adapted.M[1][3] = 0;
	// trafo_unreal_adapted.M[2][0] = 0; trafo_unreal_adapted.M[2][1] = 0; trafo_unreal_adapted.M[2][2] = 1; trafo_unreal_adapted.M[2][3] = 0;
	// trafo_unreal_adapted.M[3][0] = 0; trafo_unreal_adapted.M[3][1] = 0; trafo_unreal_adapted.M[3][2] = 0; trafo_unreal_adapted.M[3][3] = 1;
	// const_cast<FMatrix &>(m_trafo_unreal_adapted_transposed) = trafo_unreal_adapted.GetTransposed();
}

// translate a DTrack body location (translation in mm) into Unreal Location (in cm)
FVector FCoordinateConverter::from_dtrack_location(const double(&n_translation)[3])
{ 
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FCoordinateConversionHelper::from_dtrack_location()"));

	FVector ret;

	// DTrack coordinates come in mm with either Z or Y being up, which has to be configured by the user.
	// I translate to Unreal's Z being up and cm units.
	switch (m_coordinate_system) {
	default:
	case EDTrackCoordinateSystemType::CST_Normal:
		ret.X = n_translation[1] * MM_2_CM;
		ret.Y = n_translation[0] * MM_2_CM;
		ret.Z = n_translation[2] * MM_2_CM;
		break;
	case EDTrackCoordinateSystemType::CST_Unreal_Adapted:
		ret.X = n_translation[0] * MM_2_CM;
		ret.Y = -n_translation[1] * MM_2_CM;
		ret.Z = n_translation[2] * MM_2_CM;
		break;
	case EDTrackCoordinateSystemType::CST_Powerwall:
		ret.X = -n_translation[2] * MM_2_CM;
		ret.Y = n_translation[0] * MM_2_CM;
		ret.Z = n_translation[1] * MM_2_CM;
		break;
	}

	return ret;
}

// translate DTrack 3x3 rotation matrix to FRotator according to selected room calibration
// @ToDo needs a bit more and better description
FRotator FCoordinateConverter::from_dtrack_rotation(const double(&n_matrix)[9])
{
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FCoordinateConversionHelper::from_dtrack_rotation()"));
	UE_LOG(DTrackPollThreadLog, Display, TEXT("NEWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));

	// take DTrack matrix and put the values into FMatrix 
	// ( M[RowIndex][ColumnIndex], DTrack matrix comes column-wise )
	FMatrix r;
	r.M[0][0] = n_matrix[0 + 0]; r.M[0][1] = n_matrix[0 + 3]; r.M[0][2] = n_matrix[0 + 6]; r.M[0][3] = 0.0;
	r.M[1][0] = n_matrix[1 + 0]; r.M[1][1] = n_matrix[1 + 3]; r.M[1][2] = n_matrix[1 + 6]; r.M[1][3] = 0.0;
	r.M[2][0] = n_matrix[2 + 0]; r.M[2][1] = n_matrix[2 + 3]; r.M[2][2] = n_matrix[2 + 6]; r.M[2][3] = 0.0;
	r.M[3][0] = 0.0;			 r.M[3][1] = 0.0;			  r.M[3][2] = 0.0;			   r.M[3][3] = 1.0;

	FMatrix r_adapted;
	 
	switch (m_coordinate_system) {
	default:
	case EDTrackCoordinateSystemType::CST_Normal:
		r_adapted = m_trafo_normal * r * m_trafo_normal_transposed; // exchange row1 with row2
		break;

	case EDTrackCoordinateSystemType::CST_Unreal_Adapted:
		r_adapted = m_trafo_unreal_adapted * r * m_trafo_unreal_adapted_transposed;
		break;

	case EDTrackCoordinateSystemType::CST_Powerwall:
		r_adapted = m_trafo_powerwall * r * m_trafo_powerwall_transposed;
		break;
	}

	return r_adapted.GetTransposed().Rotator();	// FRotator is the Rotation representation in Euler angles.
}
