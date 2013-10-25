// HDRPostProcess.hpp
// KlayGE HDR���ڴ����� ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// �Ľ���Tone mapping (2010.7.7)
//
// 3.4.0
// ���ν��� (2006.8.1)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _HDRPOSTPROCESS_HPP
#define _HDRPOSTPROCESS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>

#include <KFL/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SumLumPostProcess : public PostProcess
	{
	public:
		explicit SumLumPostProcess(RenderTechniquePtr const & tech);
		virtual ~SumLumPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height);

	protected:
		std::vector<float4> tex_coord_offset_;

		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SumLumLogPostProcess : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcess();
	};

	class KLAYGE_CORE_API SumLumLogPostProcessCS : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcessCS();
		void Apply();
	};

	class KLAYGE_CORE_API SumLumIterativePostProcess : public SumLumPostProcess
	{
	public:
		SumLumIterativePostProcess();
	};

	class KLAYGE_CORE_API AdaptedLumPostProcess : public PostProcess
	{
	public:
		AdaptedLumPostProcess();

		void Apply();
		void OnRenderBegin();

	private:
		TexturePtr adapted_textures_[2];
		bool last_index_;

		RenderEffectParameterPtr last_lum_tex_ep_;
		RenderEffectParameterPtr frame_delta_ep_;
	};

	class KLAYGE_CORE_API AdaptedLumPostProcessCS : public PostProcess
	{
	public:
		AdaptedLumPostProcessCS();

		void Apply();
		void OnRenderBegin();

	private:
		RenderEffectParameterPtr frame_delta_ep_;
	};

	class KLAYGE_CORE_API ImageStatPostProcess : public PostProcess
	{
	public:
		ImageStatPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;
		void Apply();

	private:
		PostProcessPtr sum_lums_1st_;
		std::vector<PostProcessPtr> sum_lums_;
		PostProcessPtr adapted_lum_;
	};

	class KLAYGE_CORE_API ImageStatPostProcessCS : public PostProcess
	{
	public:
		ImageStatPostProcessCS();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;
		void Apply();

	private:
		PostProcessPtr sum_lums_1st_;
		PostProcessPtr adapted_lum_;
	};

	class KLAYGE_CORE_API LensEffectsPostProcess : public PostProcess
	{
	public:
		LensEffectsPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;
		void Apply();

	private:
		PostProcessPtr bright_pass_downsampler_;
		PostProcessPtr downsamplers_[2];
		PostProcessPtr blurs_[3];
		PostProcessPtr glow_merger_;
	};

	class KLAYGE_CORE_API FFTLensEffectsPostProcess : public PostProcess
	{
	public:
		FFTLensEffectsPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;		
		void Apply();

	private:
		PostProcessPtr bilinear_copy_pp_;
		PostProcessPtr bright_pass_pp_;
		PostProcessPtr complex_mul_pp_;
		PostProcessPtr scaled_copy_pp_;

		std::vector<TexturePtr> restore_chain_;

		TexturePtr input_tex_;

		TexturePtr resized_tex_;
		TexturePtr empty_tex_;

		TexturePtr freq_real_tex_;
		TexturePtr freq_imag_tex_;
		TexturePtr pattern_real_tex_;
		TexturePtr pattern_imag_tex_;
		TexturePtr mul_real_tex_;
		TexturePtr mul_imag_tex_;

		uint32_t width_, height_;

		GpuFftPtr fft_;
		GpuFftPtr ifft_;
	};

	class KLAYGE_CORE_API ToneMappingPostProcess : public PostProcess
	{
	public:
		ToneMappingPostProcess();
	};


	class KLAYGE_CORE_API HDRPostProcess : public PostProcess
	{
	public:
		explicit HDRPostProcess(bool fft_lens_effects);

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;
		void Apply();

	private:
		PostProcessPtr image_stat_;
		PostProcessPtr lens_effects_;
		PostProcessPtr tone_mapping_;

		bool cs_support_;
		bool fp_texture_support_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
