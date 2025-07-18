#include "../LambMesh.hlsli"

[numthreads(1, 1, 1)]
void main(in uint32_t3 groupID : SV_GroupID) {
	PayloadStruct p; 
    p.myArbitraryData = groupID.x; 
    DispatchMesh(kInstanceCount.value,1,1,p);
}