/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Character.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Framework/MovementComponent.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class CharacterController;


	class Character : public Entity
	{

	protected:

		HypPtr< CharacterController > m_Controller;
		void PerformPossession( const HypPtr< CharacterController >& inController );

		virtual void OnPossessed( const HypPtr< CharacterController >& inController ) {}
		virtual void OnUnPossessed( const HypPtr< CharacterController >& oldController ) {}

		HypPtr< CameraComponent > m_Camera;
		HypPtr< MovementComponent > m_Movement;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void SetMovementInput( const Vector3D& inVec );
		virtual void SetLookInput( const Vector2D& inVec );

	public:

		bool IsPossessed() const;
		HypPtr< CharacterController > GetController() const;

		HypPtr< CameraComponent > GetCamera() const;
		HypPtr< MovementComponent > GetMovement() const;

		virtual void SetEyeDirection( float inPitch, float inYaw );
		virtual Angle3D GetEyeDirection() const;

		friend class CharacterController;
		friend class Player;
	};

}