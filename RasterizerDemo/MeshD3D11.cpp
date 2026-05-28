#include "MeshD3D11.h"


MeshD3D11::MeshD3D11(ID3D11Device* device, const std::string& path, const std::string& objName)
{
    Initialize(device, path, objName);
}

void MeshD3D11::Initialize(ID3D11Device* device, const std::string& folderPath, const std::string& objName)
{
    this->filePath += folderPath;

    std::string path = this->filePath + objName + ".obj";

    if (!loader.LoadFile(path))
    {
        std::cerr << "Failed to load OBJ at " << path << "!\n";
        throw std::runtime_error("Failed to load OBJ!\n");
    }

    size_t nrOfVertices = 0;
    size_t nrOfIndices = 0;
    for (const auto& mesh : loader.LoadedMeshes)
    {
        nrOfVertices += mesh.Vertices.size();
        nrOfIndices += mesh.Indices.size();
    }

    std::vector<SimpleVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<DirectX::XMFLOAT3> bbVertices;
    //vertices.reserve(nrOfVertices);
    //indices.reserve(nrOfIndices);

    size_t indexOffset = 0;

    for (const auto& mesh : loader.LoadedMeshes)
    {
        SubMeshD3D11 subMesh;
        size_t startIndex = indices.size();

        ID3D11ShaderResourceView* ambientTextureSRV = nullptr;
        ID3D11ShaderResourceView* diffuseTextureSRV = nullptr;
        ID3D11ShaderResourceView* specularTextureSRV = nullptr;
        ID3D11ShaderResourceView* bumpTextureSRV = nullptr;

        

        DirectX::XMFLOAT3 ambientColor, diffuseColor, specularColor;
        float shininess = 500.0f;
        float specularIntensity = 1.0f;
        
        // Loading ambient texture
        if (!mesh.MeshMaterial.map_Ka.empty())
        {
            path = this->filePath + mesh.MeshMaterial.map_Ka;
            std::wstring wPath(path.begin(), path.end());
            HRESULT hr = DirectX::CreateWICTextureFromFile(device,
                wPath.c_str(), nullptr, &ambientTextureSRV);
            if (FAILED(hr))
            {
                std::cerr << "Failed to load ambient texture at " << path << "!\n";
                throw std::runtime_error("Failed to load ambient texture!\n");
            }
        }
        ambientColor = { mesh.MeshMaterial.Ka.X, mesh.MeshMaterial.Ka.Y, mesh.MeshMaterial.Ka.Z };


		// Loading diffuse texture
        if (!mesh.MeshMaterial.map_Kd.empty())
        {
            path = this->filePath + mesh.MeshMaterial.map_Kd;
            HRESULT hr = DirectX::CreateWICTextureFromFile(device, 
				std::wstring(path.begin(), path.end()).c_str(), nullptr, &diffuseTextureSRV);
            if (FAILED(hr))
            {
                std::cerr << "Failed to load diffuse texture at " << path << "!\n";
				throw std::runtime_error("Failed to load diffuse texture!\n");
            }
        }
        else
        {
            
        }
        diffuseColor = { mesh.MeshMaterial.Kd.X, mesh.MeshMaterial.Kd.Y, mesh.MeshMaterial.Kd.Z };


		//Load Specular Texture
        if (!mesh.MeshMaterial.map_Ks.empty())
        {
            path = this->filePath + mesh.MeshMaterial.map_Ks;
            HRESULT hr = DirectX::CreateWICTextureFromFile(device,
                std::wstring(path.begin(), path.end()).c_str(), nullptr, &specularTextureSRV);
            if (FAILED(hr))
            {
                std::cerr << "Failed to load specular texture at " << path << "!\n";
                throw std::runtime_error("Failed to load specular texture!\n");
            }
        }
		specularColor = { mesh.MeshMaterial.Ks.X, mesh.MeshMaterial.Ks.Y, mesh.MeshMaterial.Ks.Z };
		shininess = mesh.MeshMaterial.Ns;
		//specularIntensity = mesh.MeshMaterial.illum == 2 ? 1.0f : 0.0f;

        if (!mesh.MeshMaterial.map_bump.empty())
        {
            path = this->filePath + mesh.MeshMaterial.map_bump;
            HRESULT hr = DirectX::CreateWICTextureFromFile(device,
                std::wstring(path.begin(), path.end()).c_str(), nullptr, &bumpTextureSRV);
            if (FAILED(hr))
            {
                std::cerr << "Failed to load bump texture at " << path << "!\n";
                throw std::runtime_error("Failed to load bump texture!\n");

            }
        }

		// Adding Vertices to Vertex and Bouding Box buffers
		vertices.reserve(mesh.Vertices.size());
        for (const auto& vertex : mesh.Vertices)
        {
            vertices.emplace_back(SimpleVertex{
                std::array<float, 3>{ vertex.Position.X, vertex.Position.Y, vertex.Position.Z },
                std::array<float, 3>{ vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z },
                std::array<float, 2>{ vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y }
                });
            //vertices.emplace_back(vertex);
            bbVertices.emplace_back(DirectX::XMFLOAT3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z));
            //bbVertices.push_back({ vertex.Position.X, vertex.Position.Y, vertex.Position.Z });
		}

		// Adding Indices to Index buffer
		indices.reserve(mesh.Indices.size());
        for (const auto& index : mesh.Indices)
        {
            indices.emplace_back(static_cast<uint32_t>(index + indexOffset));
        }
        indexOffset += mesh.Indices.size();
        subMesh.Initialize(startIndex, mesh.Indices.size(), ambientTextureSRV, diffuseTextureSRV, specularTextureSRV);
		this->subMeshes.emplace_back(std::move(subMesh));
    }

    // Initialize buffers
    this->vertexBuffer.Initialize(device, sizeof(SimpleVertex), vertices.size(), vertices.data());
    this->indexBuffer.Initialize(device, indices.size(), indices.data());
    this->boundingBox.CreateFromPoints(this->boundingBox, bbVertices.size(), bbVertices.data(), sizeof(DirectX::XMFLOAT3));

    // Initialize sub-meshes
    //subMeshes.resize(meshInfo.subMeshInfo.size());
    //for (size_t i = 0; i < meshInfo.subMeshInfo.size(); ++i)
    //{
    //    subMeshes[i].Initialize(
    //        meshInfo.subMeshInfo[i].startIndexValue,
    //        meshInfo.subMeshInfo[i].nrOfIndicesInSubMesh,
    //        meshInfo.subMeshInfo[i].ambientTextureSRV,
    //        meshInfo.subMeshInfo[i].diffuseTextureSRV,
    //        meshInfo.subMeshInfo[i].specularTextureSRV
    //    );
    //}
}


void MeshD3D11::BindMeshBuffers(ID3D11DeviceContext* context) const
{
    // Bind vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    ID3D11Buffer* vb = vertexBuffer.GetBuffer();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    // Bind index buffer
    context->IASetIndexBuffer(this->indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const
{
    if (subMeshIndex < this->subMeshes.size())
    {
        this->subMeshes[subMeshIndex].PerformDrawCall(context);
    }
}

size_t MeshD3D11::GetNrOfSubMeshes() const { return this->subMeshes.size(); }

ID3D11ShaderResourceView* MeshD3D11::GetAmbientSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < this->subMeshes.size())
    {
        return this->subMeshes[subMeshIndex].GetAmbientSRV();
    }
    return nullptr;
}

ID3D11ShaderResourceView* MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < this->subMeshes.size())
    {
        return this->subMeshes[subMeshIndex].GetDiffuseSRV();
    }
    return nullptr;
}

ID3D11ShaderResourceView* MeshD3D11::GetSpecularSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < this->subMeshes.size())
    {
        return this->subMeshes[subMeshIndex].GetSpecularSRV();
    }
    return nullptr;
}

void MeshD3D11::createTexture(ID3D11Device* device, ID3D11ShaderResourceView** srv)
{
}

//VertexBufferD3D11 MeshD3D11::getVertexBuffer() const
//{
//    return this->vertexBuffer;
//}
//
//IndexBufferD3D11 MeshD3D11::getIndexBuffer() const
//{
//    return this->indexBuffer;
//}

DirectX::BoundingBox MeshD3D11::getBoundingBox() const { return this->boundingBox; }
