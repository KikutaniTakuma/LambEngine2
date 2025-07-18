/// ======================================
/// ==  グラフィック関係の構造体の定義  ==
/// ======================================


#pragma once

#include <unordered_map>
#include <vector>
#include <span>

#include "Transform/Transform.h"

#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"

#include "PipelineManager/Pipeline/Pipeline.h"
#include "../EngineUtils/LambPtr/LambPtr.h"
#include "../Buffer/StructuredBuffer.h"
#include "../Buffer/ConstantBuffer.h"


#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#undef max
#undef min



struct Vertex {
    // ポジション
    Vector4 pos;
    // 法線
    Vector3 normal;

    // uv座標
    Vector2 uv;

    // テクスチャのインデックス
    uint32_t textureID = 0u;

    Vector2 pad;

    bool operator==(const Vertex& right)const {
        return pos == right.pos and
            normal == right.normal and
            uv == right.uv and
            textureID == right.textureID;
    }
    bool operator!=(const Vertex& right)const {
        return not (*this == right);
    }
};

struct WVPMatrix {
    Mat4x4 worldMat;
    Mat4x4 cameraMat;
};

struct Node {
    QuaternionTransform transform;
    Mat4x4 loacalMatrix = Mat4x4::kIdentity;
    std::string name;
    std::vector<Node> children;

    Node& operator=(const Node&) = default;
};

struct VertexWeightData {
    float weight;
    uint32_t vertexIndex;
};

struct JointWeightData {
    Mat4x4 inverseBindPoseMatrix;
    std::vector<VertexWeightData> vertexWeights;
};

struct ModelData {
    std::unordered_map<std::string, JointWeightData> skinClusterData;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Node rootNode;
};

struct VertexIndexData {
    D3D12_VERTEX_BUFFER_VIEW vertexView;
    Lamb::LambPtr<ID3D12Resource> vertexResource;
    uint32_t vertexNumber;

    D3D12_INDEX_BUFFER_VIEW indexView;
    Lamb::LambPtr<ID3D12Resource> indexResource;

    uint32_t indexNumber;
    Node node;

    VertexIndexData& operator=(const VertexIndexData&) = default;
};

enum BlendType {
    kNone = Pipeline::Blend::None,
    
    kNormal = Pipeline::Blend::Normal,
    kAdd = Pipeline::Blend::Add,
    kSub = Pipeline::Blend::Sub,
    kMul = Pipeline::Blend::Mul,

    kUnenableDepthNone = (Pipeline::Blend::BlendTypeNum),
    kUnenableDepthNormal,
    kUnenableDepthAdd,
    kUnenableDepthSub,
    kUnenableDepthMul,

    kNum,
    
    kAlphaEffect,
};

static const std::array<std::string, BlendType::kNum> kBlendTypeStrs = {
    "kNone",

    "kNormal",
    "kAdd",
    "kSub",
    "kMul",

    "kUnenableDepthNone",
    "kUnenableDepthNormal",
    "kUnenableDepthAdd",
    "kUnenableDepthSub",
    "kUnenableDepthMul",
};

struct PointLight {
    Vector3 ptPos;
    float pad3;
    Vector3 ptColor;
    float ptRange;
};

struct DirectionLight {
    Vector3 ligDirection = -Vector3::kYIdentity;
    float shinness = 256.0f;
    Vector3 ligColor = Vector3::kIdentity;
    float pad0 = 0.0f;
};

struct Light {
    Vector3 ligDirection = -Vector3::kYIdentity;
    float pad0;
    Vector3 ligColor = Vector3::kIdentity;
    float pad2;
    Vector3 eyePos;
    float shinness;
    Vector3 ptPos;
    float pad3;
    Vector3 ptColor;
    float ptRange;
};

template<class T>
struct ShaderData {
    ConstantBuffer<DirectionLight> light;
    ConstantBuffer<Vector3> eyePos;
    StructuredBuffer<WVPMatrix> wvpMatrix;
    StructuredBuffer<Vector4> color;
    StructuredBuffer<T> shaderStruct;
};

template<Lamb::IsObject T>
struct DrawData {
    float depth;
    WVPMatrix wvpMatrix;
    Vector4 color;
    T shaderStruct;
};

struct ShaderFileNames {
    std::string vsFileName;
    std::string psFileName;
    std::string gsFileName;
    std::string dsFileName;
    std::string hsFileName;

    [[nodiscard]] bool operator==(const ShaderFileNames& right) const {
        return vsFileName == right.vsFileName &&
            psFileName == right.psFileName &&
            gsFileName == right.gsFileName &&
            dsFileName == right.dsFileName &&
            hsFileName == right.hsFileName;
    }
};

struct MeshShaderFileNames {
    std::string asFileName;
    std::string msFileName;
    std::string psFileName;

    [[nodiscard]] bool operator==(const MeshShaderFileNames& right) const {
        return asFileName == right.asFileName and
            msFileName == right.msFileName and
            psFileName == right.psFileName;
    }
};

struct LoadFileNames {
    std::string resourceFileName;
    ShaderFileNames shaderName;

    [[nodiscard]] bool operator==(const LoadFileNames& right) const {
        return resourceFileName == right.resourceFileName &&
            shaderName == right.shaderName;
    }
};

struct MeshLoadFileNames {
    std::string resourceFileName;
    MeshShaderFileNames shaderName;

    [[nodiscard]] bool operator==(const MeshLoadFileNames& right) const {
        return resourceFileName == right.resourceFileName &&
            shaderName == right.shaderName;
    }
};

namespace std {
    template<>
    struct hash<Vertex> {
    public:
        size_t operator()(const Vertex& data)const {
            size_t result{};

            result = std::hash<std::string>{}(
                std::to_string(data.pos.m[0]) + 
                std::to_string(data.pos.m[1]) + 
                std::to_string(data.pos.m[2]) + 
                std::to_string(data.pos.m[3]) + 
                std::to_string(data.normal.x) + 
                std::to_string(data.normal.y) + 
                std::to_string(data.normal.z) + 
                std::to_string(data.uv.x) + 
                std::to_string(data.uv.y) + 
                std::to_string(data.textureID) 
                );

            return result;
        }

    };
}

namespace std {
    template<>
    struct hash<LoadFileNames> {
    public:
        size_t operator()(const LoadFileNames& data)const {
            size_t result{};

            result = std::hash<std::string>{}(
                data.resourceFileName +
                data.shaderName.vsFileName +
                data.shaderName.psFileName +
                data.shaderName.gsFileName +
                data.shaderName.dsFileName +
                data.shaderName.hsFileName
                );

            return result;
        }

    };
}

namespace std {
    template<>
    struct hash<MeshLoadFileNames> {
    public:
        size_t operator()(const MeshLoadFileNames& data)const {
            size_t result{};

            result = std::hash<std::string>{}(
                data.resourceFileName +
                data.shaderName.msFileName +
                data.shaderName.psFileName +
                data.shaderName.asFileName
                );

            return result;
        }

    };
}


template<class tValue>
struct KeyFrame{
    float time;
    tValue value;
};


using KeyFrameVector3 = KeyFrame<Vector3>;
using KeyFrameQuaternion = KeyFrame<Quaternion>;

template<class tValue>
struct AnimationCurve {
    std::vector<KeyFrame<tValue>> keyFrames;
};

struct NodeAnimation{
    AnimationCurve<Vector3> translation;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vector3> sacle;
};

struct Animation {
    float duration;
    std::unordered_map<std::string, NodeAnimation> nodeAnimations;
};

struct Animations {
    std::vector<Animation> data;
};