/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Math.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Renderer/DataTypes.h"
#include <DirectXMath.h>


namespace Hyperion
{

	struct RVector
	{

		DirectX::XMVECTOR v;

		RVector()
			: v()
		{}

		RVector( const RVector& r )
			: v( r.v )
		{}

		RVector( const Vector4D& r )
			: v( DirectX::XMVectorSet( r.X, r.Y, r.Z, r.W ) )
		{}

		RVector( const Vector3D& r, float fill = 1.f )
			: v( DirectX::XMVectorSet( r.X, r.Y, r.Z, fill ) )
		{}

		RVector( const Vector2D& r, float fill = 1.f )
			: v( DirectX::XMVectorSet( r.X, r.Y, fill, fill ) )
		{}

		RVector( float inX, float inY, float inZ, float inW )
			: v( DirectX::XMVectorSet( inX, inY, inZ, inW ) )
		{}

		RVector( const DirectX::XMVECTOR& r )
			: v( r )
		{}

	};


	struct RMatrix
	{

		DirectX::XMMATRIX m;

		RMatrix()
			: m()
		{}

		RMatrix( const RMatrix& r )
			: m( r.m )
		{}

		RMatrix( const Matrix& r )
			: m( r.GetData() )
		{}

		RMatrix( const DirectX::XMMATRIX& r )
			: m( r )
		{}

		RMatrix( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
			: m( m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 )
		{}

		RMatrix( const RVector& r0, const RVector& r1, const RVector& r2, const RVector& r3 )
			: m( r0.v, r1.v, r2.v, r3.v )
		{}

		RMatrix Transpose() const
		{
			return RMatrix( DirectX::XMMatrixTranspose( m ) );
		}

		RMatrix operator* ( const RMatrix& r )
		{
			return RMatrix( DirectX::XMMatrixMultiply( m, r.m ) );
		}

	};


	struct RFrustum
	{

		DirectX::XMVECTOR m_Planes[ 6 ];
		bool m_bInitialized;

		RFrustum() 
			: m_bInitialized( false ), m_Planes()
		{}

		// TODO: Calculate Z-Far from the projection matrix?
		void Build( const RMatrix& inViewMatrix, const RMatrix& inProjectionMatrix, float inZFar )
		{
			float z_Min, r;
			DirectX::XMFLOAT4X4 r_ProjMatrix, r_Frustum;

			// Dump the projection matrix into a 4x4 float so we can easily access indivisual cells
			DirectX::XMStoreFloat4x4( &r_ProjMatrix, inProjectionMatrix.m );

			z_Min = -r_ProjMatrix._43 / r_ProjMatrix._33;
			r = inZFar / ( inZFar - z_Min );
			r_ProjMatrix._33 = r;
			r_ProjMatrix._43 = -r * z_Min;

			// Put the 4x4 float back into the projection matrix
			DirectX::XMMATRIX newProjection = DirectX::XMLoadFloat4x4( &r_ProjMatrix );

			// Multiply view and projection matrix, and dump the result into another 4x4 float
			DirectX::XMStoreFloat4x4( &r_Frustum, DirectX::XMMatrixMultiply( inViewMatrix.m, newProjection ) );

			// Calculate Near Plane
			m_Planes[ 0 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 + r_Frustum._13,
				r_Frustum._24 + r_Frustum._23,
				r_Frustum._34 + r_Frustum._33,
				r_Frustum._44 + r_Frustum._43
			) );

			// Calculate Far Plane
			m_Planes[ 1 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 - r_Frustum._13,
				r_Frustum._24 - r_Frustum._23,
				r_Frustum._34 - r_Frustum._33,
				r_Frustum._44 - r_Frustum._43
			) );

			// Calculate Left Plane
			m_Planes[ 2 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 + r_Frustum._11,
				r_Frustum._24 + r_Frustum._21,
				r_Frustum._34 + r_Frustum._31,
				r_Frustum._44 + r_Frustum._41
			) );

			// Calculate Right Plane
			m_Planes[ 3 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 - r_Frustum._11,
				r_Frustum._24 - r_Frustum._21,
				r_Frustum._34 - r_Frustum._31,
				r_Frustum._44 - r_Frustum._41
			) );

			// Calculate Top Plane
			m_Planes[ 4 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 - r_Frustum._12,
				r_Frustum._24 - r_Frustum._22,
				r_Frustum._34 - r_Frustum._32,
				r_Frustum._44 - r_Frustum._42
			) );

			// Calculate Bottom Plane
			m_Planes[ 5 ] = DirectX::XMPlaneNormalize( DirectX::XMVectorSet(
				r_Frustum._14 + r_Frustum._12,
				r_Frustum._24 + r_Frustum._22,
				r_Frustum._34 + r_Frustum._32,
				r_Frustum._44 + r_Frustum._42
			) );

			m_bInitialized = true;
		}

		/*
		*	RFrustum::CoherentOBBTest
		*	- Checks oriented bounding box against the frustum
		*	- Returns a value less than zero if the bounds intersect or are within the frustum
		*	- Returns a plane index, 0 or greater if the bounds are outside of the frustum, the plane index is the plane that
		*	  caused the test to fail, and can be passed back into the function next iteration for efficiency
		*/
		int CoherentOBBTest( const OBB& inBounds, int inState )
		{
			auto topFrontRight		= DirectX::XMVectorSet( inBounds.TopFrontRight.X, inBounds.TopFrontRight.Y, inBounds.TopFrontRight.Z, 1.f );
			auto topFrontLeft		= DirectX::XMVectorSet( inBounds.TopFrontLeft.X, inBounds.TopFrontLeft.Y, inBounds.TopFrontLeft.Z, 1.f );
			auto topBackRight		= DirectX::XMVectorSet( inBounds.TopBackRight.X, inBounds.TopBackRight.Y, inBounds.TopBackRight.Z, 1.f );
			auto topBackLeft		= DirectX::XMVectorSet( inBounds.TopBackLeft.X, inBounds.TopBackLeft.Y, inBounds.TopBackLeft.Z, 1.f );
			auto bottomFrontRight	= DirectX::XMVectorSet( inBounds.BottomFrontRight.X, inBounds.BottomFrontRight.Y, inBounds.BottomFrontRight.Z, 1.f );
			auto bottomFrontLeft	= DirectX::XMVectorSet( inBounds.BottomFrontLeft.X, inBounds.BottomFrontLeft.Y, inBounds.BottomFrontLeft.Z, 1.f );
			auto bottomBackRight	= DirectX::XMVectorSet( inBounds.BottomBackRight.X, inBounds.BottomBackRight.Y, inBounds.BottomBackRight.Z, 1.f );
			auto bottomBackLeft		= DirectX::XMVectorSet( inBounds.BottomBackLeft.X, inBounds.BottomBackLeft.Y, inBounds.BottomBackLeft.Z, 1.f );

			// We want to check the plane index specified first
			if( inState >= 0 )
			{
				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], topFrontRight ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], topFrontLeft ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], topBackRight ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], topBackLeft ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], bottomFrontRight ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], bottomFrontLeft ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], bottomBackRight ) ) < 0.f &&
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ inState ], bottomBackLeft ) ) < 0.f )
				{
					return inState;
				}
			}

			for( int i = 0; i < 6; i++ )
			{
				// If we already checked a plane, dont check it again
				if( i == inState ) { continue; }

				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topBackLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomBackLeft ) ) >= 0.f )
				{
					continue;
				}

				return i;
			}

			return -1;
		}

		/*
		*	AABBTest
		*	- Returns true if the given AABB intersects the frustum
		*/
		bool AABBTest( const DirectX::XMFLOAT3& inMin, const DirectX::XMFLOAT3& inMax, const DirectX::XMFLOAT3& inPosition, const DirectX::XMVECTOR& inQuaternion )
		{
			// Calculate the other points
			DirectX::XMVECTOR bottomFrontLeft = DirectX::XMLoadFloat3( &inMin );
			DirectX::XMVECTOR bottomFrontRight = DirectX::XMVectorSet( inMax.x, inMin.y, inMin.z, 1.f );
			DirectX::XMVECTOR bottomBackLeft = DirectX::XMVectorSet( inMin.x, inMin.y, inMax.z, 1.f );
			DirectX::XMVECTOR bottomBackRight = DirectX::XMVectorSet( inMax.x, inMin.y, inMax.z, 1.f );

			DirectX::XMVECTOR topFrontLeft = DirectX::XMVectorSet( inMin.x, inMax.y, inMin.z, 1.f );
			DirectX::XMVECTOR topFrontRight = DirectX::XMVectorSet( inMax.x, inMax.y, inMin.z, 1.f );
			DirectX::XMVECTOR topBackLeft = DirectX::XMVectorSet( inMin.x, inMax.y, inMax.z, 1.f );
			DirectX::XMVECTOR topBackRight = DirectX::XMLoadFloat3( &inMax );

			// Rotate Verticies
			auto rotMatrix = DirectX::XMMatrixRotationQuaternion( inQuaternion );
			bottomFrontLeft = DirectX::XMVector3TransformCoord( bottomFrontLeft, rotMatrix );
			bottomFrontRight = DirectX::XMVector3TransformCoord( bottomFrontRight, rotMatrix );
			bottomBackLeft = DirectX::XMVector3TransformCoord( bottomBackLeft, rotMatrix );
			bottomBackRight = DirectX::XMVector3TransformCoord( bottomBackRight, rotMatrix );
			topFrontLeft = DirectX::XMVector3TransformCoord( topFrontLeft, rotMatrix );
			topFrontRight = DirectX::XMVector3TransformCoord( topFrontRight, rotMatrix );
			topBackLeft = DirectX::XMVector3TransformCoord( topBackLeft, rotMatrix );
			topBackRight = DirectX::XMVector3TransformCoord( topBackRight, rotMatrix );

			// Translate verticies
			if( inPosition.x != 0.f || inPosition.y != 0.f || inPosition.z != 0.f )
			{
				auto offset = DirectX::XMVectorSet( inPosition.x, inPosition.y, inPosition.z, 0.f );

				bottomFrontLeft = DirectX::XMVectorAdd( bottomFrontLeft, offset );
				bottomFrontRight = DirectX::XMVectorAdd( bottomFrontRight, offset );
				bottomBackLeft = DirectX::XMVectorAdd( bottomBackLeft, offset );
				bottomBackRight = DirectX::XMVectorAdd( bottomBackRight, offset );
				topFrontLeft = DirectX::XMVectorAdd( topFrontLeft, offset );
				topFrontRight = DirectX::XMVectorAdd( topFrontRight, offset );
				topBackLeft = DirectX::XMVectorAdd( topBackLeft, offset );
				topBackRight = DirectX::XMVectorAdd( topBackRight, offset );
			}

			// We want to determine if any of the points are within the view frustum
			// For this, we loop through each plane, and check if all points are on the 'wrong' side of the plane
			// If any point is on the 'correct' side, we can skip to the next iteration
			// If we get to the end of the loop, and we didnt find any plane with all points on the 'wrong' side, then we passed
			for( int i = 0; i < 6; i++ )
			{

				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], topBackLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], bottomBackLeft ) ) >= 0.f )
				{
					continue;
				}

				return false;
			}

			return true;
		}


		bool SphereTest( const DirectX::XMFLOAT3& inCenter, float inRadius )
		{
			auto CenterVec = DirectX::XMLoadFloat3( &inCenter );

			for( int i = 0; i < 6; i++ )
			{
				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], CenterVec ) ) < -inRadius )
					return true;
			}

			return false;
		}


		bool RectCenterTest( const DirectX::XMFLOAT3& CenterPosition, const DirectX::XMFLOAT3& HalfSize )
		{
			// Load all of the points we will be checking into vectors now
			DirectX::XMVECTOR CenterVec = DirectX::XMLoadFloat3( &CenterPosition );

			DirectX::XMVECTOR TopFrontRight = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopFrontLeft = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopBackRight = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopBackLeft = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomFrontRight = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, -HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomFrontLeft = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, -HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomBackRight = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, -HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomBackLeft = DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, -HalfSize.y, -HalfSize.z, 1.f ) );

			for( int i = 0; i < 6; i++ )
			{
				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopBackLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomBackLeft ) ) >= 0.f )
				{
					continue;
				}

				return true;
			}

			return false;
		}


		bool CubeCenterTest( const DirectX::XMFLOAT3& CenterPosition, float Radius )
		{
			return RectCenterTest( CenterPosition, DirectX::XMFLOAT3( Radius, Radius, Radius ) );
		}


		bool RectCornersTest( const DirectX::XMFLOAT3& FirstCorner, const DirectX::XMFLOAT3& OppositeCorner )
		{
			DirectX::XMVECTOR TopFrontRight = DirectX::XMLoadFloat3( &OppositeCorner );
			DirectX::XMVECTOR TopFrontLeft = DirectX::XMVectorSet( FirstCorner.x, OppositeCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR TopBackRight = DirectX::XMVectorSet( OppositeCorner.x, OppositeCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR TopBackLeft = DirectX::XMVectorSet( FirstCorner.x, OppositeCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR BottomFrontRight = DirectX::XMVectorSet( OppositeCorner.x, FirstCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR BottomFrontLeft = DirectX::XMVectorSet( FirstCorner.x, FirstCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR BottomBackRight = DirectX::XMVectorSet( OppositeCorner.x, FirstCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR BottomBackLeft = DirectX::XMVectorSet( FirstCorner.x, FirstCorner.y, FirstCorner.z, 1.f );

			for( int i = 0; i < 6; i++ )
			{
				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], TopBackLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomFrontRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomFrontLeft ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomBackRight ) ) >= 0.f ||
					DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], BottomBackLeft ) ) >= 0.f )
				{
					continue;
				}

				return true;
			}

			return false;
		}

	};


	class RMath
	{

	public:

		RMath() = delete;

		static RMatrix GenerateLookAtMatrix( const Vector3D& inPosition, const Quaternion& inRotation, bool bLeftHanded = true )
		{
			auto posVec		= DirectX::XMVectorSet( inPosition.X, inPosition.Y, inPosition.Z, 1.f );
			auto rotMatrix = DirectX::XMMatrixRotationQuaternion(
				bLeftHanded ?
				DirectX::XMVectorSet( -inRotation.X, -inRotation.Y, -inRotation.Z, inRotation.W ) :
				DirectX::XMVectorSet( inRotation.X, inRotation.Y, inRotation.Z, inRotation.W )
			);

			static DirectX::XMVECTOR up		= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
			static DirectX::XMVECTOR dir	= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );

			auto tUp	= DirectX::XMVector3TransformCoord( up, rotMatrix );
			auto tDir	= DirectX::XMVectorAdd( DirectX::XMVector3TransformCoord( dir, rotMatrix ), posVec );

			return RMatrix(
				bLeftHanded ?
				DirectX::XMMatrixLookAtLH( posVec, tDir, tUp ) :
				DirectX::XMMatrixLookAtRH( posVec, tDir, tUp )
			);
		}

		static RMatrix GenerateDefaultLookAtMatrix( bool bLeftHanded = true )
		{
			static DirectX::XMVECTOR pos	= DirectX::XMVectorSet( 0.f, 0.f, -0.1f, 1.f ); // TODO: Should z = 0?
			static DirectX::XMVECTOR dir	= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
			static DirectX::XMVECTOR up		= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );

			return RMatrix(
				bLeftHanded ?
				DirectX::XMMatrixLookAtLH( pos, dir, up ) :
				DirectX::XMMatrixLookAtRH( pos, dir, up )
			);
		}

		static RMatrix GeneratePerspectiveMatrix( float inFOV, const ScreenResolution& inResolution, float inNear, float inFar, bool bLeftHanded = true )
		{
			float aspectRatio = (float) inResolution.Width / (float) inResolution.Height;
			return RMatrix(
				bLeftHanded ?
				DirectX::XMMatrixPerspectiveFovLH( inFOV, aspectRatio, inNear, inFar ) :
				DirectX::XMMatrixPerspectiveFovRH( inFOV, aspectRatio, inNear, inFar )
			);
		}

		static RMatrix GenerateOrthographicMatrix( float inWidth, float inHeight, float inNear, float inFar, bool bLeftHanded = true )
		{
			return RMatrix(
				bLeftHanded ?
				DirectX::XMMatrixOrthographicLH( inWidth, inHeight, inNear, inFar ) :
				DirectX::XMMatrixOrthographicRH( inWidth, inHeight, inNear, inFar )
			);
		}

		static RMatrix GenerateWorldMatrix( const Transform& inTransform, bool bLeftHanded = true )
		{
			auto output = DirectX::XMMatrixIdentity();
			output *= DirectX::XMMatrixRotationQuaternion(
				bLeftHanded ?
				DirectX::XMVectorSet( -inTransform.Rotation.X, -inTransform.Rotation.Y, -inTransform.Rotation.Z, inTransform.Rotation.W ) :
				DirectX::XMVectorSet( inTransform.Rotation.X, inTransform.Rotation.Y, inTransform.Rotation.Z, inTransform.Rotation.W )
			);
			output *= DirectX::XMMatrixTranslation( inTransform.Position.X, inTransform.Position.Y, inTransform.Position.Z );

			return output;
		}

		static OBB GenerateOrientedBoundingBox( const AABB& inBounds, const Transform& inTransform )
		{
					// Create OBB in place of AABB
			auto bfl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Min.Y, inBounds.Min.Z, 1.f );
			auto bfr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Min.Y, inBounds.Min.Z, 1.f );
			auto bbl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Min.Y, inBounds.Max.Z, 1.f );
			auto bbr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Min.Y, inBounds.Max.Z, 1.f );
			auto tfl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Max.Y, inBounds.Min.Z, 1.f );
			auto tfr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Max.Y, inBounds.Min.Z, 1.f );
			auto tbl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Max.Y, inBounds.Max.Z, 1.f );
			auto tbr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Max.Y, inBounds.Max.Z, 1.f );

			// Now we need to rotate these points, and then translate them
			auto worldMatrix = DirectX::XMMatrixRotationQuaternion( DirectX::XMVectorSet( inTransform.Rotation.X, inTransform.Rotation.Y, inTransform.Rotation.Z, inTransform.Rotation.W ) );
			worldMatrix *= DirectX::XMMatrixTranslation( inTransform.Position.X, inTransform.Position.Y, inTransform.Position.Z );

			bfl = DirectX::XMVector3TransformCoord( bfl, worldMatrix );
			bfr = DirectX::XMVector3TransformCoord( bfr, worldMatrix );
			bbl = DirectX::XMVector3TransformCoord( bbl, worldMatrix );
			bbr = DirectX::XMVector3TransformCoord( bbr, worldMatrix );
			tfl = DirectX::XMVector3TransformCoord( tfl, worldMatrix );
			tfr = DirectX::XMVector3TransformCoord( tfr, worldMatrix );
			tbl = DirectX::XMVector3TransformCoord( tbl, worldMatrix );
			tbr = DirectX::XMVector3TransformCoord( tbr, worldMatrix );

			OBB outBounds {};

			outBounds.BottomFrontLeft	= Vector3D( bfl.m128_f32 );
			outBounds.BottomFrontRight	= Vector3D( bfr.m128_f32 );
			outBounds.BottomBackLeft	= Vector3D( bbl.m128_f32 );
			outBounds.BottomBackRight	= Vector3D( bbr.m128_f32 );
			outBounds.TopFrontLeft		= Vector3D( tfl.m128_f32 );
			outBounds.TopFrontRight		= Vector3D( tfr.m128_f32 );
			outBounds.TopBackLeft		= Vector3D( tbl.m128_f32 );
			outBounds.TopBackRight		= Vector3D( tbr.m128_f32 );

			return outBounds;
		}

	};

}