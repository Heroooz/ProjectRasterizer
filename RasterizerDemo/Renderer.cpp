#include "Renderer.h"

using namespace DirectX;

Renderer::Renderer(Window& window) : window(window), device(nullptr), immediateContext(nullptr), swapChain(nullptr), rtv(nullptr), dsTexture(nullptr),
                                     dsView(nullptr), viewport(), vShader(nullptr), pShader(nullptr), inputLayout(nullptr), vertexBuffer(nullptr), 
                                     vsConstantBuffer(nullptr), psConstantBuffer(nullptr), texture(nullptr), srv(nullptr), samplerState(nullptr), 
                                     renderTargetD3D11(), depthBufferD3D11(), vsConstantBufferD3D11(), psConstantBufferD3D11(),
                                     vertexBufferD3D11(), camera(), rotation(0.0f) {

    if (!Renderer::Initialize()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        throw std::runtime_error("Failed to initialize renderer!");
    }
}

Renderer::~Renderer() {
	//if (samplerState) samplerState->Release();
    if (samplerState) samplerState->~SamplerD3D11();
	if (srv) srv->Release();
	if (texture) texture->Release();
	if (vertexBuffer) vertexBuffer->Release();
	if (psConstantBuffer) psConstantBuffer->Release();
	if (vsConstantBuffer) vsConstantBuffer->Release();
	//if (inputLayout) inputLayout->Release();

    if (psShader[0]) psShader[0]->~ShaderD3D11();
    if (psShader[1]) psShader[1]->~ShaderD3D11();
    if (vsShader) vsShader->~ShaderD3D11();
    if (csShader) csShader->~ShaderD3D11();

    if (inputLayout) inputLayout->~InputLayoutD3D11();
	if (pShader) pShader->Release();
	if (vShader) vShader->Release();
	if (dsView) dsView->Release();
	if (dsTexture) dsTexture->Release();
	//if (rtv) rtv->Release();
	if (swapChain) swapChain->Release();
	if (immediateContext) immediateContext->Release();
	if (device) device->Release();
    

    if (scene) scene->~Scene();
    if (pCamera) pCamera->Release();
    if (pWorldMatrix) pWorldMatrix->Release();
}

bool Renderer::Initialize() {

    // SetupD3D11
    if (!SetupDeviceAndSwapChain()) {
        std::cerr << "Failed to setup device and swap chain!" << std::endl;
        throw std::runtime_error("Failed to setup device and swap chain!");
        return false;
    }
	// Setup Pipeline, shaders, input layout, texture, sampler state
    if (!SetupPipeline(device, immediateContext, vsShader, psShader[0], psShader[1], csShader, inputLayout, texture, srv, samplerState)) {
        std::cerr << "Failed to setup pipeline!" << std::endl;
        throw std::runtime_error("Failed to setup pipeline!");
        return false;
    }

    SetupRenderTarget();
    SetupDepthStencil();
    SetupViewport();

    // G-Buffers
    this->positionBuffer.Initialize(device, window.GetWidth(), window.GetHeight());
    this->normalBuffer.Initialize(device, window.GetWidth(), window.GetHeight());
    this->diffuseBuffer.Initialize(device, window.GetWidth(), window.GetHeight());

    this->rtvArr[0] = this->positionBuffer.GetRTV();
    this->rtvArr[1] = this->normalBuffer.GetRTV();
    this->rtvArr[2] = this->diffuseBuffer.GetRTV();

    this->srvArr[0] = this->positionBuffer.GetSRV();
    this->srvArr[1] = this->normalBuffer.GetSRV();
    this->srvArr[2] = this->diffuseBuffer.GetSRV();


	CreateUnorderedAccessView();

    // End of SetupD3D11


    // Creating Quad
    SimpleVertex simpleQuad[] =
    {
        { {-1.0f, 1.0f, 0.0f}, {0, 0, -1}, {0, 0} },
        { {1.0f, 1.0f, 0.0f}, {0, 0, -1}, {1, 0} },
        { {-1.0, -1.0f, 0.0f}, {0, 0, -1}, {0, 1} },
        { {1.0f, -1.0f, 0.0f}, {0, 0, -1}, {1, 1} }
    };

    struct MaterialBuffer
    {
        DirectX::XMFLOAT3 ambientFactor = { 0.5f, 0.5f, 0.5f };
        float shininess = 100.0f;
        DirectX::XMFLOAT3 diffuseFactor = { 0.5f, 0.5f, 0.5f };
        float parallax = 0.0f;
        DirectX::XMFLOAT3 specularFactor = { 0.2f, 0.2f, 0.2f };
        float padding3 = 0.0f;
        int hasAmbientTexture = 0;
        int hasDiffuseTexture = 1;
        int hasSpecularTexture = 0;
        int hasNormalTexture = 0;
    } quadMaterial;
    psConstantBufferD3D11.Initialize(device, sizeof(MaterialBuffer), &quadMaterial);
    psConstantBuffer = psConstantBufferD3D11.GetBuffer();
   
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(float) * 4;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = simpleQuad;

    vertexBuffers[0].Initialize(device, sizeof(SimpleVertex), 4, simpleQuad);
    ID3D11Buffer* buffer = vertexBuffers[0].GetBuffer();
    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &buffer);

    //vertexBuffers[1].Initialize(device, sizeof(SimpleVertex), 4, simpleQuad); // Can have same vb
    //buffer = vertexBuffers[1].GetBuffer();
    //hr = device->CreateBuffer(&bufferDesc, &initData, &buffer);


    Transform transform1 =
    {
        {0, 0, 20},
        {0, 0, 0},
        {1, 1, 1},
    };
    Transform transform2 =
    {
        {0, 0, 10},
        { 0, 3.141592f, 0},
        { 1, 1, 1}
    };

    worldMatrices[0] =
        XMMatrixScaling(transform1.scale[0], transform1.scale[1], transform1.scale[2]) *
        XMMatrixRotationRollPitchYaw(transform1.rotation[0], transform1.rotation[1], transform1.rotation[2]) *
        XMMatrixTranslation(transform1.position[0], transform1.position[1], transform1.position[2]);
    worldMatrices[1] =
        XMMatrixScaling(transform2.scale[0], transform2.scale[1], transform2.scale[2]) *
        XMMatrixRotationRollPitchYaw(transform2.rotation[0], transform2.rotation[1], transform2.rotation[2]) *
        XMMatrixTranslation(transform2.position[0], transform2.position[1], transform2.position[2]);

    DirectX::XMFLOAT4X4 worldTransform;

    DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixTranspose(worldMatrices[0]));
    worldMatriceBuffers[0].Initialize(device, sizeof(XMFLOAT4X4), &worldTransform);
    worldMatriceBuffers[0].UpdateBuffer(immediateContext, &worldTransform);


    DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixTranspose(worldMatrices[1]));
    worldMatriceBuffers[1].Initialize(device, sizeof(XMFLOAT4X4), &worldTransform); 
    worldMatriceBuffers[1].UpdateBuffer(immediateContext, &worldTransform);



	// Setup constant buffers (for vertex shader and pixel shader)
    //CreateVSConstantBuffer(device, vsConstantBufferD3D11, matrixArr, rotation, window.GetWidth(), window.GetHeight());

	// Setup camera (projection info and initialize)
    ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XMConvertToRadians(90.0f);
	projInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 100.0f;
    camera.Initialize(device, projInfo, DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f));

    // Creating the Scene (w objs and light)
    LoadObjects();
    CreateLights(device);


    // Binding The Sampler To The Shaders
    ID3D11SamplerState* pSamplerState = samplerState->GetSamplerState();
    immediateContext->PSSetSamplers(0, 1, &pSamplerState);
    immediateContext->CSSetSamplers(0, 1, &pSamplerState);


    pSamplerState->Release();
    buffer->Release();



    return true;
}

void Renderer::Render() {

	time.Update(); // Update frame timing

    
    ClearBuffers();
    //float clearColour[4] = { 0.1f, 0.4f, 0.5f, 1 };
    //immediateContext->ClearRenderTargetView(rtv, clearColour);
    //immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);



    immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    immediateContext->RSSetViewports(1, &viewport);
	vsShader->BindShader(immediateContext);
    psShader[0]->BindShader(immediateContext);

    // Binding camera to VS, PS, CS
    camera.UpdateInternalConstantBuffer(immediateContext);
    CameraBuffer camPS = {};
    camPS.viewProjMatrix = camera.GetViewProjectionMatrix();
    camPS.cameraPosition = camera.GetPosition();
    camPS.padding = 0.0f;
    ConstantBufferD3D11 camBufferPS(device, sizeof(CameraBuffer), &camPS);
    pCamera = camBufferPS.GetBuffer();
    immediateContext->VSSetConstantBuffers(0, 1, &pCamera);
    immediateContext->PSSetConstantBuffers(0, 1, &pCamera);
    immediateContext->CSSetConstantBuffers(0, 1, &pCamera);

    GeometryPass();



    //immediateContext->OMSetRenderTargets(0, nullptr, dsView);

    // Drawing the quads (Same: vertexbuffer, texture and material, different: worldmatrices)
    immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    vertexBuffer = vertexBuffers[0].GetBuffer();
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    immediateContext->PSSetShaderResources(1, 1, &srv);
    immediateContext->PSSetConstantBuffers(1, 1, &psConstantBuffer);
    for (int i = 0; i < 0; i++)
    {
	    // Bind and set pipeline states, then draw
	    // Sending stuff to VS
        //immediateContext->VSSetShader(vShader, nullptr, 0);
        XMFLOAT4X4 worldMatrixT;
        DirectX::XMStoreFloat4x4(&worldMatrixT, DirectX::XMMatrixTranspose(worldMatrices[i]));
        worldMatriceBuffers[i].UpdateBuffer(immediateContext, &worldMatrixT);
        pWorldMatrix = worldMatriceBuffers[i].GetBuffer();

        immediateContext->VSSetConstantBuffers(1, 1, &pWorldMatrix);
        immediateContext->Draw(4, 0);
        // Sending stuff to PS
        //immediateContext->PSSetShader(pShader, nullptr, 0);
        //immediateContext->PSSetConstantBuffers(1, 1, &lightPS);
        //immediateContext->PSSetConstantBuffers(0, 1, &psConstantBuffer);
        //immediateContext->OMSetRenderTargets(1, &rtv, dsView);
        //immediateContext->OMSetRenderTargets(3, rtvArr, dsView);
    }

   
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    scene->DrawObjects(immediateContext);

    LightPass();

    swapChain->Present(0, 0);
}



void Renderer::GeometryPass()
{
    immediateContext->OMSetRenderTargets(3, rtvArr, dsView);
    
    psShader[0]->BindShader(immediateContext);
    immediateContext->PSSetShaderResources(0, 3, srvNULL);
}

void Renderer::LightPass()
{
    immediateContext->OMSetRenderTargets(0, nullptr, dsView);
    csShader->BindShader(immediateContext);
    immediateContext->CSSetShaderResources(0, 3, srvArr);
    immediateContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    ID3D11ShaderResourceView* srvSpotLight = this->scene->GetLightBufferSRV();
    ID3D11ShaderResourceView* srvDirLight = this->scene->GetLightBufferSRV(true);

    immediateContext->CSSetShaderResources(3, 1, &srvSpotLight);
    immediateContext->CSSetShaderResources(4, 1, &srvDirLight);

    // Felberäkningar i Blinn-Phong är min bästa gissnign
    ID3D11Buffer* nrofLights = this->scene->GetnrofLightBuffer();
    immediateContext->CSSetConstantBuffers(1, 1, &nrofLights);

    // Dispatching threads to CS
    UINT dispatchX = (window.GetWidth() + 7) / 8;
    UINT dispatchY = (window.GetHeight() + 7) / 8;
    immediateContext->Dispatch(dispatchX, dispatchY, 1);


    // Unbinding
    immediateContext->CSSetShaderResources(0, 3, srvNULL);
    immediateContext->CSSetUnorderedAccessViews(0, 1, &uavNULL, nullptr);

    srvSpotLight->Release();
    srvDirLight->Release();
    nrofLights->Release();
    
}

void Renderer::ClearBuffers()
{
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    immediateContext->ClearRenderTargetView(rtv, clearColor);

    for (int i = 0; i < 3; i++)
    {
        immediateContext->ClearRenderTargetView(rtvArr[i], clearColor);
    }
    immediateContext->ClearUnorderedAccessViewFloat(uav, clearColor);
    immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}




bool Renderer::SetupDeviceAndSwapChain() {
    UINT flags = 0;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferDesc.Width = window.GetWidth();
    swapChainDesc.BufferDesc.Height = window.GetHeight();
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Default
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    //swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = window.GetHWND();
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &immediateContext);

    return !(FAILED(hr));
}

void Renderer::SetupRenderTarget() {
	renderTargetD3D11.Initialize(device, swapChain, window.GetWidth(), window.GetHeight());
	rtv = renderTargetD3D11.GetRTV();
}

void Renderer::SetupDepthStencil() {
	depthBufferD3D11.Initialize(device, window.GetWidth(), window.GetHeight());
    dsView = depthBufferD3D11.GetDSV(0);
}

void Renderer::SetupViewport() {
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(window.GetWidth());
    viewport.Height = static_cast<float>(window.GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    immediateContext->RSSetViewports(1, &viewport);
}

//void Renderer::CreateVertexBuffer(ID3D11Device* device, VertexBufferD3D11& vertexBufferD3D11, int nrOfVertices, void* vertexData) {
//	vertexBufferD3D11.Initialize(device, sizeof(SimpleVertex), nrOfVertices, vertexData);
//	vertexBuffer = vertexBufferD3D11.GetBuffer();
//}

//void Renderer::CreateVSConstantBuffer(ID3D11Device* device, ConstantBufferD3D11& vsConstantBufferD3D11, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT) 
//{
//    DirectX::XMMATRIX worldMatrix = CreateWorldMatrix({0.0f, 0.0f, 0.0f}, { 0.0f, rotation, 0.0f }, { 1.0f, 1.0f, 1.0f });
//    DirectX::XMFLOAT4X4 worldMatrixTransposed;
//    DirectX::XMStoreFloat4x4(&worldMatrixTransposed, DirectX::XMMatrixTranspose(worldMatrix));
//
//    worldMatrixBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4X4), &worldMatrixTransposed);
//    pWorldMatrix = worldMatrixBuffer.GetBuffer();
//}

void Renderer::CreateLights(ID3D11Device* device) 
{
    LightData data1 = {};
    data1.perLightInfo.initialPosition = { 3.0f, 20.0f, -2.5f };
    data1.perLightInfo.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    data1.perLightInfo.intensity = 0.7f;
    data1.perLightInfo.angle = XM_PI;

    LightData data2 = {};
    data2.perLightInfo.initialPosition = { 0.2f, 5.0f, -10.0f };
    data2.perLightInfo.color = { 0.0f,1.0f,0.0f,1.0f };
    data2.perLightInfo.intensity = 0.7f;
    data2.perLightInfo.angle = XM_PI;
    data2.perLightInfo.isDir = true;

    scene->AddLight(device, data1);
    scene->AddLight(device, data2);

    scene->InitializeLight(device);
}

bool Renderer::CreateUnorderedAccessView()
{
    ID3D11Texture2D* backbuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)))) {
        std::cerr << "Failed to get back buffer for UAV creation!" << std::endl;
        return false;
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray = { 0, 0, 1 };


    if (FAILED(device->CreateUnorderedAccessView(backbuffer, &desc, &uav))) {
        backbuffer->Release();
        std::cerr << "Failed to create UAV!" << std::endl;
        return false;
    }

    backbuffer->Release();
    return true;
}

void Renderer::LoadObjects()
{
    //scene->AddObject(device, "Horse/", "Horse", XMFLOAT3(0, 0, 10), XMFLOAT3(0, PI, 0), XMFLOAT3(1, 1, 1));
    scene->AddObject(device, "SimpleObjects/", "Untitled", { -5, 2, 2 }, { 0, 0, 0 }, { 0.7f, 0.7f, 0.7f });
    //scene->AddObject(device, "Cat/", "12221_Cat_v1_l3", XMFLOAT3(1, 1, 5), XMFLOAT3(-XM_PI / 2, XM_PI, 0), XMFLOAT3(0.05f, 0.05f, 0.05f));
    //scene->AddObject(device, "Box/", "box", XMFLOAT3(0, -2, 2), XMFLOAT3(0, 0, 0), XMFLOAT3(2, 2, 2));

    scene->AddObject(device, "Torch/", "torch", { 0.2f, 5.0f, -10.0f }, { 0.0f, XM_PI, 0.0f }, { 0.03f, 0.03f, 0.03f });

    scene->AddObject(device, "Fountain/", "fountain", { 0, 0, 0 }, { 0, XM_PI, 0 }, { 1, 1, 1 });
    scene->AddObject(device, "Circle/", "circle", { 0, 0.5, 0 }, { XM_PI, 0, 0 }, { 3, 3, 3 });

    scene->AddObject(device, "Duck/", "rubberduckie", { 0.0f, 0.4f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { 0.0f, 0.4f, -2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { 2.0f, 0.4f, 0.0f }, { 0.0f, XM_PIDIV2, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { -2.0f, 0.4f, 0.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { 1.4f, 0.4f, 1.4f }, { 0.0f, XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { 1.4f, 0.4f, -1.4f }, { 0.0f, 3 * XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { -1.4f, 0.4f, 1.4f }, { 0.0f, -XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene->AddObject(device, "Duck/", "rubberduckie", { -1.4f, 0.4f, -1.4f }, { 0.0f, -3.0f * XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);


    //scene->AddObject(device, "Fish/", "AnglerFish", { -5.0f, 2.0f, 2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.7f, 0.7f, 0.7f });
    //scene->AddObject(device, "Fish/", "Blobfish", { -5.0f, 1.0f, -2.0f }, { 0.0f, -3.0f * XM_PIDIV4,0.0f }, { 0.5f, 0.5f, 0.5f });

    //scene->AddObject(device, "Windmill/", "low-poly-mill", { 15.0f, 0.0f ,20.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.1f, 0.1f, 0.1f });
}
ID3D11DeviceContext*& Renderer::GetContext()
{
    return this->immediateContext;
}
CameraD3D11& Renderer::GetCamera()
{
    return this->camera;
}

Scene& Renderer::GetScene()
{
    return *this->scene;
}

float Renderer::GetDeltatime()
{
    this->time.Update();
    return this->time.GetDeltaTime();
}


DirectX::XMMATRIX CreateWorldMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale) 
{
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(rotation.x);
    return DirectX::XMMatrixMultiply(translationMatrix, rotationMatrix);
}
