#include "application.h"

namespace fractal
{
	Application::Application()
	{
		m_fractalWindow.Init(L"Fractal", 200, 100, 1000, 700);
	}
	void Application::Run()
	{
		MSG msg{};
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}