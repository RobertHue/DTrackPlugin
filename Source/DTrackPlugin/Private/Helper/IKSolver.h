#pragma once

#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Math/Rotator.h"
#include "Containers/Array.h"
#include <cstdlib>

/*
IK-Solver:
	for further information see:
	https://www.math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
	https://github.com/dfelinto/blender/tree/3068ea34e414e7af6d20af7ab92c69cbd9a33414/intern/iksolver/intern and
	Introduction to Inverse Kinematics with
	Jacobian Transpose, Pseudoinverse and Damped
	Least Squares methods by Samuel R. Buss (San Diego)
*/



class IKSolver; // forward declaration

class IKSegment {
	friend class IKSolver;

public:
	// number of degrees of freedom
	int NumberOfDoF() const { return m_num_DoF; }

	// unique id for this segment, for identification in the jacobian
	int DoFId() const { return m_DoF_id; }
	void SetDoFId(int dof_id) { m_DoF_id = dof_id; }

	// set joint limits
	// void SetLimit(int, double, double) { }

private:
	int m_num_DoF;
	int m_DoF_id;

	double m_length;
	double m_angle;

	FVector pos;
	FRotator rot;
	FVector rotationAxis;	// the axis of rotation as a unit vector


};


/*
	An IK Solver using a Jacobian-Matrix to solve the IK problem

	The name of this class stems from the blender project's IK solver

	Damped least squares aka Levenberg-Marquardt method
*/
class IKSolver {

public:
	IKSolver() {
	
	}

	// this method tries to solve the IK by applying the transpose Jacobian...
	void solve(TArray<IKSegment> segments) {
		// joint angles lengths and other values..

		int sum_of_DoFs = 0;
		for (int i = 0; i < segments.Num(); ++i) { sum_of_DoFs += segments[i].m_num_DoF; }

		const int n = sum_of_DoFs; // Sum of all DoFs
		const int m = 3 * 1; // l*k = dim * num_of_end_effectors


		//////////////////////////////////
		/// CREATE THE JACOBIAN-MATRIX /// 
		//////////////////////////////////
		int mat_size = m * n;
		double* jacobian = (double*)(malloc(mat_size * sizeof(double)));	// pro segment eine spalte in der jacobian matrix
		if (jacobian == NULL) { return; }

		// s_x = position of end effector
		// p_i = position of the joint
		// a_i = the unit vector with the direction of the rotation axis of p_i

		IKSegment& end_effector = segments[segments.Num()-1];

		for (int i = 0; i < (n); ++i) 
		{
			// Berechne Vektor von joint-Positon zu End-Effektor-Position und den Jacobi-Eintrag für die erste Spalte
			FVector qi = (end_effector.pos - segments[i].pos);
			FVector jacobianEntry = FVector::CrossProduct(segments[i].rotationAxis, qi);

			// Weise Jacobi-Eintrag der ersten Spalte zu
			jacobian[i + 0] = jacobianEntry.X;
			jacobian[i + 3] = jacobianEntry.Y;
			jacobian[i + 6] = jacobianEntry.Z;
		}

		//////////////////////////
		/// SOLVE THE JACOBIAN /// 
		//////////////////////////
		// by using Damped Least Squares (Levenberg-Marquardt algorithm):
		// if lambda is zero its the pseudoinverse algorithm
		// if it's too large the delta_theta will only change minimal leading to more iterations...





		// TODO / note: use SVD to optmize the Damped Least Square method...




		////////////////////////////////////
		/// UPDATE THE SUBMODEL's JOINTS /// 
		////////////////////////////////////



		if (jacobian != NULL) { free(jacobian); }
	}


private:
	transposedJacobian() {

	}

};