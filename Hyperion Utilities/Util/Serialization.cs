using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion
{
    enum StringEncoding
    {
        ASCII = 0,
        UTF8 = 1,
        UTF16_LE = 2,
        UTF16_BE = 3,
        UTF32_LE = 4,
        UTF32_BE = 5
    };

    static class Serialization
    {
        public static byte GetUInt8( byte[] inData, int inOffset = 0 )
        {
            if( inData.Length <= inOffset )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize UInt8, not enough data" );
                return 0;
            }

            return inData[ inOffset ];
        }

        public static UInt16 GetUInt16( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 2 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize UInt16, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 2 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 2 );
                Array.Reverse( data );

                return BitConverter.ToUInt16( data, 0 );
            }
            else
            {
                return BitConverter.ToUInt16( inData, inOffset );
            }
        }

        public static UInt32 GetUInt32( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 4 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize UInt32, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 4 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 4 );
                Array.Reverse( data );

                return BitConverter.ToUInt32( data, 0 );
            }
            else
            {
                return BitConverter.ToUInt32( inData, inOffset );
            }
        }


        public static UInt64 GetUInt64( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 8 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize UInt64, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 8 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 8 );
                Array.Reverse( data );

                return BitConverter.ToUInt64( data, 0 );
            }
            else
            {
                return BitConverter.ToUInt64( inData, inOffset );
            }
        }

        public static Int16 GetInt16( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 2 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize Int16, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 2 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 2 );
                Array.Reverse( data );

                return BitConverter.ToInt16( data, 0 );
            }
            else
            {
                return BitConverter.ToInt16( inData, inOffset );
            }
        }

        public static Int32 GetInt32( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 4 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize Int32, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 4 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 4 );
                Array.Reverse( data );

                return BitConverter.ToInt32( data, 0 );
            }
            else
            {
                return BitConverter.ToInt32( inData, inOffset );
            }
        }


        public static Int64 GetInt64( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 8 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize UInt64, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 8 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 8 );
                Array.Reverse( data );

                return BitConverter.ToInt64( data, 0 );
            }
            else
            {
                return BitConverter.ToInt64( inData, inOffset );
            }
        }



        public static float GetFloat( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 4 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize float, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 4 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 4 );
                Array.Reverse( data );

                return BitConverter.ToSingle( data, 0 );
            }
            else
            {
                return BitConverter.ToSingle( inData, inOffset );
            }
        }


        public static double GetDouble( byte[] inData, int inOffset = 0, bool bIsLittleEndian = false )
        {
            if( inData.Length - inOffset < 8 )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize float, not enough data!" );
                return 0;
            }

            if( BitConverter.IsLittleEndian != bIsLittleEndian )
            {
                byte[] data = new byte[ 8 ];
                Array.ConstrainedCopy( inData, inOffset, data, 0, 8 );
                Array.Reverse( data );

                return BitConverter.ToDouble( data, 0 );
            }
            else
            {
                return BitConverter.ToDouble( inData, inOffset );
            }
        }


        public static bool GetBoolean( byte[] inData, int inOffset = 0 )
        {
            if( inData.Length <= inOffset )
            {
                Core.WriteLine( "[Warning] Serailization: Failed to deserialize boolean, not enough data!" );
                return false;
            }

            return inData[ inOffset ] != 0;
        }


        public static string GetString( byte[] inData, int inOffset = 0, int inLength = 0, StringEncoding inEncoding = StringEncoding.ASCII )
        {
            if( inLength == 0 ) { inLength = inData.Length - inOffset; }

            if( inData.Length < inOffset + inLength )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize string, not enough data!" );
                return null;
            }

            try
            {
                if( inEncoding == StringEncoding.ASCII )
                {
                    return Encoding.ASCII.GetString( inData, inOffset, inLength );
                }
                else if( inEncoding == StringEncoding.UTF8 )
                {
                    return Encoding.UTF8.GetString( inData, inOffset, inLength );
                }
                else if( inEncoding == StringEncoding.UTF16_LE )
                {
                    return Encoding.Unicode.GetString( inData, inOffset, inLength );
                }
                else if( inEncoding == StringEncoding.UTF16_BE )
                {
                    return Encoding.BigEndianUnicode.GetString( inData, inOffset, inLength );
                }
                else if( inEncoding == StringEncoding.UTF32_LE )
                {
                    return Encoding.UTF32.GetString( inData, inOffset, inLength );
                }
                else if( inEncoding == StringEncoding.UTF32_BE )
                {
                    var e = new UTF32Encoding( true, false );
                    return e.GetString( inData, inOffset, inLength );
                }
                else
                {
                    throw new Exception( "Unkown encoding type" );
                }
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[Warning] Serialization: Failed to deserialize ", Enum.GetName( typeof( StringEncoding ), inEncoding ), " string! ", Ex.Message );
                return null;
            }
        }


        public static byte[] FromBoolean( bool inValue )
        {
            byte[] outData = new byte[ 1 ];
            if( inValue ) { outData[ 0 ] = 1; } else { outData[ 0 ] = 0; }
            return outData;
        }

        public static byte[] FromUInt8( byte inValue )
        {
            return new byte[] { inValue };
        }

        public static byte[] FromUInt16( UInt16 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromInt16( Int16 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromUInt32( UInt32 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromInt32( Int32 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromUInt64( UInt64 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromInt64( Int64 inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromFloat( float inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromDouble( double inValue, bool bLittleEndian = false )
        {
            byte[] data = BitConverter.GetBytes( inValue );
            if( BitConverter.IsLittleEndian != bLittleEndian )
            {
                Array.Reverse( data );
            }

            return data;
        }

        public static byte[] FromString( string inValue, StringEncoding inEncoding = StringEncoding.ASCII )
        {
            switch( inEncoding )
            {
                case StringEncoding.ASCII:
                    return Encoding.ASCII.GetBytes( inValue );
                case StringEncoding.UTF8:
                    return Encoding.UTF8.GetBytes( inValue );
                case StringEncoding.UTF16_LE:
                    return Encoding.Unicode.GetBytes( inValue );
                case StringEncoding.UTF16_BE:
                    return Encoding.BigEndianUnicode.GetBytes( inValue );
                case StringEncoding.UTF32_LE:
                    return Encoding.UTF32.GetBytes( inValue );
                case StringEncoding.UTF32_BE:
                    return new UTF32Encoding( true, false ).GetBytes( inValue );
                default:
                    Core.WriteLine( "[Warning] Serialization: Failed to serialize string, invalid string encoding (", Enum.GetName( typeof( StringEncoding ), inEncoding ), ")" );
                    return null;
            }
        }


    }
}
