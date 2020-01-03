/*==================================================================================================
	Hyperion Engine
	Tests/RenderTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Framework/World.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{

	class TestComponent : public Component
	{

	public:

		std::string debug_name;

		TestComponent( const std::string& inName )
			: debug_name( inName )
		{
			std::cout << "[TEST] " << debug_name << ": Constructor\n";
		}

		~TestComponent()
		{
			std::cout << "[TEST] " << debug_name << ": Destructor\n";
		}

		String GetDebugName() const override
		{
			return "test_component";
		}

	protected:

		void OnCreate() override
		{
			std::cout << "[TEST] " << debug_name << ": OnCreate\n";
		}

		void OnDestroy() override
		{
			std::cout << "[TEST] " << debug_name << ": OnDestroy\n";
		}

		void OnSpawn( const HypPtr< World >& inWorld ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnSpawn [World: " << inWorld->GetIdentifier() << "]\n";
		}

		void OnDespawn( const HypPtr< World >& oldWorld ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnDespawn [Old World: " << ( oldWorld ? oldWorld->GetIdentifier() : 0 ) << "]\n";
		}

		void OnAttach( const HypPtr< Entity >& inOwner, const HypPtr< Component >& inParent ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnAttach [Owner: " << ( inOwner ? inOwner->GetIdentifier() : 0 ) << "] [Parent: " << ( inParent ? inParent->GetIdentifier() : 0 ) << "]\n";
		}

		void OnDetach( const HypPtr< Entity >& oldOwner, const HypPtr< Component >& oldParent ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnDetach [Old Owner: " << ( oldOwner ? oldOwner->GetIdentifier() : 0 ) << "] [Old Parent: " << ( oldParent ? oldParent->GetIdentifier() : 0 ) << "]\n";
		}

		void OnChildAdded( const HypPtr< Component >& newChild ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnChildAdded [Child: " << newChild->GetIdentifier() << "]\n";
		}

		void OnChildRemoved( const HypPtr< Component >& oldChild ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnChildRemoved [Child: " << oldChild->GetIdentifier() << "]\n";
		}

		void OnLocalTransformChanged() override
		{
			std::cout << "[TEST] " << debug_name << ": OnLocalTransformChanged\n";

			auto local = GetTransform();
			std::cout << "\tPos: [" << local.Position.X << ", " << local.Position.Y << ", " << local.Position.Z << "]\tRot: [" << local.Rotation.Pitch << ", " << local.Rotation.Yaw << ", " << local.Rotation.Roll << "]\tScale: [" << local.Scale.X << ", " << local.Scale.Y << ", " << local.Scale.Z << "]\n";
		}

		void OnWorldTransformChanged() override
		{
			std::cout << "[TEST] " << debug_name << ": OnWorldTransformChanged\n";

			auto local = GetWorldTransform();
			std::cout << "\tPos: [" << local.Position.X << ", " << local.Position.Y << ", " << local.Position.Z << "]\tRot: [" << local.Rotation.Pitch << ", " << local.Rotation.Yaw << ", " << local.Rotation.Roll << "]\tScale: [" << local.Scale.X << ", " << local.Scale.Y << ", " << local.Scale.Z << "]\n";
		}

	};

	class TestEntity : public Entity
	{

	public:

		std::string debug_name;

		TestEntity( const std::string& inName )
			: debug_name( inName )
		{
			std::cout << "[TEST] " << debug_name << ": Constructor\n";
		}

		~TestEntity()
		{
			std::cout << "[TEST] " << debug_name << ": Destructor\n";
		}

		String GetDebugName() const override
		{
			return "test_entity";
		}

	protected:

		void OnCreate() override
		{
			std::cout << "[TEST] " << debug_name << ": OnCreate\n";
		}

		void OnDestroy() override
		{
			std::cout << "[TEST] " << debug_name << ": OnDestroy\n";
		}

		void OnSpawn( const HypPtr< World >& inNewWorld ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnSpawn [World Id: " << inNewWorld->GetIdentifier() << "]\n";
		}

		void OnDespawn( const HypPtr< World >& inOldWorld ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnDespawn [World Id: " << inOldWorld->GetIdentifier() << "]\n";
		}

		void OnChildAdded( const HypPtr< Entity >& inChild ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnChildAdded [Child Id: " << inChild->GetIdentifier() << "]\n";
		}

		void OnChildRemoved( const HypPtr< Entity >& inChild ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnChildRemoved [Child Id: " << inChild->GetIdentifier() << "]\n";
		}

		void OnParentChanged( const HypPtr< Entity >& newParent, const HypPtr< Entity >& oldParent ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnParentChanged [New Parent: " << ( newParent ? newParent->GetIdentifier() : 0 ) << "] [Old Parent: " << ( oldParent ? oldParent->GetIdentifier() : 0 ) << "]\n";
		}

		void OnComponentAdded( const HypPtr< Component >& newComponent ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnComponentAdded [Comp Id: " << newComponent->GetIdentifier() << "]\n";
		}

		void OnComponentRemoved( const HypPtr< Component >& oldComponent ) override
		{
			std::cout << "[TEST] " << debug_name << ": OnComponentRemoved [Comp Id: " << oldComponent->GetIdentifier() << "]\n";
		}

		void OnLocalTransformChanged() override
		{
			std::cout << "[TEST] " << debug_name << ": OnLocalTransformChanged\n";
			
			auto local = GetTransform();
			std::cout << "\tPos: [" << local.Position.X << ", " << local.Position.Y << ", " << local.Position.Z << "]\tRot: [" << local.Rotation.Pitch << ", " << local.Rotation.Yaw << ", " << local.Rotation.Roll << "]\tScale: [" << local.Scale.X << ", " << local.Scale.Y << ", " << local.Scale.Z << "]\n";
		}

		void OnWorldTransformChanged() override
		{
			std::cout << "[TEST] " << debug_name << ": OnWorldTransformChanged\n";

			auto local = GetWorldTransform();
			std::cout << "\tPos: [" << local.Position.X << ", " << local.Position.Y << ", " << local.Position.Z << "]\tRot: [" << local.Rotation.Pitch << ", " << local.Rotation.Yaw << ", " << local.Rotation.Roll << "]\tScale: [" << local.Scale.X << ", " << local.Scale.Y << ", " << local.Scale.Z << "]\n";
		}

		void BindUserInput( InputManager& inManager ) override
		{
			std::cout << "[TEST] " << debug_name << ": Binding User Input...\n";
		}

	};



namespace Tests
{

	void RunRendererTests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running rederer test...\n";


		std::cout << "\n----> Rederer Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}
}
}