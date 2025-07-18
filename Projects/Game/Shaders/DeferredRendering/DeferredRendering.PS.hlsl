#include "DeferredRendering.hlsli"
#include "../OutputStructs.hlsli"
#include "../SkyBoxShader/AirSkyBox.hlsli"

#define PI 3.14159265358f

struct DirectionLight{
    float32_t3 ligDirection;
    float32_t shinness;
    float32_t3 ligColor;
    float32_t pad;
};

struct DeferredRenderingData{
    float32_t3 eyePos;
    uint32_t rightNum;
    DirectionLight directionLight;
    uint32_t isDirectionLight;
    uint32_t isShadow;
    float32_t environmentCoefficient;
};

struct PointLight {
    float32_t3 ptPos;
    float32_t3 ptColor;
    float32_t ptRange;
};


struct AtmosphericParams {
	float32_t3 cameraPosition;   // カメラの位置
	float32_t pad;
	float32_t3 lightDirection;   // 太陽光の方向（正規化ベクトル）
	float32_t rayleighScattering; // Rayleigh散乱係数
	float32_t mieScattering;     // Mie散乱係数
	float32_t mieG;              // Mie散乱位相関数のg値（0から1の範囲）
};

struct Mat4x4{
    float32_t4x4 value;
};


ConstantBuffer<DeferredRenderingData> gDeferredRenderingState : register(b0);
ConstantBuffer<AtmosphericParams> gAtmosphericParams : register(b1);
ConstantBuffer<Mat4x4> gCameraMatrix : register(b2);
ConstantBuffer<Mat4x4> gLightCameraMatrix : register(b3);

Texture2D<float32_t4> gColorTexture : register(t0);
Texture2D<float32_t4> gNormalTexture : register(t1);
Texture2D<float32_t4> gWorldPositionTexture : register(t2);
Texture2D<float32_t4> gDistortionTexture : register(t3);

// ポイントサンプラー
SamplerState gPointSmp : register(s0);

float32_t3 SkyColor(float32_t3 gWorldPos, float32_t3 gNormal, float32_t3 cameraPos){
    const float32_t g = gAtmosphericParams.mieG;
	const float32_t g2 = g * g;
	const float32_t kRayleighScattering = gAtmosphericParams.rayleighScattering;
	const float32_t kMieScattering = gAtmosphericParams.mieScattering;

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
    float32_t3 lightDirection = normalize(gAtmosphericParams.lightDirection + viewDirection);

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

PixelShaderOutPut main(Output input) {
    float32_t2 uvDistortion = gDistortionTexture.Sample(gPointSmp, input.uv).xy;
    float32_t2 scrollDistortion = gDistortionTexture.Sample(gPointSmp, input.uv + uvDistortion).xy;
    if(scrollDistortion.x == 0.0f && scrollDistortion.y == 0.0f) {
        uvDistortion = 0;
    }

    float32_t2 uv = input.uv + uvDistortion;
    uv = clamp(uv, float32_t2(0.0f,0.0f), float32_t2(1.0f, 1.0f));

    float32_t4 color = gColorTexture.Sample(gPointSmp, uv);
    float32_t4 worldPosition = gWorldPositionTexture.Sample(gPointSmp, uv);
    float32_t3 normal = gNormalTexture.Sample(gPointSmp, uv).xyz;
    float32_t len = length(normal);
    normal = normalize(normal);
    PixelShaderOutPut outputColor;

    float32_t3 eyePos = gDeferredRenderingState.eyePos;

    float32_t4 ndcPos = mul(worldPosition, gCameraMatrix.value);
    float32_t4 shadowNDCPos = mul(worldPosition, gLightCameraMatrix.value);

    float32_t depth = pow(ndcPos.w, 100.0f);
    float32_t shadowDepth = pow(shadowNDCPos.w, 100.0f);
    float32_t shadowWeight = 1.0f;

    if(gDeferredRenderingState.isShadow && shadowDepth < depth) {
        shadowWeight = 0.1f;
    }


    if(len != 0.0f){
        float32_t3 skyColor = SkyColor(worldPosition.xyz, normal, eyePos);
        color.rgb += skyColor * gDeferredRenderingState.environmentCoefficient;
    }

    if(gDeferredRenderingState.isDirectionLight == 1 && len != 0.0f){
        float32_t3 ligDirection = gDeferredRenderingState.directionLight.ligDirection;
        float32_t3 ligColor = gDeferredRenderingState.directionLight.ligColor;
        float32_t shinness = gDeferredRenderingState.directionLight.shinness;
 
        // ディレクションライト拡散反射光
        float32_t t = dot(normal, ligDirection);
        t = saturate(t);

        float32_t3 diffDirection = ligColor * t;

        // 鏡面反射光
        float32_t3 toEye = eyePos - worldPosition.xyz;
        toEye = normalize(toEye);
    
        float32_t3 refVec = reflect(-ligDirection, normal);
        refVec = normalize(refVec);

        t = dot(refVec, toEye);

        t = pow(saturate(t), shinness);
        float32_t3 specDirection = ligColor * t;
    
        float32_t3 lig = (diffDirection + specDirection) * shadowWeight;
        lig += 0.2f;
        outputColor.color.rgb = color.rgb * lig;
    }else{
        outputColor.color.rgb = color.rgb;
    }

    outputColor.color.a = 1.0f;

    return outputColor;
}