#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSGIPostProcess.hpp>

namespace KlayGE
{
	SSGIPostProcess::SSGIPostProcess()
			: PostProcess(L"SSGI")
	{
		input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("shading_tex", TexturePtr()));

		output_pins_.push_back(std::make_pair("out_tex", TexturePtr()));

		this->Technique(SyncLoadRenderEffect("SSGI.fxml")->TechniqueByName("SSGI"));

		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		proj_param_ = technique_->Effect().ParameterByName("proj");
		inv_proj_param_ = technique_->Effect().ParameterByName("inv_proj");
	}

	void SSGIPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = camera.InverseProjMatrix();
	}
}
