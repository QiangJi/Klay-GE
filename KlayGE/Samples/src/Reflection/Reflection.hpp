#ifndef _SCREENSPACEREFLECTION_HPP
#define _SCREENSPACEREFLECTION_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class ScreenSpaceReflectionApp : public KlayGE::App3DFramework
{
public:
	ScreenSpaceReflectionApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void MinSampleNumHandler(KlayGE::UISlider const & sender);
	void MaxSampleNumHandler(KlayGE::UISlider const & sender);
	void EnbleReflectionHandler(KlayGE::UICheckBox const & sender);

private:
	KlayGE::TrackballCameraController tb_controller_;

	KlayGE::SceneObjectPtr teapot_;
	KlayGE::SceneObjectPtr dino_;

	KlayGE::LightSourcePtr point_light_;

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::DeferredRenderingLayerPtr deferred_rendering_;

	KlayGE::TexturePtr back_refl_tex_;
	KlayGE::TexturePtr back_refl_ds_tex_;
	KlayGE::FrameBufferPtr back_refl_fb_;

	KlayGE::CameraPtr screen_camera_;
	KlayGE::CameraPathControllerPtr screen_camera_path_;

	KlayGE::function<KlayGE::RenderModelPtr()> teapot_ml_;
	KlayGE::function<KlayGE::RenderModelPtr()> dino_ml_;
	KlayGE::function<KlayGE::TexturePtr()> y_cube_tl_;
	KlayGE::function<KlayGE::TexturePtr()> c_cube_tl_;
	uint32_t loading_percentage_;

	KlayGE::UIDialogPtr parameter_dialog_;
	int id_min_sample_num_static_;
	int id_min_sample_num_slider_;
	int id_max_sample_num_static_;
	int id_max_sample_num_slider_;
	int id_enable_reflection_;
};

#endif