using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion
{

	struct Vector2D
	{
		public float x;
		public float y;

		public Vector2D( float ix, float iy )
		{
			x = ix;
			y = iy;
		}
	}

	struct Vector3D
	{
		public float x;
		public float y;
		public float z;

		public Vector3D( float ix, float iy, float iz )
		{
			x = ix;
			y = iy;
			z = iz;
		}
	}

	struct Vector4D
	{
		public float x;
		public float y;
		public float z;
		public float w;

		public Vector4D( float ix, float iy, float iz, float iw )
		{
			x = ix;
			y = iy;
			z = iz;
			w = iw;
		}
	}


	struct Vertex
	{
		public float x;
		public float y;
		public float z;
		public float nx;
		public float ny;
		public float nz;
		public float u;
		public float v;

		public Vertex( float ix, float iy, float iz, float inx, float iny, float inz, float iu, float iv )
		{
			x = ix;
			y = iy;
			z = iz;
			nx = inx;
			ny = iny;
			nz = inz;
			u = iu;
			v = iv;
		}

		public Vector3D GetPosition()
		{
			return new Vector3D( x, y, z );
		}

		public Vector3D GetNormal()
		{
			return new Vector3D( nx, ny, nz );
		}

		public Vector2D GetTextureCoords()
		{
			return new Vector2D( u, v );
		}
	}
}
