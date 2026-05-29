#pragma once

#include <iostream>
#include <d3d11_4.h>

class GBuffer
{
private:
	ID3D11Texture2D* texture = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;

public:
	GBuffer() = default;
	GBuffer(ID3D11Device* device, UINT width, UINT height);
	~GBuffer();
	GBuffer(const GBuffer& other) = delete;
	GBuffer& operator=(const GBuffer& other) = delete;
	GBuffer(GBuffer&& other) = delete;
	GBuffer& operator=(GBuffer&& other) = delete;

	void Initialize(ID3D11Device* device, UINT width, UINT height);

	ID3D11Texture2D* GetTexture() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11RenderTargetView* GetRTV() const;
};