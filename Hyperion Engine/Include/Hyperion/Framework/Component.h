/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Component.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"

#include <vector>
#include <string>
#include <map>

namespace Hyperion
{

	class Entity;
	class World;

	class Component : public Object
	{

	public:

		HYPERION_GROUP_OBJECT( CACHE_COMPONENT );

	protected:

		Vector3D m_Position;
		Angle3D m_Rotation;
		Vector3D m_Scale;

		std::weak_ptr< Entity > m_Owner;
		std::weak_ptr< World > m_World;
		std::weak_ptr< Component > m_Parent;
		std::map< std::string, std::shared_ptr< Component > > m_Children;

		std::string m_Name;

		void PerformAttach();
		void PerformDetach();

	public:

		Component();
		~Component();

		inline Vector3D GetPosition() const		{ return m_Position; }
		inline Angle3D GetRotation() const		{ return m_Rotation; }
		inline Vector3D GetScale() const		{ return m_Scale; }

		inline std::string GetName() const						{ return m_Name; }
		inline std::weak_ptr< Entity > GetOwner() const			{ return m_Owner; }
		inline std::weak_ptr< Component > GetParent() const		{ return m_Parent; }
		inline bool IsRootComponent() const						{ return m_Parent.expired(); }

		Vector3D GetWorldPosition() const;
		Angle3D GetWorldRotation() const;
		Vector3D GetWorldScale() const;

		virtual void OnAttach();
		virtual void OnDetach();

		/*
			Rendering Stuff
		*/
	public:

		

		friend class Entity;

	};

}