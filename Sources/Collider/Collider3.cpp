/*************************************************************************
> File Name: Collider3.cpp
> Project Name: CubbyFlow
> Author: Chan-Ho Chris Ohk
> Purpose: Abstract base class for generic collider object.
> Created Time: 2017/05/19
> Copyright (c) 2018, Chan-Ho Chris Ohk
*************************************************************************/
#include <Collider/Collider3.h>

namespace CubbyFlow
{
	Collider3::Collider3()
	{
		// Do nothing
	}

	Collider3::~Collider3()
	{
		// Do nothing
	}

	void Collider3::ResolveCollision(double radius, double restitutionCoefficient, Vector3D* newPosition, Vector3D* newVelocity)
	{
		ColliderQueryResult colliderPoint;

		GetClosestPoint(m_surface, *newPosition, &colliderPoint);

		// Check if the new position is penetrating the surface
		if (IsPenetrating(colliderPoint, *newPosition, radius))
		{
			// Target point is the closest non-penetrating position from the new position.
			Vector3D targetNormal = colliderPoint.normal;
			Vector3D targetPoint = colliderPoint.point + radius * targetNormal;
			Vector3D colliderVelAtTargetPoint = colliderPoint.velocity;

			// Get new candidate relative velocity from the target point.
			Vector3D relativeVel = *newVelocity - colliderVelAtTargetPoint;
			double normalDotRelativeVel = targetNormal.Dot(relativeVel);
			Vector3D relativeVelN = normalDotRelativeVel * targetNormal;
			Vector3D relativeVelT = relativeVel - relativeVelN;

			// Check if the velocity is facing opposite direction of the surface normal
			if (normalDotRelativeVel < 0.0)
			{
				// Apply restitution coefficient to the surface normal component of the velocity
				Vector3D deltaRelativeVelN = (-restitutionCoefficient - 1.0) * relativeVelN;
				relativeVelN *= -restitutionCoefficient;

				// Apply friction to the tangential component of the velocity
				// From Bridson et al., Robust Treatment of Collisions,
				// Contact and Friction for Cloth Animation, 2002
				// http://graphics.stanford.edu/papers/cloth-sig02/cloth.pdf
				if (relativeVelT.LengthSquared() > 0.0)
				{
					double frictionScale = std::max(1.0 - m_frictionCoeffient * deltaRelativeVelN.Length() / relativeVelT.Length(), 0.0);
					relativeVelT *= frictionScale;
				}

				// Reassemble the components
				*newVelocity = relativeVelN + relativeVelT + colliderVelAtTargetPoint;
			}

			// Geometric fix
			*newPosition = targetPoint;
		}
	}

	double Collider3::FrictionCoefficient() const
	{
		return m_frictionCoeffient;
	}

	void Collider3::SetFrictionCoefficient(double newFrictionCoeffient)
	{
		m_frictionCoeffient = std::max(newFrictionCoeffient, 0.0);
	}

	const Surface3Ptr& Collider3::Surface() const
	{
		return m_surface;
	}

	void Collider3::SetSurface(const Surface3Ptr& newSurface)
	{
		m_surface = newSurface;
	}

	void Collider3::GetClosestPoint(const Surface3Ptr& surface, const Vector3D& queryPoint, ColliderQueryResult* result) const
	{
		result->distance = surface->ClosestDistance(queryPoint);
		result->point = surface->ClosestPoint(queryPoint);
		result->normal = surface->ClosestNormal(queryPoint);
		result->velocity = VelocityAt(queryPoint);
	}

	bool Collider3::IsPenetrating(const ColliderQueryResult& colliderPoint, const Vector3D& position, double radius)
	{
		// If the new candidate position of the particle is on the other side of
		// the surface OR the new distance to the surface is less than the
		// particle's radius, this particle is in colliding state.
		return (position - colliderPoint.point).Dot(colliderPoint.normal) < 0.0 || colliderPoint.distance < radius;
	}

	void Collider3::Update(double currentTimeInSeconds, double timeIntervalInSeconds)
	{
		m_surface->UpdateQueryEngine();

		if (m_onUpdateCallback)
		{
			m_onUpdateCallback(this, currentTimeInSeconds, timeIntervalInSeconds);
		}
	}

	void Collider3::SetOnBeginUpdateCallback(const OnBeginUpdateCallback& callback)
	{
		m_onUpdateCallback = callback;
	}
}