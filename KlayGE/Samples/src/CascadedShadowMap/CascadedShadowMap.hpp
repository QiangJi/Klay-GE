#ifndef _DEFERREDRENDERING_HPP
#define _DEFERREDRENDERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class CascadedShadowMapApp : public KlayGE::App3DFramework
{
public:
	CascadedShadowMapApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void CSMTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void CascadesChangedHandler(KlayGE::UIComboBox const & sender);
	void PSSMFactorChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::RenderModelPtr ground_model_;
	KlayGE::RenderModelPtr scene_model_;
	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::function<KlayGE::RenderModelPtr()> ground_ml_;
	KlayGE::function<KlayGE::RenderModelPtr()> model_ml_;
	KlayGE::function<KlayGE::TexturePtr()> y_cube_tl_;
	KlayGE::function<KlayGE::TexturePtr()> c_cube_tl_;
	uint32_t loading_percentage_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayerPtr deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;

	int id_csm_type_combo_;
	int id_cascades_combo_;
	int id_pssm_factor_static_;
	int id_pssm_factor_slider_;
	int id_ctrl_camera_;

	KlayGE::SunLightSourcePtr sun_light_;

	size_t num_objs_rendered_;
	size_t num_renderable_rendered_;
	size_t num_primitives_rendered_;
	size_t num_vertices_rendered_;

	uint32_t num_cascades_;
	float pssm_factor_;
};

#endif		// _DEFERREDRENDERING_HPP
