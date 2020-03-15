/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Library/Math.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
namespace Math
{
	
	uint32 Pow( uint32 base, uint32 exp );

	template< typename _Ty >
	_Ty Max( _Ty a, _Ty b )
	{
		return a > b ? a : b;
	}

	template< typename _Ty >
	_Ty Max( _Ty a, _Ty b, _Ty c )
	{
		return( a > b ? ( a > c ? a : c ) : ( b > c ? b : c ) );
	}

	template< typename _Ty >
	_Ty Min( _Ty a, _Ty b )
	{
		return a < b ? a : b;
	}

	template< typename _Ty >
	_Ty Min( _Ty a, _Ty b, _Ty c )
	{
		return( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
	}

	template< typename _Ty >
	_Ty Clamp( _Ty In, _Ty Min, _Ty Max )
	{
		return In < Min ? Min : ( In > Max ? Max : In );
	}

	static float PIf;
	static double PId;

}
}