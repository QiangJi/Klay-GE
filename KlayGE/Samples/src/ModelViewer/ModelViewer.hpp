#ifndef _MODELVIEWER_HPP
#define _MODELVIEWER_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include "Model.hpp"

class ModelViewerApp : public KlayGE::App3DFramework
{
public:
	ModelViewerApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OpenModel(std::string const & name);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OpenHandler(KlayGE::UIButton const & sender);
	void SaveAsHandler(KlayGE::UIButton const & sender);
	void SkinnedHandler(KlayGE::UICheckBox const & sender);
	void FrameChangedHandler(KlayGE::UISlider const & sender);
	void PlayHandler(KlayGE::UICheckBox const & sender);
	void SmoothMeshHandler(KlayGE::UICheckBox const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);
	void MeshChangedHandler(KlayGE::UIComboBox const & sender);
	void VisualizeChangedHandler(KlayGE::UIComboBox const & sender);
	void LineModeChangedHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::PointLightSourcePtr point_light_;

	KlayGE::SceneObjectPtr model_;
	KlayGE::SceneObjectPtr axis_;
	KlayGE::SceneObjectPtr grid_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpsController_;
	KlayGE::TrackballCameraController tbController_;

	KlayGE::DeferredRenderingLayerPtr deferred_rendering_;

	float last_time_;
	float frame_;

	bool skinned_;
	bool play_;
	KlayGE::UIDialogPtr dialog_animation_;
	KlayGE::UIDialogPtr dialog_model_;
	int id_open_;
	int id_save_as_;
	int id_skinned_;
	int id_frame_static_;
	int id_frame_slider_;
	int id_play_;
	int id_smooth_mesh_;
	int id_fps_camera_;
	int id_hdr_;
	int id_mesh_;
	int id_vertex_streams_;
	int id_textures_;
	int id_visualize_;
	int id_line_mode_;

	std::string last_file_path_;
};

#endif		// _MODELVIEWER_HPP
