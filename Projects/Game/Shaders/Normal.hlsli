float32_t3 NormalToTangent(float32_t3 normal)
{
    float32_t3 tangent;

    float32_t3 a = abs(normal.x) < 0.999f ? float32_t3(1, 0, 0) : float32_t3(0, 1, 0);

    tangent = normalize(cross(a, normal));

    return tangent;
}

float32_t3 CalcBinormal(float32_t3 normal, float32_t3 tangent){
	return normalize(cross(normal, tangent));
}

float32_t3 BlendNormal(float32_t3 normal, float32_t3 tangent, float32_t3 binormal, float32_t3 normalMap){
	return normalize(tangent * normalMap.x + binormal * normalMap.y + normal * normalMap.z);
}