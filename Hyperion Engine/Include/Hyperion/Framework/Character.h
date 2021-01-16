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

	private:

		bool ProcessKeyBinding( const String& inKey );
		bool ProcessAxisBinding( const String& inKey, float inValue );

	protected:

		HypPtr< CharacterController > m_Controller;
		void PerformPossession( const HypPtr< CharacterController >& inController );

		virtual void OnPossessed( const HypPtr< CharacterController >& inController ) {}
		virtual void OnUnPossessed( const HypPtr< CharacterController >& oldController ) {}

		HypPtr< CameraComponent > m_Camera;
		HypPtr< MovementComponent > m_Movement;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void MoveForward( float inScalar );
		virtual void MoveRight( float inScalar );
		virtual void LookUp( float inScalar );
		virtual void LookRight( float inScalar );

		// Old input system
		virtual bool HandleKeyBinding( const String& inKey );
		virtual bool HandleAxisBinding( const String& inAxis, float inValue );

	public:

		bool IsPossessed() const;
		HypPtr< CharacterController > GetController() const;

		HypPtr< CameraComponent > GetCamera() const;
		HypPtr< MovementComponent > GetMovement() const;

		friend class CharacterController;
		friend class Player;
	};

}