#include "DCEM.h"

DCEM::DCEM(ID3D11Device* device, XMFLOAT3 initPos, UINT width, UINT height, std::unique_ptr<ShaderD3D11>& DCEMPS, std::unique_ptr<ShaderD3D11>& normalPS, std::unique_ptr<ShaderD3D11>& DCEMCapturePS, std::string objName)
{
	this->DCEMPS = DCEMPS.get();
	this->normalPS = normalPS.get();
	this->cubeMapCapturePS = DCEMCapturePS.get();


	ProjectionInfo projectionInfo = {};
	projectionInfo.aspectRatio = 1.0f;
	projectionInfo.fovAngleY = DirectX::XM_PIDIV2;
	projectionInfo.nearZ = 0.1f;
	projectionInfo.farZ = 100.0f;

	XMFLOAT3 targets[6] =
	{
		{ 1.0f,  0.0f,  0.0f},
		{-1.0f,  0.0f,  0.0f},
		{ 0.0f,  1.0f,  0.0f},
		{ 0.0f, -1.0f,  0.0f},
		{ 0.0f,  0.0f,  1.0f},
		{ 0.0f,  0.0f, -1.0f}
	};
	XMFLOAT3 ups[6] =
	{
		{ 0.0f, 1.0f,  0.0f},
		{ 0.0f, 1.0f,  0.0f},
		{ 0.0f, 0.0f, -1.0f},
		{ 0.0f, 0.0f,  1.0f},
		{ 0.0f, 1.0f,  0.0f},
		{ 0.0f, 1.0f,  0.0f}
	};

	ComPtr<ID3D11DeviceContext> immediateContext;
	device->GetImmediateContext(immediateContext.GetAddressOf());

	// For each side:
	for (int i = 0; i < 6; i++)
	{
		// Set Camera
		this->cameras[i] = CameraD3D11(device, projectionInfo, initPos);

		XMFLOAT3 target = { initPos.x + targets[i].x, initPos.y + targets[i].y, initPos.z + targets[i].z };
		this->cameras[i].LookAt(initPos, target, ups[i]);

		this->cameras[i].UpdateInternalConstantBuffer(immediateContext.Get());
	}

	// Adding the mesh
	const std::string folderPath = "SimpleObjects/";
	//const std::string objName = "cube";
	this->mesh = std::make_unique<MeshD3D11>(device, folderPath, objName);
	XMMATRIX world = XMMatrixTranslation(initPos.x, initPos.y, initPos.z);
	this->worldMatrix = world;

	XMFLOAT4X4 world4x4T;
	DirectX::XMStoreFloat4x4(&world4x4T, XMMatrixTranspose(world));
	this->worldMatrixBuffer.Initialize(device, sizeof(XMFLOAT4X4), &world4x4T);

	Initialize(device, initPos, width, height);
}

void DCEM::Initialize(ID3D11Device* device, XMFLOAT3 initPos, UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if (FAILED(device->CreateTexture2D(&desc, nullptr, this->texture.GetAddressOf())))
		throw std::runtime_error("Failed to create texture cube!");

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateRenderTargetView(texture.Get(), &rtvDesc, this->rtv[i].GetAddressOf())))
			throw std::runtime_error("Could not create texture cube rtv");
	}

	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//srvDesc.TextureCube.MipLevels = 1;
	//srvDesc.TextureCube.MostDetailedMip = 0;

	if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf())))
		throw std::runtime_error("Could not create SRV for DCEM");


	 // Creating the depth stencil
	 // Depth Texture
	 ComPtr<ID3D11Texture2D> depthTexture;
	 D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	 ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	 depthTextureDesc.Width = width;
	 depthTextureDesc.Height = height;
	 depthTextureDesc.MipLevels = 1;
	 depthTextureDesc.ArraySize = 1;
	 depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	 depthTextureDesc.SampleDesc.Count = 1;
	 depthTextureDesc.SampleDesc.Quality = 0;
	 depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	 depthTextureDesc.CPUAccessFlags = 0;
	 depthTextureDesc.MiscFlags = 0;
	 
	 if (FAILED(device->CreateTexture2D(&depthTextureDesc, nullptr, depthTexture.GetAddressOf())))
	 	throw std::runtime_error("Failed to create depth texture for DCEM");
	 
	 // Depth Stencil View for whole cube
	 D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	 dsvDesc.Format = depthTextureDesc.Format;
	 dsvDesc.Flags = 0;
	 dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	 
	 if (FAILED(device->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, this->dsView.GetAddressOf())))
	 	throw std::runtime_error("Failed to create depth stencil view for DCEM");
	 
	 ZeroMemory(&this->viewport, sizeof(D3D11_VIEWPORT));
	 this->viewport = { 0, 0, (float)width, (float)height, 0, 1};
	 // TopLeftX, TopLeftY, Width, Height, MinDepth, MaxDepth
	 
}

void DCEM::Update(ID3D11DeviceContext* context)
{
	//for (size_t i = 0; i < 6; ++i)
	//{
	//	cameras[i].UpdateInternalConstantBuffer(context);
	//}
}

void DCEM::GenerateCubemap(ID3D11DeviceContext* context, const std::vector<Objects*>& sceneObjects)
{
	cubeMapCapturePS->BindShader(context);

	context->RSSetViewports(1, &viewport);

	for (int face = 0; face < 6; ++face)
	{
		float clear[4] = { 0,0,0,1 };
		context->ClearRenderTargetView(rtv[face].Get(), clear);
		context->ClearDepthStencilView(dsView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		ID3D11RenderTargetView* faceRTV = rtv[face].Get();
		context->OMSetRenderTargets(1, &faceRTV, dsView.Get());

		cameras[face].UpdateInternalConstantBuffer(context);

		ID3D11Buffer* camBuffer = cameras[face].GetConstantBuffer();
		context->VSSetConstantBuffers(0, 1, &camBuffer);
		context->PSSetConstantBuffers(0, 1, &camBuffer);

		for (auto obj : sceneObjects)
		{
			obj->drawObject(context);
		}
	}
	normalPS->BindShader(context);
}

void DCEM::Draw(ID3D11DeviceContext* context)
{
	DCEMPS->BindShader(context);
	context->PSSetShaderResources(4, 1, this->srv.GetAddressOf());

	ID3D11Buffer* pMatrix = this->worldMatrixBuffer.GetBuffer();
	context->VSSetConstantBuffers(1, 1, &pMatrix);
	context->PSSetConstantBuffers(1, 1, &pMatrix);

	this->mesh->BindMeshBuffers(context);

	for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); ++i) {
		this->mesh->PerformSubMeshDrawCall(context, i);
	}

	ComPtr<ID3D11ShaderResourceView> srvNULL = nullptr;
	context->PSSetShaderResources(4, 1, srvNULL.GetAddressOf());

	normalPS->BindShader(context);
}

ID3D11Buffer* DCEM::GetCameraVP(int nr) { return this->cameras[nr].GetConstantBuffer(); }

const ID3D11RenderTargetView* DCEM::GetRTV(int nr) { return this->rtv[nr].Get(); }

const ID3D11ShaderResourceView* DCEM::GetSRV() { return this->srv.Get(); }

const ID3D11DepthStencilView* DCEM::GetdsView() { return this->dsView.Get(); }

const D3D11_VIEWPORT DCEM::GetViewport() { return this->viewport; }
