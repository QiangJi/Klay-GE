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

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	class NullAudioBuffer : public AudioBuffer
	{
	public:
		NullAudioBuffer()
			: AudioBuffer(AudioDataSource::NullObject())
		{
		}

		void Play(bool /*loop*/)
			{ }
		void Reset()
			{ }
		void Stop()
			{ }

		void Volume(float /*vol*/)
			{ }

		bool IsPlaying() const
			{ return false; }
		bool IsSound() const
			{ return true; }

		float3 Position() const
			{ return float3::Zero(); }
		void Position(float3 const & /*v*/)
			{ }
		float3 Velocity() const
			{ return float3::Zero(); }
		void Velocity(float3 const & /*v*/)
			{ }
		float3 Direction() const
			{ return float3::Zero(); }
		void Direction(float3 const & /*v*/)
			{ }
	};


	AudioBuffer::AudioBuffer(AudioDataSourcePtr const & dataSource)
			: dataSource_(dataSource),
				format_(dataSource->Format()),
				freq_(dataSource->Freq())
	{
	}

	AudioBuffer::~AudioBuffer()
	{
	}

	AudioBufferPtr AudioBuffer::NullObject()
	{
		static AudioBufferPtr obj = MakeSharedPtr<NullAudioBuffer>();
		return obj;
	}
}
