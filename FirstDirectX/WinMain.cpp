#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevinstance, LPSTR lmCmdLine, int nCmdShow)
{
	try 
	{
		return App{}.Go();
	}
	catch (const ErrorException& e) {
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e) {
		MessageBoxA(nullptr, e.what(), "Standard Error Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...) {
		MessageBoxA(nullptr, "No Details", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}

}

