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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "VertexDisplacement.hpp"

using namespace std;
using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

namespace
{
	int const LENGTH = 4;
	int const WIDTH = 3;

	class FlagRenderable : public RenderablePlane
	{
	public:
		FlagRenderable(int length_segs, int width_segs)
			: RenderablePlane(static_cast<float>(LENGTH), static_cast<float>(WIDTH), length_segs, width_segs, true, false)
		{
			technique_ = SyncLoadRenderEffect("VertexDisplacement.fxml")->TechniqueByName("VertexDisplacement");

			*(technique_->Effect().ParameterByName("flag_tex")) = ASyncLoadTexture("powered_by_klayge.dds", EAH_GPU_Read | EAH_Immutable);
			*(technique_->Effect().ParameterByName("half_length")) = LENGTH / 2.0f;
			*(technique_->Effect().ParameterByName("half_width")) = WIDTH / 2.0f;
			*(technique_->Effect().ParameterByName("lightDir")) = float3(1, 0, -1);

			AABBox const & pos_bb = this->PosBound();
			*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();
		}

		void SetAngle(float angle)
		{
			*(technique_->Effect().ParameterByName("currentAngle")) = angle;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(technique_->Effect().ParameterByName("modelview")) = camera.ViewMatrix();
			*(technique_->Effect().ParameterByName("proj")) = camera.ProjMatrix();
			*(technique_->Effect().ParameterByName("mvp")) = camera.ViewProjMatrix();
		}
	};

	class FlagObject : public SceneObjectHelper
	{
	public:
		FlagObject(int length_segs, int width_segs)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = MakeSharedPtr<FlagRenderable>(length_segs, width_segs);
		}

		void Update(float app_time, float /*elapsed_time*/)
		{
			checked_pointer_cast<FlagRenderable>(renderable_)->SetAngle(app_time / 0.4f);
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


int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	VertexDisplacement app;
	app.Create();
	app.Run();

	return 0;
}

VertexDisplacement::VertexDisplacement()
						: App3DFramework("VertexDisplacement")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/VertexDisplacement");
}

bool VertexDisplacement::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 1)
	{
		return false;
	}
	return true;
}

void VertexDisplacement::InitObjects()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	flag_ = MakeSharedPtr<FlagObject>(8, 6);
	flag_->AddToSceneManager();

	this->LookAt(float3(0, 0, -10), float3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&VertexDisplacement::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("VertexDisplacement.uiml"));
}

void VertexDisplacement::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void VertexDisplacement::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VertexDisplacement::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Vertex displacement", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
	font_->RenderText(0, 54, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t VertexDisplacement::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
