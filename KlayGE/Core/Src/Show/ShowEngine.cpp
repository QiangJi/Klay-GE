// ShowEngine.cpp
// KlayGE �������� ʵ���ļ�
// Ver 1.2.8.11
// ��Ȩ����(C) ������, 2001--2002
// Homepage: http://www.klayge.org
//
// 1.2.8.10
// ��string�����ַ���ָ�� (2002.10.27)
//
// 1.2.8.11
// ����UNICODE���� (2002.11.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class NullShowEngine : public ShowEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring name(L"Null Show Engine");
			return name;
		}

		bool IsComplete()
		{
			return true;
		}

		void Load(std::string const & /*fileName*/)
		{
		}

		TexturePtr PresentTexture()
		{
			return Texture::NullObject();
		}

		ShowState State(long /*timeout*/)
		{
			return SS_Stopped;
		}

	private:
		void DoPlay()
		{
		}

		void DoStop()
		{
		}

		void DoPause()
		{
		}
	};

	ShowEngine::~ShowEngine()
	{
	}

	// ���ؿն���
	//////////////////////////////////////////////////////////////////////////////////
	ShowEnginePtr ShowEngine::NullObject()
	{
		static ShowEnginePtr obj = MakeSharedPtr<NullShowEngine>();
		return obj;
	}

	// ���Բ���
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPlay() const
	{
		return (SS_Stopped == this->state_) || (SS_Paused == this->state_);
	}

	// ����ֹͣ
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanStop() const
	{
		return (SS_Playing == this->state_) || (SS_Paused == this->state_);
	}

	// ������ͣ
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPause() const
	{
		return (SS_Playing == this->state_) || (SS_Paused == this->state_);
	}

	// ��ʼ�����
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::IsInitialized() const
	{
		return this->state_ != SS_Uninit;
	}

	// ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Play()
	{
		if (this->CanPlay())
		{
			this->DoPlay();

			state_ = SS_Playing;
		}
	}

	// ��ͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Pause()
	{
		if (this->CanPause())
		{
			this->DoPause();

			state_ = SS_Paused;
		}
	}

	// ֹͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Stop()
	{
		if (this->CanStop())
		{
			this->DoStop();

			state_ = SS_Stopped;
		}
	}
}
