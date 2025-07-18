#pragma once
#include <unordered_map>
#include <string>
#include <Utils/SafePtr.h>

#include <Effekseer.h>

class EffectDataManager {
private:
	EffectDataManager();
	EffectDataManager(const EffectDataManager&) = delete;
	EffectDataManager(EffectDataManager&&) = delete;

	const EffectDataManager& operator=(const EffectDataManager&) = delete;
	const EffectDataManager& operator=(EffectDataManager&&) = delete;
	
public:
	~EffectDataManager() = default;

public:
	static const Lamb::SafePtr<EffectDataManager>& GetInstance();

	static void Initialize();
	static void Finalize();

public:
	void Load(const std::string& fileName);
	
	const Effekseer::EffectRef& Get(const std::string& fileName);

private:
	static Lamb::SafePtr<EffectDataManager> pInstance_;

private:
	std::unordered_map<std::string, Effekseer::EffectRef> effects_;
	Lamb::SafePtr<class EffekseerControler> effekseerContoler_;
};