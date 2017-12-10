/*************************************************************************
> File Name: PhysicsAnimation.h
> Project Name: CubbyFlow
> Author: Chan-Ho Chris Ohk
> Purpose: Abstract base class for physics-based animation.
> Created Time: 2017/04/23
> Copyright (c) 2017, Chan-Ho Chris Ohk
*************************************************************************/
#ifndef CUBBYFLOW_PHYSICS_ANIMATION_H
#define CUBBYFLOW_PHYSICS_ANIMATION_H

#include <Animation/Animation.h>

namespace CubbyFlow
{
	//!
	//! \brief Abstract base class for physics-based animation.
	//!
	//! This class represents physics-based animation by adding time-integration
	//! specific functions to Animation class.
	//!
	class PhysicsAnimation : public Animation
	{
	public:
		//! Default constructor.
		PhysicsAnimation();

		//! Destructor.
		virtual ~PhysicsAnimation();

		//!
		//! \brief Returns true if fixed sub-timestepping is used.
		//!
		//! When performing a time-integration, it is often required to take
		//! sub-timestepping for better results. The sub-stepping can be either
		//! fixed rate or adaptive, and this function returns which feature is
		//! currently selected.
		//!
		//! \return True if using fixed sub time steps, false otherwise.
		//!
		bool IsUsingFixedSubTimeSteps() const;

		//!
		//! \brief Sets true if fixed sub-timestepping is used.
		//!
		//! When performing a time-integration, it is often required to take
		//! sub-timestepping for better results. The sub-stepping can be either
		//! fixed rate or adaptive, and this function sets which feature should be
		//! selected.
		//!
		//! \param[in] isUsing True to enable fixed sub-stepping.
		//!
		void SetIsUsingFixedSubTimeSteps(bool isUsing);

		//!
		//! \brief Returns the number of fixed sub-timesteps.
		//!
		//! When performing a time-integration, it is often required to take
		//! sub-timestepping for better results. The sub-stepping can be either
		//! fixed rate or adaptive, and this function returns the number of fixed
		//! sub-steps.
		//!
		//! \return The number of fixed sub-timesteps.
		//!
		unsigned int NumberOfFixedSubTimeSteps() const;

		//!
		//! \brief Sets the number of fixed sub-timesteps.
		//!
		//! When performing a time-integration, it is often required to take
		//! sub-timestepping for better results. The sub-stepping can be either
		//! fixed rate or adaptive, and this function sets the number of fixed
		//! sub-steps.
		//!
		//! \param[in] numberOfSteps The number of fixed sub-timesteps.
		//!
		void SetNumberOfFixedSubTimeSteps(unsigned int numberOfSteps);

		//! Advances a single frame.
		void AdvanceSingleFrame();

		//!
		//! \brief Returns current frame.
		//!
		Frame CurrentFrame() const;

		//!
		//! \brief Sets current frame cursor (but do not invoke update()).
		//!
		void SetCurrentFrame(const Frame& frame);

		//!
		//! \brief Returns current time in seconds.
		//!
		//! This function returns the current time which is calculated by adding
		//! current frame + sub-timesteps it passed.
		//!
		double CurrentTimeInSeconds() const;

	protected:
		//!
		//! \brief Called when a single time-step should be advanced.
		//!
		//! When Animation::update function is called, this class will internally
		//! subdivide a frame into sub-steps if needed. Each sub-step, or time-step,
		//! is then taken to move forward in time. This function is called for each
		//! time-step, and a subclass that inherits PhysicsAnimation class should
		//! implement this function for its own physics model.
		//!
		//! \param[in] timeIntervalInSeconds The time interval in seconds
		//!
		virtual void OnAdvanceTimeStep(double timeIntervalInSeconds) = 0;

		//!
		//! \brief Returns the required number of sub-timesteps for given time interval.
		//!
		//! The required number of sub-timestep can be different depending on the
		//! physics model behind the implementation. Override this function to
		//! implement own logic for model specific sub-timestepping for given
		//! time interval.
		//!
		//! \param[in] timeIntervalInSeconds The time interval in seconds.
		//!
		//! \return The required number of sub-timesteps.
		//!
		virtual unsigned int NumberOfSubTimeSteps(double timeIntervalInSeconds) const;

		//!
		//! \brief Called at frame 0 to initialize the physics state.
		//!
		//! Inheriting classes can override this function to setup initial condition
		//! for the simulation.
		//!
		virtual void OnInitialize();

	private:
		Frame m_currentFrame;
		bool m_isUsingFixedSubTimeSteps = true;
		unsigned int m_numberOfFixedSubTimeSteps = 1;
	    bool m_hasInitialized = false;
		double m_currentTime = 0.0;

		void OnUpdate(const Frame& frame) final;

		void AdvanceTimeStep(double timeIntervalInSeconds);

		void Initialize();
	};

	using PhysicsAnimationPtr = std::shared_ptr<PhysicsAnimation>;
}

#endif