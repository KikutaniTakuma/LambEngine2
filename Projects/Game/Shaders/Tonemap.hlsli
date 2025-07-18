struct TonemapParams
{
	float32_t3 mToe;
	float32_t2 mMid;
	float32_t3 mShoulder;
	float32_t2 mBx;
	float32_t2 mBy;	// only used for InvTonemap
};

float32_t3 Tonemap(const TonemapParams tc, float32_t3 x)
{
	float32_t3 toe = - tc.mToe.x * rcp(x + tc.mToe.y) + tc.mToe.z;
	float32_t3 mid = tc.mMid.x * x + tc.mMid.y;
	float32_t3 shoulder = - tc.mShoulder.x * rcp(x + tc.mShoulder.y) + tc.mShoulder.z;

	float32_t3 result = lerp(toe, mid, step(tc.mBx.x, x));
	result = lerp(result, shoulder, step(tc.mBx.y, x));
	return result;
}
