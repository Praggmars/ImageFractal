#include "fractalwindow.h"
#include <vector>
#include <wincodec.h>
#include <iostream>

#pragma comment (lib, "windowscodecs.lib")


constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW;
constexpr DWORD WINDOW_EX_STYLE = WS_EX_OVERLAPPEDWINDOW | WS_EX_STATICEDGE;

namespace fractal
{
	void FractalWindow::InitGraphics(int width, int height)
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, D3D11_SDK_VERSION, &m_device3D, nullptr, &m_context3D);

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		ThrowIfFailed(m_device3D->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState));
		m_context3D->RSSetState(m_rasterizerState);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = TRUE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.DepthEnable = TRUE;
		ThrowIfFailed(m_device3D->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));
	}
	void FractalWindow::CreateWindowsizeDependentResources(int width, int height)
	{
		AutoReleasePtr<IDXGIFactory> factory;
		ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

		m_renderTargetView.Release();
		m_swapChain.Release();
		m_depthStencilBuffer.Release();
		m_depthStencilView.Release();

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = m_window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		ThrowIfFailed(factory->CreateSwapChain(m_device3D, &swapChainDesc, &m_swapChain));

		AutoReleasePtr<ID3D11Texture2D> backBuffer;
		ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));
		ThrowIfFailed(m_device3D->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView));
		backBuffer.Release();

		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		ThrowIfFailed(m_device3D->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		ThrowIfFailed(m_device3D->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

		m_fractalParams.screenSize.x = static_cast<float>(width);
		m_fractalParams.screenSize.y = static_cast<float>(height);

		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width = m_fractalParams.screenSize.x;
		m_viewport.Height = m_fractalParams.screenSize.y;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_context3D->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		m_context3D->OMSetDepthStencilState(m_depthStencilState, 0);
		m_context3D->RSSetViewports(1, &m_viewport);
	}
	void FractalWindow::CreateResources()
	{
		D3D11_BUFFER_DESC bufferDesc{};
		float2 vertices[] = {
			{ -1.0f, -1.0f },
			{ -1.0f,  1.0f },
			{ 1.0f,  1.0f },
			{ 1.0f, -1.0f }
		};
		unsigned indices[] = {
		0, 1, 2, 2, 3, 0
		};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, &initData, &m_vertexBuffer));
		bufferDesc.ByteWidth = sizeof(indices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		initData.pSysMem = indices;
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer));

		CreateVertexShader();
		CreatePixelShader();

		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		bufferDesc.ByteWidth = sizeof(FractalParams);
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, NULL, &m_cbFractalParams));

		UINT stride = sizeof(float2);
		UINT offset = 0;
		UINT bufferNumber = 0;
		m_context3D->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_context3D->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_context3D->PSSetConstantBuffers(0, 1, &m_cbFractalParams);
	}
	void FractalWindow::CreateVertexShader()
	{
		const char* vsCode = R"(
struct VertexInputType
{
	float2 position : POSITION;
};
struct PixelInputType
{
	float4 windowPosition : SV_POSITION;
	float2 position : POSITION;
};
PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	output.windowPosition = float4(input.position, 1.0f, 1.0f);
	output.position = input.position;
	return output;
})";
		AutoReleasePtr<ID3DBlob> shaderByteCode;
		AutoReleasePtr<ID3DBlob> errorMessage;
		ThrowIfFailed(
			D3DCompile(
				vsCode, strlen(vsCode), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &shaderByteCode, &errorMessage),
			errorMessage ?
			reinterpret_cast<const char*>(errorMessage->GetBufferPointer()) :
			nullptr);
		ThrowIfFailed(m_device3D->CreateVertexShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_vertexShader));
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		ThrowIfFailed(m_device3D->CreateInputLayout(&inputLayoutDesc, 1, shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), &m_inputLayout));
		m_context3D->IASetInputLayout(m_inputLayout);
		m_context3D->VSSetShader(m_vertexShader, NULL, 0);
	}
	void FractalWindow::CreatePixelShader()
	{
		const char* psCode(R"(
Texture2D imageTexture;
SamplerState samplerState;

cbuffer Data
{
	float2 center;
	float2 steepness;
	float2 screenSize;
	float zoom;
	float iterationCount;
};
struct PixelInputType
{
	float4 windowPosition : SV_POSITION;
	float2 position : POSITION;
};

inline float2 ComplexMul(float2 z1, float2 z2)
{
	return float2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z1.y * z2.x);
}
inline float2 ComplexDiv(float2 z1, float2 z2)
{
	return float2(z1.x * z2.x + z1.y * z2.y, z1.y * z2.x - z1.x * z2.y) / (z2.x * z2.x + z2.y * z2.y);
}
float2 Func(float2 z)
{
	return ComplexMul(z, ComplexMul(z, z)) - float2(1.0f, 0.0f);
}
float2 Deriv(float2 z)
{
	return 3.0f * ComplexMul(z, z);
}
float NewtonFractalIter(float2 coord, float3 param)
{
	float2 z = coord + (param.gb - 0.5f) * 1.0f;
	float tolerance = 1e-5f;
	float i = 0.0f;
	while (i < iterationCount)
	{
		z -= ComplexMul(steepness, ComplexDiv(Func(z), Deriv(z)));
		if (length(z - float2(1.0f, 0.0f)) < tolerance)
			break;
		if (length(z - float2(-0.5f, 0.8660254f)) < tolerance)
			break;
		if (length(z - float2(-0.5f, -0.8660254f)) < tolerance)
			break;
		i += max(0.1f, sqrt(param.r));
	}
	return i / iterationCount;
}

float MandelbrotIter(float2 coord, float3 param)
{
	float i = 0.0f;
	float2 z = 0.0f;
	float2 c = coord + (param.xy - 0.5f) * 2.0f;
	while (i < iterationCount)
	{
		z = ComplexMul(z, z) + c;
		if (length(z) > 4.0f)
			break;
		i += param.z;
	}
	return i / iterationCount;
}

float JuliaIter(float2 coord, float3 param)
{
	float i = 0.0f;
	float2 z = coord;
	float2 c = param.xy * 2.0f - 1.0f;
	while (i < iterationCount)
	{
		z = ComplexMul(z, z) + c;
		if (length(z) > 4.0f)
			break;
		i += 1.0f;
	}
	return i / iterationCount;
}

float4 IterationsToColor(float r)
{
	float R = abs(r * 6.0f - 3.0f) - 1.0f;
	float G = 2.0f - abs(r * 6.0f - 2.0f);
	float B = 2.0f - abs(r * 6.0f - 4.0f);
	return float4(saturate(float3(R, G, B))*(1.0f - R * 0.49f), 1.0f);
}

float4 main(PixelInputType input) : SV_TARGET
{
	float2 texCoords = input.windowPosition.xy / screenSize;
	float4 color = imageTexture.Sample(samplerState, texCoords);
	float2 coord = float2(input.position.x * screenSize.x / screenSize.y, input.position.y) / zoom + center;
	float iter = NewtonFractalIter(coord, color.xyz);
	return IterationsToColor(iter);
})");
		AutoReleasePtr<ID3DBlob> shaderByteCode;
		AutoReleasePtr<ID3DBlob> errorMessage;
		ThrowIfFailed(
			D3DCompile(
				psCode, strlen(psCode), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &shaderByteCode, &errorMessage),
			errorMessage ?
			reinterpret_cast<const char*>(errorMessage->GetBufferPointer()) :
			nullptr);
		ThrowIfFailed(m_device3D->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_pixelShader));
		m_context3D->PSSetShader(m_pixelShader, NULL, 0);
	}
	void FractalWindow::DropFileEvent(HDROP hDrop)
	{
		wchar_t filename[MAX_PATH];
		filename[0] = '\0';
		DragQueryFile(hDrop, 0, filename, MAX_PATH);

		try
		{
			LoadImageFile(filename);
			Redraw();
		}
		catch (std::exception& e)
		{
			MessageBoxA(m_window, e.what(), "Error", MB_OK);
		}

		DragFinish(hDrop);
	}
	void FractalWindow::MouseMove(WPARAM wparam, LPARAM lparam)
	{
		float2 cursor = { static_cast<float>(LOWORD(lparam)), static_cast<float>(HIWORD(lparam)) };

		bool needRedraw = false;
		if (wparam & MK_LBUTTON)
		{
			m_fractalParams.center.x -= 2.0f * (cursor.x - m_prevCursor.x) / (m_fractalParams.zoom * m_fractalParams.screenSize.y);
			m_fractalParams.center.y += 2.0f * (cursor.y - m_prevCursor.y) / (m_fractalParams.zoom * m_fractalParams.screenSize.y);
			needRedraw = true;
		}
		if (wparam & MK_RBUTTON)
		{
			m_fractalParams.steepness = ScreenToCoords(cursor);
			needRedraw = true;
		}
		if (needRedraw)
		{
			Redraw();
		}
		m_prevCursor = cursor;
	}
	void FractalWindow::MouseWheel(WPARAM wparam, LPARAM lparam)
	{
		float delta = GET_WHEEL_DELTA_WPARAM(wparam) < 0 ? 1.0f / 1.1f : 1.1f;
		if (wparam & MK_CONTROL)
		{
			m_fractalParams.iterationCount *= delta;
			Redraw();
		}
		else if (wparam & MK_SHIFT)
		{
			m_windowScale *= delta;
			SetClientSize(m_imageWidth * m_windowScale, m_imageHeight * m_windowScale);
		}
		else
		{
			m_fractalParams.zoom *= delta;
		}
	}
	void FractalWindow::KeyDown(WPARAM wparam)
	{
		switch (wparam)
		{
		case 'R':
			DefaultFractalParams();
			break;
		case VK_SPACE:
			FitWindow();
			break;
		}
	}
	void FractalWindow::Resize()
	{
		if (m_swapChain)
		{
			RECT rect;
			GetClientRect(m_window, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			CreateWindowsizeDependentResources(max(1, width), max(1, height));
		}
	}
	void FractalWindow::Paint()
	{
		if (m_swapChain)
		{
			float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_context3D->ClearRenderTargetView(m_renderTargetView, color);
			D3D11_MAPPED_SUBRESOURCE resource;
			if (SUCCEEDED(m_context3D->Map(m_cbFractalParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
			{
				memcpy(resource.pData, &m_fractalParams, sizeof(FractalParams));
				m_context3D->Unmap(m_cbFractalParams, 0);
			}
			m_context3D->DrawIndexed(6, 0, 0);
			m_swapChain->Present(1, 0);
		}
	}
	void FractalWindow::Redraw()
	{
		InvalidateRect(m_window, nullptr, false);
	}
	void FractalWindow::DefaultFractalParams()
	{
		m_fractalParams.center = { 0.0f, 0.0f };
		m_fractalParams.steepness = { 1.0f, 0.0f };
		m_fractalParams.zoom = 1.0f;
		m_fractalParams.iterationCount = 100.0f;
	}
	float2 FractalWindow::ScreenToCoords(float2 screen)
	{
		return float2{
			(screen.x / m_fractalParams.screenSize.x - 0.5f) * 2.0f * m_fractalParams.screenSize.x / m_fractalParams.screenSize.y / m_fractalParams.zoom + m_fractalParams.center.x,
			(screen.y / m_fractalParams.screenSize.y - 0.5f) * -2.0f / m_fractalParams.zoom + m_fractalParams.center.y
		};
	}
	float2 FractalWindow::CoordsToScreen(float2 coords)
	{
		return float2{
			((coords.x - m_fractalParams.center.x) * m_fractalParams.zoom / m_fractalParams.screenSize.x / m_fractalParams.screenSize.y * 0.5f + 0.5f) * m_fractalParams.screenSize.x,
			((coords.y - m_fractalParams.center.y) * m_fractalParams.zoom * -0.5f + 0.5f) * m_fractalParams.screenSize.y
		};
	}
	void FractalWindow::SetClientSize(int width, int height)
	{
		RECT rect;
		GetWindowRect(m_window, &rect);
		int x = rect.left;
		int y = rect.top;
		rect.top = 0;
		rect.left = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRectEx(&rect, WINDOW_STYLE, false, WINDOW_EX_STYLE);
		MoveWindow(m_window, x, y, rect.right - rect.left, rect.bottom - rect.top, false);
	}
	void FractalWindow::FitWindow()
	{
		m_windowScale = 1.0f;
		SetClientSize(m_imageWidth, m_imageHeight);
	}
	void FractalWindow::LoadImageFile(const wchar_t* filename)
	{
		m_image.Release();

		AutoReleasePtr<IWICImagingFactory> factory;
		AutoReleasePtr<IWICBitmapDecoder> decoder;
		AutoReleasePtr<IWICBitmapFrameDecode> frame;
		AutoReleasePtr<IWICFormatConverter> converter;
		std::vector<BYTE> pixels;

		ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&factory)));
		ThrowIfFailed(factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder));
		ThrowIfFailed(decoder->GetFrame(0, &frame));
		ThrowIfFailed(factory->CreateFormatConverter(&converter));
		ThrowIfFailed(converter->Initialize(frame, GUID_WICPixelFormat32bppPRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom));
		ThrowIfFailed(converter->GetSize(&m_imageWidth, &m_imageHeight));
		pixels.resize(4ull * m_imageWidth * m_imageHeight);
		ThrowIfFailed(converter->CopyPixels(nullptr, m_imageWidth * 4, m_imageWidth * m_imageHeight * 4, pixels.data()));

		AutoReleasePtr<ID3D11Texture2D> texture;
		D3D11_TEXTURE2D_DESC t2dd{};
		t2dd.Width = m_imageWidth;
		t2dd.Height = m_imageHeight;
		t2dd.MipLevels = 0;
		t2dd.ArraySize = 1;
		t2dd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		t2dd.SampleDesc.Count = 1;
		t2dd.SampleDesc.Quality = 0;
		t2dd.CPUAccessFlags = 0;
		t2dd.Usage = D3D11_USAGE_DEFAULT;
		t2dd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		t2dd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		ThrowIfFailed(m_device3D->CreateTexture2D(&t2dd, nullptr, &texture));
		m_context3D->UpdateSubresource(texture, 0, nullptr, pixels.data(), static_cast<UINT>(m_imageWidth * 4ull), 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
		srvd.Format = t2dd.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.MipLevels = -1;
		ThrowIfFailed(m_device3D->CreateShaderResourceView(texture, &srvd, &m_image));
		m_context3D->GenerateMips(m_image);

		m_context3D->PSSetShaderResources(0, 1, &m_image);
	}
	FractalWindow::FractalWindow() :
		m_window(NULL),
		m_viewport{ 0 },
		m_prevCursor{ 0.0f, 0.0f },
		m_imageWidth(0),
		m_imageHeight(0),
		m_windowScale(1.0f)
	{
		DefaultFractalParams();
	}
	FractalWindow::~FractalWindow()
	{
		DestroyWindow(m_window);
	}
	void FractalWindow::Init(const wchar_t* windowName, int x, int y, int width, int height)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(wc);
		wc.hInstance = GetModuleHandle(NULL);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = windowName;
		wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LPARAM {
			if (msg == WM_CREATE)
			{
				LRESULT(*wndProc)(HWND, UINT, WPARAM, LPARAM) = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {
					return reinterpret_cast<FractalWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->MessageHandler(hwnd, msg, wparam, lparam);
				};
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams));
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc));
				return 0;
			}
			return DefWindowProc(hwnd, msg, wparam, lparam);
		};
		RegisterClassEx(&wc);

		RECT rect = { x, y, x + width, y + height };
		AdjustWindowRectEx(&rect, WINDOW_STYLE, false, WINDOW_EX_STYLE);

		m_window = CreateWindowEx(WINDOW_EX_STYLE, windowName, windowName, WINDOW_STYLE,
			rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, wc.hInstance, this);
		ShowWindow(m_window, SW_SHOWDEFAULT);
		DragAcceptFiles(m_window, true);
		InitGraphics(width, height);
		CreateWindowsizeDependentResources(max(1, width), max(1, height));
		CreateResources();
	}

	LRESULT FractalWindow::MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
			MouseMove(wparam, lparam);
			return 0;
		case WM_MOUSEWHEEL:
			MouseWheel(wparam, lparam);
			return 0;
		case WM_KEYDOWN:
			KeyDown(wparam);
			return 0;
		case WM_SIZE:
			Resize();
			return 0;
		case WM_PAINT:
			Paint();
			return 0;
		case WM_DROPFILES:
			DropFileEvent((HDROP)wparam);
			return 0;;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}