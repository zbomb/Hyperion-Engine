/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Type.h
	© 2020, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <typeinfo>


/*
*	Register Object Type Macro
*	- NOTE: Must be place in .cpp file (preferably in the .cpp file used to define the type)
*	- NOTE: Also, the .cpp file this macro is used from, MUST contain a method definition for that class (even if its just a dummy function)
*/
#define HYPERION_REGISTER_OBJECT_TYPE( __class__, __parent__, ... ) \
Hyperion::RTTI::RegistryEntry __g__HyperionType__##__class__ = Hyperion::RTTI::RegistryEntry(\
	typeid( Hyperion::__class__ ).hash_code(),\
	#__class__,\
	typeid( Hyperion::__parent__ ).hash_code(),\
	[](){ return Hyperion::CreateObject< Hyperion::__class__ >( ##__VA_ARGS__ ); } )

#define HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( __class__, __parent__ ) \
Hyperion::RTTI::RegistryEntry __g_HyperionType__##__class__ = Hyperion::RTTI::RegistryEntry(\
	typeid( Hyperion::__class__ ).hash_code(),\
	#__class__,\
	typeid( Hyperion::__parent__ ).hash_code(),\
	nullptr )


#define HYPERION_OBJECT_TYPE_NAME "object"


namespace Hyperion
{
	template< typename _Ty >
	class HypPtr;

	class Object;


	/*
	*	Run Time Type Information Library
	*	- Tracks info about object-derived types
	*	- Backbone behind the 'Type' system
	*/
	class RTTI
	{

	public:

		/*
		*	TypeInfo Structure
		*	- Holds info about a specific object-derived tyoe
		*/
		struct TypeInfo
		{
			size_t Identifier;
			String Name;
			std::vector< size_t > ParentChain;
			std::vector< size_t > ChildrenList;
			std::function< HypPtr< Object >() > CreateFunc;

			TypeInfo()
				: Identifier( 0 ), Name(), ParentChain(), CreateFunc( nullptr ), ChildrenList()
			{}

			TypeInfo( size_t inId, const String& inName, std::function< HypPtr< Object >() > inCreateFunc, const std::vector< size_t >& inChain )
				: Identifier( inId ), Name( String::ToLower( inName ) ), ParentChain( inChain ), CreateFunc( inCreateFunc ), ChildrenList()
			{}

			TypeInfo( size_t inId, std::function< HypPtr< Object >() > inCreateFunc, const String& inName )
				: Identifier( inId ), Name( String::ToLower( inName ) ), CreateFunc( inCreateFunc ), ChildrenList()
			{}
		};

		/*
		*	RegistryEntry Class
		*	- Used to create entries in the RTTI type registry
		*	- Do not use directly.. there will be a macro to make things easier
		*/
		class RegistryEntry
		{

		public:

			RegistryEntry() = delete;
			RegistryEntry( const RegistryEntry& ) = delete;

			RegistryEntry( size_t inIdentifier, const String& inName, size_t inParent, std::function< HypPtr< Object >() > inCreateFunc );

		};


	private:

		//static std::map< size_t, std::shared_ptr< TypeInfo > > m_TypeInfoList;
		//static std::shared_ptr< TypeInfo > m_ObjectTypeInfo;

	public:

		static void Initialize();

		/*
		*	bool [result] Hyperion::RTTI::TypeExists( size_t [Identifier of the type to lookup, found using std::type_info] )
		*	- Checks if a type is registered with the RTTI system
		*	- Performs lookup based on the identifier assigned to the class type
		*	- Faster way to lookup if a type exists (looking up by name is slower)
		*/
		static bool TypeExists( size_t inIdentifier );

		/*
		*	bool [result] Hyperion::RTTI::TypeExists( const String& [name of the type to lookup, found using std::type_info] )
		*	- Checks if a type is registered with the RTTI system
		*	- Performs lookup based on the identifier assigned to the class type
		*	- Slower way to lookup if a type exists (looking up by identifier is faster)
		*/
		static bool TypeExists( const String& inName );

		/*
		*	bool [result] Hyperion::RTTI::TypeExists< _Ty [Type to lookup] >()
		*	- Checks if a type is registered with the RTTI system
		*	- Performs lookup based on a template type parameter
		*/
		template< typename _Ty,
					typename = typename std::enable_if< std::is_base_of< Hyperion::Object, _Ty >::value >::type >
		static bool TypeExists()
		{
			return TypeExists( typeid( _Ty ).hash_code() );
		}

		/*
		*	shared_ptr< TypeInfo > [Result] Hyperion::RTTI::GetTypeInfo( size_t [Identifier of the type to lookup, found using std::type_info] )
		*	- Gets the info entry about a specified type
		*	- Performs lookup based on the identifier assigned to the class type
		*	- Fastest way to find type info (looking up by name is slower!)
		*/
		static std::shared_ptr< TypeInfo > GetTypeInfo( size_t inIdentifier );

		/*
		*	shared_ptr< TypeInfo > [Result] Hyperion::RTTI::GetTypeInfo< _Ty [Type to lookup] >()
		*	- Gets the info entry about a specified type
		*	- Performs lookup based on a template type parameter
		*/
		template< typename _Ty >
		static std::shared_ptr< TypeInfo > GetTypeInfo()
		{
			return GetTypeInfo( typeid( _Ty ).hash_code() );
		}
		
		/*
		*	shared_ptr< TypeInfo > [Result] Hyperion::RTTI::GetTypeInfo( const String& [name of the type to lookup] )
		*	- Gets the info entry about a specified type
		*	- Performs lookup based on the identifier assigned to the class type
		*	- Slower way to find type info (looking up by identifier is faster!)
		*/
		static std::shared_ptr< TypeInfo > GetTypeInfo( const String& inName );

		/*
		*	std::shared_ptr< TypeInfo > Hyperion::RTTI::GetObjectType()
		*/
		static std::shared_ptr< TypeInfo > GetObjectType();

	private:

		/*
		*	bool RegisterType( const TypeInfo& [info describing the type thats being registered] )
		*	- Internal function used by 'RegistryEntry' to register new types
		*/
		static bool RegisterType( size_t inIdentifier, const String& inName, std::function< HypPtr< Object >() > inCreateFunc, size_t inInherit );


		friend class RegistryEntry;
	};

}
