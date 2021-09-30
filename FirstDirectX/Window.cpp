#pragma once
#include "Window.h"
#include <string>
#include <sstream>
#include "resource.h"

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
	:
	hInstance(GetModuleHandle(nullptr))
{
	//register window class
	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMessageSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(
		GetInstance(), MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, 32, 32, 0
	));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage(
		GetInstance(), MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, 16, 16, 0
	));
	RegisterClassExW(&wc);
}

Window::WindowClass::~WindowClass(){
	UnregisterClassW(GetName(), GetInstance());
}

LPCWSTR Window::WindowClass::GetName() noexcept{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept{
	return wndClass.hInstance;
}

//Window stuff
Window::Window(int w, int h, LPCWSTR name)
	:
	width(w),
	height(h)
{
	RECT rect;
	rect.left = 100;
	rect.right = w + rect.left;
	rect.top = 100;
	rect.bottom = h + rect.top;
	if ((AdjustWindowRect(&rect, WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU, FALSE)) == 0)
	{
		throw WND_LAST_EXCEPT();
	}
	//create window and get hwnd
	hWnd = CreateWindowExW(0, WindowClass::GetName(), name,	WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, WindowClass::GetInstance(), this);
	if (!hWnd)
	{
		throw WND_LAST_EXCEPT();
	}
	// show newly created window
	ShowWindow(hWnd, SW_SHOW);
	//create graphics obj
	pGfx = std::make_unique<Graphics>(hWnd);
}

Window::~Window(){
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string& title) {
	HWND* winHandler = &hWnd;
	if (SetWindowTextA(hWnd, title.c_str()) == 0) {
		//throw WND_LAST_EXCEPT();
		std::string i = title;
	}
}

std::optional<int> Window::ProcessMessage() 
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0 , 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}

Graphics& Window::Gfx()
{
	if (pGfx == nullptr)
	{
		throw WND_NOGFX_EXCEPT();
	}
	return *pGfx;
}

LRESULT CALLBACK Window::HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
	if (msg == WM_CREATE)
	{
		// extract ptr to window class from creation data
		//* pCreate = reinterpret_cast<LPCREATESTRUCT*>(lParam);
		Window* pWnd = static_cast<Window*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
		// set WinAPI-managed user data to store ptr to window instance
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMessageThunk));
		// forward message to window instance handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	else 
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	// if we get a message before the WM_NCCREATE message, handle with default handler
}

LRESULT CALLBACK Window::HandleMessageThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// retrieve ptr to window instance
	int i = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	// forward message to window instance handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);	
}
LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg)
	{
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;	
		break;
	}
	case WM_KILLFOCUS:
	{
		keyboard.ClearState();
	}
	//keyboard message	
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (!(lParam & 0x40000000) || keyboard.AutorepeatIsEnabled()) {
			keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	}
	case WM_KEYUP:
	{
		keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	}
	case WM_CHAR:
	{
		keyboard.OnChar(static_cast<unsigned char>(wParam));
		break;
	}
	//mouse message
	case WM_MOUSEMOVE:
	{
		POINTS pt = MAKEPOINTS(lParam);
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow()) {
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		else 
		{
			if (wParam & (MK_LBUTTON | MK_LBUTTON)) {
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(pt.x, pt.y, wParam);
		break;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//Exception
Window::HRException::HRException(int line, const char* file, HRESULT hResult) noexcept
	:
	Exception(line, file),
	hResult(hResult)
{}

const char* Window::HRException::what() const noexcept {
	std::ostringstream output;
	output << " [Error Code] " << GetErrorCode() << std::endl
		<< " [Description] " << GetErrorString() << std::endl
		<< GetOriginString();
	whatBuffer = output.str();
	return whatBuffer.c_str();

}

const char* Window::HRException::GetType() const noexcept
{
	return "Window Error Exception";
}

std::string Window::HRException::GetErrorString() const noexcept {
	return TranslateErrorCode(hResult);
}

HRESULT Window::HRException::GetErrorCode() const noexcept {
	return hResult;
}

std::string Window::Exception::TranslateErrorCode(HRESULT hResult) noexcept {
	char* pMsgBuffer = nullptr;
	DWORD nMsgLen = FormatMessageA
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hResult,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuffer),
		0, nullptr
	);
	if (nMsgLen == 0) {
		return "Unidentified Error Code";
	}
	std::string errorString = pMsgBuffer;
	LocalFree(pMsgBuffer);
	return errorString;
}

const char* Window::NoGfxException::GetType() const noexcept
{
	return "Chili Window Exception [No Graphics]";
}	