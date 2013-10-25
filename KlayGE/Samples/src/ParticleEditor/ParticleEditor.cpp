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
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Camera.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#ifdef Bool
#undef Bool		// foreach
#endif

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "ParticleEditor.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable()
			: RenderableHelper(L"Terrain")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = SyncLoadRenderEffect("ParticleEditor.fxml");
			depth_tech_ = effect->TechniqueByName("TerrainDepth");
			color_tech_ = effect->TechniqueByName("Terrain");
			technique_ = color_tech_;

			*(effect->ParameterByName("grass_tex")) = ASyncLoadTexture("grass.dds", EAH_GPU_Read | EAH_Immutable);

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 vertices[] =
			{
				float3(-10, 0, +10),
				float3(+10, 0, +10),
				float3(-10, 0, -10),
				float3(+10, 0, -10),
			};

			ElementInitData init_data;
			init_data.row_pitch = sizeof(vertices);
			init_data.slice_pitch = 0;
			init_data.data = &vertices[0];
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]));
			pos_aabb_.Min().y() = -0.1f;
			pos_aabb_.Max().y() = +0.1f;
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("proj")) = proj;

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1.0f / camera.FarPlane());
		}

		virtual void Pass(PassType type) KLAYGE_OVERRIDE
		{
			switch (type)
			{
			case PT_OpaqueDepth:
				technique_ = depth_tech_;
				break;

			default:
				technique_ = color_tech_;
				break;
			}
		}

	private:
		RenderTechniquePtr depth_tech_;
		RenderTechniquePtr color_tech_;
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject()
			: SceneObjectHelper(MakeSharedPtr<TerrainRenderable>(), SOA_Cullable)
		{
		}
	};

	int const NUM_PARTICLE = 4096;


	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};
}


int SampleMain()
{
	ParticleEditorApp app;
	app.Create();
	app.Run();

	return 0;
}

ParticleEditorApp::ParticleEditorApp()
					: App3DFramework("Particle Editor")
{
	ResLoader::Instance().AddPath("../../Samples/media/ParticleEditor");
}

bool ParticleEditorApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
	{
		return false;
	}

	return true;
}

void ParticleEditorApp::InitObjects()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-1.2f, 0.5f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpsController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&ParticleEditorApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	terrain_ = MakeSharedPtr<TerrainObject>();
	terrain_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
	{
		depth_texture_support_ = true;
	}
	else
	{
		depth_texture_support_ = false;
	}

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	if (!depth_texture_support_)
	{
		scene_depth_buffer_ = rf.MakeFrameBuffer();
		scene_depth_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	}
	else
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToSM");
	}

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");

	UIManager::Instance().Load(ResLoader::Instance().Open("ParticleEditor.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	id_save_as_ = dialog_->IDFromName("SaveAs");
	id_gravity_static_ = dialog_->IDFromName("GravityStatic");
	id_gravity_slider_ = dialog_->IDFromName("GravitySlider");
	id_density_static_ = dialog_->IDFromName("DensityStatic");
	id_density_slider_ = dialog_->IDFromName("DensitySlider");
	id_freq_static_ = dialog_->IDFromName("FreqStatic");
	id_freq_slider_ = dialog_->IDFromName("FreqSlider");
	id_angle_static_ = dialog_->IDFromName("AngleStatic");
	id_angle_slider_ = dialog_->IDFromName("AngleSlider");
	id_detail_x_static_ = dialog_->IDFromName("DetailXStatic");
	id_detail_x_slider_ = dialog_->IDFromName("DetailXSlider");
	id_detail_y_static_ = dialog_->IDFromName("DetailYStatic");
	id_detail_y_slider_ = dialog_->IDFromName("DetailYSlider");
	id_detail_z_static_ = dialog_->IDFromName("DetailZStatic");
	id_detail_z_slider_ = dialog_->IDFromName("DetailZSlider");
	id_min_velocity_static_ = dialog_->IDFromName("MinVelocityStatic");
	id_min_velocity_slider_ = dialog_->IDFromName("MinVelocitySlider");
	id_max_velocity_static_ = dialog_->IDFromName("MaxVelocityStatic");
	id_max_velocity_slider_ = dialog_->IDFromName("MaxVelocitySlider");
	id_min_life_static_ = dialog_->IDFromName("MinLifeStatic");
	id_min_life_slider_ = dialog_->IDFromName("MinLifeSlider");
	id_max_life_static_ = dialog_->IDFromName("MaxLifeStatic");
	id_max_life_slider_ = dialog_->IDFromName("MaxLifeSlider");
	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_particle_alpha_from_button_ = dialog_->IDFromName("ParticleAlphaFromButton");
	id_particle_alpha_to_button_ = dialog_->IDFromName("ParticleAlphaToButton");
	id_particle_color_from_button_ = dialog_->IDFromName("ParticleColorFromButton");
	id_particle_color_to_button_ = dialog_->IDFromName("ParticleColorToButton");
	id_curve_type_ = dialog_->IDFromName("CurveTypeCombo");
	id_size_over_life_ = dialog_->IDFromName("SizeOverLifePolyline");
	id_mass_over_life_ = dialog_->IDFromName("MassOverLifePolyline");
	id_opacity_over_life_ = dialog_->IDFromName("OpacityOverLifePolyline");

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::OpenHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::SaveAsHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UISlider>(id_gravity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::GravityChangedHandler, this, KlayGE::placeholders::_1));
	this->GravityChangedHandler(*dialog_->Control<UISlider>(id_gravity_slider_));
	dialog_->Control<UISlider>(id_density_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DensityChangedHandler, this, KlayGE::placeholders::_1));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));
	dialog_->Control<UISlider>(id_freq_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::FreqChangedHandler, this, KlayGE::placeholders::_1));
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));
	dialog_->Control<UISlider>(id_angle_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::AngleChangedHandler, this, KlayGE::placeholders::_1));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));
	dialog_->Control<UISlider>(id_detail_x_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailXChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailXChangedHandler(*dialog_->Control<UISlider>(id_detail_x_slider_));
	dialog_->Control<UISlider>(id_detail_y_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailYChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailYChangedHandler(*dialog_->Control<UISlider>(id_detail_y_slider_));
	dialog_->Control<UISlider>(id_detail_z_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailZChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailZChangedHandler(*dialog_->Control<UISlider>(id_detail_z_slider_));
	dialog_->Control<UISlider>(id_min_velocity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MinVelocityChangedHandler, this, KlayGE::placeholders::_1));
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));
	dialog_->Control<UISlider>(id_max_velocity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MaxVelocityChangedHandler, this, KlayGE::placeholders::_1));
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));
	dialog_->Control<UISlider>(id_min_life_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MinLifeChangedHandler, this, KlayGE::placeholders::_1));
	this->MinLifeChangedHandler(*dialog_->Control<UISlider>(id_min_life_slider_));
	dialog_->Control<UISlider>(id_max_life_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MaxLifeChangedHandler, this, KlayGE::placeholders::_1));
	this->MaxLifeChangedHandler(*dialog_->Control<UISlider>(id_max_life_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::FPSCameraHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UITexButton>(id_particle_alpha_from_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleAlphaFromHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_alpha_to_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleAlphaToHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_color_from_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleColorFromHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_color_to_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleColorToHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UIComboBox>(id_curve_type_)->OnSelectionChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::CurveTypeChangedHandler, this, KlayGE::placeholders::_1));
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	ElementFormat fmt;
	if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
	{
		fmt = EF_B10G11R11F;
	}
	else if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
		fmt = EF_ARGB8;
	}
	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	if (caps.rendertarget_format_support(EF_R16F, 1, 0))
	{
		fmt = EF_R16F;
	}
	else if (caps.rendertarget_format_support(EF_R32F, 1, 0))
	{
		fmt = EF_R32F;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

		fmt = EF_ABGR16F;
	}
	scene_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}

	RenderViewPtr ds_view;
	if (depth_texture_support_)
	{
		scene_ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		ds_view = rf.Make2DDepthStencilRenderView(*scene_ds_tex_, 0, 1, 0);

		depth_to_linear_pp_->InputPin(0, scene_ds_tex_);
		depth_to_linear_pp_->OutputPin(0, scene_depth_tex_);
	}
	else
	{
		ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);

		scene_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_depth_tex_, 0, 1, 0));
		scene_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
	}

	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 1, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	copy_pp_->InputPin(0, scene_tex_);

	if (ps_)
	{
		ps_->SceneDepthTexture(scene_depth_tex_);
	}

	UIManager::Instance().SettleCtrls();
}

void ParticleEditorApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ParticleEditorApp::OpenHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(nullptr, IDC_WAIT));

		if (last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = fn;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);

		this->LoadParticleSystem(fn);

		SetCursor(cur);
	}
#endif
}

void ParticleEditorApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ".psml";

	if (GetSaveFileNameA(&ofn))
	{
		this->SaveParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::FreqChangedHandler(KlayGE::UISlider const & sender)
{
	float freq = static_cast<float>(sender.GetValue() * 10);
	particle_emitter_->Frequency(freq);

	std::wostringstream stream;
	stream << "Freq: " << freq;
	dialog_->Control<UIStatic>(id_freq_static_)->SetText(stream.str());
}

void ParticleEditorApp::AngleChangedHandler(KlayGE::UISlider const & sender)
{
	float angle = static_cast<float>(sender.GetValue());
	particle_emitter_->EmitAngle(angle * DEG2RAD);

	std::wostringstream stream;
	stream << "Emit Angle: " << angle;
	dialog_->Control<UIStatic>(id_angle_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailXChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.x() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail X: " << p.x();
	dialog_->Control<UIStatic>(id_detail_x_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailYChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.y() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail Y: " << p.y();
	dialog_->Control<UIStatic>(id_detail_y_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailZChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.z() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail Z: " << p.z();
	dialog_->Control<UIStatic>(id_detail_z_static_)->SetText(stream.str());
}

void ParticleEditorApp::MinVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	particle_emitter_->MinVelocity(velocity);

	std::wostringstream stream;
	stream << "Min Vel.: " << velocity;
	dialog_->Control<UIStatic>(id_min_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::MaxVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	particle_emitter_->MaxVelocity(velocity);

	std::wostringstream stream;
	stream << "Max Vel.: " << velocity;
	dialog_->Control<UIStatic>(id_max_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::MinLifeChangedHandler(KlayGE::UISlider const & sender)
{
	float min_life = static_cast<float>(sender.GetValue());
	particle_emitter_->MinLife(min_life);

	std::wostringstream stream;
	stream << "Min Life: " << min_life;
	dialog_->Control<UIStatic>(id_min_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::MaxLifeChangedHandler(KlayGE::UISlider const & sender)
{
	float max_life = static_cast<float>(sender.GetValue());
	particle_emitter_->MaxLife(max_life);

	std::wostringstream stream;
	stream << "Max Life: " << max_life;
	dialog_->Control<UIStatic>(id_max_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::GravityChangedHandler(KlayGE::UISlider const & sender)
{
	float gravity = sender.GetValue() / 100.0f;
	ps_->Gravity(gravity);

	std::wostringstream stream;
	stream << "Gravity: " << gravity;
	dialog_->Control<UIStatic>(id_gravity_static_)->SetText(stream.str());
}

void ParticleEditorApp::DensityChangedHandler(KlayGE::UISlider const & sender)
{
	float density = sender.GetValue() / 100.0f;
	ps_->MediaDensity(density);

	std::wostringstream stream;
	stream << "Density: " << density;
	dialog_->Control<UIStatic>(id_density_static_)->SetText(stream.str());
}

void ParticleEditorApp::FPSCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpsController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpsController_.DetachCamera();
	}
}

void ParticleEditorApp::ChangeParticleAlphaFromHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		ps_->ParticleAlphaFromTex(fn);
		this->LoadParticleAlpha(id_particle_alpha_from_button_, ps_->ParticleAlphaFromTex());
	}
#endif
}

void ParticleEditorApp::ChangeParticleAlphaToHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		ps_->ParticleAlphaToTex(fn);
		this->LoadParticleAlpha(id_particle_alpha_to_button_, ps_->ParticleAlphaToTex());
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorFromHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().r());
	clr_srgb.g() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().g());
	clr_srgb.b() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().b());

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = clr_srgb.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		Color clr_srgb(occ.rgbResult);
		Color clr_linear;
		clr_linear.r() = MathLib::srgb_to_linear(clr_srgb.b());
		clr_linear.g() = MathLib::srgb_to_linear(clr_srgb.g());
		clr_linear.b() = MathLib::srgb_to_linear(clr_srgb.r());
		ps_->ParticleColorFrom(clr_linear);
		this->LoadParticleColor(id_particle_color_from_button_, clr_linear);
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorToHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(ps_->ParticleColorTo().r());
	clr_srgb.g() = MathLib::linear_to_srgb(ps_->ParticleColorTo().g());
	clr_srgb.b() = MathLib::linear_to_srgb(ps_->ParticleColorTo().b());

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = clr_srgb.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		Color clr_srgb(occ.rgbResult);
		Color clr_linear;
		clr_linear.r() = MathLib::srgb_to_linear(clr_srgb.b());
		clr_linear.g() = MathLib::srgb_to_linear(clr_srgb.g());
		clr_linear.b() = MathLib::srgb_to_linear(clr_srgb.r());
		ps_->ParticleColorTo(clr_linear);
		this->LoadParticleColor(id_particle_color_to_button_, clr_linear);
	}
#endif
}

void ParticleEditorApp::CurveTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	uint32_t ct = sender.GetSelectedIndex();
	dialog_->GetControl(id_size_over_life_)->SetVisible(false);
	dialog_->GetControl(id_mass_over_life_)->SetVisible(false);
	dialog_->GetControl(id_opacity_over_life_)->SetVisible(false);
	switch (ct)
	{
	case 0:
		dialog_->GetControl(id_size_over_life_)->SetVisible(true);
		break;

	case 1:
		dialog_->GetControl(id_mass_over_life_)->SetVisible(true);
		break;

	default:
		dialog_->GetControl(id_opacity_over_life_)->SetVisible(true);
		break;
	}
}

void ParticleEditorApp::LoadParticleAlpha(int id, std::string const & name)
{
	TexturePtr tex = SyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable);

	TexturePtr tex_for_button;
	if (EF_R8 == tex->Format())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr cpu_tex = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, EF_R8, 1, 0, EAH_CPU_Read, nullptr);
		tex->CopyToTexture(*cpu_tex);

		std::vector<uint8_t> data(tex->Width(0) * tex->Height(0) * 4);
		{
			Texture::Mapper mapper(*cpu_tex, 0, 0, TMA_Read_Only, 0, 0, tex->Width(0), tex->Height(0));
			uint8_t const * p = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < tex->Height(0); ++ y)
			{
				for (uint32_t x = 0; x < tex->Width(0); ++ x)
				{
					uint8_t d = p[y * mapper.RowPitch() + x];
					data[(y * tex->Width(0) + x) * 4 + 0] = d;
					data[(y * tex->Width(0) + x) * 4 + 1] = d;
					data[(y * tex->Width(0) + x) * 4 + 2] = d;
					data[(y * tex->Width(0) + x) * 4 + 3] = 0xFF;
				}
			}
		}

		ElementInitData init_data;
		init_data.data = &data[0];
		init_data.row_pitch = tex->Width(0) * 4;
		tex_for_button = rf.MakeTexture2D(cpu_tex->Width(0), cpu_tex->Height(0), 1, 1,
			rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);
	}
	else
	{
		tex_for_button = tex;
	}
	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleColor(int id, KlayGE::Color const & clr)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

	ElementFormat fmt = caps.texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8;

	uint32_t data;
	data = 0xFF000000 | ((EF_ABGR8 == fmt) ? clr.ABGR() : clr.ARGB());
	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	TexturePtr tex_for_button = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);

	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleSystem(std::string const & name)
{
	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->ClearCtrlPoints();

	if (ps_)
	{
		ps_->DelFromSceneManager();
	}
	ps_ = SyncLoadParticleSystem(name);
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);
	ps_->AddToSceneManager();

	if (scene_depth_tex_)
	{
		ps_->SceneDepthTexture(scene_depth_tex_);
	}

	particle_emitter_ = ps_->Emitter(0);
	particle_updater_ = ps_->Updater(0);

	particle_emitter_->MinSpin(-PI / 2);
	particle_emitter_->MaxSpin(+PI / 2);
	particle_emitter_->ModelMatrix(MathLib::translation(0.0f, 0.1f, 0.0f));

	this->LoadParticleAlpha(id_particle_alpha_from_button_, ResLoader::Instance().Locate(ps_->ParticleAlphaFromTex()));
	this->LoadParticleAlpha(id_particle_alpha_to_button_, ResLoader::Instance().Locate(ps_->ParticleAlphaToTex()));

	this->LoadParticleColor(id_particle_color_from_button_, ps_->ParticleColorFrom());
	this->LoadParticleColor(id_particle_color_to_button_, ps_->ParticleColorTo());

	dialog_->Control<UISlider>(id_freq_slider_)->SetValue(static_cast<int>(particle_emitter_->Frequency() * 0.1f + 0.5f));
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));

	dialog_->Control<UISlider>(id_angle_slider_)->SetValue(static_cast<int>(particle_emitter_->EmitAngle() * RAD2DEG + 0.5f));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));

	dialog_->Control<UISlider>(id_detail_x_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().x() * 1000.0f + 0.5f));
	this->DetailXChangedHandler(*dialog_->Control<UISlider>(id_detail_x_slider_));

	dialog_->Control<UISlider>(id_detail_y_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().y() * 1000.0f + 0.5f));
	this->DetailYChangedHandler(*dialog_->Control<UISlider>(id_detail_y_slider_));

	dialog_->Control<UISlider>(id_detail_z_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().z() * 1000.0f + 0.5f));
	this->DetailZChangedHandler(*dialog_->Control<UISlider>(id_detail_z_slider_));

	dialog_->Control<UISlider>(id_min_velocity_slider_)->SetValue(static_cast<int>(particle_emitter_->MinVelocity() * 100.0f + 0.5f));
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));

	dialog_->Control<UISlider>(id_max_velocity_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxVelocity() * 100.0f + 0.5f));
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));

	dialog_->Control<UISlider>(id_min_life_slider_)->SetValue(static_cast<int>(particle_emitter_->MinLife()));
	this->MinLifeChangedHandler(*dialog_->Control<UISlider>(id_min_life_slider_));

	dialog_->Control<UISlider>(id_max_life_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxLife()));
	this->MaxLifeChangedHandler(*dialog_->Control<UISlider>(id_max_life_slider_));

	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife());
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife());
	dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife());
}

void ParticleEditorApp::SaveParticleSystem(std::string const & name)
{
	KlayGE::SaveParticleSystem(ps_, name);
}

void ParticleEditorApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Particle System", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ParticleEditorApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	if (depth_texture_support_)
	{
		switch (pass)
		{
		case 0:
			{
				re.BindFrameBuffer(scene_buffer_);

				float q = this->ActiveCamera().FarPlane() / (this->ActiveCamera().FarPlane() - this->ActiveCamera().NearPlane());
				float2 near_q(this->ActiveCamera().NearPlane() * q, q);
				depth_to_linear_pp_->SetParam(0, near_q);
			
				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife(dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife(dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->GetCtrlPoints());

				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_Need_Flush;

		default:
			depth_to_linear_pp_->Apply();

			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

			copy_pp_->Apply();

			terrain_->Visible(false);
			ps_->Visible(true);

			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
	}
	else
	{
		switch (pass)
		{
		case 0:
			{
				re.BindFrameBuffer(scene_depth_buffer_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(100.0f, 100.0f, 100.0f, 1), 1.0f, 0);

				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife(dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife(dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->GetCtrlPoints());

				terrain_->Pass(PT_OpaqueDepth);
				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_Need_Flush;

		case 1:
			{
				re.BindFrameBuffer(scene_buffer_);
			
				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				terrain_->Pass(PT_OpaqueShading);
				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_Need_Flush;

		default:
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

			copy_pp_->Apply();

			terrain_->Visible(false);
			ps_->Visible(true);

			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
	}
}
