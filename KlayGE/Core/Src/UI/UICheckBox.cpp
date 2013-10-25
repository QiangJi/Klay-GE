// UICheckBox.cpp
// KlayGE ͼ���û����渴ѡ���� ʵ���ļ�
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
#include <KlayGE/Window.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UICheckBox::UICheckBox(UIDialogPtr const & dialog)
					: UIControl(UICheckBox::Type, dialog),
						checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UICheckBox::UICheckBox(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UICheckBox::UICheckBox(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bChecked, uint8_t hotkey, bool bIsDefault)
					: UIControl(UICheckBox::Type, dialog),
						checked_(bChecked), pressed_(false),
						text_(strText)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UICheckBox::InitDefaultElements()
	{
		UIElement Element;

		// Box
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 0));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Middle);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 1);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		// Check
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 1));

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}
	}

	void UICheckBox::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UICheckBox::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			if (pressed_)
			{
				pressed_ = false;
				this->SetCheckedInternal(!checked_);
			}
		}
	}

	void UICheckBox::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
	{
		if (buttons & MB_Left)
		{
			pressed_ = true;

			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}
		}
	}

	void UICheckBox::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			pressed_ = false;

			if (this->ContainsPoint(pt))
			{
				this->SetCheckedInternal(!checked_);
			}
		}
	}

	void UICheckBox::SetCheckedInternal(bool bChecked)
	{
		checked_ = bChecked;

		this->OnChangedEvent()(*this);
	}

	void UICheckBox::UpdateRects()
	{
		button_rc_ = text_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		text_rc_.left() += static_cast<int32_t>(1.25f * button_rc_.Width());

		bounding_box_ = button_rc_ | text_rc_;
	}

	void UICheckBox::Render()
	{
		UI_Control_State iState = UICS_Normal;

		if (!visible_)
		{
			iState = UICS_Hidden;
		}
		else
		{
			if (!enabled_)
			{
				iState = UICS_Disabled;
			}
			else
			{
				if (pressed_)
				{
					iState = UICS_Pressed;
				}
				else
				{
					if (is_mouse_over_)
					{
						iState = UICS_MouseOver;
					}
					else
					{
						if (has_focus_)
						{
							iState = UICS_Focus;
						}
					}
				}
			}
		}

		UIElementPtr pElement = elements_[0];

		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, button_rc_);
		this->GetDialog()->DrawString(text_, *pElement, text_rc_);

		if (!checked_)
		{
			iState = UICS_Hidden;
		}

		pElement = elements_[1];

		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, button_rc_);
	}

	std::wstring const & UICheckBox::GetText() const
	{
		return text_;
	}

	void UICheckBox::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}

	void UICheckBox::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->SetCheckedInternal(!checked_);
	}
}
