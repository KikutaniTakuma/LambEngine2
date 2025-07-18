static const float32_t kAmp[4] = {0.8f, 0.8f, 0.4f,0.9f};
static const float32_t kFreq[4] = {0.4f, 1.8f, 1.0f,1.2f};
static const float32_t kSteep[4] = {0.2f, 0.3f, 0.7f,0.4f};
static const float32_t kSpeed[4] = {20.0f, 30.0f, 10.0f,30.0f};
static const float32_t2 kDir[4] = { float32_t2(0.47f, 0.35f), float32_t2(-0.96f, 0.23f), float32_t2(0.77f, -1.47f), float32_t2(-0.3f, -0.2f) };

float32_t3 GerstnerWave(
    float32_t amp, 
    float32_t freq, 
    float32_t steep, 
    float32_t speed, 
    float32_t2 dir, 
    float32_t2 v, 
    float32_t time
) {
	float32_t3 p;
	float32_t2 d = normalize(dir.xy);
	float32_t q = steep;
	float32_t f = (dot(d, v) * freq + time * speed);

    const float kTau = 2.0f * 3.14159265358979323f;
    if(kTau <= f){
        float a = f * rcp(kTau);
        a = a - frac(a);

        f -= a * kTau;
    }
    else if(f <= -kTau){
        float a = f * rcp(kTau);
        a = a - frac(a);

        f += a * kTau;
    }

	p.xz = d.xy * q * amp * cos(f);
	p.y = amp * sin(f);
 
	return p;
}

float32_t3 GenerateWave(
    float32_t3 pos,
    float32_t time
){
    float32_t3 p = 0.0f;

    for(int32_t i = 0; i < 4; i++){
        p += GerstnerWave(kAmp[i], kFreq[i], kSteep[i], kSpeed[i], kDir[i], pos.xz, time);
    }

    return p;
}