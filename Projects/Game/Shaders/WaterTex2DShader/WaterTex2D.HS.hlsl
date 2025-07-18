#include "WaterTex2D.hlsli"

ConstantHullShaderOutPut ConstantsHS_Main( InputPatch<VertexShaderOutputToHull, 3> input, uint32_t PatchID : SV_PrimitiveID ) {
	ConstantHullShaderOutPut output;
	uint32_t instanceID = input[0].instanceID;

	WaterData data = kWaterData[instanceID];

	// 頂点をワールド座標へ
	float32_t3 worldPositions[3];
	{
		for(int32_t i = 0; i < 3; ++i){
			worldPositions[i] = mul(input[i].worldPosition, kWvpMat[instanceID].worldMat).xyz;
		}
	}

	// ポリゴンエッジの座標
	float32_t3 edgePositions[3];
	edgePositions[0] = (worldPositions[0] + worldPositions[1]) * 0.5f;
	edgePositions[1] = (worldPositions[1] + worldPositions[2]) * 0.5f;
	edgePositions[2] = (worldPositions[2] + worldPositions[0]) * 0.5f;

	// 距離によるエッジの分割数を計算
	float32_t3 edgeFactors;
	{
		for(int32_t i = 0; i < 3; ++i){
			float32_t3 edgePos = edgePositions[i];
			float32_t posToCameraLength = length(edgePos - data.cameraPosition);

			float32_t edgeDivision = float32_t(data.edgeDivision) * ((data.distanceThreshold) * rcp(max(posToCameraLength, data.distanceThreshold)));
			edgeDivision = clamp(edgeDivision, 1.0f, float32_t(data.edgeDivision));

			edgeFactors[i] = edgeDivision;
		}
	}

	float32_t3 tessFactors = edgeFactors;

	// ポリゴンの結合方法が入力と反対方向で設定されているのでそれに合わせて出力
	output.Edges[0] = tessFactors.x;
	output.Edges[2] = tessFactors.y;
	output.Edges[1] = tessFactors.z;
	output.Inside = min(dot(tessFactors, float32_t3(1.0f, 1.0f, 1.0f)) * rcp(3.0f), data.insideDivision);

	return output;
}

[domain("tri")]
[partitioning("integer")] 
[outputtopology("triangle_ccw")] // 分割後のポリゴン結合方法の設定 左回り
[outputcontrolpoints(3)] // トポロジーコントロールポイント 
[patchconstantfunc("ConstantsHS_Main")] // 
HullShaderOutPut main( InputPatch<VertexShaderOutputToHull, 3> input, uint32_t i : SV_OutputControlPointID, uint32_t PatchID : SV_PrimitiveID ) {
	HullShaderOutPut output;
	uint32_t ID = 2 - i;

	output.position = input[ID].position;
	output.normal = input[ID].normal;
	output.worldPosition = input[ID].worldPosition;
	output.uv = input[ID].uv;
	output.textureID = input[ID].textureID;
	output.instanceID = input[ID].instanceID;

	return output;
}