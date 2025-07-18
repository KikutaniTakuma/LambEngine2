#include "SkyBox.hlsli"
#include "AirSkyBox.hlsli"
#define PI 3.14159265358f

struct AtmosphericParams {
	float32_t3 cameraPosition;   // カメラの位置
	float32_t pad;
	float32_t3 lightDirection;   // 太陽光の方向（正規化ベクトル）
	float32_t rayleighScattering; // Rayleigh散乱係数
	float32_t mieScattering;     // Mie散乱係数
	float32_t mieG;              // Mie散乱位相関数のg値（0から1の範囲）
};

ConstantBuffer<AtmosphericParams> gAtmosphericParams : register(b1);

PixelOutPut main(VertexOutput input)
{
	const float32_t g = gAtmosphericParams.mieG;
	const float32_t g2 = g * g;
	const float32_t kRayleighScattering = gAtmosphericParams.rayleighScattering;
	const float32_t kMieScattering = gAtmosphericParams.mieScattering;

	const float32_t kRayleighSun = kRayleighScattering * kESun;
	const float32_t kMieSun = kMieScattering * kESun;
	const float32_t kRayleigh4PI = kRayleighScattering * 4.0f * PI;
	const float32_t kMie4PI = kMieScattering * 4.0f * PI;

	float32_t3 cameraPos = gAtmosphericParams.cameraPosition;
	float32_t3 worldPos = input.worldPosition.xyz - cameraPos;
	cameraPos.y += kInnerRadius;
	cameraPos.xz = 0.0f;
	worldPos = IntersectionPos(normalize(worldPos), float32_t3(0.0f, kInnerRadius, 0.0f), kOuterRadius);
    float32_t3 viewDirection = normalize(worldPos - cameraPos);
    float32_t3 lightDirection = normalize(gAtmosphericParams.lightDirection + viewDirection);

	float32_t3 v3Ray = worldPos - cameraPos;
	float32_t fFar = length(v3Ray);
	v3Ray = normalize(v3Ray);

	if(dot(v3Ray, normalize(cameraPos)) < -0.07f){
		PixelOutPut output;
		output.color = 0.0;
		output.color.a = 1.0f;
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

	PixelOutPut output;
	output.color.rgb = rayleighPhase * c0 + miePhase * c1;
	output.color.rgb = output.color.rgb;

	output.color.a = 1.0f;

	return output;
}