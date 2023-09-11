#pragma once
#include <MyDirectXBuild.h>

#include <Windows.h>
#include <string>
#include <chrono>

namespace EScene
{
	/// <summary>
	/// Thread Object:
	/// <para> >Runs a function given by a pointer of function defenetion </para>
	/// <para> >On destruction, the thread will wait until the thread finishes </para>
	/// </summary>
	class Thread
	{
	public:
		Thread()
		{
			refCount = new int;
			*refCount = -1;
			//-1 Means dead/null object
		}
		Thread(const Thread& otherThread)
		{
			this->operator=(otherThread);
		}
		/// <summary>
		/// Create a Thread Object
		/// </summary>
		/// <param name="threadFunctionP">Pointer to the function being used.</param>
		/// <param name="ThreadInput">Input for the given thread function.Default none</param>
		Thread(LPTHREAD_START_ROUTINE threadFunctionP,LPVOID ThreadInput = NULL) 
		{
			CreateThread(NULL, NULL, threadFunctionP, ThreadInput, 0, &ThreadID);
			refCount = new int;
			*refCount = 1;
		}
		~Thread() 
		{
			*refCount -= 1;
			wait();
		}
		/// <summary>
		/// Wait for Thread to finish
		/// </summary>
		/// <param name="tMilli">By default it will wait infinately</param>
		void wait(int tMilli = 0)
		{
			if (tMilli == 0)tMilli = INFINITE;
			if (*refCount < 0)return;
			WaitForSingleObject(&ThreadID, tMilli);
		}
		void operator=(const Thread& otherThread)
		{
			ThreadID = otherThread.ThreadID;
			refCount = otherThread.refCount;
			if (*refCount > 0) *refCount += 1;
		}
	private:
		int* refCount;
		DWORD ThreadID = 0;
	};

	//Basic Game Object
	class Object
	{
	public:
		//Object Tick
		virtual void Update() {};
		//Draw Object
		virtual  void Draw() {};

		void DestroySound() {};
	};
	/// <summary>
	/// Delta Time Object:
	/// <para> >Should be ticked/updated every frame. </para>
	/// <para> >Can return DeltaTime between frames. </para>
	/// </summary>
	class Time
	{
	public:
		Time() 
		{
			pastTime = std::chrono::high_resolution_clock::now();
		}
		void Update()
		{
			//Update DeltaTime
			std::chrono::high_resolution_clock Clock;
			std::chrono::high_resolution_clock::time_point timeNow = Clock.now();
			DeltaTime = timeNow - pastTime;
			pastTime = timeNow;
		}
		double GetMilliSeconds() 
		{
			//Get DeltaTime in milliseconds
			std::chrono::duration<double, std::milli> rTime = DeltaTime;
			return rTime.count();
		}
		double GetSeconds()
		{
			//Get DletaTime in Secoond
			std::chrono::duration<double, std::ratio<1>> rTime = DeltaTime;
			return rTime.count();
		}
	private:
		std::chrono::high_resolution_clock::time_point pastTime;
		std::chrono::high_resolution_clock::duration DeltaTime;
	}Time;

	//Key input
	namespace Key
	{
		//Change char to keycode
		int CharToKey(wchar_t InputChar)
		{
			SHORT r = VkKeyScanW(InputChar);

			BYTE rByte[2];
			memcpy(&rByte, &r, sizeof(BYTE) * 2);
			return rByte[0];
		}
		//Check if keycode is pressed
		bool Check(int vKeyCode) { return GetAsyncKeyState(vKeyCode); }
		bool CheckChar(wchar_t InputChar) { return GetAsyncKeyState(CharToKey(InputChar)); }

	}
}

namespace EScene
{
	/// <summary>
	/// Imformation for EGame::Window Class
	/// </summary>
	struct WindowInfo
	{
		HWND Hwnd;
		LPCWSTR WindowName;
		LPCWSTR WindowClassName;
		HINSTANCE hInstance;
		HMENU MenuHandle;
		LONG WndProcP; //Parent Window Procedure
		WNDCLASS WindowClass;
	};

	class Window
	{
	public:
		Window() {}
		Window(WNDCLASS WindowClass, LPCWSTR WindowName, DWORD WindowType, RECT Position = {0, 0, 100, 100}, HMENU Menu = NULL)
		{
			Info.WindowClass = WindowClass;
			Info.MenuHandle = Menu;

			Info.Hwnd = CreateWindow(
				WindowClass.lpszClassName,
				WindowName,
				WindowType,
				Position.left, Position.top,
				Position.right, Position.bottom,
				NULL, Menu,
				WindowClass.hInstance,NULL
			);
		}
		Window(
			EScene::Window& parentWindow,
			LPCWSTR WindowName,DWORD WindowType,
			RECT Position = { 0,0,100,100 } , HMENU Menu = NULL,
			LPCWSTR WindowClassName = NULL)
		{
			WindowInfo parentInfo = parentWindow.GetInfo();
			if (!IsWindow(parentInfo.Hwnd)) return;
			if (WindowClassName == NULL) 
			{
				WindowClassName = parentInfo.WindowClass.lpszClassName;
			}
			WindowType = (WindowType | WS_CHILD); //Force child window type
			Info.Hwnd = CreateWindow(
				WindowClassName,
				WindowName, WindowType,
				Position.left, Position.top,
				Position.right, Position.bottom,
				parentWindow.Info.Hwnd, Menu,
				parentInfo.WindowClass.hInstance,
				NULL
			);
			Info.WindowClass = parentInfo.WindowClass;
			Info.WindowName = WindowName;
			Info.MenuHandle = Menu;
		}
		//Gets the message for itself or any of its child windows.
		
		bool GetMSG(){
			if (!IsWindow(Info.Hwnd)) { return false;  }
			MSG Message;
			int messageInt = GetMessage(&Message, Info.Hwnd, 0, 0);
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			if (messageInt > 0) { return true; }
			else { return false; }
		}
		HWND GetHwnd() { return Info.Hwnd; }
		WindowInfo GetInfo() { return Info; }
		RECT GetSize() {
			RECT rRect = { 0 };
			GetWindowRect(Info.Hwnd, &rRect);
			return rRect;
		}
		RECT GetClientSize() {
			RECT rRect;
			GetClientRect(Info.Hwnd, &rRect);
			return rRect;
		}
		void Move(RECT Position) {
			SetWindowPos(Info.Hwnd,0,
				Position.left,
				Position.top,
				Position.right,
				Position.bottom,
				0
			);
		}
		void Show(int ShowType = SW_SHOW)
		{
			ShowWindow(Info.Hwnd, ShowType);
		}
		void Clear(HBRUSH Colour = HBRUSH(RGB(0,0,0)))
		{
			PAINTSTRUCT PS;
			HDC DC = BeginPaint(Info.Hwnd, &PS);
			FillRect(DC, &PS.rcPaint, Colour);
			EndPaint(Info.Hwnd, &PS);
		}
		void Destory()
		{
			DestroyWindow(Info.Hwnd);
		}
		operator bool() { return IsWindow(Info.Hwnd); }

	private:
		WindowInfo Info;
		LPCWSTR WName;
		LPCWSTR ClassName;
		HINSTANCE parentHinstance;
	};

	// Window class that supports DirectX 11 Graphics
	class DXWindow : public Window
	{
	public:
		DXWindow() {}
		DXWindow(Window& parentWindow, LPCWSTR WindowName, DWORD WindowType, RECT Position = { 0,0,100,100 })
			:Window(parentWindow,WindowName,WindowType,Position){
			RenderTarget = EDX::HWNDRenderTarget(GetHwnd());
		}
		DXWindow(WNDCLASS WindowClass, LPCWSTR WindowName, DWORD WindowType, RECT Position = {0,0,300,300}, HMENU Menu = NULL)
			:Window(WindowClass, WindowName, WindowType, Position, Menu)
		{
			RenderTarget = EDX::HWNDRenderTarget(GetHwnd());
		}
		void SetAsRenderTarget()
		{
			RenderTarget.SetAsRenderTarget();
		}
		void ReSizeBuffer(){
			RenderTarget.ResizeBuffer();
		}
		void Clear(DirectX::XMFLOAT4 colour = DirectX::XMFLOAT4(0, 0, 0, 1)) {
			RenderTarget.Clear(colour);
		}
		void Flip(int vSync = 1) {
			RenderTarget.Flip(vSync);
		}
	private:
		EDX::HWNDRenderTarget RenderTarget;
	};
}






