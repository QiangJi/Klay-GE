// UIButton.cpp
// KlayGE ͼ���û����水ť�� ʵ���ļ�
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

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIButton::UIButton(UIDialogPtr const & dialog)
					: UIControl(UIButton::Type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIButton::UIButton(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIButton::UIButton(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
					: UIControl(UIButton::Type, dialog),
						pressed_(false),
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

	void UIButton::InitDefaultElements()
	{
		UIElement Element;

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Button, 0));
			Element.SetFont(0);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 200.0f / 255);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1.0f);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		// Fill layer
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Button, 1), Color(1, 1, 1, 0));
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 1, 1, 160.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(0, 0, 0, 60.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 30.0f / 255);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}
	}

	void UIButton::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UIButton::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			if (pressed_)
			{
				pressed_ = false;
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
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

	void UIButton::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			pressed_ = false;

			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}

			if (this->ContainsPoint(pt))
			{
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::Render()
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

		// Background fill layer
		//TODO: remove magic numbers
		UIElementPtr pElement = elements_[0];

		IRect rcWindow = bounding_box_;

		// Blend current color
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, rcWindow);
		this->GetDialog()->DrawString(text_, *pElement, rcWindow);

		// Main button
		pElement = elements_[1];


		// Blend current color
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, rcWindow);
		this->GetDialog()->DrawString(text_, *pElement, rcWindow);
	}

	std::wstring const & UIButton::GetText() const
	{
		return text_;
	}

	void UIButton::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}

	void UIButton::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->OnClickedEvent()(*this);
	}
}
