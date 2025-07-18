#include "Line.hlsli"

struct VertxData {
	float4x4 wvp;
	float4 color;
};

StructuredBuffer<VertxData> vertxData: register(t0);

static const uint32_t kNumVertex = 2u;
static const float32_t4 kPositions[kNumVertex] = {
    {0.0f,0.0f,0.0f,1.0f},
    {1.0f,0.0f,0.0f,1.0f}
};

VertexOutput main(uint32_t vertexID : SV_VertexID, uint32_t instanceId : SV_InstanceID) {
    VertexOutput output;
    output.pos = mul(kPositions[vertexID],vertxData[instanceId].wvp);
    output.color = vertxData[instanceId].color;

    return output;
}