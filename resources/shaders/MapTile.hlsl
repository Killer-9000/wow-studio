
const static float NaN = 0.0f / 0.0f;

struct VSInput
{
    float3 pos : ATTRIB0;
    uint vertexIndex : SV_VertexID;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 colour;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer ViewProjection
{
    float4x4 ViewProjectionMatrix;
    float3 CameraPosition;
}

cbuffer TileData
{
    bool _discard;
    bool hole[9 * 9 + 8 * 8];
    float height[9 * 9 + 8 * 8];
    float4 colour[9 * 9 + 8 * 8];
    float3 normal[9 * 9 + 8 * 8];
    bool shadow[9 * 9 + 8 * 8];
};

void VSMain(in VSInput input, out PSInput output)
{
    if (hole[input.vertexIndex] || _discard)
    {
        output.pos = float4(NaN);
        return;
    }
    input.pos.y += height[input.vertexIndex];
    output.pos = mul(ViewProjectionMatrix, float4(input.pos, 1.0f));
    
    output.colour = colour[input.vertexIndex];
    output.colour -= (float) shadow[input.vertexIndex] / 0.8f;
}

void PSMain(in PSInput input, out PSOutput output)
{
    output.color = float4(1.0f);
};
