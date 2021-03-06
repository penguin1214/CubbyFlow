/*************************************************************************
> File Name: GridSmokeSolver2.h
> Project Name: CubbyFlow
> Author: Chan-Ho Chris Ohk
> Purpose: 2-D grid-based smoke solver.
> Created Time: 2017/08/17
> Copyright (c) 2018, Chan-Ho Chris Ohk
*************************************************************************/
#ifndef CUBBYFLOW_GRID_SMOKE_SOLVER2_H
#define CUBBYFLOW_GRID_SMOKE_SOLVER2_H

#include <Solver/Grid/GridFluidSolver2.h>

namespace CubbyFlow
{
	//!
	//! \brief      2-D grid-based smoke solver.
	//!
	//! This class extends GridFluidSolver2 to implement smoke simulation solver.
	//! It adds smoke density and temperature fields to define the smoke and uses
	//! buoyancy force to simulate hot rising smoke.
	//!
	//! \see Fedkiw, Ronald, Jos Stam, and Henrik Wann Jensen.
	//!     "Visual simulation of smoke." Proceedings of the 28th annual conference
	//!     on Computer graphics and interactive techniques. ACM, 2001.
	//!
	class GridSmokeSolver2 : public GridFluidSolver2
	{
	public:
		class Builder;

		//! Default constructor.
		GridSmokeSolver2();

		//! Constructs solver with initial grid size.
		GridSmokeSolver2(
			const Size2& resolution,
			const Vector2D& gridSpacing,
			const Vector2D& gridOrigin);

		//! Destructor.
		virtual ~GridSmokeSolver2();

		//! Returns smoke diffusion coefficient.
		double GetSmokeDiffusionCoefficient() const;

		//! Sets smoke diffusion coefficient.
		void SetSmokeDiffusionCoefficient(double newValue);

		//! Returns temperature diffusion coefficient.
		double GetTemperatureDiffusionCoefficient() const;

		//! Sets temperature diffusion coefficient.
		void SetTemperatureDiffusionCoefficient(double newValue);

		//!
		//! \brief      Returns the buoyancy factor which will be multiplied to the
		//!     smoke density.
		//!
		//! This class computes buoyancy by looking up the value of smoke density
		//! and temperature, compare them to the average values, and apply
		//! multiplier factor to the diff between the value and the average. That
		//! multiplier is defined for each smoke density and temperature separately.
		//! For example, negative smoke density buoyancy factor means a heavier
		//! smoke should sink.
		//!
		//! \return     The buoyancy factor for the smoke density.
		//!
		double GetBuoyancySmokeDensityFactor() const;

		//!
		//! \brief      Sets the buoyancy factor which will be multiplied to the
		//!     smoke density.
		//!
		//! This class computes buoyancy by looking up the value of smoke density
		//! and temperature, compare them to the average values, and apply
		//! multiplier factor to the diff between the value and the average. That
		//! multiplier is defined for each smoke density and temperature separately.
		//! For example, negative smoke density buoyancy factor means a heavier
		//! smoke should sink.
		//!
		//! \param newValue The new buoyancy factor for smoke density.
		//!
		void SetBuoyancySmokeDensityFactor(double newValue);

		//!
		//! \brief      Returns the buoyancy factor which will be multiplied to the
		//!     temperature.
		//!
		//! This class computes buoyancy by looking up the value of smoke density
		//! and temperature, compare them to the average values, and apply
		//! multiplier factor to the diff between the value and the average. That
		//! multiplier is defined for each smoke density and temperature separately.
		//! For example, negative smoke density buoyancy factor means a heavier
		//! smoke should sink.
		//!
		//! \return     The buoyancy factor for the temperature.
		//!
		double GetBuoyancyTemperatureFactor() const;

		//!
		//! \brief      Sets the buoyancy factor which will be multiplied to the
		//!     temperature.
		//!
		//! This class computes buoyancy by looking up the value of smoke density
		//! and temperature, compare them to the average values, and apply
		//! multiplier factor to the diff between the value and the average. That
		//! multiplier is defined for each smoke density and temperature separately.
		//! For example, negative smoke density buoyancy factor means a heavier
		//! smoke should sink.
		//!
		//! \param newValue The new buoyancy factor for temperature.
		//!
		void SetBuoyancyTemperatureFactor(double newValue);

		//!
		//! \brief      Returns smoke decay factor.
		//!
		//! In addition to the diffusion, the smoke also can fade-out over time by
		//! setting the decay factor between 0 and 1.
		//!
		//! \return     The decay factor for smoke density.
		//!
		double GetSmokeDecayFactor() const;

		//!
		//! \brief      Sets the smoke decay factor.
		//!
		//! In addition to the diffusion, the smoke also can fade-out over time by
		//! setting the decay factor between 0 and 1.
		//!
		//! \param[in]  newValue The new decay factor.
		//!
		void SetSmokeDecayFactor(double newValue);

		//!
		//! \brief      Returns temperature decay factor.
		//!
		//! In addition to the diffusion, the smoke also can fade-out over time by
		//! setting the decay factor between 0 and 1.
		//!
		//! \return     The decay factor for smoke temperature.
		//!
		double GetTemperatureDecayFactor() const;

		//!
		//! \brief      Sets the temperature decay factor.
		//!
		//! In addition to the diffusion, the temperature also can fade-out over
		//! time by setting the decay factor between 0 and 1.
		//!
		//! \param[in]  newValue The new decay factor.
		//!
		void SetTemperatureDecayFactor(double newValue);

		//! Returns smoke density field.
		ScalarGrid2Ptr GetSmokeDensity() const;

		//! Returns temperature field.
		ScalarGrid2Ptr GetTemperature() const;

		//! Returns builder fox GridSmokeSolver2.
		static Builder GetBuilder();

	protected:
		void OnEndAdvanceTimeStep(double timeIntervalInSeconds) override;

		void ComputeExternalForces(double timeIntervalInSeconds) override;

	private:
		size_t m_smokeDensityDataID = 0;
		size_t m_temperatureDataID = 0;
		double m_smokeDiffusionCoefficient = 0.0;
		double m_temperatureDiffusionCoefficient = 0.0;
		double m_buoyancySmokeDensityFactor = -0.000625;
		double m_buoyancyTemperatureFactor = 5.0;
		double m_smokeDecayFactor = 0.001;
		double m_temperatureDecayFactor = 0.001;

		void ComputeDiffusion(double timeIntervalInSeconds);

		void ComputeBuoyancyForce(double timeIntervalInSeconds);
	};

	//! Shared pointer type for the GridSmokeSolver2.
	using GridSmokeSolver2Ptr = std::shared_ptr<GridSmokeSolver2>;
	
	//!
	//! \brief Front-end to create GridSmokeSolver2 objects step by step.
	//!
	class GridSmokeSolver2::Builder final : public GridFluidSolverBuilderBase2<GridSmokeSolver2::Builder>
	{
	public:
		//! Builds GridSmokeSolver2.
		GridSmokeSolver2 Build() const;

		//! Builds shared pointer of GridSmokeSolver2 instance.
		GridSmokeSolver2Ptr MakeShared() const;
	};
}

#endif