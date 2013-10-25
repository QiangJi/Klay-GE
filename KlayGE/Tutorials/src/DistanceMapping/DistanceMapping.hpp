#ifndef _DISTANCEMAPPING_HPP
#define _DISTANCEMAPPING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class DistanceMapping : public KlayGE::App3DFramework
{
public:
	DistanceMapping();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::SceneObjectPtr light_proxy_;
	KlayGE::LightSourcePtr light_;
};

#endif		// _DISTANCEMAPPING_HPP
