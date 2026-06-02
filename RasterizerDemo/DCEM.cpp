#include "DCEM.h"

DCEM::DCEM(ID3D11Device* device, XMFLOAT3 initPos)
{
	Initialize(device, initPos);
}

DCEM::~DCEM()
{
	for (int i = 0; i < 6; i++)
	{
		//if (rtv[i])
		//{
		//	rtv[i]->Release();
		//	rtv[i] = nullptr;
		//}
		if (uav[i])
		{
			uav[i]->Release();
			uav[i] = nullptr;
		}
	}
	if (srv) { srv->Release(); srv = nullptr; };
	if (dsView) { dsView->Release(); dsView = nullptr; }
	if (mesh) { delete mesh; mesh = nullptr; }
}

void DCEM::Initialize(ID3D11Device* device, XMFLOAT3 initPos)
{
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 1024;
	desc.Height = 1024;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
		throw std::runtime_error("Failed to create texture cube!");

	//D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.Format = desc.Format;
	//rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	//rtvDesc.Texture2DArray.ArraySize = 1;
	//rtvDesc.Texture2DArray.MipSlice = 0;

	//for (int i = 0; i < 6; i++)
	//{
	//	rtvDesc.Texture2DArray.FirstArraySlice = i;
	//	if (FAILED(device->CreateRenderTargetView(texture, &rtvDesc, &this->rtv[i])))
	//		throw std::runtime_error("");
	//}

	if (FAILED(device->CreateShaderResourceView(texture, nullptr, &this->srv)))
		throw std::runtime_error("");

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.ArraySize = 1;

	ProjectionInfo projInfo1;
	projInfo1.aspectRatio = 1.0f;
	projInfo1.fovAngleY = DirectX::XM_PIDIV2;
	projInfo1.nearZ = 0.1f;
	projInfo1.farZ = 100.0f;
	float upRot[6] = { DirectX::XM_PIDIV2, -DirectX::XM_PIDIV2, 0.0f, 0.0f, 0.0f, DirectX::XM_PI };
	float rightRot[6] = { 0.0f,0.0f, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2, 0.0f,0.0f };

	ID3D11DeviceContext* immediatecontext;
	device->GetImmediateContext(&immediatecontext);

	// For each side:
	for (int i = 0; i < 6; i++)
	{
		// Create UAV
		uavDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateUnorderedAccessView(texture, &uavDesc, &this->uav[i])))
			throw std::runtime_error("Could not create UAV for DCEM!");

		// Set Camera
		cameras[i].Initialize(device, projInfo1, initPos);
		cameras[i].RotateUp(upRot[i]);
		cameras[i].RotateRight(rightRot[i]);
		cameras[i].UpdateInternalConstantBuffer(immediatecontext);
	}

	// Creating the depth stencil

	// Depth Texture
	ID3D11Texture2D* depthTexture;
	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = 1024;
	depthTextureDesc.Height = 1024;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&depthTextureDesc, nullptr, &depthTexture)))
		throw std::runtime_error("");


	// Depth STencil View for whole cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthTextureDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	if (FAILED(device->CreateDepthStencilView(depthTexture, &dsvDesc, &this->dsView)))
		throw std::runtime_error("");

	texture->Release();
	depthTexture->Release();
	immediatecontext->Release();

	ZeroMemory(&this->viewport, sizeof(D3D11_VIEWPORT));
	this->viewport = { 0, 0, 1024, 1024, 0, 1 };
	// TopLeftX, TopLeftY, Width, Height, MinDepth, MaxDepth

	// Adding the mesh
	const std::string folderPath = "SimpleObjects/";
	const std::string objName = "cube";
	this->mesh = new MeshD3D11(device, folderPath, objName);
	XMMATRIX world = XMMatrixTranslation(initPos.x, initPos.y, initPos.z);
	this->worldMatrix = world;

	XMFLOAT4X4 world4x4T;
	XMStoreFloat4x4(&world4x4T, XMMatrixTranspose(world));
	this->worldMatrixBuffer.Initialize(device, sizeof(XMFLOAT4X4), &world4x4T);

}

void DCEM::Update(ID3D11DeviceContext* context)
{
	this->cameras->UpdateInternalConstantBuffer(context);
}

void DCEM::Draw(ID3D11DeviceContext* context)
{
	ID3D11Buffer* pMatrix = this->worldMatrixBuffer.GetBuffer();
	this->mesh->BindMeshBuffers(context);
	context->VSSetConstantBuffers(1, 1, &pMatrix);

	context->PSSetShaderResources(4, 1, &this->srv);
	
	// One Cube = One Mesh :)
	this->mesh->PerformSubMeshDrawCall(context, 0);

	ID3D11ShaderResourceView* srvNULL = nullptr;
	context->PSSetShaderResources(4, 1, &srvNULL);
}

ID3D11Buffer* DCEM::GetCameraVP(int nr) { return this->cameras[nr].GetConstantBuffer();}

const ID3D11UnorderedAccessView* DCEM::GetUAV(int nr) { return this->uav[nr]; }

//const ID3D11RenderTargetView* DCEM::GetRTV(int nr) { return this->rtv[nr]; }

const ID3D11ShaderResourceView* DCEM::GetSRV() { return this->srv; }

const ID3D11DepthStencilView* DCEM::GetdsView() { return this->dsView; }

const D3D11_VIEWPORT DCEM::GetViewport() { return this->viewport; }
