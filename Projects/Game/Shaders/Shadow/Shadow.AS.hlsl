struct PayloadStruct { 
    uint32_t myArbitraryData; 
}; 

struct InstanceCount{
	uint32_t value;
};

ConstantBuffer<InstanceCount> kInstanceCount : register(b0);

[numthreads(1, 1, 1)]
void main(in uint32_t3 groupID : SV_GroupID) {
	PayloadStruct p; 
    p.myArbitraryData = groupID.x; 
    DispatchMesh(kInstanceCount.value,1,1,p);
}