// AudioBuffer.cpp
// KlayGE �������� ͷ�ļ�
// Ver 2.0.4
// ��Ȩ����(C) ������, 2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// ���ν��� (2004.4.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/AudioDataSource.hpp>

namespace KlayGE
{
	class NullAudioDataSource : public AudioDataSource
	{
	public:
		void Open(ResIdentifierPtr const & /*file*/)
		{
		}
		void Close()
		{
		}

		AudioFormat Format() const
			{ return AF_Unknown; }
		uint32_t Freq() const
			{ return 0; }

		size_t Size()
			{ return 0; }

		size_t Read(void* /*data*/, size_t /*size*/)
			{ return 0; }
		void Reset()
			{ }
	};

	AudioDataSource::~AudioDataSource()
	{
	}

	AudioDataSourcePtr AudioDataSource::NullObject()
	{
		static AudioDataSourcePtr obj = MakeSharedPtr<NullAudioDataSource>();
		return obj;
	}

	AudioFormat AudioDataSource::Format() const
	{
		return this->format_;
	}

	uint32_t AudioDataSource::Freq() const
	{
		return this->freq_;
	}

	
	class NullAudioDataSourceFactory : public AudioDataSourceFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Audio Data Source Factory");
			return name;
		}

		AudioDataSourcePtr MakeAudioDataSource()
		{
			return AudioDataSource::NullObject();
		}
	};

	AudioDataSourceFactoryPtr AudioDataSourceFactory::NullObject()
	{
		static AudioDataSourceFactoryPtr obj = MakeSharedPtr<NullAudioDataSourceFactory>();
		return obj;
	}
}
