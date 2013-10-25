// D3D11Adapter.cpp
// KlayGE D3D11������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11Adapter.hpp>

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	D3D11Adapter::D3D11Adapter()
					: adapter_no_(0)
	{
	}

	D3D11Adapter::D3D11Adapter(uint32_t adapter_no, IDXGIAdapter1Ptr const & adapter)
					: adapter_no_(adapter_no),
						adapter_(adapter)
	{
		adapter_->GetDesc1(&adapter_desc_);
	}

	// ��ȡ�豸�����ַ���
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const D3D11Adapter::Description() const
	{
		return std::wstring(adapter_desc_.Description);
	}

	// ��ȡ֧�ֵ���ʾģʽ��Ŀ
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D11Adapter::NumVideoMode() const
	{
		return modes_.size();
	}

	// ��ȡ��ʾģʽ
	/////////////////////////////////////////////////////////////////////////////////
	D3D11VideoMode const & D3D11Adapter::VideoMode(size_t index) const
	{
		BOOST_ASSERT(index < modes_.size());

		return modes_[index];
	}

	// ö����ʾģʽ
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11Adapter::Enumerate()
	{
		std::vector<DXGI_FORMAT> formats;
		formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
		formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		formats.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
		formats.push_back(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
		formats.push_back(DXGI_FORMAT_R10G10B10A2_UNORM);

		UINT i = 0;
		IDXGIOutput* output;
		while (adapter_->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			if (output != nullptr)
			{
				typedef KLAYGE_DECLTYPE(formats) FormatsType;
				KLAYGE_FOREACH(FormatsType::reference format, formats)
				{
					UINT num = 0;
					output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, 0);
					if (num > 0)
					{
						std::vector<DXGI_MODE_DESC> mode_descs(num);
						output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, &mode_descs[0]);

						typedef KLAYGE_DECLTYPE(mode_descs) ModeDescsType;
						KLAYGE_FOREACH(ModeDescsType::reference mode_desc, mode_descs)
						{
							D3D11VideoMode const video_mode(mode_desc.Width, mode_desc.Height,
								mode_desc.Format);

							// ����ҵ�һ����ģʽ, ����ģʽ�б�
							if (std::find(modes_.begin(), modes_.end(), video_mode) == modes_.end())
							{
								modes_.push_back(video_mode);
							}
						}
					}
				}

				output->Release();
			}

			++ i;
		}

		std::sort(modes_.begin(), modes_.end());
	}

	void D3D11Adapter::ResetAdapter(IDXGIAdapter1Ptr const & ada)
	{
		adapter_ = ada;
		adapter_->GetDesc1(&adapter_desc_);
		modes_.resize(0);
	}
}
