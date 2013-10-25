#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "GlobalIllumination.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			light.Position(float3(0, 12, -4.8f));
		}
	};

	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	GlobalIlluminationApp app;
	app.Create();
	app.Run();

	return 0;
}

GlobalIlluminationApp::GlobalIlluminationApp()
			: App3DFramework("GlobalIllumination"),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/GlobalIllumination");
}

bool GlobalIlluminationApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void GlobalIlluminationApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 18, -3), float3(-13.6f, 17.55f, -2.8f));
	this->Proj(0.1f, 500.0f);

	loading_percentage_ = 0;
	c_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read | EAH_Immutable);
	y_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read | EAH_Immutable);
	model_ml_ = ASyncLoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	spot_light_ = MakeSharedPtr<SpotLightSource>();
	spot_light_->Attrib(LightSource::LSA_IndirectLighting);
	spot_light_->Position(float3(0, 12, -4.8f));
	spot_light_->Direction(float3(0, 0, 1));
	spot_light_->Color(float3(6.0f, 5.88f, 4.38f));
	spot_light_->Falloff(float3(1, 0.1f, 0));
	spot_light_->OuterAngle(PI / 4);
	spot_light_->InnerAngle(PI / 6);
	spot_light_->BindUpdateFunc(SpotLightSourceUpdate());
	spot_light_->AddToSceneManager();

	spot_light_src_ = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_)->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&GlobalIlluminationApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("GlobalIllumination.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssgi_ = dialog_->IDFromName("SSGI");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_cg_ = dialog_->IDFromName("CG");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::IllumChangedHandler, this, KlayGE::placeholders::_1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::ILScaleChangedHandler, this, KlayGE::placeholders::_1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssgi_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::SSGIHandler, this, KlayGE::placeholders::_1));
	this->SSGIHandler(*dialog_->Control<UICheckBox>(id_ssgi_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::SSVOHandler, this, KlayGE::placeholders::_1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::HDRHandler, this, KlayGE::placeholders::_1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::AAHandler, this, KlayGE::placeholders::_1));
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::ColorGradingHandler, this, KlayGE::placeholders::_1));
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(KlayGE::bind(&GlobalIlluminationApp::CtrlCameraHandler, this, KlayGE::placeholders::_1));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	sky_box_->AddToSceneManager();
}

void GlobalIlluminationApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void GlobalIlluminationApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void GlobalIlluminationApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void GlobalIlluminationApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void GlobalIlluminationApp::SSGIHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSGIEnabled(0, sender.GetChecked());
}

void GlobalIlluminationApp::SSVOHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSVOEnabled(0, sender.GetChecked());
}

void GlobalIlluminationApp::HDRHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().HDREnabled(sender.GetChecked());
}

void GlobalIlluminationApp::AAHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().PPAAEnabled(sender.GetChecked() ? 1 : 0);
}

void GlobalIlluminationApp::ColorGradingHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().ColorGradingEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::CtrlCameraHandler(UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

void GlobalIlluminationApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Global Illumination", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t GlobalIlluminationApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 10)
			{
				TexturePtr y_cube_tex = y_cube_tl_();
				TexturePtr c_cube_tex = c_cube_tl_();
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tex, c_cube_tex);
				if (!!y_cube_tex && !!c_cube_tex)
				{
					loading_percentage_ = 10;
				}
			}
			else if (loading_percentage_ < 80)
			{
				scene_model_ = model_ml_();
				if (scene_model_)
				{
					loading_percentage_ = 80;
				}
			}
			else
			{
				uint32_t n = (scene_model_->NumMeshes() + 20 - 1) / 20;
				uint32_t s = (loading_percentage_ - 80) * n;
				uint32_t e = std::min(s + n, scene_model_->NumMeshes());
				for (uint32_t i = s; i < e; ++ i)
				{
					SceneObjectPtr so = MakeSharedPtr<SceneObjectHelper>(scene_model_->Mesh(i),
						SceneObject::SOA_Cullable);
					so->AddToSceneManager();
					scene_objs_.push_back(so);
				}

				++ loading_percentage_;
			}
		}
	}

	return deferred_rendering_->Update(pass);
}
