#include "../Normal.hlsli"
#include "../PerlinNoise.hlsli"
#include "WaterTex2D.hlsli"

[maxvertexcount(3)]
void main(
	triangle DomainShaderOutPutToGeometory input[3],
	inout TriangleStream<GeometoryOutPut> outStream
){
	GeometoryOutPut output[3];
	uint32_t instanceID = input[0].instanceID;

	// お水の処理
	const float32_t2 kRandomVec = kWaterData[instanceID].randomVec;
    const float32_t kDensity = kWaterData[instanceID].density;

	for(uint32_t i = 0; i < 3; ++i){
		DomainShaderOutPutToGeometory inputTmp = input[i];

		// ワールドポジション計算
    	output[i].position = inputTmp.position;
		float32_t4 worldPosition = mul(output[i].position, kWvpMat[instanceID].worldMat);
		output[i].worldPosition = worldPosition;

		

		/* PerlinNoiseを使用した波
		float32_t wavePower = 6.0f;
		float32_t epsilon = 0.0001f;
		float32_t subUV = 1.0f * rcp(400.0f) * epsilon;
		float32_t height = CreateNoise(inputTmp.uv, kRandomVec, kDensity) * wavePower + waveHeight;
		float32_t up = CreateNoise(float32_t2(inputTmp.uv.x, inputTmp.uv.y + subUV), kRandomVec, kDensity) * wavePower + Waves(float32_t3(output[i].worldPosition.x, output[i].worldPosition.y, output[i].worldPosition.z + epsilon), instanceID);
		float32_t down = CreateNoise(float32_t2(inputTmp.uv.x, inputTmp.uv.y - subUV), kRandomVec, kDensity) * wavePower + Waves(float32_t3(output[i].worldPosition.x, output[i].worldPosition.y, output[i].worldPosition.z - epsilon), instanceID);
		float32_t right = CreateNoise(float32_t2(inputTmp.uv.x + subUV, inputTmp.uv.y), kRandomVec, kDensity) * wavePower + Waves(float32_t3(output[i].worldPosition.x + epsilon, output[i].worldPosition.y, output[i].worldPosition.z), instanceID);
		float32_t left = CreateNoise(float32_t2(inputTmp.uv.x - subUV, inputTmp.uv.y), kRandomVec, kDensity) * wavePower + Waves(float32_t3(output[i].worldPosition.x - epsilon, output[i].worldPosition.y, output[i].worldPosition.z), instanceID);
		*/
		
		float32_t wavePower = 0.2f;
		float32_t time = kWaterData[instanceID].time;

		// 波の高さ
		float32_t3 gerstner = GenerateWave(worldPosition.xyz, time) * wavePower;
		gerstner.y += Waves(worldPosition.xyz, instanceID);

		output[i].normal = GenerateNormal(worldPosition.xyz, kWaterData[instanceID].time, instanceID);;

		output[i].worldPosition.xyz += gerstner;

		
		output[i].position = mul(output[i].worldPosition, kWvpMat[instanceID].cameraMat);

		output[i].uv = inputTmp.uv;
		output[i].textureID = inputTmp.textureID;
		output[i].instanceID = inputTmp.instanceID;

		outStream.Append(output[i]);
	}

	outStream.RestartStrip();
}