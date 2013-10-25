// ShaderObject.cpp
// KlayGE shader������ ʵ���ļ�
// Ver 3.5.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.5.0
// ���ν��� (2006.11.2)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <string>
#include <vector>

#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class NullShaderObject : public ShaderObject
	{
	public:
		bool AttachNativeShader(ShaderType /*type*/, RenderEffect const & /*effect*/, std::vector<uint32_t> const & /*shader_desc_ids*/,
			std::vector<uint8_t> const & /*native_shader_block*/)
		{
			is_validate_ = true;
			return true;
		}

		void ExtractNativeShader(ShaderType /*type*/, RenderEffect const & /*effect*/, std::vector<uint8_t>& native_shader_block)
		{
			native_shader_block.clear();
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, std::vector<uint32_t> const & /*shader_desc_ids*/)
		{
			is_validate_ = true;
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & /*shared_so*/)
		{
			is_validate_ = true;
		}

		void LinkShaders(RenderEffect const & /*effect*/)
		{
			is_validate_ = true;
		}

		ShaderObjectPtr Clone(RenderEffect const & /*effect*/)
		{
			return ShaderObject::NullObject();
		}

		void Bind()
		{
		}

		void Unbind()
		{
		}
	};

	ShaderObjectPtr ShaderObject::NullObject()
	{
		static ShaderObjectPtr obj = MakeSharedPtr<NullShaderObject>();
		return obj;
	}

	ShaderObject::ShaderObject()
		: has_discard_(false), has_tessellation_(false),
			cs_block_size_x_(0), cs_block_size_y_(0), cs_block_size_z_(0)
	{
	}
}
