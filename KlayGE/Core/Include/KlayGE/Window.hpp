// Window.hpp
// KlayGE Window�� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2007-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ����Core (2008.10.16)
//
// 3.7.0
// ʵ���Ե�linux֧�� (2008.5.19)
//
// 3.6.0
// ���ν��� (2007.6.26)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_HPP
#define _WINDOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/RenderSettings.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4512 4913 6011)
#endif
#include <boost/signals2.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#else
#include <agile.h>
#endif
#elif defined KLAYGE_PLATFORM_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <glloader/glloader.h>
#elif defined KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#endif

#include <string>

namespace KlayGE
{
	class KLAYGE_CORE_API Window
	{
	public:
		Window(std::string const & name, RenderSettings const & settings);
		Window(std::string const & name, RenderSettings const & settings, void* native_wnd);
		~Window();

#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void Recreate();

		HWND HWnd() const
		{
			return wnd_;
		}
#else
		void SetWindow(Platform::Agile<Windows::UI::Core::CoreWindow> const & window);

		Platform::Agile<Windows::UI::Core::CoreWindow> GetWindow() const
		{
			return wnd_;
		}
#endif
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* XDisplay() const
		{
			return x_display_;
		}

		::XVisualInfo* VisualInfo() const
		{
			return vi_;
		}

		::Window XWindow() const
		{
			return x_window_;
		}
#elif defined KLAYGE_PLATFORM_ANDROID
		::ANativeWindow* AWindow() const
		{
			return a_window_;
		}
#endif

		int32_t Left() const
		{
			return left_;
		}
		int32_t Top() const
		{
			return top_;
		}

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}

		bool Active() const
		{
			return active_;
		}
		void Active(bool active)
		{
			active_ = active;
		}
		bool Ready() const
		{
			return ready_;
		}
		void Ready(bool ready)
		{
			ready_ = ready;
		}
		bool Closed() const
		{
			return closed_;
		}
		void Closed(bool closed)
		{
			closed_ = closed;
		}

	public:
		typedef boost::signals2::signal<void(Window const & wnd, bool active)> ActiveEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> PaintEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> EnterSizeMoveEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> ExitSizeMoveEvent;
		typedef boost::signals2::signal<void(Window const & wnd, bool active)> SizeEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> SetCursorEvent;
		typedef boost::signals2::signal<void(Window const & wnd, wchar_t ch)> CharEvent;
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		typedef boost::signals2::signal<void(Window const & wnd, HRAWINPUT ri)> RawInputEvent;
#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
		typedef boost::signals2::signal<void(Window const & wnd, HTOUCHINPUT hti, uint32_t num_inputs)> TouchEvent;
#endif
#endif
#endif
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id)> PointerDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id)> PointerUpEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id, bool down)> PointerUpdateEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id, int32_t wheel_delta)> PointerWheelEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> CloseEvent;

		ActiveEvent& OnActive()
		{
			return active_event_;
		}
		PaintEvent& OnPaint()
		{
			return paint_event_;
		}
		EnterSizeMoveEvent& OnEnterSizeMove()
		{
			return enter_size_move_event_;
		}
		ExitSizeMoveEvent& OnExitSizeMove()
		{
			return exit_size_move_event_;
		}
		SizeEvent& OnSize()
		{
			return size_event_;
		}
		SetCursorEvent& OnSetCursor()
		{
			return set_cursor_event_;
		}
		CharEvent& OnChar()
		{
			return char_event_;
		}
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		RawInputEvent& OnRawInput()
		{
			return raw_input_event_;
		}
#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
		TouchEvent& OnTouch()
		{
			return touch_event_;
		}
#endif
#endif
#endif
		PointerDownEvent& OnPointerDown()
		{
			return pointer_down_event_;
		}
		PointerUpEvent& OnPointerUp()
		{
			return pointer_up_event_;
		}
		PointerUpdateEvent& OnPointerUpdate()
		{
			return pointer_update_event_;
		}
		PointerWheelEvent& OnPointerWheel()
		{
			return pointer_wheel_event_;
		}
		CloseEvent& OnClose()
		{
			return close_event_;
		}

	private:
		ActiveEvent active_event_;
		PaintEvent paint_event_;
		EnterSizeMoveEvent enter_size_move_event_;
		ExitSizeMoveEvent exit_size_move_event_;
		SizeEvent size_event_;
		SetCursorEvent set_cursor_event_;
		CharEvent char_event_;
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		RawInputEvent raw_input_event_;
#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
		TouchEvent touch_event_;
#endif
#endif
#endif
		PointerDownEvent pointer_down_event_;
		PointerUpEvent pointer_up_event_;
		PointerUpdateEvent pointer_update_event_;
		PointerWheelEvent pointer_wheel_event_;
		CloseEvent close_event_;

#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
#elif defined KLAYGE_PLATFORM_LINUX
	public:
		void MsgProc(XEvent const & event);
#elif defined KLAYGE_PLATFORM_ANDROID
	public:
		static void HandleCMD(android_app* app, int32_t cmd);
		static int32_t HandleInput(android_app* app, AInputEvent* event);
#endif

	private:
		int32_t left_;
		int32_t top_;
		uint32_t width_;
		uint32_t height_;

		bool active_;
		bool ready_;
		bool closed_;

#if defined KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_COMPILER_GCC
		std::string name_;
#else
		std::wstring wname_;
#endif

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND wnd_;
		WNDPROC default_wnd_proc_;
#else
		Platform::Agile<Windows::UI::Core::CoreWindow> wnd_;
#endif
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* x_display_;
		::GLXFBConfig* fbc_;
		::XVisualInfo* vi_;
		::Window x_window_;
		::Atom wm_delete_window_;
#elif defined KLAYGE_PLATFORM_ANDROID
		::ANativeWindow* a_window_;
#endif

		bool external_wnd_;
	};
}

#endif		// _WINDOW_HPP
