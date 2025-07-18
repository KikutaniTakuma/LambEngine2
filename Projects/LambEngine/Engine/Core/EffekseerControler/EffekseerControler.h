#pragma once
#include "Utils/SafePtr.h"
#ifndef _DEBUG
#pragma comment(lib, "Effekseer.lib")
#pragma comment(lib, "EffekseerRendererDX12.lib")
#pragma comment(lib, "LLGI.lib")
#else
#pragma comment(lib, "Effekseerd.lib")
#pragma comment(lib, "EffekseerRendererDX12d.lib")
#pragma comment(lib, "LLGId.lib")
#endif
#include <Effekseer.h>
#include <EffekseerRendererDX12.h>

#include <Math/Matrix.h>
#include <Math/Vector3.h>


class EffekseerControler final {
private:
	EffekseerControler();

	EffekseerControler(const EffekseerControler&) = delete;
	EffekseerControler(EffekseerControler&&) = delete;

	EffekseerControler& operator=(const EffekseerControler&) = delete;
	EffekseerControler& operator=(EffekseerControler&&) = delete;

public:
	~EffekseerControler();
public:
	static const Lamb::SafePtr<EffekseerControler>& GetInstance();
	static void Initialize();
	static void Finalize();
	

private:
	static Lamb::SafePtr<EffekseerControler> pInstane_;

public:
	::Effekseer::ManagerRef& GetEfkManager();

public:
	// フレーム開始処理
	void NewFrame();

	// フレーム終了処理
	void EndFrame();

	// 描画処理
	void Draw();

	/// <summary>
	/// 描画設定(1framで1度のみ呼び出し可能)
	/// </summary>
	/// <param name="time">時間(s)</param>
	/// <param name="camera">カメラマトリックス</param>
	/// <param name="projection">プロジェクションマトリックス</param>
	void Update(float time, const Mat4x4& camera, const Mat4x4& projection);

	/// <summary>
	/// Effectの更新頻度設定
	/// </summary>
	/// <param name="freq">更新頻度(2.0fの場合、2frameに1回更新される)</param>
	void SetUpdateFrequency(float freq);

private:
	::Effekseer::ManagerRef efkManager_;
	::Effekseer::Backend::GraphicsDeviceRef graphicsDevice_;
	::EffekseerRenderer::RendererRef efkRenderer_;
	::Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> efkMemoryPool_;
	::Effekseer::RefPtr<EffekseerRenderer::CommandList> efkCommandList_;
	float freq_ = 1u;

	Lamb::SafePtr<class DirectXCommand> directXCommand_;
};