struct VSInput
{
    uint vertexID : SV_VERTEXID;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 colour : TEXCOORD;
};

struct PSOutput
{
    float4 colour : SV_TARGET;
};

cbuffer ViewProjection
{
    float4x4 ViewProjectionMatrix;
    float3 CameraPosition;
};

void VSMain(in VSInput input, out PSInput output)
{
    static float2 position[] =
    {
        // Up
        { -0.95f,  0.95f },
        {  0.00f,  0.05f },
        {  0.95f,  0.95f },
        
        // Down
        {  0.95f, -0.95f },
        {  0.00f, -0.05f },
        { -0.95f, -0.95f },
        
        // Left
        { -0.95f, -0.95f },
        { -0.05f,  0.00f },
        { -0.95f,  0.95f },
        
        // Right
        {  0.95f, -0.95f },
        {  0.05f,  0.00f },
        {  0.95f,  0.95f },
    };
    
    static float3 colour[] =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
    };
    
    output.pos = mul(ViewProjectionMatrix, float4(position[input.vertexID], 10.0f, 1.0f));
    output.colour = float4(colour[input.vertexID], 1.0f);
}

void PSMain(in PSInput input, out PSOutput output)
{
    output.colour = float4(input.colour, 1.0f);
}
