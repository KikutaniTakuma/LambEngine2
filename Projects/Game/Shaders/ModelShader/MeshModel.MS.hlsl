#include "../LambMesh.hlsli"

uint32_t3 UnpackPrimitive(uint32_t primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint32_t3(primitive & 0x3ff, (primitive >> 10) & 0x3ff, (primitive >> 20) & 0x3ff);
}

uint32_t3 GetPrimitive(Meshlet m, uint32_t index)
{
    return UnpackPrimitive(gPrimitiveIndices[m.primitiveOffset + index]);
}

uint32_t GetVertexIndex(Meshlet m, uint32_t localIndex)
{
    localIndex = m.vertexOffset + localIndex;

	return gUniqueVertexIndices[localIndex];
}

[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main(
	uint32_t groupThreadID : SV_GroupThreadID,
    uint32_t groupID : SV_GroupID,
	in payload PayloadStruct meshPayload,
    out vertices MSOutput verts[64],
	out indices uint32_t3 tris[126]
){
	Meshlet meshlet = gMeshlets[meshPayload.myArbitraryData];

	// スレッドグループの頂点数とプリミティブ数
	// mainメソッド内で一回だけ呼び出し可能
	SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

	if(groupThreadID < meshlet.primitiveCount){
		tris[groupThreadID] = GetPrimitive(meshlet, groupThreadID);
	}

	if(groupThreadID < meshlet.vertexCount) {
		uint32_t index = GetVertexIndex(meshlet, groupThreadID);
		MSInput input = gVertices[index];
		MSOutput output = (MSOutput)0;

		float32_t4 localPos = input.position;
		float32_t4 worldPos = mul(localPos, gTransform[groupID].worldMat);
		float32_t4 projPos = mul(worldPos, gTransform[groupID].cameraMat);

		output.position = projPos;
		output.worldPosition = worldPos;

		output.normal = mul(input.normal, (float32_t3x3)gTransform[groupID].worldMat);
		output.normal = normalize(output.normal);
		output.uv = input.uv;
		output.textureID = input.textureID;

		verts[groupThreadID] = output;
		verts[groupThreadID].instanceID = groupID;
	}
}