// UIStatic.cpp
// KlayGE ͼ���û����澲̬���� ʵ���ļ�
// Ver 3.6.0
// ��Ȩ����(C) ������, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// ���ν��� (2007.6.27)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIStatic::UIStatic(UIDialogPtr const & dialog)
					: UIControl(UIStatic::Type, dialog)
	{
		this->InitDefaultElements();
	}

	UIStatic::UIStatic(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog)
	{
		this->InitDefaultElements();
	}

	UIStatic::UIStatic(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bIsDefault)
					: UIControl(UIStatic::Type, dialog),
						text_(strText)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetIsDefault(bIsDefault);
	}

	void UIStatic::InitDefaultElements()
	{
		UIElement Element;

		{
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}
	}

	void UIStatic::Render()
	{
		if (!visible_)
		{
			return;
		}

		UI_Control_State iState = UICS_Normal;

		if (!enabled_)
		{
			iState = UICS_Disabled;
		}

		UIElement& element = *elements_[0];
		element.FontColor().SetState(iState);
		this->GetDialog()->DrawString(text_, element, bounding_box_);
	}

	void UIStatic::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}
}
