#include "Utils/Flg.h"

namespace Lamb {
	Flg::Flg() :
		flg_(false),
		preFlg_(false)
	{
		
	}

	Flg::Flg(const Flg& right):
		Flg{}
	{
		*this = right;
	}
	Flg::Flg(Flg&& right) noexcept :
		Flg{}
	{
		*this = right;
	}

	Flg::~Flg() {
		
	}

	Flg::Flg(bool right) :
		flg_(right),
		preFlg_(false)
	{
		
	}

	void Flg::Update() {
		preFlg_ = flg_;
	}
}