#include "Renderer.h"

using namespace DirectX;

Renderer::Renderer(Window& window, Scene& scene) : window(window), device(nullptr), immediateContext(nullptr), swapChain(nullptr), rtv(nullptr),
                                     dsView(nullptr), viewport(), inputLayout(nullptr), 
                                     vsConstantBuffer(nullptr), psConstantBuffer(nullptr), texture(nullptr), srv(nullptr), samplerState(nullptr), 
                                     renderTargetD3D11(), depthBufferD3D11(), vsConstantBufferD3D11(), psConstantBufferD3D11(),
                                     camera(), rotation(0.0f) {

    if (!Renderer::Initialize(scene)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        throw std::runtime_error("Failed to initialize renderer!");
    }
}

bool Renderer::Initialize(Scene& scene) {

    // SetupD3D11
    if (!SetupDeviceAndSwapChain()) {
        std::cerr << "Failed to setup device and swap chain!" << std::endl;
        throw std::runtime_error("Failed to setup device and swap chain!");
        return false;
    }
	// Setup Pipeline, shaders, input layout, texture, sampler state
    if (!SetupPipeline(device.Get(), vsShader, psShader[0], psShader[1], dcemShader, csShader, hullShader, domainShader, particleShaders[0], particleShaders[1],
        particleShaders[2], particleShaders[3], inputLayout, texture, srv, samplerState, shadowSampler)) 
    {
        std::cerr << "Failed to setup pipeline!" << std::endl;
        throw std::runtime_error("Failed to setup pipeline!");
        return false;
    }

    SetupRenderTarget();
    SetupDepthStencil();
    SetupViewport();


    // G-Buffers
    this->positionBuffer.Initialize(device.Get(), window.GetWidth(), window.GetHeight());
    this->normalBuffer.Initialize(device.Get(), window.GetWidth(), window.GetHeight());
    this->diffuseBuffer.Initialize(device.Get(), window.GetWidth(), window.GetHeight());

    this->rtvArr[0] = this->positionBuffer.GetRTV();
    this->rtvArr[1] = this->normalBuffer.GetRTV();
    this->rtvArr[2] = this->diffuseBuffer.GetRTV();

    this->srvArr[0] = this->positionBuffer.GetSRV();
    this->srvArr[1] = this->normalBuffer.GetSRV();
    this->srvArr[2] = this->diffuseBuffer.GetSRV();


	CreateUnorderedAccessView();

	// Setup camera (projection info and initialize)
    ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XM_PIDIV2;
	projInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 200.0f;
    camera.Initialize(device.Get(), projInfo, DirectX::XMFLOAT3(0.0f, 2.0f, -4.0f));
    

    // LOADING THE SCENE
    LoadObjects(scene);
    CreateLights(device.Get(), scene);
    InitializeParticles(scene);


    // Binding The Sampler To The Shaders
    ComPtr<ID3D11SamplerState> pSamplerState = samplerState->GetSamplerState();
    immediateContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());
    pSamplerState = shadowSampler->GetSamplerState();
    immediateContext->CSSetSamplers(0, 1, pSamplerState.GetAddressOf());

    return true;
}

void Renderer::Render(Scene& scene, bool tessellation, bool shadow, bool ParticlesOn) {

    this->camera.UpdateInternalConstantBuffer(immediateContext.Get());
    
    ClearBuffers();
    //scene.DrawTree();


    immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
    static UINT stride = sizeof(SimpleVertex);
    static UINT offset = 0;

    immediateContext->RSSetViewports(1, &viewport);
	vsShader->BindShader(immediateContext.Get());
    psShader[0]->BindShader(immediateContext.Get());
   
    if (shadow)
        ShadowPass(scene, tessellation);

    
    // Drawing Particles
    if (ParticlesOn)
    {
        DrawParticles(scene);
    }

    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    scene.GenerateDCEM(immediateContext.Get());

    GeometryPass(scene, tessellation);
    LightPass(scene, shadow);

    swapChain->Present(0, 0);
}

void Renderer::ShadowPass(Scene& scene, bool tessellate)
{
    immediateContext->PSSetShader(nullptr, nullptr, 0);

    immediateContext->HSSetShader(nullptr, nullptr, 0);
    immediateContext->DSSetShader(nullptr, nullptr, 0);

    ID3D11DepthStencilView* dsv;
    // For each Spotlight
    for (int i = 0; i < scene.GetNrOfSpotLights(); i++)
    {
        dsv = scene.GetShadowMapDSV(i);
        immediateContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        immediateContext->OMSetRenderTargets(0, nullptr, dsv);

        ComPtr<ID3D11Buffer> pShadowCam = scene.GetShadowCamera(i);
        immediateContext->VSSetConstantBuffers(0, 1, pShadowCam.GetAddressOf());
        
        scene.DrawObjects(immediateContext.Get(), &camera, false);
        scene.DrawDCEM(immediateContext.Get());
    }

    // For each DirLight
    for (int i = 0; i < scene.GetNrOfDirLight(); i++)
    {
        dsv = scene.GetShadowMapDSV(i, true);
        immediateContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        immediateContext->OMSetRenderTargets(0, nullptr, dsv);

        ComPtr<ID3D11Buffer> pShadowCam = scene.GetShadowCamera(i, true);
        immediateContext->VSSetConstantBuffers(0, 1, pShadowCam.GetAddressOf());
        scene.DrawObjects(immediateContext.Get(), &camera, false);
        scene.DrawDCEM(immediateContext.Get());
    }
    dsv = nullptr;
    immediateContext->OMSetRenderTargets(0, nullptr, dsv);
}


void Renderer::GeometryPass(Scene& scene, bool tessellation)
{
    immediateContext->OMSetRenderTargets(3, rtvArr->GetAddressOf(), dsView.Get());
    immediateContext->RSSetViewports(1, &viewport);
    
    vsShader->BindShader(immediateContext.Get());
    psShader[0]->BindShader(immediateContext.Get());

    // For Switching camera view :)
    pCamera = scene.GetShadowCamera(0);
    pCamera = camera.GetConstantBuffer();
    immediateContext->VSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    immediateContext->PSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    if (tessellation)
    {
        hullShader->BindShader(immediateContext.Get());
        domainShader->BindShader(immediateContext.Get());

        immediateContext->HSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
        immediateContext->DSSetConstantBuffers(0, 1, pCamera.GetAddressOf());

        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    }
    else
    {
        immediateContext->HSSetShader(nullptr, nullptr, 0);
        immediateContext->DSSetShader(nullptr, nullptr, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    scene.DrawObjects(immediateContext.Get(), &camera, tessellation);
    scene.DrawDCEM(immediateContext.Get());
}

void Renderer::LightPass(Scene& scene, bool shadow)
{
    immediateContext->OMSetRenderTargets(0, nullptr, dsView.Get());
    csShader->BindShader(immediateContext.Get());
    pCamera = camera.GetConstantBuffer();
    immediateContext->CSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    immediateContext->CSSetShaderResources(0, 3, srvArr->GetAddressOf());
    immediateContext->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

    ComPtr<ID3D11ShaderResourceView> srvSpotlightMap = scene.GetShadowMapSRV();
    ComPtr<ID3D11ShaderResourceView> srvDirLightMap = scene.GetShadowMapSRV(true);
    ComPtr<ID3D11ShaderResourceView> srvSpotlight = scene.GetLightBufferSRV();
    ComPtr<ID3D11ShaderResourceView> srvDirLight = scene.GetLightBufferSRV(true);

    immediateContext->CSSetShaderResources(3, 1, srvSpotlightMap.GetAddressOf());
    immediateContext->CSSetShaderResources(4, 1, srvDirLightMap.GetAddressOf());
    immediateContext->CSSetShaderResources(5, 1, srvSpotlight.GetAddressOf());
    immediateContext->CSSetShaderResources(6, 1, srvDirLight.GetAddressOf());

    //ComPtr<ID3D11SamplerState> pSampler = samplerState.get();
    //immediateContext->CSSetSamplers(0, 1, pSampler.GetAddressOf());


    ID3D11Buffer* nrofLights = scene.GetnrofLightBuffer();
    immediateContext->CSSetConstantBuffers(1, 1, &nrofLights);

    // Dispatching threads to CS
    UINT dispatchX = (window.GetWidth() + 7) / 8;
    UINT dispatchY = (window.GetHeight() + 7) / 8;
    immediateContext->Dispatch(dispatchX, dispatchY, 1);


    // Unbinding
    immediateContext->CSSetShaderResources(0, 3, srvNULL->GetAddressOf());
    immediateContext->CSSetShaderResources(3, 3, srvNULL->GetAddressOf());
    immediateContext->CSSetShaderResources(6, 1, srvNULL[0].GetAddressOf());
    immediateContext->CSSetUnorderedAccessViews(0, 1, uavNULL.GetAddressOf(), nullptr);
    immediateContext->CSSetShader(nullptr, nullptr, 0);
}

void Renderer::DrawParticles(Scene& scene)
{
    // Drawing the particles
    if (true)
    {
        immediateContext->HSSetShader(nullptr, nullptr, 0);
        immediateContext->DSSetShader(nullptr, nullptr, 0);
        Particles* particles = scene.GetParticles();
        UINT nrofParticles = particles->GetNrOfParticles();

        ComPtr<ID3D11SamplerState> pSamplerState = samplerState->GetSamplerState();
        immediateContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());

        ID3D11ShaderResourceView* psrv = particles->GetSRV();
        ID3D11ShaderResourceView* ptexture = particles->GetTexture();

        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        immediateContext->IASetInputLayout(nullptr);

        // Binding VS
        particleShaders[0]->BindShader(immediateContext.Get());
        immediateContext->VSSetShaderResources(0, 1, &psrv);

        // Binding GS
        particleShaders[1]->BindShader(immediateContext.Get());
        pCamera = camera.GetConstantBuffer();
        immediateContext->GSSetConstantBuffers(0, 1, pCamera.GetAddressOf());

        immediateContext->RSSetViewports(1, &viewport);

        // Binding PS
        particleShaders[2]->BindShader(immediateContext.Get());
        immediateContext->PSSetShaderResources(0, 1, &ptexture);

        // Draw the Particles
        immediateContext->OMSetRenderTargets(3, rtvArr->GetAddressOf(), dsView.Get());
        immediateContext->Draw(nrofParticles, 0);

        // Unbinding
        immediateContext->VSSetShaderResources(0, 1, srvNULL[0].GetAddressOf());
        immediateContext->PSSetShaderResources(0, 1, srvNULL[0].GetAddressOf());
        immediateContext->VSSetShader(nullptr, nullptr, 0);
        immediateContext->GSSetShader(nullptr, nullptr, 0);
        immediateContext->PSSetShader(nullptr, nullptr, 0);
        immediateContext->OMSetRenderTargets(0, nullptr, dsView.Get());

        // Resetting to standard values
        immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
        vsShader->BindShader(immediateContext.Get());
        //immediateContext->VSSetConstantBuffers(0, 0, pCamera.GetAddressOf());
    }
}

void Renderer::ClearBuffers()
{
    float clearColor[4] = { 0.53f, 0.81f, 0.93f, 0.53f };

    immediateContext->ClearRenderTargetView(rtv.Get(), clearColor);

    for (int i = 0; i < 3; i++)
    {
        immediateContext->ClearRenderTargetView(rtvArr[i].Get(), clearColor);
    }
    immediateContext->ClearUnorderedAccessViewFloat(uav.Get(), clearColor);
    immediateContext->ClearDepthStencilView(dsView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Renderer::CreateLights(ComPtr<ID3D11Device> device, Scene& scene) 
{
    // Dirlight SOl
    LightData sun = {};
    sun.perLightInfo.initialPosition = { 0.0f, 2.0f, 0.0f };
    sun.perLightInfo.color = { 1.0f,1.0f,1.0f,1.0f };
    sun.perLightInfo.intensity = 0.05f;
    sun.perLightInfo.angle = XM_PI;
    sun.perLightInfo.isDir = true;
    sun.perLightInfo.rotationX = 0;
    sun.perLightInfo.rotationY = XM_PIDIV2;

    // Spotlight Ficklampa
    LightData red = {};
    red.perLightInfo.initialPosition = { 0.4f, 3.5f, -11.4f };
    red.perLightInfo.color = { 1.0f, 0.0f, 0.0f, 5.0f };
    red.perLightInfo.intensity = 0.5f;
    red.perLightInfo.angle = XM_PIDIV2;
    red.perLightInfo.rotationX = 0;
    red.perLightInfo.rotationY = 0;
    red.perLightInfo.fovAngleY = XM_PIDIV4;
    red.perLightInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
    red.perLightInfo.nearZ = 1.0f;
    red.perLightInfo.farZ = 100.0f;

    LightData blue = {};
    blue.perLightInfo.initialPosition = { -9.5f, 3.0f, 1.0f };
    blue.perLightInfo.color = { 0.0f, 0.0f, 1.0f, 1.0f };
    blue.perLightInfo.intensity = 0.5f;
    blue.perLightInfo.angle = XM_PIDIV2;
    blue.perLightInfo.rotationX = XM_PIDIV2;
    blue.perLightInfo.rotationY = 0;
    blue.perLightInfo.fovAngleY = XM_PIDIV4;
    blue.perLightInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
    blue.perLightInfo.nearZ = 1.0f;
    blue.perLightInfo.farZ = 100.0f;

    LightData green = {};
    green.perLightInfo.initialPosition = { 14.7f, 3.0f, -5.0f };
    green.perLightInfo.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    green.perLightInfo.intensity = 0.4f;
    green.perLightInfo.angle = XM_PIDIV2;
    green.perLightInfo.rotationX = XM_PI;
    green.perLightInfo.rotationY = 0;
    green.perLightInfo.fovAngleY = XM_PIDIV4;
    green.perLightInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
    green.perLightInfo.nearZ = 1.0f;
    green.perLightInfo.farZ = 100.0f;


    scene.AddLight(device.Get(), sun);

    scene.AddLight(device.Get(), blue);
    scene.AddLight(device.Get(), red);
    scene.AddLight(device.Get(), green);

    // Loading in the lights
    scene.InitializeLight(device.Get());
}



void Renderer::LoadObjects(Scene& scene)
{
    scene.AddObject(device.Get(), "NOPCube/", "cube", { 0.0f, 10.0f, 0.0f }, { XM_PIDIV4, 0, XM_PIDIV4 }, { 0.7f, 0.7f, 0.7f }, false, false);
    scene.AddObject(device.Get(), "NOPCube/", "cube", { -5.0f, 2.0f, -12.0f }, { 0, 0, 0 }, { 1, 1, 1 });

    scene.AddDCEM(device.Get(), { 0.0f, 4.0f, 4.0f }, { 1.0f, 1.0f, 1.0f }, 1024, 1024, psShader[1], psShader[0], dcemShader);
    scene.AddDCEM(device.Get(), { 0, 8, 8 }, { 3.0f, 3.0f, 3.0f }, 1024, 1024, psShader[1], psShader[0], dcemShader);

    scene.AddObject(device.Get(), "Torch/", "torch", { 0.2f, 3.0f, -15.0f }, { 0.0f, XM_PI, 0.0f }, { 0.03f, 0.03f, 0.03f }, false);    // red
    scene.AddObject(device.Get(), "Torch/", "torch", { -15.0f, 3.0f, 1.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.03f, 0.03f, 0.03f }, false);    // blue
    scene.AddObject(device.Get(), "Torch/", "torch", { 12.7f, 2.0f, -16.3f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.03f, 0.03f, 0.03f }, false);    // blue


    scene.AddObject(device.Get(), "house_obj/", "house", { 13.0f, 0.0f, 4.0f }, { 0.0f, XM_PI, 0.0f }, { 2.0f, 2.0f, 2.0f }, false);

    scene.AddObject(device.Get(), "mushrooms/amanita/", "amanita_a_low", { 15.0f, 0.0f, -10.0f }, { 0.0f, XM_PI, 0.0f }, { 5, 5, 5 }, false);
    scene.AddObject(device.Get(), "mushrooms/amanita/", "amanita_b_low", { 7.0f, 0.0f, 2.0f }, { 0.0f, XM_PI, 0.0f }, { 7, 7, 7 }), false;
    scene.AddObject(device.Get(), "mushrooms/amanita/", "amanita_a_low", { 5.0f, 0.0f, 8.0f }, { 0.0f, XM_PI, 0.0f }, { 5, 5, 5 }), false;
    scene.AddObject(device.Get(), "mushrooms/morel/", "morel_low", { -5.0f, 0.5f, -2.0f }, { 0.0f, XM_PI, 0.0f }, { 4, 4, 4 });
    scene.AddObject(device.Get(), "mushrooms/morel/", "morel_low", { 8.0f, 0.0f, 6.0f }, { 0.0f, XM_PI, 0.0f }, { 5, 5, 5 });
    scene.AddObject(device.Get(), "mushrooms/chanterelles/", "chanterelles_low", { 6.0f, 0.0f, 2.0f }, { 0.0f, XM_PI, 0.0f }, { 4, 4, 4 }, false);
    scene.AddObject(device.Get(), "mushrooms/chanterelles/", "chanterelles_low", { 10.0f, 0.0f, 20.0f }, { 0.0f, XM_PI, 0.0f }, { 7, 7, 7 }), false;
    scene.AddObject(device.Get(), "mushrooms/chanterelles/", "chanterelles_low", { -15.0f, 0.0f, 12.0f }, { 0.0f, XM_PI, 0.0f }, { 5, 5, 5 }), false;

    scene.AddObject(device.Get(), "Farm/", "Wood", { -15, 0, -5 }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    scene.AddObject(device.Get(), "toy/", "wooden_tractor_toy_1", { -20, 0, 20 }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });


    scene.AddObject(device.Get(), "Fountain/", "fountain", { 0, 0, 0 }, { 0, XM_PI, 0 }, { 1, 1, 1 });
    scene.AddObject(device.Get(), "SimpleObjects/", "circle", { 0, 0.5, 0 }, { XM_PI, 0, 0 }, { 3, 3, 3 });
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { 2.0f, 0.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.2f, 0.2f, 0.2f }, false, false);
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { -2.0f, 0.0f, -2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.2f, 0.2f, 0.2f }, false, false);

    scene.AddObject(device.Get(), "SimpleObjects/", "plane1", { 0.0f,-0.12f,0.0f }, { 0.0f ,0.0f, 0.0f }, { 50.0f, 50.0f, 50.0f }, false);
    //scene.AddObject(device.Get(), "SimpleObjects/", "reverse_cube", { 0.0f, 0.0f ,0.0f }, { 0.0f, 0.0f, 0.0f }, { 50.0f, 50.0f, 50.0f }, false);

}

void Renderer::InitializeParticles(Scene& scene)
{
    scene.AddParticles(device.Get(), sizeof(ParticleData), 128, nullptr, false, true, true);
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

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), &device, nullptr, &immediateContext);

    return !(FAILED(hr));
}

void Renderer::SetupRenderTarget() {
    renderTargetD3D11.Initialize(device.Get(), swapChain.Get(), window.GetWidth(), window.GetHeight());
    rtv = renderTargetD3D11.GetRTV();
}

void Renderer::SetupDepthStencil() {
    depthBufferD3D11.Initialize(device.Get(), window.GetWidth(), window.GetHeight());
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


    if (FAILED(device->CreateUnorderedAccessView(backbuffer, &desc, uav.GetAddressOf()))) {
        backbuffer->Release();
        std::cerr << "Failed to create UAV!" << std::endl;
        return false;
    }

    backbuffer->Release();
    return true;
}

void Renderer::UpdateParticles(Scene& scene)
{
    scene.UpdateParticles(immediateContext.Get(), particleShaders[3].get());
}


ID3D11Device* Renderer::GetDevice()
{
    return this->device.Get();
}
CameraD3D11& Renderer::GetCamera()
{
    return this->camera;
}


