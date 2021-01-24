/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Library/Geometry.h
	© 2021, Zachary Berry
==================================================================================================*/


/*
*	NOTES:
*	
*	DX11 uses a left-handed coordinate system by default, which we are currently sticking with for the time being until we can change it
*	So... +X is towards the right, +Y is up, and +z is forward
*	Then... pitch is roataion along world X, Yaw is around the Y axis, and roll is around the Z axis
*	
*	DirectX applies PitchYawRoll rotations in the order of Roll -> Pitch -> Yaw
*	The order of rotations around axis is... ZXY
*	
*	Were going to be using Quaternions to represent the roataion of an object in 3D space to ensure rotations are applied as
*	expected in all cases, avoiding issues such as gimbal lock and what not
*	
*	TODO:
* 
*	Use elipson to have a 'margin of error' when comparing floats for equivalnce
*/



#pragma once

#include "Hyperion/Hyperion.h"



namespace Hyperion
{

	/*
	*	Forward Declarations
	*/
	struct ViewState;
	struct Vector2D;
	struct Vector3D;
	struct Vector4D;
	struct Quaternion;
	struct Transform;
	struct Matrix;


	/*==============================================================================================
	*	Angle Types
	*	TODO: 2D Angle?
	==============================================================================================*/
	class Angle3D
	{

	public:

		/*
			Data Members
		*/
		float Pitch, Yaw, Roll;

		/*
			Constructors
		*/
		explicit Angle3D( float inPitch = 0.f, float inYaw = 0.f, float inRoll = 0.f );
		Angle3D( const Angle3D& inOther );
		Angle3D( Angle3D&& inOther ) noexcept;

		/*
			Operators
		*/
		Angle3D operator+( const Angle3D& inOther ) const;
		Angle3D operator-( const Angle3D& inOther ) const;
		Angle3D& operator=( const Angle3D& Other );
		Angle3D& operator=( Angle3D&& Other ) noexcept;
		bool operator==( const Angle3D& Other ) const;
		bool operator!=( const Angle3D& Other ) const;

		/*
			Member Functions
		*/
		void Clear();
		void ClampContents();
		Vector3D GetDirectionVector() const;
		String ToString() const;
		Quaternion ToQuaternion() const;
	};


	/*==============================================================================================
	*	Vector Types
	==============================================================================================*/
	struct Vector2D
	{

	public:

		/*
			Data Members
		*/
		float X, Y;

		/*
			Constructors
		*/
		explicit Vector2D( float inX = 0.f, float inY = 0.f );
		Vector2D( const Vector2D& inOther );
		Vector2D( Vector2D&& inOther ) noexcept;

		/*
			Assignment and Equality
		*/
		Vector2D& operator=( const Vector2D& Other );
		Vector2D& operator=( Vector2D&& Other ) noexcept;

		bool operator==( const Vector2D& inOther ) const;
		bool operator!=( const Vector2D& inOther ) const;

		/*
			Static Functions
		*/
		static float Distance( const Vector2D& First, const Vector2D& Second );

		/*
			Member Functions
		*/
		void Clear();

		inline float Distance( const Vector2D& Other ) const
		{
			return Vector2D::Distance( *this, Other );
		}

		float Length() const;
		Vector2D GetNormalized() const;
		bool IsNormalized() const;
		String ToString() const;
		float Dot( const Vector2D& other ) const;
		Vector2D GetNegated() const;

		/*
		*	Operators
		*/
		inline float operator*( const Vector2D& other ) const { return Dot( other ); }
		Vector2D operator*( float inScale ) const;
		Vector2D operator/( float inScale ) const;

		Vector4D operator+( const Vector4D& other ) const;
		Vector3D operator+( const Vector3D& other ) const;
		Vector2D operator+( const Vector2D& Other ) const;
		Vector2D operator-( const Vector2D& Other ) const;
		inline Vector2D operator-() const { return GetNegated(); }
	};


	struct Vector3D
	{

	public:

		/*
			Data Members
		*/
		float X, Y, Z;

		/*
			Constructors
		*/
		explicit Vector3D( float inX = 0.f, float inY = 0.f, float inZ = 0.f );
		Vector3D( const Vector3D& inOther );
		Vector3D( Vector3D&& inOther ) noexcept;

		/*
			Operators
		*/

		Vector3D& operator=( const Vector3D& Other );
		Vector3D& operator=( Vector3D&& Other ) noexcept;

		bool operator==( const Vector3D& inOther ) const;
		bool operator!=( const Vector3D& inOther ) const;

		/*
			Static Functions
		*/
		static float Distance( const Vector3D& First, const Vector3D& Second );
		static Vector3D GetWorldUp();
		static Vector3D GetWorldRight();
		static Vector3D GetWorldForward();

		/*
			Member Function
		*/
		void Clear();

		inline float Distance( const Vector3D& Other ) const
		{
			return Vector3D::Distance( *this, Other );
		}

		float Length() const;
		Vector3D GetNormalized() const;
		bool IsNormalized() const;
		Angle3D GetEulerAngles() const;
		String ToString() const;
		float Dot( const Vector3D& other ) const;
		Vector3D Cross( const Vector3D& other ) const;
		Vector3D GetNegated() const;

		Vector3D operator*( const Matrix& other ) const;
		inline float operator*( const Vector3D& other ) const { return Dot( other ); }
		inline Vector3D operator^( const Vector3D& other ) const { return Cross( other ); }

		Vector3D operator-( const Vector3D& Other ) const;
		Vector3D operator*( float inScale ) const;
		Vector3D operator/( float inScale ) const;

		Vector4D operator+( const Vector4D& other ) const;
		Vector3D operator+( const Vector3D& Other ) const;
		Vector3D operator+( const Vector2D& other ) const;

		Vector3D operator-( const Vector2D& other ) const;
		inline Vector3D operator-() const { return GetNegated(); }



	};

	
	struct Vector4D
	{

	public:

		/*
			Data Members
		*/
		float X, Y, Z, W;

		/*
			Constructors
		*/
		explicit Vector4D( float inX = 0.f, float inY = 0.f, float inZ = 0.f, float inW = 0.f );
		Vector4D( const Vector4D& inOther );
		Vector4D( Vector4D&& inOther ) noexcept;

		/*
			Assignment
		*/
		Vector4D& operator=( Vector4D&& Other ) noexcept;
		Vector4D& operator=( const Vector4D& Other );
		Vector4D& operator=( const Vector3D& other );
		Vector4D& operator=( const Vector2D& other );

		bool operator==( const Vector4D& Other ) const;
		bool operator!=( const Vector4D& Other ) const;

		bool operator==( const Vector3D& other ) const;
		bool operator!=( const Vector3D& other ) const;

		bool operator==( const Vector2D& other ) const;
		bool operator!=( const Vector2D& other ) const;

		/*
			Static Functions
		*/
		static float Distance( const Vector4D& First, const Vector4D& Second );

		/*
			Member Functions
		*/
		void Clear();
		float Distance( const Vector4D& Other ) const;
		float Length() const;
		Vector4D GetNormalized() const;
		bool IsNormalized() const;
		String ToString() const;
		float Dot( const Vector4D& other ) const;
		Vector3D GetHomogenous3D() const;
		Vector4D GetHomogenous4D() const;
		bool IsHomogenous() const;
		Vector4D GetNegated() const;

		/*
		*	Operations
		*/
		Vector4D operator*( const Matrix& other ) const;
		Vector4D operator*( float inScalar ) const;
		Vector4D operator/( float inScalar ) const;
		inline float operator*( const Vector4D& other ) const { return Dot( other ); }
		Vector4D operator+( const Vector4D& other ) const;
		Vector4D operator-( const Vector4D& other ) const;
		Vector4D operator+( const Vector3D& other ) const;
		Vector4D operator-( const Vector3D& other ) const;
		Vector4D operator+( const Vector2D& other ) const;
		Vector4D operator-( const Vector2D& other ) const;
		inline Vector4D operator-() const { return GetNegated(); }
	};


	struct Quaternion
	{
		float W, X, Y, Z;

		Quaternion();
		Quaternion( float inW, float inX, float inY, float inZ );
		Quaternion( const Vector3D& inAxis, float inRotationDegrees );
		explicit Quaternion( const Matrix& inMatrix );
		explicit Quaternion( const Angle3D& inEuler );
		Quaternion( const Quaternion& other );
		Quaternion( Quaternion&& other ) noexcept;

		Quaternion& operator=( const Quaternion& other );
		Quaternion& operator=( Quaternion&& other ) noexcept;

		bool operator==( const Quaternion& other ) const;
		bool operator!=( const Quaternion& other ) const;

		Quaternion operator*( const Quaternion& other ) const;
		Quaternion operator*( const Angle3D& inEuler ) const;
		Quaternion operator-() const;

		bool IsNormalized() const;
		float GetLength() const;
		Quaternion GetNormalized() const;
		Vector3D GetRotationAxis() const;
		float GetRotationAmount() const;
		Matrix GetRotationMatrix() const;
		Angle3D GetEulerAngles() const;

		void Clear();

		Vector3D RotateVector( const Vector3D& inVector ) const;
		void SetFromEuler( const Angle3D& inEuler );
	};


	/*==============================================================================================
	*	Transform
	==============================================================================================*/
	struct Transform
	{

	public:

		/*
			Data Members
		*/
		Vector3D Position;
		Quaternion Rotation;;
		Vector3D Scale;

		/*
			Constructors
		*/
		Transform( const Vector3D& inPosition = Vector3D(), const Quaternion& inRotation = Quaternion(), const Vector3D& inScale = Vector3D() );
		Transform( const Vector3D& inPosition, const Angle3D& inRotation, const Vector3D& inScale = Vector3D() );
		Transform( const Transform& inOther );
		Transform( Transform&& inOther ) noexcept;

		Transform& operator=( const Transform& inOther );
		Transform& operator=( Transform&& inOther ) noexcept;

		Transform operator+( const Transform& inOther ) const;
		Transform operator-( const Transform& inOther ) const;

		bool operator==( const Transform& inOther ) const;
		bool operator!=( const Transform& inOther ) const;

		/*
			Member Functions
		*/
		void Clear();

		Transform RotateTransform( const Quaternion& inRotation ) const;
		Transform RotateTransform( const Angle3D& inEuler ) const;
		Transform TranslateTransform( const Vector3D& inVec ) const;
		Transform ScaleTransform( const Vector3D& inVec ) const;
	};



	struct Matrix
	{
		float data[ 16 ];

		Matrix();
		Matrix( float ia0, float ia1, float ia2, float ia3,
				float ib0, float ib1, float ib2, float ib3,
				float ic0, float ic1, float ic2, float ic3,
				float id0, float id1, float id2, float id3 );
		Matrix( const float* in );
		explicit Matrix( const Vector4D& inVec1, const Vector4D& inVec2 = Vector4D(), const Vector4D& inVec3 = Vector4D(), const Vector4D& inVec4 = Vector4D() );
		Matrix( const Matrix& Other );

		const float* GetData() const;
		void operator=( const float* inData );
		Matrix& operator=( const Matrix& Other );
		void AssignData( const float* in );

		template< uint8 index >
		void AssignVector( const Vector4D& Other );

		void AssignVector( const Vector4D& inVec, int inIndex );

		template< int index >
		Vector4D GetVector();

		Vector4D GetVector( int inIndex );

		template< int row, int col >
		float GetValue() const;

		template< int row, int col >
		void SetValue( float inVal );

		Vector4D Transform( const Vector4D& inVec ) const;
		Vector3D Transform( const Vector3D& inVec ) const;

		const float* operator[]( int inRow ) const;

		Matrix operator*( float inScalar ) const;
		Matrix operator*( const Matrix& other ) const;
		Matrix operator+( const Matrix& other ) const;
		Matrix operator-( const Matrix& other ) const;
	};


	/*==============================================================================================
	*	Vertex Types
	==============================================================================================*/
	#pragma pack( push, 1 )
	struct Vertex3D
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct WindowVertex
	{
		float x, y, z;
		float u, v;
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct VertexColor3D
	{
		float x, y, z;
		float r, g, b, a;
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct Vertex2D
	{
		float x, y;
		float u, v;
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct VertexColor2D
	{
		float x, y;
		float r, g, b, a;
	};
	#pragma pack( pop )


	/*==============================================================================================
	*	Axis-Aligned Bounding Box
	==============================================================================================*/
	struct AABB
	{
		AABB()
			: Min(), Max()
		{}

		AABB( const AABB& Other )
			: Min( Other.Min ), Max( Other.Max )
		{}

		AABB( AABB&& Other ) noexcept
			: Min( std::move( Other.Min ) ), Max( std::move( Other.Max ) )
		{}

		AABB& operator=( const AABB& Other )
		{
			Max = Other.Max;
			Min = Other.Min;

			return *this;
		}

		AABB& operator=( AABB&& Other ) noexcept
		{
			Max = std::move( Other.Max );
			Min = std::move( Other.Min );

			return *this;
		}

		Vector3D Min;
		Vector3D Max;
	};


	/*==============================================================================================
	*	Bounding Sphere
	==============================================================================================*/
	struct BoundingSphere
	{
		BoundingSphere()
			: Center(), Radius( 0.f )
		{}

		BoundingSphere( const BoundingSphere& Other )
			: Center( Other.Center ), Radius( Other.Radius )
		{}

		BoundingSphere( BoundingSphere&& Other ) noexcept
			: Center( std::move( Other.Center ) ), Radius( std::move( Other.Radius ) )
		{}

		BoundingSphere& operator=( const BoundingSphere& Other )
		{
			Center = Other.Center;
			Radius = Other.Radius;

			return *this;
		}

		BoundingSphere& operator=( BoundingSphere&& Other ) noexcept
		{
			Center = std::move( Other.Center );
			Radius = std::move( Other.Radius );

			return *this;
		}

		Vector3D Center;
		float Radius;
	};


	/*==============================================================================================
	*	Library Functions
	==============================================================================================*/
	class Geometry
	{

	public:

		Geometry() = delete;


		/*
			CalculateScreenSizeInPixels
			* Returns the DIAMETER of the sphere in screen space pixels
		*/
		static float CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight );
		static float CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight );

		static Vector3D GetDirectionVectorFromAngle( const Angle3D& inAngle );

	};

}