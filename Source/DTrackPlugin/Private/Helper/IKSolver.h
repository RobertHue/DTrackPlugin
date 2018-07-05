#pragma once

#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Math/Rotator.h"
#include "Containers/Array.h"
#include "Helper/CoordinateConverter.h"
#include <cstdlib>
#include <queue>	// for traversing the Tree of IKSegment 
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>

#include <Eigen/Dense>

/*
This component is experimental... Use at your own risk!

IK-Solver:
	for further information see:
	https://www.math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
	https://github.com/dfelinto/blender/tree/3068ea34e414e7af6d20af7ab92c69cbd9a33414/intern/iksolver/intern and
	Introduction to Inverse Kinematics with
	Jacobian Transpose, Pseudoinverse and Damped
	Least Squares methods by Samuel R. Buss (San Diego)

	Some parts (SDLS and JacobianTranspose) from Samuel R. Buss so here my acknowledge for that:
	* Mathematics Subpackage (VrMath)
	*
	* Author: Samuel R. Buss
	*
	* Software accompanying the book
	*		3D Computer Graphics: A Mathematical Introduction with OpenGL,
	*		by S. Buss, Cambridge University Press, 2003.
	*
	* Software is "as-is" and carries no warranty.  It may be used without
	*   restriction, but if you modify it, please change the filenames to
	*   prevent confusion between different versions.  Please acknowledge
	*   all use of the software in any publications or products based on it.
	*
	* Bug reports: Sam Buss, sbuss@ucsd.edu.
	* Web page: http://math.ucsd.edu/~sbuss/MathCG
*/



class IKSolver; // forward declaration

class IKSegment {
	friend class IKSolver;

public:
	IKSegment(int num_DoF) :
		m_num_DoF(num_DoF),
		m_parent(nullptr), 
		m_child(nullptr)
	{};
	virtual ~IKSegment() {};


	// number of degrees of freedom
	int NumberOfDoF() const { return m_num_DoF; }

	// unique id for this segment, for identification in the jacobian
	int DoFId() const { return m_DoF_id; }
	void SetDoFId(int dof_id) { m_DoF_id = dof_id; }

	// set joint limits
	// void SetLimit(int, double, double) { }

	// tree structure access
	void SetParent(IKSegment *parent)
	{
		if (m_parent == parent)		return;

		//if (m_parent)
			//m_parent->RemoveChild(this);

		if (parent) {
		//	m_sibling = parent->m_child;
			parent->m_child = this;
		} 

		m_parent = parent;
	}

	IKSegment *Child() const
	{
		return m_child;
	}

	IKSegment *Parent() const
	{
		return m_parent;
	}

	/// set the rotation axis as unit vector
	/// note: this method changes the passed FVector to be a unit vector!
	void SetRotationAxis(FVector unrealVector) {
		unrealVector.Normalize(); // inplace normalize
		unitVecRotationAxis = Eigen::Vector3d(unrealVector.X, unrealVector.Y, unrealVector.Z);
	}
	void SetPosition(FVector unrealPosition) {
		pos = Eigen::Vector3d(unrealPosition.X, unrealPosition.Y, unrealPosition.Z);
	}

	double GetTheta() const {
		return m_theta;
	}

private:
	int m_num_DoF;
	int m_DoF_id;

	double m_length;
	double m_theta;   // the angle along the rotation axis unitVecRotationAxis

	Eigen::Vector3d pos;
	Eigen::Vector3d unitVecRotationAxis;	// the axis of rotation as a unit vector
							
	// tree structure variables
	IKSegment *m_parent;
	IKSegment *m_child; // the child nodes


	// enum class Purpose { JOINT, EFFECTOR }; the effector is the last segment in the chain
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*
	An IK Solver using a Jacobian-Matrix to solve the IK problem

	The name of this class stems from the blender project's IK solver

	Damped least squares aka Levenberg-Marquardt method
*/
class IKSolver {

public:
	IKSolver(IKSegment *rootSegment) 
		: 
		m_rootSegment(rootSegment),
		m_jacobian(),
		m_mRows(0), m_nCols(0),
		m_nDoFs(0), m_nEndEffectors(0), m_nJoints(0), m_nSegments(0)
	{
		//////////////////////////////////
		/// INITIAL SETUP              ///
		//////////////////////////////////
		SetupIKSolver(rootSegment);		// Sets nDofs nEndEffectors nJoints and nSegments
		m_nCols = m_nDoFs;				// Sum of all DoFs
		m_mRows = 3 * m_nEndEffectors;	// k*1 = dim * num_of_end_effectors

		//////////////////////////////////
		/// CREATE THE JACOBIAN-MATRIX /// 
		//////////////////////////////////
		m_dS.resize(m_mRows);			// (Target positions) - (End effector positions)
		m_dTheta.resize(m_nCols);		// Changes in joint angles

		//////////////////////////////////
		/// CREATE THE JACOBIAN-MATRIX /// 
		//////////////////////////////////
		//int mat_size = m_nCols * m_mRows; 
		//m_jacobian = (double*)(malloc(mat_size * sizeof(double)));	// pro segment eine spalte in der jacobian matrix
		//if (m_jacobian == NULL) { return; } // could not malloc so return...
		m_jacobian.resize(m_mRows, m_nCols);	// is column-major
	}

	virtual ~IKSolver() {
		//if (m_jacobian != NULL) { free(m_jacobian); }
	}

	// this method tries to solve the IK by applying the SDLS Jacobian...
	void Solve(FVector unrealTargetPos) {
		// calculate the difference from end_effector to target_pos (dS)
		Eigen::Vector3d targetPos(unrealTargetPos.X, unrealTargetPos.Y, unrealTargetPos.Z);
		m_dS = targetPos - m_end_effector->pos;

		ComputeJacobian();

		//////////////////////////
		/// SOLVE THE JACOBIAN /// 
		//////////////////////////
		CalcDeltaThetasTranspose();
		// CalcDeltaThetasDLS();
		// CalcDeltaThetasSDLS();
		  
		////////////////////////////////////
		/// UPDATE THE SUBMODEL's JOINTS /// 
		////////////////////////////////////
		// The delta theta values have been computed in dTheta array
		// Apply the delta theta values to the joints
		// Nothing is done about joint limits for now.
		UE_LOG(LogTemp, Warning, TEXT("pSeg"));
		IKSegment *pSeg = m_rootSegment;
		int i = 0; // Eigen::Index
		std::cout << "i : " << i << std::endl;
		while (pSeg != nullptr) {
			if (i < m_dTheta.size()) {
				UE_LOG(LogTemp, Warning, TEXT("ok: m_dTheta just has : %d rows %d cols"), m_dTheta.rows(), m_dTheta.cols());
				// std::cout << "ok: m_dTheta just has " << m_dTheta.rows() << " rows and " << m_dTheta.cols() << " cols." << std::endl;
				pSeg->m_theta = m_dTheta(i);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("error: m_dTheta just has : %d rows %d cols"), m_dTheta.rows(), m_dTheta.cols());
				//std::cout << "error: m_dTheta just has " << m_dTheta.rows() << " rows and " << m_dTheta.cols() << " cols." << std::endl;
			} 
			  
			// advance
			++i;
			UE_LOG(LogTemp, Warning, TEXT("i : %d"), i);
			pSeg = pSeg->Child();
			//std::cout << "after advance op" << std::endl;
			UE_LOG(LogTemp, Warning, TEXT("after advance op"));
		}
	}

	IKSegment *GetRootSegment() const {
		return m_rootSegment;
	}


	// Does a Tree level-order traversal to get the number of degrees of freedom (total sum)
	void SetupIKSolver(IKSegment * segment)
	{
		IKSegment *tmpSegment = segment;

		while (tmpSegment != nullptr)
		{
			///////////////////////////////////
			// do your task here:
			// std::cout << v->NumberOfDoF << " ";

			// mostly every segment has a DoF of 1
			m_nDoFs += tmpSegment->NumberOfDoF();

			// the effector is the last segment in the kinematic chain
			if (tmpSegment->Child()==nullptr) { ++m_nEndEffectors; m_end_effector = tmpSegment; }
			else { ++m_nJoints; }
			///////////////////////////////////  
			// go to next segment: (advance)
			tmpSegment = tmpSegment->Child();
		}

		/*
		// Create a queue
		//std::queue<IKSegment *> q;

		// Push all the root's neighboors
		for (size_t i = 0; i < segment->Child.size(); ++i) {
		q.push((segment->Child));
		}

		int countPulled = 0;
		int level = 0;
		std::cout << "\n" << level << ": ";

		while (!q.empty()) {
		// Dequeue a node from front
		IKSegment *v = q.front();
		++countPulled;

		///////////////////////////////////
		// do your task here:
		std::cout << v->NumberOfDoF << " ";

		// mostly every segment has a DoF of 1
		m_nDoFs += v->NumberOfDoF;

		// the effector is the last segment in the kinematic chain
		if (v->Child.isEmpty()) { ++m_nEndEffectors; end_effectors.push_back(v); }
		else					{ ++m_nJoints; }
		///////////////////////////////////

		for (size_t i = 0; i < v->Child.size(); ++i) {
		q.push((v->Child));
		}

		// Pop the visited node
		q.pop();
		if (countPulled % 8 == 0) {
		std::cout << "\n" << ++level << ": ";
		}
		}
		std::cout << std::endl << std::endl;
		*/

		m_nSegments = m_nJoints + m_nEndEffectors;
	}


private:

	// Compute the deltaS vector, dS (the error in end effector positions
	// Compute the J and K matrices (the Jacobians)
	void ComputeJacobian() {

		IKSegment *segNode = m_rootSegment;

		for (int n = 0; n < m_nCols; ++n)
		{
			// entry inside the jacobian-Matrix is: (targetPos - endEffectorPos) = e
			// unit vector pointing along axis of rotation x ( end_effector_position - position_of_the_join)  
			Eigen::Vector3d diff = m_end_effector->pos - segNode->pos;	
			UE_LOG(LogTemp, Warning, TEXT("m_dS x : %f"), diff[0]);
			UE_LOG(LogTemp, Warning, TEXT("m_dS y : %f"), diff[1]);
			UE_LOG(LogTemp, Warning, TEXT("m_dS z : %f"), diff[2]);
			Eigen::Vector3d jacobiEntry = segNode->unitVecRotationAxis.cross(diff);
			m_jacobian.col(n) = jacobiEntry;
			UE_LOG(LogTemp, Warning, TEXT("jacobiEntry x : %f"), jacobiEntry[0]);
			UE_LOG(LogTemp, Warning, TEXT("jacobiEntry y : %f"), jacobiEntry[1]);
			UE_LOG(LogTemp, Warning, TEXT("jacobiEntry z : %f"), jacobiEntry[2]);

			// next segment:
			segNode = segNode->Child();
		}
		// std::cout << "m_jacobian : " << m_jacobian << std::endl;
	}

	void CalcDeltaThetasTranspose() {

		// const double MaxAngleJtranspose = 30.0 * DEG_TO_RAD;

		Eigen::MatrixXd jacobianTranspose = m_jacobian.transpose();

		m_dTheta = jacobianTranspose * m_dS;

		// Scale back the dTheta values greedily 
		m_dT = m_jacobian * m_dTheta;						// dT = J * dTheta
		double alpha = m_dS.dot(m_dT) / m_dT.squaredNorm();	// Dot(dS, dT) / dT.NormSq();
		assert(alpha>0.0);

		// Also scale back to be have max angle change less than MaxAngleJtranspose
		// double maxChange = m_dTheta.maxCoeff();		// MaxAbs();
		// double beta = MaxAngleJtranspose / maxChange;
		// m_dTheta *= std::min(alpha, beta);
		m_dTheta *= alpha;
	}

	/*
	void CalcDeltaThetasDLS()
	{
		//const double MaxAngleDLS = 45.0 * DEG_TO_RAD;

		// const MatrixRmn& J = ActiveJacobian();
		const double dampingLambda = 1.1;			// Optimal for the DLS "double Y" shape (any lower gives jitter)
		const double dampingLambdaSq = dampingLambda * dampingLambda;

		// MatrixRmn::MultiplyTranspose(J, J, U);		// U = J * (J^T)
		Eigen::MatrixXd U = m_jacobian  * (m_jacobian.transpose());
		
		//U.AddToDiagonal(DampingLambdaSq);
		for (int i = 0; i < U.rows(); ++i) {
			U(i, i) += dampingLambdaSq;
		}

		// Use the next four lines instead of the succeeding two lines for the DLS method with clamped error vector e.
		// CalcdTClampedFromdS();
		// VectorRn dTextra(3*nEffector);
		// U.Solve( dT, &dTextra );
		// J.MultiplyTranspose( dTextra, dTheta );

		// Use these two lines for the traditional DLS method
		m_dT = U.inverse() * m_dS;	//U.Solve(dS, &dT);
		m_dTheta = m_jacobian.transpose() * m_dT;	//J.MultiplyTranspose(dT, dTheta);

		// Scale back to not exceed maximum angle changes
		//double maxChange = m_dTheta.maxCoeff();		// MaxAbs();
		//if (maxChange > MaxAngleDLS) {
		//	m_dTheta *= MaxAngleDLS / maxChange;
		//}
	}*/

	/*
	void CalcDeltaThetasSDLS()
	{
		Eigen::MatrixXd Jnorms;	// Norms of 3-vectors in active Jacobian (SDLS only)
		const double MaxAngleSDLS = 45.0 * DEG_TO_RAD;
		const Eigen::MatrixXd& J = m_jacobian;

		// Compute Singular Value Decomposition 

		//J.ComputeSVD(U, w, V);
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(m_jacobian, Eigen::ComputeThinU | Eigen::ComputeThinV);
		m_U = svd.matrixU();
		m_w = svd.singularValues().asDiagonal();
		m_V = svd.matrixV();
		 
		// Next line for debugging only
		// assert(J.DebugCheckSVD(U, w, V));

		// Calculate response vector dTheta that is the SDLS solution.
		//	Delta target values are the dS values
		int nRows = J.rows();
		int numEndEffectors = static_cast<int>(nRows / 3); // Equals the number of rows of J divided by three
		int nCols = J.cols();
		m_dTheta.setZero();	// Zero out the entire vector

		// Calculate the norms of the 3-vectors in the Jacobian J
		long i;
		//const double *jx	 = J.GetPtr();
		//double *jnx		 = Jnorms.GetPtr();
		int idx_jx = 0;
		int idx_jnx = 0;
		Jnorms.resize(1, nCols * numEndEffectors);
		for (i = nCols * numEndEffectors; i>0; i--) {
			double accumSq = m_jacobian(idx_jx) * m_jacobian(idx_jx); // Square(*(jx++));
			++idx_jx;
			accumSq += m_jacobian(idx_jx) * m_jacobian(idx_jx);
			++idx_jx;
			accumSq += m_jacobian(idx_jx) * m_jacobian(idx_jx);
			Jnorms(idx_jnx) = std::sqrt(accumSq); 
			++idx_jnx;
		}

		// Clamp the dS values
		CalcdTClampedFromdS();

		// Loop over each singular vector
		for (i = 0; i<nRows; i++) {

			double wiInv = m_w[i];
			if (NearZero(wiInv, 1.0e-10)) {
				continue;
			}
			wiInv = 1.0 / wiInv;

			double N = 0.0;						// N is the quasi-1-norm of the i-th column of U
			double alpha = 0.0;					// alpha is the dot product of dT and the i-th column of U

			//const double *dTx = m_dT.GetPtr();
			//const double *ux = m_U.GetColumnPtr(i);
			int idx_dTx = 0;
			int idx_ux = 0;
			long j;
			for (j = numEndEffectors; j>0; j--) {
				double tmp;
				++idx_dTx;
				alpha += m_U(idx_ux) * m_dT(idx_dTx);
				++idx_ux;
				tmp = m_U(idx_ux) * m_U(idx_ux);
				++idx_dTx;
				alpha += m_U(idx_ux) * m_dT(idx_dTx);
				++idx_ux;
				tmp += m_U(idx_ux) * m_U(idx_ux);
				++idx_dTx;
				alpha += m_U(idx_ux) * m_dT(idx_dTx);
				++idx_ux; 
				tmp += m_U(idx_ux) * m_U(idx_ux);
				N += sqrt(tmp);
			}

			// M is the quasi-1-norm of the response to angles changing according to the i-th column of V
			//		Then is multiplied by the wiInv value.
			double M = 0.0;
			//double *vx = V.GetColumnPtr(i);
			//jnx = Jnorms.GetPtr();
			int idx_Vx = 0;
			int idx_Jnx = 0;

			for (j = nCols; j>0; j--) {
				double accum = 0.0;
				for (long k = numEndEffectors; k>0; k--) {
					++idx_Jnx;
					accum += Jnorms(idx_Jnx);
				}
				++idx_Vx;
				M += fabs(m_V(idx_Vx)*accum); 
			}
			M *= fabs(wiInv);

			double gamma = MaxAngleSDLS;
			if (N<M) {
				gamma *= N / M;				// Scale back maximum permissable joint angle
			}

			// Calculate the dTheta from pure pseudoinverse considerations
			double scale = alpha * wiInv;			// This times i-th column of V is the psuedoinverse response
			//m_dPreTheta.LoadScaled(V.GetColumnPtr(i), scale);
			m_dPreTheta = m_V; // copy a matrix into another...
			m_dPreTheta *= scale;
			// Now rescale the dTheta values.
			double max = m_dPreTheta.maxCoeff();
			double rescale = (gamma) / (gamma + max);
			// m_dTheta.AddScaled(dPreTheta, rescale);
			m_dTheta = m_dPreTheta * rescale;

			//if ( gamma<max) {
			//dTheta.AddScaled( dPreTheta, gamma/max );
			//}
			//else {
			//dTheta += dPreTheta;
			//}
		}

		// Scale back to not exceed maximum angle changes
		double maxChange = m_dTheta.maxCoeff();
		if (maxChange>MaxAngleSDLS) {
			m_dTheta *= MaxAngleSDLS / (MaxAngleSDLS + maxChange);
			//dTheta *= MaxAngleSDLS/maxChange;
		}
	}

private:
	void CalcdTClampedFromdS()
	{
	long len = m_dS.size(); // size == the number of coeff
	long j = 0;
	for (long i = 0; i<len; i += 3, j++) {
	double normSq = m_dS[i]*m_dS[i] + m_dS[i + 1] * m_dS[i + 1] + m_dS[i + 2] * m_dS[i + 2];
	if (normSq>(m_dSclamp[j]*m_dSclamp[j])) {
	double factor = m_dSclamp[j] / sqrt(normSq);
	m_dT[i] = m_dS[i] * factor;
	m_dT[i + 1] = m_dS[i + 1] * factor;
	m_dT[i + 2] = m_dS[i + 2] * factor;
	}
	else {
	m_dT[i] = m_dS[i];
	m_dT[i + 1] = m_dS[i + 1];
	m_dT[i + 2] = m_dS[i + 2];
	}
	}
	}
	inline bool NearZero(double x, double tolerance) {
	return (std::fabs(x) <= tolerance);
	}
	*/


private:
	IKSegment *m_rootSegment;			// tree / kinematic chain associated with this Jacobian matrix
	Eigen::MatrixXd m_jacobian;		// the real J
	int m_mRows;			// Total number of rows the real J (= 3*number of end effectors for now) DIM = 3
	int m_nCols;			// Total number of columns in the real J (= number of joints for now)


	int m_nDoFs;
	int m_nEndEffectors;
	int m_nJoints;
	int m_nSegments;	// m_nSegments = m_nJoints + m_nEndEffectors;

	Eigen::Vector3d m_dS;			// delta s
	Eigen::Vector3d m_dT;			// delta t		--  these are delta S values clamped to smaller magnitude
	Eigen::VectorXd m_dTheta;		// Changes in joint angles
	
	IKSegment *m_end_effector;

	//Eigen::Vector3d m_dSclamp;		// Value to clamp magnitude of dT at.

	/* // only for SDLS
	Eigen::MatrixXd m_U;		// J = U * Diag(w) * V^T	(Singular Value Decomposition)
	Eigen::Vector3d m_w;
	Eigen::MatrixXd m_V; 
	Eigen::MatrixXd m_dPreTheta;
	*/
};