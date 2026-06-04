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
    if (!SetupPipeline(device.Get(), vsShader, psShader[0], psShader[1], csShader, hullShader, domainShader,
                    inputLayout, texture, srv, samplerState, shadowSampler)) 
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
    psConstantBufferD3D11.Initialize(device.Get(), sizeof(MaterialBuffer), &quadMaterial);
    psConstantBuffer = psConstantBufferD3D11.GetBuffer();
   
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(float) * 4;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = simpleQuad;

    vertexBuffer.Initialize(device.Get(), sizeof(SimpleVertex), 4, simpleQuad);
    ComPtr<ID3D11Buffer> buffer = vertexBuffer.GetBuffer();
    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, buffer.GetAddressOf());

    //vertexBuffers[1].Initialize(device.Get(), sizeof(SimpleVertex), 4, simpleQuad); // Can have same vb
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
    worldMatriceBuffers[0].Initialize(device.Get(), sizeof(XMFLOAT4X4), &worldTransform);
    worldMatriceBuffers[0].UpdateBuffer(immediateContext.Get(), &worldTransform);


    DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixTranspose(worldMatrices[1]));
    worldMatriceBuffers[1].Initialize(device.Get(), sizeof(XMFLOAT4X4), &worldTransform);
    worldMatriceBuffers[1].UpdateBuffer(immediateContext.Get(), &worldTransform);



	// Setup constant buffers (for vertex shader and pixel shader)
    //CreateVSConstantBuffer(device.Get(), vsConstantBufferD3D11, matrixArr, rotation, window.GetWidth(), window.GetHeight());

	// Setup camera (projection info and initialize)
    ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XMConvertToRadians(90.0f);
	projInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 100.0f;
    camera.Initialize(device.Get(), projInfo, DirectX::XMFLOAT3(0.0f, 0.0f, -2.0f));

    // Creating the Scene (w objs and light)
    LoadObjects(scene);
    CreateLights(device.Get(), scene);


    // Binding The Sampler To The Shaders
    ComPtr<ID3D11SamplerState> pSamplerState = samplerState->GetSamplerState();
    immediateContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());
    pSamplerState = shadowSampler->GetSamplerState();
    immediateContext->CSSetSamplers(0, 1, pSamplerState.GetAddressOf());

    return true;
}

void Renderer::Render(Scene& scene, bool tesselation, bool shadow) {

	time.Update(); // Update frame timing

    
    ClearBuffers();
    //float clearColour[4] = { 0.1f, 0.4f, 0.5f, 1 };
    //immediateContext->ClearRenderTargetView(rtv, clearColour);
    //immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);



    immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    immediateContext->RSSetViewports(1, &viewport);
	vsShader->BindShader(immediateContext.Get());
    psShader[0]->BindShader(immediateContext.Get());

    // Binding camera to VS, PS, CS
    camera.UpdateInternalConstantBuffer(immediateContext.Get());
    CameraBuffer camPS = {};
    camPS.viewProjMatrix = camera.GetViewProjectionMatrix();
    camPS.cameraPosition = camera.GetPosition();
    //camPS.cameraPosition = scene.GetCameraPos(0);
    //camPS.viewProjMatrix = scene.GetCameraVP(0);
    camPS.padding = 0.0f;
    ConstantBufferD3D11 camBufferPS(device.Get(), sizeof(CameraBuffer), &camPS);
    pCamera = camBufferPS.GetBuffer();
    //immediateContext->VSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    //immediateContext->PSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    //immediateContext->CSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
   
    if (shadow)
        ShadowPass(scene, tesselation);

    pCamera = camBufferPS.GetBuffer();
    GeometryPass(tesselation);
    scene.DrawObjects(immediateContext.Get(), tesselation);
    scene.DrawDCEM(immediateContext.Get());
    LightPass(scene, shadow);
    swapChain->Present(0, 0);

    // Drawing the quads (Same: vertexbuffer, texture and material, different: worldmatrices)
    //immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //ID3D11Buffer* pvertexBuffer = vertexBuffer.GetBuffer();
    //immediateContext->IASetVertexBuffers(0, 1, &pvertexBuffer, &stride, &offset);
    //immediateContext->PSSetShaderResources(1, 1, srv.GetAddressOf());
    //immediateContext->PSSetConstantBuffers(1, 1, &psConstantBuffer);
    //for (int i = 0; i < 0; i++)
    //{
	   // // Bind and set pipeline states, then draw
	   // // Sending stuff to VS
    //    //immediateContext->VSSetShader(vShader, nullptr, 0);
    //    XMFLOAT4X4 worldMatrixT;
    //    DirectX::XMStoreFloat4x4(&worldMatrixT, DirectX::XMMatrixTranspose(worldMatrices[i]));
    //    worldMatriceBuffers[i].UpdateBuffer(immediateContext.Get(), &worldMatrixT);
    //    pWorldMatrix = worldMatriceBuffers[i].GetBuffer();
    //    immediateContext->VSSetConstantBuffers(1, 1, pWorldMatrix.GetAddressOf());
    //    immediateContext->Draw(4, 0);
    //    // Sending stuff to PS
    //    //immediateContext->PSSetShader(pShader, nullptr, 0);
    //    //immediateContext->PSSetConstantBuffers(1, 1, lightPS.GetAddressOf());
    //    //immediateContext->PSSetConstantBuffers(0, 1, psConstantBuffer.GetAddressOf());
    //    //immediateContext->OMSetRenderTargets(1, rtv.GetAdressOf(), dsView.Get());
    //    //immediateContext->OMSetRenderTargets(3, rtvArr.GetAdressOf(), dsView.Get());
    //}

    //scene.DrawObjects(immediateContext.Get(), tesselation);
    //scene.DrawDCEM(immediateContext.Get());
	scene.GenerateDCEM(immediateContext.Get());
    immediateContext->RSSetViewports(1, &viewport);
    immediateContext->OMSetRenderTargets(3, rtvArr->GetAddressOf(), dsView.Get());

    scene.DrawObjects(immediateContext.Get(), tesselation);

    scene.DrawDCEM(immediateContext.Get());


}

void Renderer::ShadowPass(Scene& scene, bool tesselate)
{
    immediateContext->PSSetShader(nullptr, nullptr, 0);

    ComPtr<ID3D11DepthStencilView> dsv;
    // For each Spotlight
    for (int i = 0; i < scene.GetNrOfSpotLights(); i++)
    {
        dsv = scene.GetShadowMapDSV(i);
        immediateContext->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        immediateContext->OMSetRenderTargets(0, nullptr, dsv.Get());

        ComPtr<ID3D11Buffer> pShadowCam = scene.GetShadowCamera(i);
        immediateContext->VSSetConstantBuffers(0, 1, pShadowCam.GetAddressOf());
        
        scene.DrawObjects(immediateContext.Get(), tesselate);
        scene.DrawDCEM(immediateContext.Get());
    }

    // For each DirLight
    for (int i = 0; i < scene.GetNrOfDirLight(); i++)
    {
        dsv = scene.GetShadowMapDSV(i, true);
        immediateContext->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        immediateContext->OMSetRenderTargets(0, nullptr, dsv.Get());

        ComPtr<ID3D11Buffer> pShadowCam = scene.GetShadowCamera(i, true);
        immediateContext->VSSetConstantBuffers(0, 1, pShadowCam.GetAddressOf());
        scene.DrawObjects(immediateContext.Get(), tesselate);
        scene.DrawDCEM(immediateContext.Get());
    }
    dsv = nullptr;
    immediateContext->OMGetRenderTargets(0, nullptr, dsv.GetAddressOf());
}


void Renderer::GeometryPass(bool tesselation)
{
    immediateContext->OMSetRenderTargets(3, rtvArr->GetAddressOf(), dsView.Get());
    
    psShader[0]->BindShader(immediateContext.Get());
    immediateContext->VSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    immediateContext->PSSetConstantBuffers(0, 1, pCamera.GetAddressOf());
    if (tesselation)
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
}

void Renderer::LightPass(Scene& scene, bool shadow)
{
    immediateContext->OMSetRenderTargets(0, nullptr, dsView.Get());
    csShader->BindShader(immediateContext.Get());
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
}

void Renderer::ClearBuffers()
{
    float clearColor[4] = { 0.1f, 0.4f, 0.5f, 1.0f };

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
    LightData data2 = {};
    data2.perLightInfo.initialPosition = { 0.0f, 20.0f, 0.0f };
    data2.perLightInfo.color = { 1.0f,1.0f,1.0f,1.0f };
    data2.perLightInfo.intensity = 0.1f;
    data2.perLightInfo.angle = XM_PI;
    data2.perLightInfo.isDir = true;
    data2.perLightInfo.rotationX = 0;
    data2.perLightInfo.rotationY = XM_PIDIV2;

    // Spotlight Ficklampa
    LightData data1 = {};
    data1.perLightInfo.initialPosition = { 0.4f, 3.5f, -11.4f };
    data1.perLightInfo.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    data1.perLightInfo.intensity = 0.7f;
    data1.perLightInfo.angle = XM_PI;
    data1.perLightInfo.rotationX = 0;
    data1.perLightInfo.rotationY = 0;
    data1.perLightInfo.fovAngleY = XM_PIDIV2;
    data1.perLightInfo.aspectRatio = 1;
    data1.perLightInfo.nearZ = 1.0f;
    data1.perLightInfo.farZ = 100.0f;

    scene.AddLight(device.Get(), data2);
    scene.AddLight(device.Get(), data1);

    scene.InitializeLight(device.Get());
}



void Renderer::LoadObjects(Scene& scene)
{
    //scene->AddObject(device.Get(), "Horse/", "Horse", XMFLOAT3(0, 0, 10), XMFLOAT3(0, PI, 0), XMFLOAT3(1, 1, 1));
    scene.AddObject(device.Get(), "NPCube2/", "cube", { -5, 2, 2 }, { 0, 0, 0 }, { 0.7f, 0.7f, 0.7f });
    //scene->AddObject(device.Get(), "Cat/", "12221_Cat_v1_l3", XMFLOAT3(1, 1, 5), XMFLOAT3(-XM_PI / 2, XM_PI, 0), XMFLOAT3(0.05f, 0.05f, 0.05f));
    //scene->AddObject(device.Get(), "Box/", "box", XMFLOAT3(0, -2, 2), XMFLOAT3(0, 0, 0), XMFLOAT3(2, 2, 2));

    scene.AddDCEM(device.Get(), { 0, 4, 0 }, 1024, 1024, psShader[1], psShader[0], false);

    scene.AddObject(device.Get(), "Torch/", "torch", { 0.2f, 3.0f, -15.0f }, { 0.0f, XM_PI, 0.0f }, { 0.03f, 0.03f, 0.03f });
    scene.AddObject(device.Get(), "FarmAnimals/", "pig", { 1.50f, 2.0f, 5.0f }, { 0.0f, XM_PI, 0.0f }, { 0.1f, 0.1f, 0.1f });
    scene.AddObject(device.Get(), "Windmill/", "low-poly-mill", { 15.0f, 10.0f ,20.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.1f, 0.1f, 0.1f });
    scene.AddObject(device.Get(), "house_obj/", "house", { 3.0f, 2.0f, 5.0f }, { 0.0f, XM_PI, 0.0f }, { 0.7f, 0.7f, 0.7f });
    scene.AddObject(device.Get(), "SimpleObjects/", "sphere", { 5.0f, 2.0f, 2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.7f, 0.7f, 0.7f });

    scene.AddObject(device.Get(), "SimpleObjects/", "plane", { 0.0f,-0.12f,0.0f }, { XM_PIDIV2,0.0f,0.0f }, { 1000,1000,1000 });



    scene.AddObject(device.Get(), "Fountain/", "fountain", { 0, 0, 0 }, { 0, XM_PI, 0 }, { 1, 1, 1 });
    scene.AddObject(device.Get(), "Circle/", "circle", { 0, 0.5, 0 }, { XM_PI, 0, 0 }, { 3, 3, 3 });
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { 0.0f, 0.4f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { 0.0f, 0.4f, -2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { 2.0f, 0.4f, 0.0f }, { 0.0f, XM_PIDIV2, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    scene.AddObject(device.Get(), "Duck/", "rubberduckie", { -2.0f, 0.4f, 0.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    //scene->AddObject(device.Get(), "Duck/", "rubberduckie", { 1.4f, 0.4f, 1.4f }, { 0.0f, XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    //scene->AddObject(device.Get(), "Duck/", "rubberduckie", { 1.4f, 0.4f, -1.4f }, { 0.0f, 3 * XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    //scene->AddObject(device.Get(), "Duck/", "rubberduckie", { -1.4f, 0.4f, 1.4f }, { 0.0f, -XM_PIDIV4, 0.0f }, { 0.2f, 0.2f, 0.2f }, false);
    //scene->AddObject(device.Get(), "Duck/", "rubberduckie", { -1.4f, 0.4f, -1.4f }, { 0.0f, -3.0f * XM_PIDIV4, 0.0f }, { 1.0f, 1.0f, 1.0f }, false);


    //scene->AddObject(device.Get(), "Fish/", "AnglerFish", { -5.0f, 2.0f, 2.0f }, { 0.0f, XM_PI, 0.0f }, { 0.7f, 0.7f, 0.7f });
    //scene->AddObject(device.Get(), "Fish/", "Blobfish", { -5.0f, 1.0f, -2.0f }, { 0.0f, -3.0f * XM_PIDIV4,0.0f }, { 0.5f, 0.5f, 0.5f });

    //scene->AddObject(device.Get(), "Windmill/", "low-poly-mill", { 15.0f, 0.0f ,20.0f }, { 0.0f, -XM_PIDIV2, 0.0f }, { 0.1f, 0.1f, 0.1f });
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

ID3D11Device* Renderer::GetDevice()
{
    return this->device.Get();
}
CameraD3D11& Renderer::GetCamera()
{
    return this->camera;
}

//Scene* Renderer::GetScene()
//{
//    return this->scene.get();
//}

float Renderer::GetDeltatime()
{
    this->time.Update();
    return this->time.GetDeltaTime();
}
