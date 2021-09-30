#pragma once
#include "App.h"
#include <iomanip> 
#include <string>

App::App()
	:
	window(800,600,L"First DirectX")
{}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessage())
		{
			//if there's msg, return the exit code
			return *ecode;
		}
		DoFrame();
	}
	int i = 0;
}

void App::DoFrame() {
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	window.Gfx().clearBuffer(c, c, 1);
	window.Gfx().DrawTriangle(
		-timer.Peek(),
		0.0f,
		0.0f
	);
	window.Gfx().DrawTriangle(
		timer.Peek(),
		(float)window.mouse.GetPosX() / 400.0f - 1.0f,
		(float)-window.mouse.GetPosY() / 300.0f + 1.0f
	);
	window.Gfx().EndFrame();
}
