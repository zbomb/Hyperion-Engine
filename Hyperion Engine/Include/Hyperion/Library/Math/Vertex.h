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
	struct Vertex3D
	{
		float X, Y, Z;
		float tU, tV;
		float nX, nY, nZ;
		float tX, tY, tZ;
		float bX, bY, bZ;
	};

	/*
		structure VertexColor3D
	*/
	struct VertexColor3D
	{
		float X, Y, Z;
		float R, G, B, A;
	};

	/*
		structure Vertex2D
		- Data type that holds all info for a vertex used in a 2d context (UI)
	*/
	struct Vertex2D
	{
		float X, Y;
		float tU, tV;
	};

	/*
		structure VertexColor2D
		- Data type that holds a colored 2d vertex
	*/
	struct VertexColor2D
	{
		float X, Y;
		float R, G, B, A;
	};

}