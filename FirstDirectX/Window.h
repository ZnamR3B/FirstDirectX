#pragma once
#include <comip.h>
#include "MyWin.h"
#include "ErrorException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>


class Window
{
public:
	class Exception : public ErrorException
	{
		using ErrorException::ErrorException;
	public:
		static std::string TranslateErrorCode(HRESULT hResult) noexcept;
	};
	class HRException : public Exception
	{
	public:
		HRException(int line, const char* file, HRESULT hResult) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;		
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hResult;
	};
	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
	class WindowClass
	{
	public: 
		static LPCWSTR GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator =(const WindowClass&) = delete;
		static constexpr LPCWSTR wndClassName = L"DirectX Engine Window";
		static WindowClass wndClass; //instance as a singleton
		HINSTANCE hInstance;
	};
public:
	Window(int width, int height, LPCWSTR name);
	~Window();
	Window(const Window&) = delete;
	Window& operator =(const Window&) = delete;
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessage();
	Graphics& Gfx();
private:
	//LRESULT CALLBACK HandleMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMessageThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	Keyboard keyboard;
	Mouse mouse;
private:
	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
};

//error exception helper macro
#define WND_EXCEPT(hResult) Window::HRException(__LINE__, __FILE__, (hResult))
#define WND_LAST_EXCEPT() Window::HRException(__LINE__, __FILE__, GetLastError())
#define WND_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )