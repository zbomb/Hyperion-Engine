using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Hyperion
{
    static class Hashing
    {

        public static uint ELFHash( byte[] inData )
        {
			uint output = 0;
			uint x = 0;

			foreach( var b in inData )
			{ 
				output = ( output << 4 ) + b;
				if( ( x = output & 0xF0000000 ) != 0 )
				{
					output ^= ( x >> 24 );
				}

				output &= ~x;
			}

			return output;
		}

    }
}
