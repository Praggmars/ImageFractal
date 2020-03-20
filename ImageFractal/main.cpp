#include "application.h"

int Main()
{
	try
	{
		fractal::Application app;
		app.Run();
	}
	catch (std::exception & ex)
	{
		std::string error = ex.what();
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR szCmdLine, _In_ INT nCmdShow)
{
	return Main();
}
int main()
{
	return Main();
}