using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{

	struct ModelSubObject
	{
		public Vertex[] VertexList;
		public uint[] IndexList;

		public byte MaterialSlot;
	}

	struct ModelLOD
	{
		public float MinScreenSize;
		public ModelSubObject[] SubObjects;
	}

	class StaticModel
	{
		// Bounding Info
		public Vector3D BoundingSphereCenter;
		public float BoundingSphereRadius;
		public Vector3D AxisAlignedBoundsMin;
		public Vector3D AxisAlignedBoundsMax;

		public ModelLOD[] LODList;

		// Editor Stuff
		public string Path;
	}

	static class ModelManager
	{
		public static uint StaticModelMinSize = 220;


		public static void RegisterConsoleCommands()
		{
			CommandInfo debugModelCommand = new CommandInfo
			{
				Base = "model_create_debug",
				Usage = "model_create_debug [path]",
				Description = "Creates a basic static model and writes it to file",
				MinArgs = 1,
				MaxArgs = -1,
				Callback = CreateDebugModel
			};

			Core.RegisterCommand( debugModelCommand );
		}

		public static void OpenImporter()
		{

		}

		public static void CloseImporter()
		{

		}

		public static bool IsImporterOpen()
		{
			return false;
		}


		public static void CreateDebugModel( string[] inArgs )
		{
			string path = String.Concat( inArgs );
			if( !path.EndsWith( ".hsm" ) )
			{
				Core.WriteLine( "=> Failed to write debug model, path invalid and doesnt end with .hsm" );
				return;
			}

			// So.... lets just create a model thats a cube, single LOD, single subobject, single material slot
			StaticModel model = new StaticModel();
			model.AxisAlignedBoundsMin = new Vector3D( 0.0f, 0.0f, 0.0f );
			model.AxisAlignedBoundsMax = new Vector3D( 5.0f, 5.0f, 5.0f );
			model.BoundingSphereCenter = new Vector3D( 2.5f, 2.5f, 2.5f );
			model.BoundingSphereRadius = 4.331f;
			model.Path = path;
			model.LODList = new ModelLOD[ 1 ];
			model.LODList[ 0 ].MinScreenSize = 0.0f;
			model.LODList[ 0 ].SubObjects = new ModelSubObject[ 1 ];

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList = new Vertex[ 36 ];
			model.LODList[ 0 ].SubObjects[ 0 ].IndexList = new uint[ 36 ]
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
				24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
			};
			model.LODList[ 0 ].SubObjects[ 0 ].MaterialSlot = 0;

			// Front Face, Bottom Triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 0 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 1 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 2 ].v = 1.0f;

			// Front Face, Top Triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 3 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 4 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].nz = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 5 ].v = 1.0f;

			// Right face, bottom triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 6 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 7 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 8 ].v = 1.0f;

			// Right face, top triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 9 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 10 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].nx = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 11 ].v = 1.0f;

			// Left face, bottom triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 12 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 13 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 14 ].v = 1.0f;

			// Left face, top triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 15 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 16 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].nx = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 17 ].v = 1.0f;

			// Top face, back triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 18 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 19 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 20 ].v = 1.0f;

			// Top face, front triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 21 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 22 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].ny = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 23 ].v = 1.0f;

			// Bottom face, back triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 24 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 25 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 26 ].v = 1.0f;

			// Bottom face, front triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 27 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 28 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].z = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].ny = -1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].nz = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 29 ].v = 0.0f;

			// Back face, bottom triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 30 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 31 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 32 ].v = 1.0f;

			// Back face, top triangle
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].x = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].u = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 33 ].v = 1.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].y = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 34 ].v = 0.0f;

			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].x = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].y = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].z = 5.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].nx = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].ny = 0.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].nz = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].u = 1.0f;
			model.LODList[ 0 ].SubObjects[ 0 ].VertexList[ 35 ].v = 1.0f;

			Write( model );
		}


		public static StaticModel Read( string inPath )
		{
			if( ( inPath?.Length ?? 0 ) < 5 || !inPath.EndsWith( ".hsm" ) )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file.. path was invalid" );
				return null;
			}

			// Figure out the clean and full paths
			string cleanPath = inPath;
			if( cleanPath.StartsWith( "content/" ) || cleanPath.StartsWith( "Content/" ) )
			{
				cleanPath = cleanPath.Substring( 8 );
			}

			string fullPath = "content/" + cleanPath;

			// Next, check if the file exists
			if( !File.Exists( fullPath ) )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file! File didnt exist at \"", fullPath, "\"" );
				return null;
			}

			// Open the file, and check if its long enough
			byte[] fileData = null;

			try
			{
				using( var f = File.OpenRead( fullPath ) )
				{
					using( var reader = new BinaryReader( f ) )
					{
						fileData = reader.ReadBytes( (int)f.Length );
					}
				}

				if( fileData == null ) { throw new Exception( "File data was null?" ); }
			}
			catch( Exception e )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file! The file couldnt be opened (", e.Message, ")" );
				return null;
			}

			// Ensure the file is long enough for the model file
			if( fileData.Length < StaticModelMinSize )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file! There wasnt enough data!" );
				return null;
			}

			// Read validation bytes to ensure its a valid model file, and also the byte order
			int offset			= 0;
			ulong validBytes    = Serialization.GetUInt64( fileData, offset, false );
			bool bLittleEndian	= false;

			if( validBytes == 0x1000D99D28FFA1 )
			{
				bLittleEndian = true;
			}
			else if( validBytes != 0x1AA1FF289DD90010 )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file! \"", fullPath, "\" is not a valid model file" );
				return null;
			}

			offset += 8;

			// Skip 8 bytes of padding
			offset += 8;

			// Next, check the toolset version
			UInt16 toolsetVersion = Serialization.GetUInt16( fileData, offset, bLittleEndian );
			if( toolsetVersion < 0x00001 )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file because the toolset version is incompatible" );
				return null;
			}

			offset += 2;

			byte lodCount = Serialization.GetUInt8( fileData, offset );
			if( lodCount == 0 || lodCount > 25 )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file because the LOD count is invalid" );
				return null;
			}

			offset++;

			// 5 bytes of reserved space
			offset += 5;

			// Create the model object
			var Output = new StaticModel();

			// Read bounding sphere information
			Output.BoundingSphereCenter = Serialization.GetVector3D( fileData, offset, bLittleEndian );
			offset += 12;

			Output.BoundingSphereRadius = Serialization.GetFloat( fileData, offset, bLittleEndian );
			offset += 4;

			// Axis aligned bounding box
			Output.AxisAlignedBoundsMin = Serialization.GetVector3D( fileData, offset, bLittleEndian );
			offset += 12;

			Output.AxisAlignedBoundsMax = Serialization.GetVector3D( fileData, offset, bLittleEndian );
			offset += 12;

			// Ensure theres enough data to read in all of the LODs
			uint lodEnd = (uint)offset + ( (uint)lodCount * 24 );
			if( lodEnd > fileData.Length )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to read model file, hit end of file!" );
				return null;
			}

			// Now we need to loop through and read info about each LOD
			Output.LODList = new ModelLOD[ lodCount ];
			int subobj = -1;

			for( byte lod = 0; lod < lodCount; lod++ )
			{
				// First, we need to read the minimum screen size
				Output.LODList[ lod ].MinScreenSize = Serialization.GetFloat( fileData, offset, bLittleEndian );
				offset += 4;

				uint subobjOffset = Serialization.GetUInt32( fileData, offset, bLittleEndian );
				offset += 4;

				byte subobjCount = Serialization.GetUInt8( fileData, offset );
				offset++;

				// 3 bytes of reserved space
				offset += 3;

				// Collision data (NOT IMPLEMENETED) 12 bytes
				offset += 12;

				// Validate the number of subobjects
				if( subobj < 0 )
				{
					subobj = ( int ) subobjCount;
					if( subobj <= 0 )
					{
						Core.WriteLine( "[Warning] Failed to read model file, invalid number of subobject in LOD ", (uint) lod );
						return null;
					}
				}
				else
				{
					if( subobj != (int) subobjCount )
					{
						Core.WriteLine( "[Warning] Failed to read model file, inconsistent number of subobjects in LOD ", ( uint ) lod );
						return null;
					}
				}

				// Ensure the offset + length is within bounds
				uint subObjEnd = subobjOffset + ( (uint)subobjCount * 24 );
				if( subObjEnd > fileData.Length )
				{
					Core.WriteLine( "[Warning] Failed to read model file, ran out of data!" );
					return null;
				}

				// Loop through and read subobjects
				Output.LODList[ lod ].SubObjects = new ModelSubObject[ subobjCount ];
				for( byte obj = 0; obj < subobjCount; obj++ )
				{
					int objOffset = (int)subobjOffset + ( (int) obj * 24 );

					// Skip object id (4 bytes)
					objOffset += 4;

					// Read material slot
					Output.LODList[ lod ].SubObjects[ obj ].MaterialSlot = Serialization.GetUInt8( fileData, objOffset );
					objOffset++;

					// Skip reserved space (3 bytes)
					objOffset += 3;

					// Vertex buffer offset and length
					uint vertOffset = Serialization.GetUInt32( fileData, objOffset, bLittleEndian );
					objOffset += 4;

					uint vertCount = Serialization.GetUInt32( fileData, objOffset, bLittleEndian );
					objOffset += 4;

					// Index buffer offset and length
					uint indOffset = Serialization.GetUInt32( fileData, objOffset, bLittleEndian );
					objOffset += 4;

					uint indCount = Serialization.GetUInt32( fileData, objOffset, bLittleEndian );
					objOffset += 4;

					// Now that we have the offsets, we need to load in the buffers
					int voff = (int)vertOffset;
					int ioff = (int)indOffset;
					uint vindex = 0;
					uint iindex = 0;

					// Check bounds for the vertex and index lists
					if( voff + ( vertCount * 32 ) > fileData.Length ||
						ioff + ( indCount * 4 ) > fileData.Length )
					{
						Core.WriteLine( "[Warning] ModelManager: Failed to read model file! Index/vertex data went out of line" );
						return null;
					}

					Output.LODList[ lod ].SubObjects[ obj ].VertexList	= new Vertex[ vertCount ];
					Output.LODList[ lod ].SubObjects[ obj ].IndexList	= new uint[ indCount ];

					while( vindex < vertCount )
					{
						Output.LODList[ lod ].SubObjects[ obj ].VertexList[ vindex ] = Serialization.GetVertex( fileData, voff ); // Verticies and indicies are little-endian always
						voff += 32;
						vindex++;
					}
					
					while( iindex < indCount )
					{
						Output.LODList[ lod ].SubObjects[ obj ].IndexList[ iindex ] = Serialization.GetUInt32( fileData, ioff, true ); // Indicies are always little-endian
						ioff += 4;
						iindex++;
					}
				}
			}

			Core.WriteLine( "=> Loaded static model \"", fullPath, "\" from disk!" );
			return Output;
		}


		public static bool Write( StaticModel inModel )
		{
			// Validate parameter
			if( inModel == null || ( inModel.Path?.Length ?? 0 ) < 5 )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. model was null or path was null" );
				return false;
			}

			// Fix up the target path
			string cleanPath = inModel.Path;
			if( cleanPath.StartsWith( "content/" ) || cleanPath.StartsWith( "content/" ) )
			{
				cleanPath = cleanPath.Substring( 8 );
			}

			var oldExt = Path.GetExtension( cleanPath );
			if( ( oldExt?.Length ?? 0 ) == 0 )
			{
				cleanPath = cleanPath + ".hsm";
			}
			else if( oldExt != ".hsm" )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. invalid extension specified" );
				return false;
			}

			string fullPath = "content/" + cleanPath;

			// Now, lets validate the actual model data, every vertex buffer needs to contain at least 3 entries, every index buffer at least 3 entries
			// and the same number of subobjects in every LOD. Also, at least a single LOD
			if( ( inModel.LODList?.Length ?? 0 ) == 0 )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. there are no valid LODs!" );
				return false;
			}

			byte index		= 0;
			int subcount	= -1;

			foreach( var lod in inModel.LODList )
			{
				if( lod.MinScreenSize < 0.0f )
				{
					Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. invalid screen size set for LOD ", ( uint ) index );
					return false;
				}

				var thisCount = lod.SubObjects?.Length ?? 0;

				if( subcount < 0 )
				{
					if( thisCount <= 0 )
					{
						Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. invalid number of sub-objects! ", thisCount );
						return false;
					}

					subcount = thisCount;
				}
				else
				{
					if( thisCount != subcount )
					{
						Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. subobject count wasnt consistent among LODs!" );
						return false;
					}
				}

				for( byte i = 0; i < thisCount; i++ )
				{
					var vlen	= lod.SubObjects[ i ].VertexList?.Length ?? 0;
					var ilen	= lod.SubObjects[ i ].IndexList?.Length ?? 0;

					if( vlen < 3 || ilen < 3 )
					{
						Core.WriteLine( "[Warning] ModelManager: Failed to write model to file.. vertex/index list invalid for LOD ", index, " at Object ", i );
						return false;
					}
				}

				index++;
			}

			// Now, that the model seems valid, lets start to serialize it, so we can write it to file
			// NOTE: Model files are always stored as little endian!
			List< byte > modelData = new List< byte >
			{
				0x10, 0x00, 0xD9, 0x9D, 0x28, 0xFF, 0xA1, 0x1A,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};

			// Toolset Version
			modelData.AddRange( new byte[] { 0x01, 0x00 } );

			// LOD List Count
			modelData.Add( (byte) inModel.LODList.Length );

			// Reserved Space
			modelData.AddRange( new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00 } );

			// Bounding Sphere
			modelData.AddRange( Serialization.FromVector3D( inModel.BoundingSphereCenter, true ) );
			modelData.AddRange( Serialization.FromFloat( inModel.BoundingSphereRadius, true ) );

			// Bounding Box
			modelData.AddRange( Serialization.FromVector3D( inModel.AxisAlignedBoundsMin, true ) );
			modelData.AddRange( Serialization.FromVector3D( inModel.AxisAlignedBoundsMax, true ) );

			// We need to calculate how long the static data + lod data + object data will be
			// This way we can properly calculate file offsets for the buffers
			uint staticDataLength	= 64;
			uint lodDataLength		= 24 * (uint)inModel.LODList.Length;
			uint objDataLength      = 24 * (uint)( inModel.LODList.Length * subcount );

			// Next, we need to generate the 3 other data sections left
			List< byte > mainData	= new List< byte >();
			List< byte > objData	= new List< byte >();
			List< byte > lodData	= new List< byte >();

			for( byte lod = 0; lod < inModel.LODList.Length; lod++ )
			{
				// Store the offset for the subobject list
				uint objOffset = staticDataLength + lodDataLength + (uint)objData.Count;

				for( byte obj = 0; obj < inModel.LODList[ lod ].SubObjects.Length; obj++ )
				{
					// Write out the vertex and idnex lists to the buffer section
					// Also, calculate the offsets to be written in the object after
					uint vertOffset = staticDataLength + lodDataLength + objDataLength + (uint) mainData.Count;
					foreach( Vertex vert in inModel.LODList[ lod ].SubObjects[ obj ].VertexList )
					{
						mainData.AddRange( Serialization.FromVertex( vert ) ); // Verticies are always little-endian
					}

					uint indxOffset = staticDataLength + lodDataLength + objDataLength + ( uint ) mainData.Count;
					foreach( uint idx in inModel.LODList[ lod ].SubObjects[ obj ].IndexList )
					{
						mainData.AddRange( Serialization.FromUInt32( idx, true ) ); // Indicies are always little-endian
					}

					// Now, lets serialize the object
					// Object Identifier
					objData.AddRange( Serialization.FromUInt32( obj, true ) );

					// Material Slot
					objData.Add( inModel.LODList[ lod ].SubObjects[ obj ].MaterialSlot );

					// 3 bytes of reserved space
					objData.AddRange( new byte[] { 0x00, 0x00, 0x00 } );

					// Vertex offset and count
					objData.AddRange( Serialization.FromUInt32( vertOffset, true ) );
					objData.AddRange( Serialization.FromUInt32( (uint)inModel.LODList[ lod ].SubObjects[ obj ].VertexList.Length, true ) );

					// Index offset and count
					objData.AddRange( Serialization.FromUInt32( indxOffset, true ) );
					objData.AddRange( Serialization.FromUInt32( ( uint ) inModel.LODList[ lod ].SubObjects[ obj ].IndexList.Length, true ) );
				}

				// Now, lets serialize the LOD
				// Min Screen Size
				lodData.AddRange( Serialization.FromFloat( inModel.LODList[ lod ].MinScreenSize, true ) );

				// Subobject offset and count (NOT BYTES)
				lodData.AddRange( Serialization.FromUInt32( objOffset, true ) );
				lodData.Add( ( byte ) inModel.LODList[ lod ].SubObjects.Length );

				// 3 Bytes of padding
				lodData.AddRange( new byte[] { 0x00, 0x00, 0x00 } );

				// Collision Data
				// TODO: Not implemented, so just 12 bytes of padding
				lodData.AddRange( new byte[] { 
					0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00 
				} );
			}

			// Now, we need to open the file and write it out
			if( File.Exists( fullPath ) )
			{
				Core.WriteLine( "=> Overwriting existing model file \"", fullPath, "\"" );
			}

			try
			{
				Directory.CreateDirectory( Path.GetDirectoryName( fullPath ) );
				using( var f = File.Open( fullPath, FileMode.Create, FileAccess.Write ) )
				{
					using( var writer = new BinaryWriter( f ) )
					{
						// First, write out the static section
						writer.Write( modelData.ToArray() );

						// Next, write out the LOD list
						writer.Write( lodData.ToArray() );

						// Then, write out the subobject list
						writer.Write( objData.ToArray() );

						// Finally, write out the buffer data
						writer.Write( mainData.ToArray() );

						writer.Flush();
					}
				}
			}
			catch( Exception e )
			{
				Core.WriteLine( "[Warning] ModelManager: Failed to write model file (", e.Message, ")" );
				return false;
			}

			Core.WriteLine( "=> Saved static model to \"", fullPath, "\"" );

			AssetIdentifierCache.RegisterAsset( cleanPath );
			return true;
		}

	}
}
