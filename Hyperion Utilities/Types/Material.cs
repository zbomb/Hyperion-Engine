using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{

    struct TextureReference
    {
        public string Path;
        public uint Hash;
    }

    enum MaterialPropertyType
    {
        Boolean = 0,
        Int32 = 1,
        UInt32 = 2,
        Float = 3,
        String = 4,
        Texture = 5
    }


    class Material
    {

        public Dictionary< string, object > Properties;
        public string Path { get; private set; }
        public uint Identifier { get; private set; }

        public Material( string inPath, uint inId )
        {
            Properties  = new Dictionary< string, object >();
            Path        = inPath;
            Identifier  = inId;
        }

        


    }
}
