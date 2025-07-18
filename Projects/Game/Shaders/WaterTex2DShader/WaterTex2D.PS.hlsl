#include "WaterTex2D.hlsli"
#include "../PerlinToNormal.hlsli"
#include "../Normal.hlsli"
#include "../SkyBoxShader/AirSkyBox.hlsli"

#define PI 3.14159265358f

float32_t3 SkyColor(uint32_t instanceID, float32_t3 gWorldPos, float32_t3 gNormal, float32_t3 cameraPos){
    const float32_t g = kWaterData[instanceID].atmosphericParams.mieG;
	const float32_t g2 = g * g;
	const float32_t kRayleighScattering = kWaterData[instanceID].atmosphericParams.rayleighScattering;
	const float32_t kMieScattering = kWaterData[instanceID].atmosphericParams.mieScattering;

	const float32_t kRayleighSun = kRayleighScattering * kESun;
	const float32_t kMieSun = kMieScattering * kESun;
	const float32_t kRayleigh4PI = kRayleighScattering * 4.0f * PI;
	const float32_t kMie4PI = kMieScattering * 4.0f * PI;

    float32_t3 cameraToPosition = normalize(gWorldPos - cameraPos);
    float32_t3 reflectedVector = reflect(cameraToPosition, gNormal);

	float32_t3 worldPos = reflectedVector + normalize(gWorldPos);
	worldPos = IntersectionPos(normalize(worldPos), float32_t3(0.0f, kInnerRadius, 0.0f), kOuterRadius);
	cameraPos.y += kInnerRadius;
    float32_t3 viewDirection = normalize(worldPos - cameraPos);
    float32_t3 lightDirection = normalize(kWaterData[instanceID].atmosphericParams.lightDirection + viewDirection);

	float32_t3 v3Ray = worldPos - cameraPos;
	float32_t fFar = length(v3Ray);
	v3Ray = normalize(v3Ray);

    if(dot(v3Ray, normalize(cameraPos)) < -0.07f){
		float32_t3 output = 0.0;

		return output;
	}

	float32_t3 v3Start = cameraPos;
	float32_t fCameraHeight = length(cameraPos);
	float32_t fStartAngle = dot(v3Ray, v3Start) * rcp(fCameraHeight);
	float32_t fStartDepth = exp(kScaleOverScaleDepth * (kInnerRadius - fCameraHeight));
	float32_t fStartOffset = fStartDepth * Scale(fStartAngle);
 
	float32_t fSampleLength = fFar * rcp(fSamples);
	float32_t fScaledLength = fSampleLength * kScale;
	float32_t3 v3SampleRay = v3Ray * fSampleLength;
	float32_t3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;
 
	float32_t3 v3FrontColor = 0.0;
	for(int32_t n = 0; n < int32_t(fSamples); n++){
		float32_t fHeight = length(v3SamplePoint);
		float32_t fDepth = exp(kScaleOverScaleDepth * (kInnerRadius - fHeight));
		float32_t fLightAngle = dot(lightDirection, v3SamplePoint) * rcp(fHeight);
		float32_t fCameraAngle = dot(v3Ray, v3SamplePoint) * rcp(fHeight);
		float32_t fScatter = (fStartOffset + fDepth * (Scale(fLightAngle) - Scale(fCameraAngle)));
		float32_t3 v3Attenuate = exp(-fScatter * (kV3InvWaveLength * kRayleigh4PI + kMie4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
 
	float32_t3 c0 = v3FrontColor * (kV3InvWaveLength * kRayleighSun);
	float32_t3 c1 = v3FrontColor * kMieSun;
	float32_t3 v3Direction = cameraPos - worldPos;
 
	float32_t fcos = dot(lightDirection, v3Direction) * rcp(length(v3Direction));
	float32_t fcos2 = fcos * fcos;
 
	float32_t rayleighPhase = 0.75f * (1.0f + fcos2);
	float32_t miePhase = 1.5f * ((1.0f - g2) * rcp(2.0f + g2)) * (1.0f + fcos2) * rcp(pow(1.0f + g2 - 2.0f * g * fcos, 1.5f));

	float32_t3 output;
	output = rayleighPhase * c0 + miePhase * c1;
	output = min(output, float32_t3(1.0f, 1.0f, 1.0f));

    return output;
}

PixelShaderOutPut4 main(GeometoryOutPut input)
{
	PixelShaderOutPut4 output;

    //float32_t3 skyColor = SkyColor(input.instanceID, input.worldPosition.xyz, input.normal, kWaterData[input.instanceID].atmosphericParams.cameraPosition);

    // お水の処理
	const float32_t2 kRandomVec = kWaterData[input.instanceID].randomVec;
    const float32_t kDensity = kWaterData[input.instanceID].density;
    
    float32_t3 perlinNormal = CreateNormal(input.uv, kRandomVec, kDensity);
    float32_t3 normal = input.normal;
    // float32_t3 normal = GenerateNormal(input.worldPosition.xyz, kWaterData[input.instanceID].time, input.instanceID);
    // float32_t3 tangent = NormalToTangent(perlinNormal);
    // float32_t3 binormal = CalcBinormal(perlinNormal, tangent);
    // float32_t3 blendNormal = BlendNormal(perlinNormal, tangent, binormal, normal);

    float32_t3 ligDirection = kLight.ligDirection;
    float32_t3 ligColor = kLight.ligColor;
    float32_t shinness = kLight.shinness;
 
    // ディレクションライト拡散反射光
    float32_t t = dot(normal, ligDirection);
    t = saturate(t);

    float32_t3 diffDirection = ligColor * t;

    // 鏡面反射光
    float32_t3 toEye = kCameraPos.pos - input.worldPosition.xyz;
    toEye = normalize(toEye);
    
    float32_t3 refVec = reflect(-ligDirection, normal);
    refVec = normalize(refVec);

    t = dot(refVec, toEye);


    t = pow(abs(t), shinness);
    float32_t3 specDirection = ligColor * t * 100.0f;
    
    float32_t3 lig = diffDirection + specDirection;
    lig += 0.2f;


    // スポットライト
    float32_t3 pointLightPos = kWaterData[input.instanceID].pointLightPos;
    float32_t pointLightRange = kWaterData[input.instanceID].pointLightRange;

    float32_t3 pointLightDir = input.worldPosition.xyz - pointLightPos;
    float32_t pointLightDistance = length(pointLightDir);
    pointLightDir = normalize(pointLightDir);

    t = dot(normal, pointLightDir);
    t = saturate(t);

    float32_t3 potinDiffDirection = ligColor * t;

    refVec = reflect(pointLightDir, normal);
    refVec = normalize(refVec);

    t = dot(refVec, toEye);


    t = pow(abs(t), shinness);
    float32_t3 pointSpecDirection = ligColor * t * 1000.0f;


    float32_t affect = 1.0f - rcp(pointLightDistance) * pointLightDistance;
    affect = max(affect, 0.0f);
    potinDiffDirection *= affect;
    pointSpecDirection *= affect;

    float32_t3 potintLig =  potinDiffDirection + pointSpecDirection;
    potintLig *= 10.0f;

    
    // 色
    //output.color0.rgb = saturate(float32_t3(1.0f,1.0f,1.0f) * (input.worldPosition.y * 0.1f));
    //output.color0.w = 1.0f;
    output.color0.rgb = kColor[input.instanceID].color.rgb * (lig + potintLig);
    output.color0.w = kColor[input.instanceID].color.w;

    // 法線
    output.color2.xyz = normal;
    output.color2.w = 1.0f;

    // ポジション
    output.color3 = input.worldPosition;

    output.color1.x = CreateNoise(input.uv + float32_t2(1.0f, 0.0f), kRandomVec, kDensity) * 0.5f;
    output.color1.x -= CreateNoise(input.uv - float32_t2(1.0f, 0.0f), kRandomVec, kDensity) * 0.5f;
    output.color1.y = CreateNoise(input.uv + float32_t2(0.0f, 1.0f), kRandomVec, kDensity) * 0.5f;
    output.color1.y -= CreateNoise(input.uv - float32_t2(0.0f, 1.0f), kRandomVec, kDensity) * 0.5f;
    output.color1.z = 0;
    output.color1.w = 1.0f;

    return output;
}