/*************************************************************************
> File Name: GridSinglePhasePressureSolver3.h
> Project Name: CubbyFlow
> Author: Chan-Ho Chris Ohk
> Purpose: 3-D single-phase pressure solver.
> Created Time: 2017/08/14
> Copyright (c) 2017, Chan-Ho Chris Ohk
*************************************************************************/
#ifndef CUBBYFLOW_SINGLE_PHASE_PRESSURE_SOLVER3_H
#define CUBBYFLOW_SINGLE_PHASE_PRESSURE_SOLVER3_H

#include <Solver/FDM/FDMLinearSystemSolver3.h>
#include <Solver/Grid/GridPressureSolver3.h>

namespace CubbyFlow
{
	//!
	//! \brief 3-D single-phase pressure solver.
	//!
	//! This class implements 3-D single-phase pressure solver. This solver encodes
	//! the boundaries like Lego blocks -- if a grid cell center is inside or
	//! outside the boundaries, it is either marked as occupied or not.
	//! In addition, this class solves single-phase flow, solving the pressure for
	//! selected fluid region only and treat other area as an atmosphere region.
	//! Thus, the pressure outside the fluid will be set to a constant value and
	//! velocity field won't be altered. This solver also computes the fluid
	//! boundary in block-like manner; If a grid cell is inside or outside the
	//! fluid, it is marked as either fluid or atmosphere. Thus, this solver in
	//! general, does not compute sub-grid structure.
	//!
	class GridSinglePhasePressureSolver3 : public GridPressureSolver3
	{
	public:
		//! Default constructor.
		GridSinglePhasePressureSolver3();

		//! Default destructor.
		virtual ~GridSinglePhasePressureSolver3();

		//!
		//! \brief Solves the pressure term and apply it to the velocity field.
		//!
		//! This function takes input velocity field and outputs pressure-applied
		//! velocity field. It also accepts extra arguments such as \p boundarySDF
		//! and \p fluidSDF that represent signed-distance representation of the
		//! boundary and fluid area. The negative region of \p boundarySDF means
		//! it is occupied by solid object. Also, the positive / negative area of
		//! the \p fluidSDF means it is occupied by fluid / atmosphere. If not
		//! specified, constant scalar field with std::numeric_limits<double>::max() will be used for
		//! \p boundarySDF meaning that no boundary at all. Similarly, a constant
		//! field with -std::numeric_limits<double>::max() will be used for \p fluidSDF
		//! which means it's fully occupied with fluid without any atmosphere.
		//!
		//! \param[in]    input                 The input velocity field.
		//! \param[in]    timeIntervalInSeconds The time interval for the sim.
		//! \param[inout] output                The output velocity field.
		//! \param[in]    boundarySDF           The SDF of the boundary.
		//! \param[in]    fluidSDF              The SDF of the fluid/atmosphere.
		//!
		void Solve(
			const FaceCenteredGrid3& input,
			double timeIntervalInSeconds,
			FaceCenteredGrid3* output,
			const ScalarField3& boundarySDF = ConstantScalarField3(std::numeric_limits<double>::max()),
			const VectorField3& boundaryVelocity = ConstantVectorField3({ 0, 0, 0 }),
			const ScalarField3& fluidSDF = ConstantScalarField3(-std::numeric_limits<double>::max())) override;

		//!
		//! \brief Returns the best boundary condition solver for this solver.
		//!
		//! This function returns the best boundary condition solver that works well
		//! with this pressure solver. Depending on the pressure solver
		//! implementation, different boundary condition solver might be used. For
		//! this particular class, an instance of GridBlockedBoundaryConditionSolver3
		//! will be returned since this pressure solver encodes boundaries
		//! like pixelated Lego blocks.
		//!
		GridBoundaryConditionSolver3Ptr SuggestedBoundaryConditionSolver() const override;

		//! Sets the linear system solver.
		void SetLinearSystemSolver(const FDMLinearSystemSolver3Ptr& solver);

		//! Returns the pressure field.
		const FDMVector3& GetPressure() const;

	private:
		FDMLinearSystem3 m_system;
		FDMLinearSystemSolver3Ptr m_systemSolver;
		Array3<char> m_markers;

		void BuildMarkers(
			const Size3& size,
			const std::function<Vector3D(size_t, size_t, size_t)>& pos,
			const ScalarField3& boundarySDF,
			const ScalarField3& fluidSDF);

		virtual void BuildSystem(const FaceCenteredGrid3& input);

		virtual void ApplyPressureGradient(const FaceCenteredGrid3& input, FaceCenteredGrid3* output);
	};

	//! Shared pointer type for the GridSinglePhasePressureSolver3.
	using GridSinglePhasePressureSolver3Ptr = std::shared_ptr<GridSinglePhasePressureSolver3>;
}

#endif