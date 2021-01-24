/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Frustum.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DirectX11Frustum
	{

	private:

		DirectX::XMVECTOR m_Planes[ 6 ];
		bool m_bValid;

	public:

		DirectX11Frustum()
			: m_bValid( false )
		{

		}


		~DirectX11Frustum()
		{
			m_bValid = false;
		}


		void Construct( const DirectX::XMMATRIX& inView, const DirectX::XMMATRIX& inProjection, float inScreenDepth )
		{
			float z_Min, r;
			DirectX::XMFLOAT4X4 r_ProjMatrix, r_Frustum;

			// Dump the projection matrix into a 4x4 float so we can easily access indivisual cells
			DirectX::XMStoreFloat4x4( &r_ProjMatrix, inProjection );

			z_Min = -r_ProjMatrix._43 / r_ProjMatrix._33;
			r = inScreenDepth / ( inScreenDepth - z_Min );
			r_ProjMatrix._33 = r;
			r_ProjMatrix._43 = -r * z_Min;

			// Put the 4x4 float back into the projection matrix
			DirectX::XMMATRIX newProjection = DirectX::XMLoadFloat4x4( &r_ProjMatrix );

			// Multiply view and projection matrix, and dump the result into another 4x4 float
			DirectX::XMStoreFloat4x4( &r_Frustum, DirectX::XMMatrixMultiply( inView, newProjection ) );

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

			m_bValid = true;
		}


		bool CheckSphere( const DirectX::XMFLOAT3& inCenter, float inRadius )
		{
			auto CenterVec = DirectX::XMLoadFloat3( &inCenter );

			for( int i = 0; i < 6; i++ )
			{
				if( DirectX::XMVectorGetX( DirectX::XMPlaneDotCoord( m_Planes[ i ], CenterVec ) ) < -inRadius )
					return true;
			}

			return false;
		}


		bool CheckRectCenter( const DirectX::XMFLOAT3& CenterPosition, const DirectX::XMFLOAT3& HalfSize )
		{
			// Load all of the points we will be checking into vectors now
			DirectX::XMVECTOR CenterVec = DirectX::XMLoadFloat3( &CenterPosition );

			DirectX::XMVECTOR TopFrontRight		= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopFrontLeft		= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopBackRight		= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR TopBackLeft		= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomFrontRight	= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, -HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomFrontLeft	= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, -HalfSize.y, HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomBackRight	= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( HalfSize.x, -HalfSize.y, -HalfSize.z, 1.f ) );
			DirectX::XMVECTOR BottomBackLeft	= DirectX::XMVectorAdd( CenterVec, DirectX::XMVectorSet( -HalfSize.x, -HalfSize.y, -HalfSize.z, 1.f ) );

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


		bool CheckCubeCenter( const DirectX::XMFLOAT3& CenterPosition, float Radius )
		{
			return CheckRectCenter( CenterPosition, DirectX::XMFLOAT3( Radius, Radius, Radius ) );
		}


		bool CheckRectCorners( const DirectX::XMFLOAT3& FirstCorner, const DirectX::XMFLOAT3& OppositeCorner )
		{
			DirectX::XMVECTOR TopFrontRight		= DirectX::XMLoadFloat3( &OppositeCorner );
			DirectX::XMVECTOR TopFrontLeft		= DirectX::XMVectorSet( FirstCorner.x, OppositeCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR TopBackRight		= DirectX::XMVectorSet( OppositeCorner.x, OppositeCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR TopBackLeft		= DirectX::XMVectorSet( FirstCorner.x, OppositeCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR BottomFrontRight	= DirectX::XMVectorSet( OppositeCorner.x, FirstCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR BottomFrontLeft	= DirectX::XMVectorSet( FirstCorner.x, FirstCorner.y, OppositeCorner.z, 1.f );
			DirectX::XMVECTOR BottomBackRight	= DirectX::XMVectorSet( OppositeCorner.x, FirstCorner.y, FirstCorner.z, 1.f );
			DirectX::XMVECTOR BottomBackLeft	= DirectX::XMVectorSet( FirstCorner.x, FirstCorner.y, FirstCorner.z, 1.f );

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

		/*
		*	DirectX11Frustum::CheckAABB
		*	- Returns true if the given AABB intersects the frustum
		*/
		bool CheckAABB( const DirectX::XMFLOAT3& inMin, const DirectX::XMFLOAT3& inMax, const DirectX::XMFLOAT3& inPosition, const DirectX::XMVECTOR& inQuaternion )
		{
			// Calculate the other points
			DirectX::XMVECTOR bottomFrontLeft		= DirectX::XMLoadFloat3( &inMin );
			DirectX::XMVECTOR bottomFrontRight		= DirectX::XMVectorSet( inMax.x, inMin.y, inMin.z, 1.f );
			DirectX::XMVECTOR bottomBackLeft		= DirectX::XMVectorSet( inMin.x, inMin.y, inMax.z, 1.f );
			DirectX::XMVECTOR bottomBackRight		= DirectX::XMVectorSet( inMax.x, inMin.y, inMax.z, 1.f );

			DirectX::XMVECTOR topFrontLeft			= DirectX::XMVectorSet( inMin.x, inMax.y, inMin.z, 1.f );
			DirectX::XMVECTOR topFrontRight			= DirectX::XMVectorSet( inMax.x, inMax.y, inMin.z, 1.f );
			DirectX::XMVECTOR topBackLeft			= DirectX::XMVectorSet( inMin.x, inMax.y, inMax.z, 1.f );
			DirectX::XMVECTOR topBackRight			= DirectX::XMLoadFloat3( &inMax );

			// Rotate Verticies
			auto rotMatrix		= DirectX::XMMatrixRotationQuaternion( inQuaternion );
			bottomFrontLeft		= DirectX::XMVector3TransformCoord( bottomFrontLeft, rotMatrix );
			bottomFrontRight	= DirectX::XMVector3TransformCoord( bottomFrontRight, rotMatrix );
			bottomBackLeft		= DirectX::XMVector3TransformCoord( bottomBackLeft, rotMatrix );
			bottomBackRight		= DirectX::XMVector3TransformCoord( bottomBackRight, rotMatrix );
			topFrontLeft		= DirectX::XMVector3TransformCoord( topFrontLeft, rotMatrix );
			topFrontRight		= DirectX::XMVector3TransformCoord( topFrontRight, rotMatrix );
			topBackLeft			= DirectX::XMVector3TransformCoord( topBackLeft, rotMatrix );
			topBackRight		= DirectX::XMVector3TransformCoord( topBackRight, rotMatrix );

			// Translate verticies
			if( inPosition.x != 0.f || inPosition.y != 0.f || inPosition.z != 0.f )
			{
				auto offset = DirectX::XMVectorSet( inPosition.x, inPosition.y, inPosition.z, 0.f );

				bottomFrontLeft		= DirectX::XMVectorAdd( bottomFrontLeft, offset );
				bottomFrontRight	= DirectX::XMVectorAdd( bottomFrontRight, offset );
				bottomBackLeft		= DirectX::XMVectorAdd( bottomBackLeft, offset );
				bottomBackRight		= DirectX::XMVectorAdd( bottomBackRight, offset );
				topFrontLeft		= DirectX::XMVectorAdd( topFrontLeft, offset );
				topFrontRight		= DirectX::XMVectorAdd( topFrontRight, offset );
				topBackLeft			= DirectX::XMVectorAdd( topBackLeft, offset );
				topBackRight		= DirectX::XMVectorAdd( topBackRight, offset );
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

	};

}