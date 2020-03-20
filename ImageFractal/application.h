#pragma once

#include "fractalwindow.h"

namespace fractal
{
	class Application
	{
		FractalWindow m_fractalWindow;

	public:
		Application();
		void Run();
	};
}