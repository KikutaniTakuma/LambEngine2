float32_t Random(float32_t2 uv){
    return frac(sin(dot(uv, float32_t2(12.9898f, 78.233f))) * 43758.5453f);
}

float32_t2 RandomVector(float32_t2 fact){
    float32_t2 angle = float32_t2(
        dot(fact, float32_t2(127.1f, 311.7f)),
        dot(fact, float32_t2(269.5f, 183.3f))
    );

    return frac(sin(angle) * 43758.5453123f) * 2.0f - 1.0f;
}