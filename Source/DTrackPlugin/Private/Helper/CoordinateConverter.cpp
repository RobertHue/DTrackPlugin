#include "CoordinateConverter.h"

FCoordinateConverter::FCoordinateConverter(EDTrackCoordinateSystemType n_coord_system)
	:
	m_trafo_normal (	// switches X and Y axis
		FPlane( 0,  1,  0,  0 ),
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 )
	),
	m_trafo_normal_transposed (		// the transposed of above
		FPlane( 0,  1,  0,  0 ),
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  0,  1,  0 ),
		FPlane( 0,  0,  0,  1 )
	),
	//m_trafo_powerwall(
	//	FPlane( 0,  0, -1,  0 ),
	//	FPlane( 1,  0,  0,  0 ),
	//	FPlane( 0,  1,  0,  0 ),
	//	FPlane( 0,  0,  0,  1 )
	//),
	//m_trafo_powerwall_transposed (
	//	FPlane(  0,  1,  0,  0 ),
	//	FPlane(  0,  0,  1,  0 ),
	//	FPlane( -1,  0,  0,  0 ),
	//	FPlane(  0,  0,  0,  1 )
	//),
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
	m_Sz(	// as in the paper of geometric tools
		FPlane( 1,  0,  0,  0 ),
		FPlane( 0,  1,  0,  0 ),
		FPlane( 0,  0, -1,  0 ),
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
	switch (m_coordinate_system) 
	{
		case EDTrackCoordinateSystemType::CST_Normal:
		{
			ret.X = n_translation[1] * MM_2_CM;
			ret.Y = n_translation[0] * MM_2_CM;
			ret.Z = n_translation[2] * MM_2_CM;
			break;
		}
		case EDTrackCoordinateSystemType::CST_Unreal_Adapted:
		{
			ret.X = n_translation[0] * MM_2_CM;
			ret.Y = -n_translation[1] * MM_2_CM;
			ret.Z = n_translation[2] * MM_2_CM;
			break;
		}
		//case EDTrackCoordinateSystemType::CST_Powerwall:
		//{
		//	ret.X = -n_translation[2] * MM_2_CM;
		//	ret.Y = n_translation[0] * MM_2_CM;
		//	ret.Z = n_translation[1] * MM_2_CM;
		//	break;
		//}
		default: 
		{ 
			// do nothing
		}
	}

	return ret;
}

// translate DTrack 3x3 rotation matrix to FRotator according to selected room calibration
// @ToDo needs a bit more and better description
FRotator FCoordinateConverter::from_dtrack_rotation(const double(&n_matrix)[9])
{
	UE_LOG(DTrackPollThreadLog, Display, TEXT("FCoordinateConversionHelper::from_dtrack_rotation()"));

	// take DTrack matrix and put the values into FMatrix 
	// ( M[RowIndex][ColumnIndex], DTrack matrix comes column-wise )
	FMatrix r;
	r.M[0][0] = n_matrix[0 + 0]; r.M[0][1] = n_matrix[0 + 3]; r.M[0][2] = n_matrix[0 + 6]; r.M[0][3] = 0.0; // x part
	r.M[1][0] = n_matrix[1 + 0]; r.M[1][1] = n_matrix[1 + 3]; r.M[1][2] = n_matrix[1 + 6]; r.M[1][3] = 0.0; // y part
	r.M[2][0] = n_matrix[2 + 0]; r.M[2][1] = n_matrix[2 + 3]; r.M[2][2] = n_matrix[2 + 6]; r.M[2][3] = 0.0; // Z part
	r.M[3][0] = 0.0;			 r.M[3][1] = 0.0;			  r.M[3][2] = 0.0;			   r.M[3][3] = 1.0;	// translationary part

	FMatrix r_adapted;
	 
	 
	switch (m_coordinate_system) 
	{  
		case EDTrackCoordinateSystemType::CST_Normal:
		{
			UE_LOG(DTrackPollThreadLog, Display, TEXT(":::::CST_Normal Calculation:::::"));
			r_adapted = m_trafo_normal * r * m_trafo_normal_transposed;
			break;   
		}
		case EDTrackCoordinateSystemType::CST_Unreal_Adapted:
		{
			UE_LOG(DTrackPollThreadLog, Display, TEXT(":::::CST_Unreal_Adapted Calculation:::::"));
			r_adapted = m_trafo_unreal_adapted * r * m_trafo_unreal_adapted_transposed;	// m_Sz * R_l * m_Sz <- needs to be transposed due to unreal interface

			// equivalent to:
			// r_adapted = r.GetTransposed();
			// r_adapted.M[0][2] = -r_adapted.M[0][2]; 
			// r_adapted.M[1][2] = -r_adapted.M[1][2];
			// r_adapted.M[2][0] = -r_adapted.M[2][0];
			// r_adapted.M[2][1] = -r_adapted.M[2][1];
			// r_adapted = r_adapted * combinedRotMatrix.GetTransposed();
			break;
		}
		//case EDTrackCoordinateSystemType::CST_Powerwall:
		//	{UE_LOG(DTrackPollThreadLog, Display, TEXT(":::::CST_Powerwall Calculation:::::"));
		//	r_adapted = m_trafo_powerwall * r * m_trafo_powerwall_transposed;
		//	break; }
		default:	
		{
			// do nothing...
		}
	} 

	// through the Rotator() its guranteed that "Gimbal Lock" is not gonna occurr, because it 
	// first converts the FMatrix to a FTransform which then uses FQuat to get a FRotator.
	return r_adapted.GetTransposed().Rotator();	// FRotator is the Rotation representation in Euler angles.
}

const EDTrackCoordinateSystemType FCoordinateConverter::getCoordinateSystemType() const
{
	return m_coordinate_system;
}
