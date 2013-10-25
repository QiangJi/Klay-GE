#ifndef _SHADOWCUBEMAP_HPP
#define _SHADOWCUBEMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

enum SM_TYPE
{
	SMT_DP,
	SMT_Cube,
	SMT_CubeOne,
	SMT_CubeOneInstance,
	SMT_CubeOneInstanceGS
};
	
class ShadowCubeMap : public KlayGE::App3DFramework
{
public:
	ShadowCubeMap();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void ScaleFactorChangedHandler(KlayGE::UISlider const & sender);
	void SMTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	
	KlayGE::RenderModelPtr scene_model_;
	KlayGE::RenderModelPtr teapot_model_;
	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr shadow_cube_buffer_;
	KlayGE::TexturePtr shadow_tex_;
	KlayGE::TexturePtr shadow_cube_tex_;
	KlayGE::PostProcessPtr sm_filter_pps_[6];

	KlayGE::FrameBufferPtr shadow_cube_one_buffer_;
	KlayGE::TexturePtr shadow_cube_one_tex_;

	KlayGE::FrameBufferPtr shadow_dual_buffers_[2];
	KlayGE::TexturePtr shadow_dual_texs_[2];
	KlayGE::RenderViewPtr shadow_dual_view_[2];
	KlayGE::TexturePtr shadow_dual_tex_;

	KlayGE::TexturePtr lamp_tex_;

	KlayGE::SceneObjectPtr light_proxy_;
	KlayGE::LightSourcePtr light_;

	KlayGE::function<KlayGE::RenderModelPtr()> model_ml_;
	KlayGE::function<KlayGE::RenderModelPtr()> teapot_ml_;
	KlayGE::function<KlayGE::TexturePtr()> lamp_tl_;
	uint32_t loading_percentage_;

	SM_TYPE sm_type_;

	float esm_scale_factor_;

	KlayGE::UIDialogPtr dialog_;
	int id_scale_factor_static_;
	int id_scale_factor_slider_;
	int id_sm_type_static_;
	int id_sm_type_combo_;
	int id_ctrl_camera_;
};

#endif		// _SHADOWCUBEMAP_HPP
