// LightShaft.hpp
// KlayGE Light shaft�� ͷ�ļ�
// Ver 4.2.0
// ��Ȩ����(C) ������, 2012
// Homepage: http://www.klayge.org
//
// 4.2.0
// ���ν��� (2012.9.23)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHTSHAFT_HPP
#define _LIGHTSHAFT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>
#include <vector>

namespace KlayGE
{
	class KLAYGE_CORE_API LightShaftPostProcess : public PostProcess
	{
	public:
		LightShaftPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void Apply();

	private:
		std::vector<PostProcessPtr> radial_blur_pps_;
		PostProcessPtr apply_pp_;
	};
}

#endif
