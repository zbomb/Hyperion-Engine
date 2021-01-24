/*==================================================================================================
	Hyperion Engine
	Source/Library/Geometry.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Framework/ViewState.h"
#include "Hyperion/Library/Math.h"


namespace Hyperion
{

	/*----------------------------------------------------------------------------------------
		Angle3D Function Definitions
	----------------------------------------------------------------------------------------*/
	Angle3D::Angle3D( float inPitch, float inYaw, float inRoll )
		: Pitch( inPitch ), Yaw( inYaw ), Roll( inRoll )
	{
	}

	Angle3D::Angle3D( const Angle3D& inOther )
		: Pitch( inOther.Pitch ), Yaw( inOther.Yaw ), Roll( inOther.Roll )
	{
	}

	Angle3D::Angle3D( Angle3D&& inOther ) noexcept
		: Pitch( std::move( inOther.Pitch ) ), Yaw( std::move( inOther.Yaw ) ), Roll( std::move( inOther.Roll ) )
	{
	}

	Angle3D Angle3D::operator+( const Angle3D& inOther ) const
	{
		return Angle3D(
			Pitch + inOther.Pitch,
			Yaw + inOther.Yaw,
			Roll + inOther.Roll
		);
	}

	Angle3D Angle3D::operator-( const Angle3D& inOther ) const
	{
		return Angle3D(
			Pitch - inOther.Pitch,
			Yaw - inOther.Yaw,
			Roll - inOther.Roll
		);
	}

	/*
		Assignment Operators
	*/
	Angle3D& Angle3D::operator=( const Angle3D& Other )
	{
		Pitch = Other.Pitch;
		Yaw = Other.Yaw;
		Roll = Other.Roll;

		return *this;
	}

	Angle3D& Angle3D::operator=( Angle3D&& Other ) noexcept
	{
		Pitch = std::move( Other.Pitch );
		Yaw = std::move( Other.Yaw );
		Roll = std::move( Other.Roll );

		return *this;
	}

	/*
		Comparison Operators
	*/
	bool Angle3D::operator==( const Angle3D& Other ) const
	{
		return( Pitch == Other.Pitch &&
				Yaw == Other.Yaw &&
				Roll == Other.Roll );
	}

	bool Angle3D::operator!=( const Angle3D& Other ) const
	{
		return( Pitch != Other.Pitch ||
				Yaw != Other.Yaw ||
				Roll != Other.Roll );
	}

	/*
		Member Functions
	*/
	void Angle3D::Clear()
	{
		Pitch = 0.f;
		Yaw = 0.f;
		Roll = 0.f;
	}


	void Angle3D::ClampContents()
	{
		// We want to get each component into the range of 0 to 360, where 0 is inclusive
		while( Pitch >= 360.f ) { Pitch -= 360.f; }
		while( Yaw >= 360.f ) { Yaw -= 360.f; }
		while( Roll >= 360.f ) { Roll -= 360.f; }
		while( Pitch < 0.f ) { Pitch += 360.f; }
		while( Yaw < 0.f ) { Yaw += 360.f; }
		while( Roll < 0.f ) { Roll += 360.f; }

		if( Pitch > 359.998f || Pitch < 0.0001f ) { Pitch = 0.f; }
		if( Yaw > 359.998f || Yaw < 0.0001f ) { Yaw = 0.f; }
		if( Roll > 359.998f || Roll < 0.0001f ) { Roll = 0.f; }
	}


	Vector3D Angle3D::GetDirectionVector() const
	{
		return Vector3D(
			sinf( HYPERION_DEG_TO_RAD( Yaw ) ) * cosf( HYPERION_DEG_TO_RAD( Pitch ) ),
			-sinf( HYPERION_DEG_TO_RAD( Pitch ) ),
			cosf( HYPERION_DEG_TO_RAD( Yaw ) ) * cosf( HYPERION_DEG_TO_RAD( Pitch ) )
		);
	}

	Quaternion Angle3D::ToQuaternion() const
	{
		return Quaternion( *this );
	}


	String Angle3D::ToString() const
	{
		return String( "P:" ).Append( std::to_string( Pitch ) ).Append( " Y:" ).Append( std::to_string( Yaw ) ).Append( " R:" ).Append( std::to_string( Roll ) );
	}


	/*----------------------------------------------------------------------------------------
		Vector2D Function Definitions
	----------------------------------------------------------------------------------------*/
	Vector2D::Vector2D( float inX, float inY )
		: X( inX ), Y( inY )
	{
	}

	Vector2D::Vector2D( const Vector2D& inOther )
		: X( inOther.X ), Y( inOther.Y )
	{
	}

	Vector2D::Vector2D( Vector2D&& inOther ) noexcept
		: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) )
	{
	}

	/*
		Assignment
	*/
	Vector2D& Vector2D::operator=( const Vector2D& Other )
	{
		X = Other.X;
		Y = Other.Y;

		return *this;
	}

	Vector2D& Vector2D::operator=( Vector2D&& Other ) noexcept
	{
		X = std::move( Other.X );
		Y = std::move( Other.Y );

		return *this;
	}

	/*
		Comparison Operators
	*/
	bool Vector2D::operator==( const Vector2D& inOther ) const
	{
		return( X == inOther.X && Y == inOther.Y );
	}

	bool Vector2D::operator!=( const Vector2D& inOther ) const
	{
		return( X != inOther.X || Y != inOther.Y );
	}

	/*
		Static Functions
	*/
	float Vector2D::Distance( const Vector2D& First, const Vector2D& Second )
	{
		float sqX = powf( Second.X - First.X, 2.f );
		float sqY = powf( Second.Y - First.Y, 2.f );

		return sqrtf( sqX + sqY );
	}

	/*
		Member Functions
	*/
	void Vector2D::Clear()
	{
		X = 0.f;
		Y = 0.f;
	}

	float Vector2D::Length() const
	{
		return sqrtf( powf( X, 2.f ) + powf( Y, 2.f ) );
	}

	Vector2D Vector2D::GetNormalized() const
	{
		auto length = Length();
		return Vector2D( X / length, Y / length );
	}

	bool Vector2D::IsNormalized() const
	{
		return Length() == 1.f;
	}

	String Vector2D::ToString() const
	{
		return String( "X: " ).Append( std::to_string( X ) ).Append( " Y:" ).Append( std::to_string( Y ) );
	}

	float Vector2D::Dot( const Vector2D& other ) const
	{
		return( ( X * other.X ) + ( Y * other.Y ) );
	}

	Vector2D Vector2D::GetNegated() const
	{
		return Vector2D( -X, -Y );
	}

	Vector2D Vector2D::operator*( float inScalar ) const
	{
		return Vector2D( X * inScalar, Y * inScalar );
	}

	Vector4D Vector2D::operator+( const Vector4D& other ) const
	{
		return Vector4D( X + other.X, Y + other.Y, other.Z, other.W );
	}

	Vector3D Vector2D::operator+( const Vector3D& other ) const
	{
		return Vector3D( X + other.X, Y + other.Y, other.Z );
	}

	Vector2D Vector2D::operator+( const Vector2D& Other ) const
	{
		return Vector2D(
			X + Other.X,
			Y + Other.Y
		);
	}

	Vector2D Vector2D::operator-( const Vector2D& Other ) const
	{
		return Vector2D(
			X - Other.X,
			Y - Other.Y
		);
	}

	Vector2D Vector2D::operator/( float inScale ) const
	{
		return Vector2D(
			X / inScale,
			Y / inScale
		);
	}


	/*----------------------------------------------------------------------------------------
		Vector3D Function Definitions
	----------------------------------------------------------------------------------------*/
	Vector3D::Vector3D( float inX, float inY, float inZ )
		: X( inX ), Y( inY ), Z( inZ )
	{
	}

	Vector3D::Vector3D( const Vector3D& inOther )
		: X( inOther.X ), Y( inOther.Y ), Z( inOther.Z )
	{
	}

	Vector3D::Vector3D( Vector3D&& inOther ) noexcept
		: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) ), Z( std::move( inOther.Z ) )
	{
	}

	/*
		Assignment
	*/
	Vector3D& Vector3D::operator=( const Vector3D& Other )
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;

		return *this;
	}

	Vector3D& Vector3D::operator=( Vector3D&& Other ) noexcept
	{
		X = std::move( Other.X );
		Y = std::move( Other.Y );
		Z = std::move( Other.Z );

		return *this;
	}

	/*
		Comparison Operators
	*/
	bool Vector3D::operator==( const Vector3D& inOther ) const
	{
		return( X == inOther.X &&
				Y == inOther.Y &&
				Z == inOther.Z );
	}

	bool Vector3D::operator!=( const Vector3D& inOther ) const
	{
		return( X != inOther.X ||
				Y != inOther.Y ||
				Z != inOther.Z );
	}

	/*
		Static Functions
	*/
	float Vector3D::Distance( const Vector3D& First, const Vector3D& Second )
	{
		float sqX = powf( Second.X - First.X, 2.f );
		float sqY = powf( Second.Y - First.Y, 2.f );
		float sqZ = powf( Second.Z - First.Z, 2.f );

		return sqrtf( sqX + sqY + sqZ );
	}

	Vector3D Vector3D::GetWorldUp()
	{
		return Vector3D( 0, 1.f, 0.f );
	}

	Vector3D Vector3D::GetWorldRight()
	{
		return Vector3D( 1.f, 0.f, 0.f );
	}

	Vector3D Vector3D::GetWorldForward()
	{
		return Vector3D( 0.f, 0.f, 1.f );
	}

	/*
		Member Function
	*/
	void Vector3D::Clear()
	{
		X = 0.f;
		Y = 0.f;
		Z = 0.f;
	}

	float Vector3D::Length() const
	{
		return sqrtf( powf( X, 2.f ) + powf( Y, 2.f ) + powf( Z, 2.f ) );
	}

	Vector3D Vector3D::GetNormalized() const
	{
		auto length = Length();
		return Vector3D( X / length, Y / length, Z / length );
	}

	bool Vector3D::IsNormalized() const
	{
		return Length() == 1.f;
	}

	Angle3D Vector3D::GetEulerAngles() const
	{
		auto norm = GetNormalized();
		return Angle3D(
			asinf( -norm.Y ),
			atanf( norm.X / norm.Z ),
			0.f
		);
	}

	String Vector3D::ToString() const
	{
		return String( "X:" ).Append( std::to_string( X ) ).Append( " Y:" ).Append( std::to_string( Y ) ).Append( " Z:" ).Append( std::to_string( Z ) );
	}

	float Vector3D::Dot( const Vector3D& other ) const
	{
		return(
			( X * other.X ) + ( Y * other.Y ) + ( Z * other.Z )
		);
	}

	Vector3D Vector3D::Cross( const Vector3D& other ) const
	{
		return Vector3D(
			( Y * other.Z ) - ( Z * other.Y ),
			( Z * other.X ) - ( X * other.Z ),
			( X * other.Y ) - ( Y * other.X )
		);
	}

	Vector3D Vector3D::GetNegated() const
	{
		return Vector3D( -X, -Y, -Z );
	}

	Vector3D Vector3D::operator*( const Matrix& other ) const
	{
		/*
		*	Although this operation isnt technically 'valid', were just going to ignore the W component, and treat it as if it was zero
		*	
		*	Matrix Layout:
		*	0	1	2	3
		*	4	5	6	7
		*	8	9	10	11
		*	12	13	14	15
		*/

		return Vector3D(
			( X * other.data[ 0 ] ) + ( Y * other.data[ 4 ] ) + ( Z * other.data[ 8 ] ),
			( X * other.data[ 1 ] ) + ( Y * other.data[ 5 ] ) + ( Z * other.data[ 9 ] ),
			( X * other.data[ 2 ] ) + ( Y * other.data[ 6 ] ) + ( Z * other.data[ 10 ] )
		);
	}

	Vector3D Vector3D::operator*( float inScalar ) const
	{
		return Vector3D( X * inScalar, Y * inScalar, Z * inScalar );
	}

	Vector3D Vector3D::operator+( const Vector3D& Other ) const
	{
		return Vector3D(
			X + Other.X,
			Y + Other.Y,
			Z + Other.Z
		);
	}

	Vector3D Vector3D::operator-( const Vector3D& Other ) const
	{
		return Vector3D(
			X - Other.X,
			Y - Other.Y,
			Z - Other.Z
		);
	}

	Vector3D Vector3D::operator/( float inScale ) const
	{
		return Vector3D(
			X / inScale,
			Y / inScale,
			Z / inScale
		);
	}

	Vector4D Vector3D::operator+( const Vector4D& other ) const
	{
		return Vector4D( X + other.X, Y + other.Y, Z + other.Y, other.W );
	}

	Vector3D Vector3D::operator+( const Vector2D& other ) const
	{
		return Vector3D( X + other.X, Y + other.Y, Z );
	}

	Vector3D Vector3D::operator-( const Vector2D& other ) const
	{
		return Vector3D( X - other.X, Y - other.Y, Z );
	}


	/*----------------------------------------------------------------------------------------
		Vector4D Function Definitions
	----------------------------------------------------------------------------------------*/
	Vector4D::Vector4D( float inX, float inY, float inZ, float inW )
		: X( inX ), Y( inY ), Z( inZ ), W( inW )
	{
	}

	Vector4D::Vector4D( const Vector4D& inOther )
		: X( inOther.X ), Y( inOther.Y ), Z( inOther.Z ), W( inOther.W )
	{
	}

	Vector4D::Vector4D( Vector4D&& inOther ) noexcept
		: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) ), Z( std::move( inOther.Z ) ), W( std::move( inOther.W ) )
	{
	}

	/*
		Assignment
	*/
	Vector4D& Vector4D::operator=( Vector4D&& Other ) noexcept
	{
		X = std::move( Other.X );
		Y = std::move( Other.Y );
		Z = std::move( Other.Z );
		W = std::move( Other.W );

		return *this;
	}

	Vector4D& Vector4D::operator=( const Vector4D& Other )
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;
		W = Other.W;

		return *this;
	}

	Vector4D& Vector4D::operator=( const Vector3D& other )
	{
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		W = 0.f;

		return *this;
	}

	Vector4D& Vector4D::operator=( const Vector2D& other )
	{
		X = other.X;
		Y = other.Y;
		Z = 0.f;
		W = 0.f;

		return *this;
	}



	/*
		Comparison Operators
	*/
	bool Vector4D::operator==( const Vector4D& Other ) const
	{
		return( X == Other.X &&
				Y == Other.Y &&
				Z == Other.Z &&
				W == Other.W );
	}

	bool Vector4D::operator!=( const Vector4D& Other ) const
	{
		return( X != Other.X ||
				Y != Other.Y ||
				Z != Other.Z ||
				W != Other.W );
	}

	bool Vector4D::operator==( const Vector3D& other ) const
	{
		return(
			X == other.X &&
			Y == other.Y &&
			Z == other.Z );
	}

	bool Vector4D::operator!=( const Vector3D& other ) const
	{
		return(
			X != other.X ||
			Y != other.Y ||
			Z != other.Z );
	}

	bool Vector4D::operator==( const Vector2D& other ) const
	{
		return( X == other.X && Y == other.Y );
	}

	bool Vector4D::operator!=( const Vector2D& other ) const
	{
		return( X != other.X || Y != other.Y );
	}

	/*
		Static Functions
	*/
	float Vector4D::Distance( const Vector4D& First, const Vector4D& Second )
	{
		float sqX = powf( Second.X - First.X, 2.f );
		float sqY = powf( Second.Y - First.Y, 2.f );
		float sqZ = powf( Second.Z - First.Z, 2.f );
		float sqW = powf( Second.W - First.W, 2.f );

		return sqrtf( sqX + sqY + sqZ + sqW );
	}

	/*
		Member Functions
	*/
	void Vector4D::Clear()
	{
		X = 0.f;
		Y = 0.f;
		Z = 0.f;
		W = 0.f;
	}

	float Vector4D::Distance( const Vector4D& Other ) const
	{
		return Vector4D::Distance( *this, Other );
	}

	float Vector4D::Length() const
	{
		return sqrtf( powf( X, 2.f ) + powf( Y, 2.f ) + powf( Z, 2.f ) + powf( W, 2.f ) );
	}

	Vector4D Vector4D::GetNormalized() const
	{
		auto length = Length();
		return Vector4D( X / length, Y / length, Z / length, W / length );
	}

	bool Vector4D::IsNormalized() const
	{
		return Length() == 1.f;
	}

	String Vector4D::ToString() const
	{
		return String( "X:" ).Append( std::to_string( X ) ).Append( " Y:" ).Append( std::to_string( Y ) ).Append( " Z:" ).Append( std::to_string( Z ) ).Append( " W:" ).Append( std::to_string( W ) );
	}

	float Vector4D::Dot( const Vector4D& other ) const
	{
		return(
			( X * other.X ) + ( Y * other.Y ) + ( Z * other.Z ) + ( W * other.W )
		);
	}

	Vector3D Vector4D::GetHomogenous3D() const
	{
		if( W == 0.f ) { return Vector3D( 0.f, 0.f, 0.f ); }

		return Vector3D(
			X / W,
			Y / W,
			Z / W
		);
	}

	Vector4D Vector4D::GetHomogenous4D() const
	{
		if( W == 0.f ) { return Vector4D( 0.f, 0.f, 0.f, 1.f ); }

		return Vector4D(
			X / W,
			Y / W,
			Z / W,
			1.f
		);
	}

	bool Vector4D::IsHomogenous() const
	{
		return( W == 1.f );
	}

	Vector4D Vector4D::GetNegated() const
	{
		return Vector4D( -X, -Y, -Z, -W );
	}

	Vector4D Vector4D::operator*( const Matrix& other ) const
	{
		/*
		*	Matrix Layout:
		*	0	1	2	3
		*	4	5	6	7
		*	8	9	10	11
		*	12	13	14	15
		* 
		*	We have to calculate the dot product of each column of the matrix againt the input vector
		*/

		return Vector4D(
			( X * other.data[ 0 ] ) + ( Y * other.data[ 4 ] ) + ( Z * other.data[ 8 ] ) + ( W * other.data[ 12 ] ),
			( X * other.data[ 1 ] ) + ( Y * other.data[ 5 ] ) + ( Z * other.data[ 9 ] ) + ( W * other.data[ 13 ] ),
			( X * other.data[ 2 ] ) + ( Y * other.data[ 6 ] ) + ( Z * other.data[ 10 ] ) + ( W * other.data[ 14 ] ),
			( X * other.data[ 3 ] ) + ( Y * other.data[ 7 ] ) + ( Z * other.data[ 11 ] ) + ( W * other.data[ 15 ] )
		);
	}

	Vector4D Vector4D::operator*( float inScalar ) const
	{
		return Vector4D( X * inScalar, Y * inScalar, Z * inScalar, W * inScalar );
	}

	Vector4D Vector4D::operator/( float inScale ) const
	{
		return Vector4D( X / inScale, Y / inScale, Z / inScale, W / inScale );
	}

	Vector4D Vector4D::operator+( const Vector4D& other ) const
	{
		return Vector4D( X + other.X, Y + other.Y, Z + other.Z, W + other.W );
	}

	Vector4D Vector4D::operator-( const Vector4D& other ) const
	{
		return Vector4D( X - other.X, Y - other.Y, Z - other.Z, W - other.W );
	}

	Vector4D Vector4D::operator+( const Vector3D& other ) const
	{
		return Vector4D( X + other.X, Y + other.Y, Z + other.Z, W );
	}

	Vector4D Vector4D::operator-( const Vector3D& other ) const
	{
		return Vector4D( X - other.X, Y - other.Y, Z - other.Z, W );
	}

	Vector4D Vector4D::operator+( const Vector2D& other ) const
	{
		return Vector4D( X + other.X, Y + other.Y, Z, W );
	}

	Vector4D Vector4D::operator-( const Vector2D& other ) const
	{
		return Vector4D( X - other.X, Y - other.Y, Z, W );
	}

	/*----------------------------------------------------------------------------------------
		Quaternion Function Definitions
	----------------------------------------------------------------------------------------*/

	Quaternion::Quaternion()
		: W( 1.f ), X( 0.f ), Y( 0.f ), Z( 0.f )
	{}

	Quaternion::Quaternion( float inW, float inX, float inY, float inZ )
		: W( inW ), X( inX ), Y( inY ), Z( inZ )
	{}

	Quaternion::Quaternion( const Vector3D& inAxis, float inRotationDegrees )
	{
		// First, we need to clamp the roataion and convert it to radians
		while( inRotationDegrees >= 360.f ) { inRotationDegrees -= 360.f; }
		while( inRotationDegrees < 0.f ) { inRotationDegrees += 360.f; }

		auto half_rot		= HYPERION_DEG_TO_RAD( inRotationDegrees ) / 2.f;
		auto sin_half_rot	= sinf( -half_rot );
		auto norm_axis		= inAxis.GetNormalized();

		W = cosf( half_rot );
		X = norm_axis.X * sin_half_rot;
		Y = norm_axis.Y * sin_half_rot;
		Z = norm_axis.Z * sin_half_rot;
	}

	// TODO: All matrix stuff needs review
	Quaternion::Quaternion( const Matrix& inMatrix )
	{
		float abs_w = sqrtf( ( 1.f + inMatrix[ 0 ][ 0 ] + inMatrix[ 1 ][ 1 ] + inMatrix[ 2 ][ 2 ] ) / 4.f );
		float abs_x = sqrtf( ( 1.f + inMatrix[ 0 ][ 0 ] - inMatrix[ 1 ][ 1 ] - inMatrix[ 2 ][ 2 ] ) / 4.f );
		float abs_y = sqrtf( ( 1.f - inMatrix[ 0 ][ 0 ] + inMatrix[ 1 ][ 1 ] - inMatrix[ 2 ][ 2 ] ) / 4.f );
		float abs_z = sqrtf( ( 1.f - inMatrix[ 0 ][ 0 ] - inMatrix[ 1 ][ 1 ] + inMatrix[ 2 ][ 2 ] ) / 4.f );

		// Find largest component, to increase accuracy
		if( abs_w > abs_x && abs_w > abs_y && abs_w > abs_z )
		{
			float denom = 4.f * abs_w;
			W = abs_w;
			X = ( ( inMatrix[ 2 ][ 1 ] - inMatrix[ 1 ][ 2 ] ) / denom );
			Y = ( ( inMatrix[ 0 ][ 2 ] - inMatrix[ 2 ][ 0 ] ) / denom );
			Z = ( ( inMatrix[ 1 ][ 0 ] - inMatrix[ 0 ][ 1 ] ) / denom );
		}
		else if( abs_x > abs_w && abs_x > abs_y && abs_x > abs_z )
		{
			float denom = 4.f * abs_x;
			W = ( ( inMatrix[ 2 ][ 1 ] - inMatrix[ 1 ][ 2 ] ) / denom );
			X = abs_x;
			Y = ( ( inMatrix[ 0 ][ 1 ] + inMatrix[ 1 ][ 0 ] ) / denom );
			Z = ( ( inMatrix[ 0 ][ 2 ] + inMatrix[ 2 ][ 0 ] ) / denom );
		}
		else if( abs_y > abs_w && abs_y > abs_x && abs_x > abs_z )
		{
			float denom = 4.f * abs_y;
			W = ( ( inMatrix[ 0 ][ 2 ] - inMatrix[ 2 ][ 0 ] ) / denom );
			X = ( ( inMatrix[ 0 ][ 1 ] + inMatrix[ 1 ][ 0 ] ) / denom );
			Y = abs_y;
			Z = ( ( inMatrix[ 1 ][ 2 ] + inMatrix[ 2 ][ 1 ] ) / denom );
		}
		else
		{
			float denom = 4.f * abs_z;
			W = ( ( inMatrix[ 1 ][ 0 ] - inMatrix[ 0 ][ 1 ] ) / denom );
			X = ( ( inMatrix[ 0 ][ 2 ] + inMatrix[ 2 ][ 0 ] ) / denom );
			Y = ( ( inMatrix[ 1 ][ 2 ] + inMatrix[ 2 ][ 1 ] ) / denom );
			Z = abs_z;
		}
	}

	/*
	*	NOTES:
	*	
	*/

	Quaternion::Quaternion( const Angle3D& inEuler )
	{
		Angle3D clamped = inEuler;
		clamped.ClampContents();

		// We have to negate the angles, since were using a left-hand system currently
		// Eventually, I would like to move to a right hand system
		float cx = cos( HYPERION_DEG_TO_RAD( -inEuler.Pitch ) / 2.f );
		float cy = cos( HYPERION_DEG_TO_RAD( -inEuler.Yaw ) / 2.f );
		float cz = cos( HYPERION_DEG_TO_RAD( -inEuler.Roll ) / 2.f );
		float sx = sin( HYPERION_DEG_TO_RAD( -inEuler.Pitch ) / 2.f );
		float sy = sin( HYPERION_DEG_TO_RAD( -inEuler.Yaw ) / 2.f );
		float sz = sin( HYPERION_DEG_TO_RAD( -inEuler.Roll ) / 2.f );

		X = ( sx * cy * cz - cx * sy * sz );
		Y = ( cx * sy * cz + sx * cy * sz );
		Z = ( cx * cy * sz + sx * sy * cz );
		W = ( cx * cy * cz - sx * sy * sz );
	}

	Quaternion::Quaternion( const Quaternion& other )
		: W( other.W ), X( other.X ), Y( other.Y ), Z( other.Z )
	{}

	Quaternion::Quaternion( Quaternion&& other ) noexcept
		: W( std::move( other.W ) ), X( std::move( other.X ) ), Y( std::move( other.Y ) ), Z( std::move( other.Z ) )
	{}

	Quaternion& Quaternion::operator=( const Quaternion& other )
	{
		W = other.W;
		X = other.X;
		Y = other.Y;
		Z = other.Z;

		return *this;
	}

	Quaternion& Quaternion::operator=( Quaternion&& other ) noexcept
	{
		W = std::move( other.W );
		X = std::move( other.X );
		Y = std::move( other.Y );
		Z = std::move( other.Z );

		return *this;
	}

	Quaternion Quaternion::operator-() const
	{
		return Quaternion(
			W, -X, -Y, -Z
		);
	}

	bool Quaternion::operator==( const Quaternion& other ) const
	{
		return( X == other.X && Y == other.Y && Z == other.Z && W == other.W );
	}

	bool Quaternion::operator!=( const Quaternion& other ) const
	{
		return( X != other.X || Y != other.Y || Z != other.Z || W != other.W );
	}

	bool Quaternion::IsNormalized() const
	{
		return(
			( ( W * W ) + ( X * X ) + ( Y * Y ) + ( Z * Z ) ) == 1.f
		);
	}

	float Quaternion::GetLength() const
	{
		return sqrtf(
			( W * W ) + ( X * X ) + ( Y * Y ) + ( Z * Z )
		);
	}

	Quaternion Quaternion::GetNormalized() const
	{
		float length = GetLength();
		return Quaternion(
			W / length,
			X / length,
			Y / length,
			Z / length
		);
	}

	Vector3D Quaternion::GetRotationAxis() const
	{
		float rot_sin_half = sinf( -acosf( W ) ); // theta = 2.f * acosf( W )
		if( rot_sin_half == 0.f ) {
			return Vector3D::GetWorldUp(); // TODO: Should we return unit vector? Or Vec3( 0, 0, 0 )?
		}

		float x_res = X / rot_sin_half;
		float y_res = Y / rot_sin_half;
		float z_res = Z / rot_sin_half;

		if( abs( x_res ) < 0.0001f ) { x_res = 0.f; }
		if( abs( y_res ) < 0.0001f ) { y_res = 0.f; }
		if( abs( z_res ) < 0.0001f ) { z_res = 0.f; }

		return Vector3D( x_res, y_res, z_res );
	}

	float Quaternion::GetRotationAmount() const
	{
		float ret = HYPERION_RAD_TO_DEG( 2.f * acosf( W ) );
		if( ret > 359.998f ) { ret = 0.f; }

		return ret;
	}

	Matrix Quaternion::GetRotationMatrix() const
	{
		float xx = X * ( X + X );
		float xy = X * ( Y + Y );
		float xz = X * ( Z + Z );
		float yy = Y * ( Y + Y );
		float yz = Y * ( Z + Z );
		float zz = Z * ( Z + Z );
		float wx = W * ( X + X );
		float wy = W * ( Y + Y );
		float wz = W * ( Z + Z );

		return Matrix(
			1.f - ( yy + zz ),	xy - wz,			xz + wy,			0.f,
			xy + wz,			1.f - ( xx + zz ),	yz - wx,			0.f,
			xz - wy,			yz + wx,			1.f - ( xx + yy ),	0.f,
			0.f,				0.f,				0.f,				1.f
		);
	}

	Angle3D Quaternion::GetEulerAngles() const
	{
		// We want to apply rotations in the order of:
		// Roll [Z] -> Pitch [X] -> Yaw [Y]
		// ZXY

		float m11 = 1.f - ( ( 2.f * Y * Y ) + ( 2.f * Z * Z ) );
		float m12 = ( 2.f * X * Y ) - ( 2.f * W * Z );
		float m21 = ( 2.f * X * Y ) + ( 2.f * W * Z );
		float m22 = 1.f - ( ( 2.f * X * X ) + ( 2.f * Z * Z ) );
		float m31 = ( 2.f * X * Z ) - ( 2.f * W * Y );
		float m32 = ( 2.f * Y * Z ) + ( 2.f * W * X );
		float m33 = 1.f - ( ( 2.f * X * X ) + ( 2.f * Y * Y ) );

		float pitch = asinf( Math::Clamp( m32, -1.f, 1.f ) );
		float yaw = 0.0;
		float roll = 0.0;

		if( abs( m32 ) < 0.9999f )
		{
			yaw = atan2f( -m31, m33 );
			roll = atan2f( -m12, m22 );
		}
		else
		{
			roll = atan2f( m21, m11 );
		}

		auto output = Angle3D( HYPERION_RAD_TO_DEG( -pitch ), HYPERION_RAD_TO_DEG( -yaw ), HYPERION_RAD_TO_DEG( -roll ) );
		output.ClampContents();

		return output;
	
	}

	Quaternion Quaternion::operator*( const Quaternion& other ) const
	{
		return Quaternion(
			( W * other.W ) - ( X * other.X ) - ( Y * other.Y ) - ( Z * other.Z ),
			( W * other.X ) + ( X * other.W ) + ( Y * other.Z ) - ( Z * other.Y ),
			( W * other.Y ) - ( X * other.Z ) + ( Y * other.W ) + ( Z * other.X ),
			( W * other.Z ) + ( X * other.Y ) - ( Y * other.X ) + ( Z * other.W )
		);
	}

	Quaternion Quaternion::operator*( const Angle3D& inEuler ) const
	{
		return this->operator*( Quaternion( inEuler ) );
	}

	Vector3D Quaternion::RotateVector( const Vector3D& inVector ) const
	{
		// First, we need to construct a quaternion from the point, where W = 0, and XYZ = XYZ
		auto inv_quat = -( *this );
		auto intern_quat = inv_quat * Quaternion( 0.f, inVector.X, inVector.Y, inVector.Z );
		auto quat_result = intern_quat * ( *this );
		return Vector3D( quat_result.X, quat_result.Y, quat_result.Z );
	}

	void Quaternion::SetFromEuler( const Angle3D& inEuler )
	{
		Angle3D clamped = inEuler;
		clamped.ClampContents();

		float rh = HYPERION_DEG_TO_RAD( clamped.Roll ) / 2.f;
		float yh = HYPERION_DEG_TO_RAD( clamped.Yaw ) / 2.f;
		float ph = HYPERION_DEG_TO_RAD( clamped.Pitch ) / 2.f;

		W = cosf( rh ) * cosf( ph ) * cosf( yh ) - sinf( -rh ) * sinf( -ph ) * sinf( -yh );
		X = cosf( rh ) * sinf( -ph ) * cosf( yh ) - sinf( -rh ) * cosf( ph ) * sinf( -yh );
		Y = cosf( rh ) * cosf( ph ) * sinf( -yh ) + sinf( -rh ) * sinf( -ph ) * cosf( yh );
		Z = cosf( rh ) * sinf( -ph ) * sinf( -yh ) + sinf( -rh ) * cosf( ph ) * cosf( yh );
	}

	void Quaternion::Clear()
	{
		W = 1.f;
		X = 0.f;
		Y = 0.f;
		Z = 0.f;
	}

	/*----------------------------------------------------------------------------------------
		Transform Function Definitions
	----------------------------------------------------------------------------------------*/

	Transform::Transform( const Vector3D& inPosition, const Quaternion& inRotation, const Vector3D& inScale )
		: Position( inPosition ), Rotation( inRotation ), Scale( inScale )
	{
	}

	Transform::Transform( const Vector3D& inPosition, const Angle3D& inEuler, const Vector3D& inScale )
		: Position( inPosition ), Rotation( inEuler ), Scale( inScale )
	{
	}

	Transform::Transform( const Transform& inOther )
		: Position( inOther.Position ), Rotation( inOther.Rotation ), Scale( inOther.Scale )
	{
	}

	Transform::Transform( Transform&& inOther ) noexcept
		: Position( std::move( inOther.Position ) ), Rotation( std::move( inOther.Rotation ) ), Scale( std::move( inOther.Scale ) )
	{
	}

	/*
		Assignment Operators
	*/
	Transform& Transform::operator=( const Transform& inOther )
	{
		Position	= inOther.Position;
		Rotation	= inOther.Rotation;
		Scale		= inOther.Scale;

		return *this;
	}

	Transform& Transform::operator=( Transform&& inOther ) noexcept
	{
		Position	= std::move( inOther.Position );
		Rotation	= std::move( inOther.Rotation );
		Scale		= std::move( inOther.Scale );

		return *this;
	}

	/*
		Arithmatic Operators
	*/
	Transform Transform::operator+( const Transform& inOther ) const
	{
		return Transform(
			Position + inOther.Position,
			Rotation * inOther.Rotation,
			Scale + inOther.Scale
		);
	}

	Transform Transform::operator-( const Transform& inOther ) const
	{
		return Transform(
			Position - inOther.Position,
			Rotation * -inOther.Rotation,
			Scale - inOther.Scale
		);
	}

	/*
		Comparison Operators
	*/
	bool Transform::operator==( const Transform& inOther ) const
	{
		return( Position == inOther.Position &&
				Rotation == inOther.Rotation &&
				Scale == inOther.Scale );
	}

	bool Transform::operator!=( const Transform& inOther ) const
	{
		return( Position != inOther.Position ||
				Rotation != inOther.Rotation ||
				Scale != inOther.Scale );
	}

	/*
		Member Functions
	*/
	void Transform::Clear()
	{
		Position.Clear();
		Rotation.Clear();
		Scale.Clear();
	}

	Transform Transform::RotateTransform( const Quaternion& inRotation ) const
	{
		return Transform(
			Position,
			Rotation * inRotation,
			Scale
		);
	}

	Transform Transform::RotateTransform( const Angle3D& inEuler ) const
	{
		return Transform(
			Position,
			Rotation * Quaternion( inEuler ),
			Scale
		);
	}

	Transform Transform::TranslateTransform( const Vector3D& inVector ) const
	{
		return Transform(
			Position + inVector,
			Rotation,
			Scale
		);
	}

	Transform Transform::ScaleTransform( const Vector3D& inVector ) const
	{
		return Transform(
			Position,
			Rotation,
			Scale + inVector
		);
	}


	/*----------------------------------------------------------------------------------------
		Matrix Function Definitions
		TODO: Matrix stuff needs review and testing
	----------------------------------------------------------------------------------------*/
	Matrix::Matrix()
		: data{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
	{}

	Matrix::Matrix( float ia0, float ia1, float ia2, float ia3,
			float ib0, float ib1, float ib2, float ib3,
			float ic0, float ic1, float ic2, float ic3,
			float id0, float id1, float id2, float id3 )
	{
		data[ 0 ] = ia0;
		data[ 1 ] = ia1;
		data[ 2 ] = ia2;
		data[ 3 ] = ia3;
		data[ 4 ] = ib0;
		data[ 5 ] = ib1;
		data[ 6 ] = ib2;
		data[ 7 ] = ib3;
		data[ 8 ] = ic0;
		data[ 9 ] = ic1;
		data[ 10 ] = ic2;
		data[ 11 ] = ic3;
		data[ 12 ] = id0;
		data[ 13 ] = id1;
		data[ 14 ] = id2;
		data[ 15 ] = id3;
	}

	Matrix::Matrix( const float* in )
	{
		memcpy_s( data, 16 * sizeof( float ), in, 16 * sizeof( float ) );
	}

	Matrix::Matrix( const Vector4D& inVec1, const Vector4D& inVec2, const Vector4D& inVec3, const Vector4D& inVec4 )
		: Matrix( inVec1.X, inVec1.Y, inVec1.Z, inVec1.W,
				  inVec2.X, inVec2.Y, inVec2.Z, inVec2.W,
				  inVec3.X, inVec3.Y, inVec3.Z, inVec3.W,
				  inVec4.X, inVec4.Y, inVec4.Z, inVec4.W )
	{}

	Matrix::Matrix( const Matrix& Other )
		: Matrix( Other.data )
	{}

	const float* Matrix::GetData() const { return data; }

	void Matrix::operator=( const float* inData )
	{
		memcpy_s( data, 16 * sizeof( float ), inData, 16 * sizeof( float ) );
	}

	Matrix& Matrix::operator=( const Matrix& Other )
	{
		AssignData( Other.GetData() );
		return *this;
	}

	void Matrix::AssignData( const float* in ) { this->operator=( in ); }

	template< uint8 index >
	void Matrix::AssignVector( const Vector4D& Other )
	{
		static_assert( index < 4 );
		auto start_offset = index * 4;

		data[ start_offset ] = Other.X;
		data[ start_offset + 1 ] = Other.Y;
		data[ start_offset + 2 ] = Other.Z;
		data[ start_offset + 3 ] = Other.W;
	}

	void Matrix::AssignVector( const Vector4D& inVec, int inIndex )
	{
		if( inIndex > 3 ) { return; }
		auto start_offset = inIndex * 4;

		data[ start_offset ] = inVec.X;
		data[ start_offset + 1 ] = inVec.Y;
		data[ start_offset + 2 ] = inVec.Z;
		data[ start_offset + 3 ] = inVec.W;
	}

	template< int index >
	Vector4D Matrix::GetVector()
	{
		static_assert( index < 4 );
		auto start_offset = index * 4;

		return Vector4D(
			data[ start_offset ],
			data[ start_offset + 1 ],
			data[ start_offset + 2 ],
			data[ start_offset + 3 ]
		);
	}

	Vector4D Matrix::GetVector( int inIndex )
	{
		if( inIndex > 3 ) { return Vector4D(); }
		auto start_offset = inIndex * 4;

		return Vector4D( data[ start_offset ], data[ start_offset + 1 ], data[ start_offset + 2 ], data[ start_offset + 3 ] );
	}

	template< int row, int col >
	float Matrix::GetValue() const
	{
		static_assert( row < 4 && col < 4 );
		return data[ ( row * 4 ) + col ];
	}

	template< int row, int col >
	void Matrix::SetValue( float inVal )
	{
		static_assert( row < 4 && col < 4 );
		data[ ( row * 4 ) + col ] = inVal;
	}

	Vector4D Matrix::Transform( const Vector4D& inVec ) const
	{
		return inVec * ( *this );
	}

	Vector3D Matrix::Transform( const Vector3D& inVec ) const
	{
		return Transform( Vector4D( inVec.X, inVec.Y, inVec.Z, 1.f ) ).GetHomogenous3D();
	}

	const float* Matrix::operator[]( int inColumn ) const
	{
		if( inColumn >= 4 ) { return nullptr; }
		return data + ( inColumn * 4 );
	}

	Matrix Matrix::operator*( float inScalar ) const
	{
		Matrix output( *this );
		for( int i = 0; i < 16; i++ )
		{
			output.data[ i ] *= inScalar;
		}

		return output;
	}

	Matrix Matrix::operator*( const Matrix& other ) const
	{
		Matrix output{};

		/*
		*	0	1	2	3
		*	4	5	6	7
		*	8	9	10	11
		*	12	13	14	15
		*/

		for( int i = 0; i < 4; i++ )
		{
			auto rowOffset = i * 4;

			for( int j = 0; j < 3; j++ )
			{
				// i = row, j = col of the output matrix, its the value we are calculating
				// Were using row 'i' from this matrix
				// Were using column 'j' from the target matrix
				output.data[ rowOffset + j ] =
					( data[ rowOffset ] * other.data[ j ] ) +
					( data[ rowOffset + 1 ] * other.data[ j + 4 ] ) +
					( data[ rowOffset + 2 ] * other.data[ j + 8 ] ) +
					( data[ rowOffset + 3 ] * other.data[ j + 12 ] );
			}
		}

		return output;
	}

	Matrix Matrix::operator+( const Matrix& other ) const
	{
		Matrix output{};
		for( int i = 0; i < 16; i++ )
		{
			output.data[ i ] = data[ i ] + other.data[ i ];
		}

		return output;
	}

	Matrix Matrix::operator-( const Matrix& other ) const
	{
		Matrix output{};
		for( int i = 0; i < 16; i++ )
		{
			output.data[ i ] = data[ i ] - other.data[ i ];
		}

		return output;
	}

	/*----------------------------------------------------------------------------------------
		Library Function Definitions
	----------------------------------------------------------------------------------------*/
	float Geometry::CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		// Calculate distance from the screen origin to the center of the boudning sphere
		float fovHalf = inFOV / 2.f;
		float dist = Vector3D::Distance( inViewPos, inBounds.Center );
		float num = dist == 0.f ? 100000000.f : ( inBounds.Radius * ( cosf( fovHalf ) / sinf( fovHalf ) ) ) / dist;

		// If we wanted the radius instead of the diameter, we would half this result
		return num * (float) ( ScreenHeight );
	}


	float Geometry::CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		return CalculateScreenSizeInPixels( inView.Position, inView.FOV, inBounds, ScreenHeight );
	}


	Vector3D Geometry::GetDirectionVectorFromAngle( const Angle3D& inAngle )
	{
		float pitch		= inAngle.Pitch;
		float yaw		= inAngle.Yaw;

		// Clamp the angle into [0,360) degrees
		while( pitch >= 360.f ) { pitch -= 360.f; }
		while( yaw >= 360.f ) { yaw -= 360.f; }
		while( pitch < 0.f ) { pitch += 360.f; }
		while( yaw < 0.f ) { yaw += 360.f; }

		// Convert to radians
		pitch	= HYPERION_DEG_TO_RAD( pitch );
		yaw		= HYPERION_DEG_TO_RAD( yaw );

		return Vector3D(
			sinf( yaw ) * cosf( pitch ),
			-sinf( pitch ),
			cosf( yaw ) * cosf( pitch )
		);
	}
}