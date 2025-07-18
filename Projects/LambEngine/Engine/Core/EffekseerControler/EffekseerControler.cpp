#include "EffekseerControler.h"

#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"

#include <Utils/EngineInfo.h>


Lamb::SafePtr<EffekseerControler> EffekseerControler::pInstane_;


EffekseerControler::EffekseerControler()
{
	Lamb::SafePtr directXDevice = DirectXDevice::GetInstance();
	directXDevice->GetDevice();
	directXCommand_ = DirectXCommand::GetMainCommandlist();

	// Create a manager of effects
	// エフェクトのマネージャーの作成
	efkManager_ = ::Effekseer::Manager::Create(8000);

	// エフェクトを複数threadで更新する設定
	efkManager_->LaunchWorkerThreads(std::thread::hardware_concurrency());

	// Create a  graphics device
	// 描画デバイスの作成
	graphicsDevice_ = ::EffekseerRendererDX12::CreateGraphicsDevice(directXDevice->GetDevice(), directXCommand_->GetCommandQueue(), 2);

	// Create a renderer of effects
	// エフェクトのレンダラーの作成
	DXGI_FORMAT format[1] = { 
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	efkRenderer_ = ::EffekseerRendererDX12::Create(graphicsDevice_, format, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, false, 8000);

	// Create a memory pool
	// メモリプールの作成
	efkMemoryPool_ = EffekseerRenderer::CreateSingleFrameMemoryPool(efkRenderer_->GetGraphicsDevice());

	// Create a command list
	// コマンドリストの作成
	efkCommandList_ = EffekseerRenderer::CreateCommandList(efkRenderer_->GetGraphicsDevice(), efkMemoryPool_);


	// Specify rendering modules
	// 描画モジュールの設定
	efkManager_->SetSpriteRenderer(efkRenderer_->CreateSpriteRenderer());
	efkManager_->SetRibbonRenderer(efkRenderer_->CreateRibbonRenderer());
	efkManager_->SetRingRenderer(efkRenderer_->CreateRingRenderer());
	efkManager_->SetTrackRenderer(efkRenderer_->CreateTrackRenderer());
	efkManager_->SetModelRenderer(efkRenderer_->CreateModelRenderer());

	// Specify a texture, model, curve and material loader
	// It can be extended by yourself. It is loaded from a file on now.
	// テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
	// ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
	efkManager_->SetTextureLoader(efkRenderer_->CreateTextureLoader());
	efkManager_->SetModelLoader(efkRenderer_->CreateModelLoader());
	efkManager_->SetMaterialLoader(efkRenderer_->CreateMaterialLoader());
	efkManager_->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	// Setup the coordinate system. This must be matched with your application.
	// 座標系を設定する。アプリケーションと一致させる必要がある。
	efkManager_->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

EffekseerControler::~EffekseerControler()
{
	efkCommandList_.Reset();
	efkMemoryPool_.Reset();
	efkRenderer_.Reset();
	graphicsDevice_.Reset();
	efkManager_.Reset();
}

const Lamb::SafePtr<EffekseerControler>& EffekseerControler::GetInstance() {
	return pInstane_;
}

void EffekseerControler::Initialize() {
	pInstane_.reset(new EffekseerControler());
}
void EffekseerControler::Finalize() {
	pInstane_.reset();
}
::Effekseer::ManagerRef& EffekseerControler::GetEfkManager()
{
	return efkManager_;
}

void EffekseerControler::NewFrame() {
	// Call on starting of a frame
	// フレームの開始時に呼ぶ
	efkMemoryPool_->NewFrame();

	// Begin a command list
	// コマンドリストを開始する。
	EffekseerRendererDX12::BeginCommandList(efkCommandList_, directXCommand_->GetCommandList());
	efkRenderer_->SetCommandList(efkCommandList_);
}

void EffekseerControler::EndFrame() {
	// End a command list
	// コマンドリストを終了する。
	efkRenderer_->SetCommandList(nullptr);
	EffekseerRendererDX12::EndCommandList(efkCommandList_);
}

void EffekseerControler::Draw() {
	// Begin to rendering effects
	// エフェクトの描画開始処理を行う。
	efkRenderer_->BeginRendering();

	// Render effects
	// エフェクトの描画を行う。
	Effekseer::Manager::DrawParameter drawParameter;
	drawParameter.ZNear = 0.0f;
	drawParameter.ZFar = 1.0f;
	drawParameter.ViewProjectionMatrix = efkRenderer_->GetCameraProjectionMatrix();
	efkManager_->Draw(drawParameter);

	// Finish to rendering effects
	// エフェクトの描画終了処理を行う。
	efkRenderer_->EndRendering();
}

void EffekseerControler::Update(float time, const Mat4x4& camera, const Mat4x4& projection) {
	const Vector3 viewPosition = camera.GetTranslate();

	// Set layer parameters
	// レイヤーパラメータの設定
	Effekseer::Manager::LayerParameter layerParameter;
	layerParameter.ViewerPosition.X = -viewPosition.x;
	layerParameter.ViewerPosition.Y = -viewPosition.y;
	layerParameter.ViewerPosition.Z = -viewPosition.z;
	efkManager_->SetLayerParameter(0, layerParameter);

	// Update the manager
	// マネージャーの更新
	Effekseer::Manager::UpdateParameter updateParameter;
	updateParameter.DeltaFrame = Lamb::MaxFPS() * Lamb::DeltaTime();
	updateParameter.UpdateInterval = freq_;
	efkManager_->Update(updateParameter);

	// Update a time
	// 時間を更新する
	efkRenderer_->SetTime(time);

	::Effekseer::Matrix44 projectionMatrix;
	for (size_t y = 0; y < projection.HeightSize(); y++) {
		for (size_t x = 0; x < projection.WidthSize(); x++) {
			projectionMatrix.Values[y][x] = projection[y][x];
		}
	}

	::Effekseer::Matrix44 cameraMatrix;
	for (size_t y = 0; y < camera.HeightSize(); y++) {
		for (size_t x = 0; x < camera.WidthSize(); x++) {
			cameraMatrix.Values[y][x] = camera[y][x];
		}
	}

	// Specify a projection matrix
	// 投影行列を設定
	efkRenderer_->SetProjectionMatrix(projectionMatrix);

	// Specify a camera matrix
	// カメラ行列を設定
	efkRenderer_->SetCameraMatrix(cameraMatrix);
}

void EffekseerControler::SetUpdateFrequency(float freq)
{
	freq_ = freq;
}
