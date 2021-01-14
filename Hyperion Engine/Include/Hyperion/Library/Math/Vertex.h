/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Library/Math/Vertex.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once



namespace Hyperion
{
	/*
		structure Vertex3D
		- Data type that holds all info for a vertex inside a model file
		- Same structure used to pass data directly into the graphics API
	*/
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

	/*
		structure VertexColor3D
	*/
	#pragma pack( push, 1 )
	struct VertexColor3D
	{
		float x, y, z;
		float r, g, b, a;
	};
	#pragma pack( pop )

	/*
		structure Vertex2D
		- Data type that holds all info for a vertex used in a 2d context (UI)
	*/
	#pragma pack( push, 1 )
	struct Vertex2D
	{
		float x, y;
		float u, v;
	};
	#pragma pack( pop )

	/*
		structure VertexColor2D
		- Data type that holds a colored 2d vertex
	*/
	#pragma pack( push, 1 )
	struct VertexColor2D
	{
		float x, y;
		float r, g, b, a;
	};
	#pragma pack( pop )

}