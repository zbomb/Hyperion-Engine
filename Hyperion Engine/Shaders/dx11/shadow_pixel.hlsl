////////////////////////////////////////////////////
//		Hyperion Engine	
//		Shadow Pixel Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


///////////////////////////
//	Structures
///////////////////////////

struct PixelInput
{
	float4 Position	: SV_Position;
};


struct PixelOutput
{
	float4 DepthOutput : SV_Target0;
};


///////////////////////////////
//	Entry Point
///////////////////////////////

PixelOutput main( PixelInput input )
{
	PixelOutput output;

	//output.DepthOutput.x = input.Position.z;
	//output.DepthOutput.y = input.Position.z * input.Position.z;
	output.DepthOutput.x = input.Position.z;
	output.DepthOutput.y = input.Position.z;
	output.DepthOutput.z = input.Position.z;
	output.DepthOutput.w = 1.f;

	return output;
}