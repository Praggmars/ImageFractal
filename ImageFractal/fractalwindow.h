#pragma once

#include "common.h"

namespace fractal
{
	struct float2
	{
		float x, y;
	};

	struct alignas(alignof(float)) FractalParams
	{
		float2 center;
		float2 steepness;
		float2 screenSize;
		float zoom;
		float iterationCount;
	};

	class FractalWindow
	{
		HWND m_window;

		AutoReleasePtr<ID3D11Device> m_device3D;
		AutoReleasePtr<ID3D11DeviceContext> m_context3D;
		AutoReleasePtr<IDXGISwapChain> m_swapChain;
		AutoReleasePtr<ID3D11RenderTargetView> m_renderTargetView;
		AutoReleasePtr<ID3D11RasterizerState> m_rasterizerState;
		AutoReleasePtr<ID3D11Texture2D> m_depthStencilBuffer;
		AutoReleasePtr<ID3D11DepthStencilView> m_depthStencilView;
		AutoReleasePtr<ID3D11DepthStencilState> m_depthStencilState;
		D3D11_VIEWPORT m_viewport;

		AutoReleasePtr<ID3D11VertexShader> m_vertexShader;
		AutoReleasePtr<ID3D11PixelShader> m_pixelShader;
		AutoReleasePtr<ID3D11Buffer> m_vertexBuffer;
		AutoReleasePtr<ID3D11Buffer> m_indexBuffer;
		AutoReleasePtr<ID3D11InputLayout> m_inputLayout;
		AutoReleasePtr<ID3D11Buffer> m_cbFractalParams;
		AutoReleasePtr<ID3D11ShaderResourceView> m_image;

		FractalParams m_fractalParams;
		unsigned m_imageWidth;
		unsigned m_imageHeight;
		float m_windowScale;

		float2 m_prevCursor;

	private:
		void InitGraphics(int width, int height);
		void CreateWindowsizeDependentResources(int width, int height);
		void CreateResources();
		void CreateVertexShader();
		void CreatePixelShader();

		void DropFileEvent(HDROP hDrop);

		void MouseMove(WPARAM wparam, LPARAM lparam);
		void MouseWheel(WPARAM wparam, LPARAM lparam);
		void KeyDown(WPARAM wparam);
		void Resize();
		void Paint();
		void Redraw();

		void DefaultFractalParams();
		float2 ScreenToCoords(float2 screen);
		float2 CoordsToScreen(float2 coords);

		void SetClientSize(int width, int height);
		void FitWindow();
		void LoadImageFile(const wchar_t* filename);

	public:
		FractalWindow();
		~FractalWindow();
		void Init(const wchar_t* windowName, int x, int y, int width, int height);

		LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};
}