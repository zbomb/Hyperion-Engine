/*==================================================================================================
	Hyperion Engine
	Source/Framework/Component.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/Component.h"
#include "Hyperion/Framework/Entity.h"

namespace Hyperion
{

	Component::Component()
	{

	}

	Component::~Component()
	{

	}

	Vector3D Component::GetWorldPosition() const
	{
		// If there is a parent component, we will calculate relative to that, otherwise, use the parent entity
		auto parentComponent = m_Parent.lock();
		if( parentComponent && parentComponent->IsValid() )
		{
			return parentComponent->GetWorldPosition() + m_Position;
		}
		else
		{
			auto ownerEntity = m_Owner.lock();
			if( ownerEntity && ownerEntity->IsValid() )
			{
				return ownerEntity->GetWorldPosition() + m_Position;
			}
			else
			{
				std::cout << "[ERROR] Component: Attempt to get world position of component.. but there is no valid parent component or owning entity!\n";
				return m_Position;
			}
		}
	}

	Angle3D Component::GetWorldRotation() const
	{
		auto parentComponent = m_Parent.lock();
		if( parentComponent && parentComponent->IsValid() )
		{
			return parentComponent->GetWorldRotation() + m_Rotation;
		}
		else
		{
			auto ownerEntity = m_Owner.lock();
			if( ownerEntity && ownerEntity->IsValid() )
			{
				return ownerEntity->GetWorldRotation() + m_Rotation;
			}
			else
			{
				std::cout << "[ERROR] Component: Attempt to get world rotation of component.. but there is no valid parent component or owning entity!\n";
				return m_Rotation;
			}
		}
	}

	Vector3D Component::GetWorldScale() const
	{
		auto parentComponent = m_Parent.lock();
		if( parentComponent && parentComponent->IsValid() )
		{
			return parentComponent->GetWorldScale() + m_Scale;
		}
		else
		{
			auto ownerEntity = m_Owner.lock();
			if( ownerEntity && ownerEntity->IsValid() )
			{
				return ownerEntity->GetWorldScale() + m_Scale;
			}
			else
			{
				std::cout << "[ERROR] Component: Attempt to get world scale of component.. but there is no valid parent component or owning entity!\n";
				return m_Scale;
			}
		}
	}

	void Component::PerformAttach()
	{
		OnAttach();
	}

	void Component::PerformDetach()
	{
		// Detach children components
		for( auto It = m_Children.begin(); It != m_Children.end(); )
		{
			std::shared_ptr< Component > Target = It->second;
			if( Target && Target->IsValid() )
			{
				Target->PerformDetach();
				It = m_Children.erase( It );
			}
			else
			{
				It++;
			}
		}

		m_Children.clear();

		OnDetach();
	}

	void Component::OnAttach()
	{

	}

	void Component::OnDetach()
	{

	}


}