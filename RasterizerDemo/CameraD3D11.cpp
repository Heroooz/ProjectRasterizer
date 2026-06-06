#include "CameraD3D11.h"
#include <DirectXMath.h>

using namespace DirectX;

CameraD3D11::CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo, const XMFLOAT3& initialPosition)
{
    Initialize(device, projectionInfo, initialPosition);
}

void CameraD3D11::Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo, const XMFLOAT3& initialPosition)
{
    this->position = initialPosition;
    this->projInfo = projectionInfo;

    orthographicCamera.cameraPosition = perspectiveCamera.cameraPosition = initialPosition;
    
    XMMATRIX viewMatrix = XMMatrixLookAtLH(
        XMLoadFloat3(&this->position),                                      // Eye Position
        XMLoadFloat3(&this->position) + XMLoadFloat3(&this->forward),       // Focus Point
        XMLoadFloat3(&this->up)                                             // Up Direction
    );

    this->perspectiveMatrix = XMMatrixPerspectiveFovLH(
        projectionInfo.fovAngleY,
        projectionInfo.aspectRatio,
        projectionInfo.nearZ,
        projectionInfo.farZ
    );

    // Same hardcoded sreen width/height as in main... Maybe move out to be more dynamic?
    this->orthographicMatrix = XMMatrixOrthographicLH(
        1920.0f,
        1080.0f,
        projectionInfo.nearZ,
        projectionInfo.farZ
    );

    XMFLOAT4X4 vpMatrix4x4;
    XMStoreFloat4x4(&vpMatrix4x4, XMMatrixTranspose(XMMatrixMultiply(viewMatrix, this->perspectiveMatrix)));
    perspectiveCamera.viewProjMatrix = vpMatrix4x4;
    VPcameraBuffer.Initialize(device, sizeof(CameraBufferForShader), &perspectiveCamera);


    XMFLOAT4X4 voMatrix4x4;
    XMStoreFloat4x4(&voMatrix4x4, XMMatrixTranspose(XMMatrixMultiply(viewMatrix, this->orthographicMatrix)));
    orthographicCamera.viewProjMatrix = voMatrix4x4;
    VOcameraBuffer.Initialize(device, sizeof(CameraBufferForShader), &orthographicCamera);
}

void CameraD3D11::MoveInDirection(float amount, const XMFLOAT3& direction)
{
    this->position.x += direction.x * amount;
    this->position.y += direction.y * amount;
    this->position.z += direction.z * amount;
}

void CameraD3D11::RotateAroundAxis(float amount, const XMFLOAT3& axis)
{
    XMVECTOR ax = XMLoadFloat3(&axis);
    XMMATRIX rotationMatrix = XMMatrixRotationAxis(ax, amount);

    XMVECTOR forwardVec = XMLoadFloat3(&this->forward);
    forwardVec = XMVector3TransformNormal(forwardVec, rotationMatrix);
    forwardVec = XMVector3Normalize(forwardVec);
    XMStoreFloat3(&this->forward, forwardVec);

    XMVECTOR rightVec = XMLoadFloat3(&this->right);
    rightVec = XMVector3TransformNormal(rightVec, rotationMatrix);
    rightVec = XMVector3Normalize(rightVec);
    XMStoreFloat3(&this->right, rightVec);

    XMVECTOR upVec = XMLoadFloat3(&this->up);
    upVec = XMVector3TransformNormal(upVec, rotationMatrix);
    upVec = XMVector3Normalize(upVec);
    XMStoreFloat3(&this->up, upVec);
}

void CameraD3D11::MoveForward(float amount) { MoveInDirection(amount, this->forward); }

void CameraD3D11::MoveRight(float amount) { MoveInDirection(amount, this->right); }

void CameraD3D11::MoveUp(float amount) { MoveInDirection(amount, this->up); }

void CameraD3D11::RotateForward(float amount) { RotateAroundAxis(amount, this->forward); }

void CameraD3D11::RotateRight(float amount) { RotateAroundAxis(amount, this->right); }

void CameraD3D11::RotateUp(float amount) { RotateAroundAxis(amount, {0,1,0}); } // Fixed up-rotation to avoid falling over

void CameraD3D11::LookAt(const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& upVector)
{
	this->position = cameraPosition;
	this->forward.x = target.x - cameraPosition.x;
	this->forward.y = target.y - cameraPosition.y;
	this->forward.z = target.z - cameraPosition.z;

	XMVECTOR forwardVec = XMLoadFloat3(&this->forward);
	forwardVec = XMVector3Normalize(forwardVec);
	XMStoreFloat3(&this->forward, forwardVec);

	XMVECTOR upVec = XMLoadFloat3(&upVector);
	upVec = XMVector3Normalize(upVec);
	XMStoreFloat3(&this->up, upVec);

	XMVECTOR rightVec = XMVector3Cross(upVec, forwardVec);
	rightVec = XMVector3Normalize(rightVec);
	XMStoreFloat3(&this->right, rightVec);
}

const XMFLOAT3& CameraD3D11::GetPosition() const { return this->position; }

const XMFLOAT3& CameraD3D11::GetForward() const { return this->forward; }

const XMFLOAT3& CameraD3D11::GetRight() const { return this->right; }

const XMFLOAT3& CameraD3D11::GetUp() const { return this->up; }

void CameraD3D11::ResetUp()
{
    XMVECTOR u, f, r;
    this->up = XMFLOAT3(0.0f, 1.0f, 0.0f);	// Reseting to (0, 1, 0)
    this->forward.y = 0.0f;					// Reseting up to (x, 0, z)

    f = XMLoadFloat3(&this->forward);		// Normalizing new forward
    XMVector3Normalize(f);
    XMStoreFloat3(&this->forward, f);

    u = XMLoadFloat3(&this->up);			// Normalizing new up
    XMVector3Normalize(u);
    XMStoreFloat3(&this->up, u);


    r = XMVector3Cross(u, f);				// Right is perpendicular to both up and forward
    XMStoreFloat3(&this->right, r);
}

void CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
    XMVECTOR eyePos = XMLoadFloat3(&this->position);
    XMVECTOR focPoint = XMLoadFloat3(&this->forward);
    XMVECTOR upVec = XMLoadFloat3(&this->up);
    XMVECTOR d = eyePos + focPoint;
    XMMATRIX viewMatrix = XMMatrixLookAtLH( eyePos, eyePos + focPoint, upVec );
    


    XMFLOAT4X4 viewproj4x4;
    XMStoreFloat4x4(&viewproj4x4, XMMatrixTranspose(XMMatrixMultiply(viewMatrix, this->perspectiveMatrix)));
    this->perspectiveCamera.viewProjMatrix = viewproj4x4;
    this->perspectiveCamera.cameraPosition = this->position;
    this->VPcameraBuffer.UpdateBuffer(context, &this->perspectiveCamera);

    XMFLOAT4X4 vieworth4x4;
    XMStoreFloat4x4(&vieworth4x4, XMMatrixTranspose(XMMatrixMultiply(viewMatrix, this->orthographicMatrix)));
    this->orthographicCamera.viewProjMatrix = vieworth4x4;
    this->orthographicCamera.cameraPosition = this->position;
    this->VOcameraBuffer.UpdateBuffer(context, &this->orthographicCamera);

    /*
    //   // XMLoadFloat3(&this->position), XMLoadFloat3(&this->forward), XMLoadFloat3(&this->up));
    //XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
    //    this->projInfo.fovAngleY, this->projInfo.aspectRatio,
    //    this->projInfo.nearZ, this->projInfo.farZ);
    //    
    //    //XMMatrixPerspectiveFovLH(this->projInfo.fovAngleY, this->projInfo.aspectRatio, this->projInfo.nearZ, this->projInfo.farZ);
    //XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
    //    
    //    //XMMatrixMultiply(viewMatrix, projectionMatrix);

    //XMFLOAT4X4 viewProjectionMatrixFloat4x4;
    //DirectX::XMStoreFloat4x4(&viewProjectionMatrixFloat4x4, XMMatrixTranspose(viewProjectionMatrix));

    //this->cameraBuffer.UpdateBuffer(context, &viewProjectionMatrixFloat4x4);
    */
}

void CameraD3D11::UpdateOrthographicBuffer(ID3D11DeviceContext* context, float orthWidth, float orthHeight)
{

    //XMFLOAT4X4 o4x4 = this->GetOrthographicProjectionMatrix();
    //this->OrthBuffer.UpdateBuffer(context, &o4x4);
}

ID3D11Buffer* CameraD3D11::GetConstantBuffer() const { return this->VPcameraBuffer.GetBuffer(); }

ID3D11Buffer* CameraD3D11::GetOrthographicConstantBuffer() const { return this->VOcameraBuffer.GetBuffer(); }

XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
    XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&this->position), XMLoadFloat3(&this->forward), XMLoadFloat3(&this->up));
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(this->projInfo.fovAngleY, this->projInfo.aspectRatio, this->projInfo.nearZ, this->projInfo.farZ);

    XMFLOAT4X4 viewProjMatrix;
    DirectX::XMStoreFloat4x4(&viewProjMatrix, XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));

    return viewProjMatrix;
}

DirectX::XMFLOAT4X4 CameraD3D11::GetOrthographicProjectionMatrix() const
{
    const XMFLOAT3 lpos = { 0.0f, -this->projInfo.farZ, 0.0f };
    const XMFLOAT3 ldir = { 0.0f,-1.0f,0.0f };
    const XMFLOAT3 lup = { 0.0f,0.0f,1.0f };


    XMMATRIX view = XMMatrixLookToLH(
        XMLoadFloat3(&lpos), 
        XMLoadFloat3(&ldir), 
        XMLoadFloat3(&lup)
    );

    XMMATRIX o = XMMatrixOrthographicLH(100.0f, 100.0f, this->projInfo.nearZ, this->projInfo.farZ);
    XMMATRIX vp = XMMatrixTranspose(XMMatrixMultiply(view, o));

    XMFLOAT4X4 o4x4;
    DirectX::XMStoreFloat4x4(&o4x4, vp);
    return o4x4;
}
