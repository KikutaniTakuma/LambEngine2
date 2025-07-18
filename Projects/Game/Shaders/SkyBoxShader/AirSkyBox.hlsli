static const float32_t InnerRadius = 10000.0f;
static const float32_t OuterRadius = 10250.0f;

static const float32_t fSamples = 2.0f;
 
static const float32_t3 kThreePrimaryColors = float32_t3(0.68f, 0.55f, 0.44f);
static const float32_t3 kV3InvWaveLength = 1.0f / pow(kThreePrimaryColors, 4.0f);
static const float32_t kOuterRadius = OuterRadius;
static const float32_t kInnerRadius = InnerRadius;
static const float32_t kESun = 20.0f;

static const float32_t kScale = 1.0f / (OuterRadius - InnerRadius);
static const float32_t kScaleDepth = 0.25f;
static const float32_t kScaleOverScaleDepth = kScale / kScaleDepth;

float32_t Scale(float32_t fcos){
	float32_t x = 1.0 - fcos;
	return kScaleDepth * exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.8f + x * 5.25f))));
}

float32_t3 IntersectionPos(float32_t3 dir, float32_t3 a, float32_t radius) {
	float32_t b = dot(a, dir);
	float32_t c = dot(a, a) - radius * radius;
	float32_t d = max(b * b - c, 0.0f);
 
	return a + dir * (-b + sqrt(d));
}