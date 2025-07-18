/// ================================
/// ==  PeraPipelineクラスの宣言  ==
/// ================================


#pragma once
#include "../PipelineObject.h"
#include "Engine/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/RenderTarget/RenderTarget.h"
#include "Math/Vector2.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include <memory>

class PeraPipeline : public PipelineObject{
public:
	PeraPipeline() = default;
	PeraPipeline(const PeraPipeline&) = delete;
	PeraPipeline(PeraPipeline&&) = delete;
	virtual ~PeraPipeline();

	PeraPipeline& operator=(const PeraPipeline&) = delete;
	PeraPipeline& operator=(PeraPipeline&&) = delete;

public:
	virtual void Use(Pipeline::Blend blendType, bool isDepth = false) override;

	virtual void Init(
		const std::string& vsShader,
		const std::string& psShader,
		std::initializer_list<DXGI_FORMAT> formtats = {DXGI_FORMAT_R32G32B32A32_FLOAT}
	) override;

public:
	virtual void SetData();

	void SetSize(uint32_t width, uint32_t height) {
		width_ = width;
		height_ = height;
	}

	RenderTarget& GetRender() {
		return *render_;
	}

	const RenderTarget& GetRender() const {
		return *render_;
	}

public:
	Vector4 color = Vector4::kIdentity;

protected:
	std::unique_ptr<RenderTarget> render_;

	std::array<std::unique_ptr<ConstantBuffer<Vector4>>, DirectXSwapChain::kBackBufferNumber> colorBuf_;
	uint32_t width_ = 0u;
	uint32_t height_ = 0u;
};