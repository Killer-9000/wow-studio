struct VSInput
{
    float3 pos : ATTRIB0;
    float2 uv : ATTRIB1;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
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

static const int gridScale = 250;
static const float tileScale = 533.333f;

void VSMain(in VSInput input, out PSInput output)
{
    float3 cameraPos = clamp(CameraPosition, -tileScale * 64 - gridScale, tileScale * 64 + gridScale);
    float3 pos = float3(cameraPos.x + (input.pos.x * gridScale), input.pos.y, cameraPos.z + (input.pos.z * gridScale));
    output.pos = mul(ViewProjectionMatrix, float4(pos, 1.0f));
    output.uv = input.uv;
}

void PSMain(in PSInput input, out PSOutput output)
{
    const static float LineThickness = 0.02f;
    const static float LineRepetition = (float) gridScale / 10000;
    if (distance(float2(0.5f, 0.5f), input.uv) < 0.49f)
    {
        if (frac(input.uv.x / LineRepetition) <= LineThickness || frac(input.uv.y / LineRepetition) <= LineThickness)
        {
            output.color = float4(1.0f);
            output.color.a = 1.0f - clamp(distance(float2(0.5f, 0.5f), input.uv) * 2, 0.4f, 1.0f);
            return;
        }
    }
    output.color = float4(0.0f);
};
