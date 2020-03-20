#pragma once

#include <Windows.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <d2d1_3.h>
#include <exception>
#include <string>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d2d1.lib")

#define SAFE_RELEASE(ptr) {if(ptr){ptr->Release();ptr=nullptr;}}

template <typename T>
class AutoReleasePtr
{
	T* m_ptr;

public:
	AutoReleasePtr() :m_ptr(nullptr) {}
	~AutoReleasePtr() { Release(); }
	void Release()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}
	operator T* () const { return m_ptr; }
	operator bool() const { return m_ptr != nullptr; }
	T* operator->() const { return m_ptr; }
	T** operator&() { return &m_ptr; }
	bool operator==(T* ptr) const { return m_ptr == ptr; }
	bool operator!=(T* ptr) const { return m_ptr != ptr; }
};

inline void ThrowIfFailed(HRESULT hr, const char* msg = nullptr)
{
	if (FAILED(hr))
	{
		char buffer[32];
		sprintf_s(buffer, "0x%8x", hr);
		throw std::exception(msg ? (std::string(buffer) + msg).c_str() : buffer);
		//throw std::exception(msg ? (std::to_string(hr) + msg).c_str() : std::to_string(hr).c_str());
	}
}