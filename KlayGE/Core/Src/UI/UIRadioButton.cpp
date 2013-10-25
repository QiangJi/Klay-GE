// UIRadioButton.cpp
// KlayGE ͼ���û����浥ѡť�� ʵ���ļ�
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
	UIRadioButton::UIRadioButton(UIDialogPtr const & dialog)
						: UIControl(UIRadioButton::Type, dialog),
							checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIRadioButton::UIRadioButton(uint32_t type, UIDialogPtr const & dialog)
						: UIControl(type, dialog),
							checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIRadioButton::UIRadioButton(UIDialogPtr const & dialog, int ID, uint32_t nButtonGroup, std::wstring const & strText, int4 const & coord_size, bool bChecked, uint8_t hotkey, bool bIsDefault)
						: UIControl(UIRadioButton::Type, dialog),
							button_group_(nButtonGroup),
							checked_(bChecked), pressed_(false), text_(strText)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UIRadioButton::InitDefaultElements()
	{
		UIElement Element;

		// Box
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_RadioButton, 0));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Middle);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 1);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		// Check
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_RadioButton, 1));

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}
	}

	void UIRadioButton::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UIRadioButton::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			if (pressed_)
			{
				pressed_ = false;

				this->GetDialog()->ClearRadioButtonGroup(button_group_);
				checked_ = !checked_;

				this->OnChangedEvent()(*this);
			}
		}
	}

	void UIRadioButton::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
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

	void UIRadioButton::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			pressed_ = false;

			if (this->ContainsPoint(pt))
			{
				this->GetDialog()->ClearRadioButtonGroup(button_group_);
				checked_ = !checked_;

				this->OnChangedEvent()(*this);
			}
		}
	}

	void UIRadioButton::SetCheckedInternal(bool bChecked, bool bClearGroup)
	{
		if (bChecked && bClearGroup)
		{
			this->GetDialog()->ClearRadioButtonGroup(button_group_);
		}

		checked_ = bChecked;
		this->OnChangedEvent()(*this);
	}

	void UIRadioButton::UpdateRects()
	{
		UIControl::UpdateRects();

		button_rc_ = text_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		text_rc_.left() += static_cast<int32_t>(1.25f * button_rc_.Width());

		bounding_box_ = button_rc_ | text_rc_;
	}

	void UIRadioButton::Render()
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
		this->GetDialog()->DrawString(text_, *pElement, text_rc_, true);

		if (!checked_)
		{
			iState = UICS_Hidden;
		}

		// Main button
		pElement = elements_[1];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, button_rc_);
	}

	std::wstring const & UIRadioButton::GetText() const
	{
		return text_;
	}

	void UIRadioButton::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}

	void UIRadioButton::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->SetCheckedInternal(true, true);
	}
}
