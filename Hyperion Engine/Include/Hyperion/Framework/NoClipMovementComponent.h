/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/NoClipMovementComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/MovementComponent.h"


namespace Hyperion
{

	class NoClipMovementComponent : public MovementComponent
	{

	private:

		bool m_bFwd, m_bBwd, m_bRight, m_bLeft;

	protected:

		void Tick( double inDelta ) override;

	public:

		NoClipMovementComponent();

		// New input system
		void MoveForward( float inScalar ) override;
		void MoveRight( float inScalar ) override;
		void LookUp( float inScalar ) override;
		void LookRight( float inScalar ) override;

		// Old input system
		bool HandleKeyBinding( const String& inBind ) override;
		bool HandleAxisBinding( const String& inBind, float inValue ) override;

	};

}