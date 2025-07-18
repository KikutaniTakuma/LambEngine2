#include "../Lamb.hlsli"
#include "./Gerstner.hlsli"

#define PI 3.14159265358f
#define kMaxRipplePoints 96

struct AtmosphericParams {
	float32_t3 cameraPosition;
	float32_t pad;
	float32_t3 lightDirection;
	float32_t rayleighScattering;
	float32_t mieScattering;
	float32_t mieG;
};

struct WaveData{
    float32_t waveStrength;
    float32_t ripples;
    float32_t waveSpeed;
    float32_t lengthAttenuation;
    float32_t timeAttenuation;

    float32_t3 ripplesPoint[kMaxRipplePoints];
    float32_t time[kMaxRipplePoints];
};

struct WaterData {
    float32_t2 randomVec;
    float32_t density;
    uint32_t edgeDivision;
    uint32_t insideDivision;
    float32_t3 cameraPosition;
    float32_t distanceThreshold;
    float32_t3 cameraDirection;
    float32_t4x4 viewportMatrix;
    WaveData waveData;
    AtmosphericParams atmosphericParams;
    float32_t time;
	float32_t3 pointLightPos;
	float32_t pointLightRange;
	float32_t pointLightAngle;
};

StructuredBuffer<WaterData> kWaterData : register(t2);


#define M_PI 3.141592653589793238462643
#define M_E 2.71828182846


float32_t Waves(float32_t3 worldPos, uint32_t instanceID){

	float32_t maxHeight = kWaterData[instanceID].waveData.waveStrength;
	float32_t waveSpeed = kWaterData[instanceID].waveData.waveSpeed;
	float32_t lengthAttenuation = kWaterData[instanceID].waveData.lengthAttenuation;
	float32_t timeAttenuation = kWaterData[instanceID].waveData.timeAttenuation;

	float32_t ripples = kWaterData[instanceID].waveData.ripples;
	float32_t waveLength = 2.0f * M_PI * rcp(ripples);


	float32_t wave = 0.0f;

	for(int32_t i = 0; i < kMaxRipplePoints; i++){
		float32_t3 ripplesPoint = kWaterData[instanceID].waveData.ripplesPoint[i];
		float32_t len = length(worldPos - ripplesPoint);
		float32_t time = kWaterData[instanceID].waveData.time[i];
		float32_t waveLen = waveSpeed * time;

		// Distance from the wave (+ if the wave has passed)
		float32_t waveToPosLen = waveLen - len;

		// Maximum height * sine wave * Returns 1 if the wave reaches, otherwise 0 * Distance decay * Time decay
		wave += maxHeight * sin(waveLength * -waveToPosLen) 
			* min(1.0f, ceil(max(0.0f, waveToPosLen))) 
			* pow(M_E, -len * lengthAttenuation) * pow(M_E, -max(waveToPosLen, 0.0f) * timeAttenuation);
	}

	return wave;
}

float32_t3 GenerateNormal(
    float32_t3 worldPosition,
    float32_t time,
    uint32_t instanceID
){     
    float32_t wavePower = 0.2f;
    const float32_t kEpsilon = 0.0001f;

	float32_t3 upPos = worldPosition.xyz;
	upPos.z += kEpsilon;
	float32_t3 downPos = worldPosition.xyz;
	downPos.z -= kEpsilon;
	float32_t3 rightPos = worldPosition.xyz;
	rightPos.x += kEpsilon;
	float32_t3 leftPos = worldPosition.xyz;
	leftPos.x -= kEpsilon;
	float32_t up = GenerateWave(upPos, time).y * wavePower + Waves(upPos, instanceID);
	float32_t down = GenerateWave(downPos, time).y * wavePower + Waves(downPos.xyz, instanceID);
	float32_t right = GenerateWave(rightPos, time).y * wavePower + Waves(rightPos, instanceID);
	float32_t left = GenerateWave(leftPos, time).y * wavePower + Waves(leftPos, instanceID);

	float32_t yx = (right - left) * rcp(2.0f * kEpsilon);
	float32_t yz = (up - down) * rcp(2.0f * kEpsilon);

	float32_t3 resultNormal = normalize(float32_t3(-yx, 1.0f, -yz));

    return resultNormal;
}