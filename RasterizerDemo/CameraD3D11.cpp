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

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(this->projInfo.fovAngleY, this->projInfo.aspectRatio, this->projInfo.nearZ, this->projInfo.farZ);
    XMFLOAT4X4 projectionMatrixFloat4x4;
    XMStoreFloat4x4(&projectionMatrixFloat4x4, projectionMatrix);

    this->cameraBuffer.Initialize(device, sizeof(XMFLOAT4X4), &projectionMatrixFloat4x4);
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
    XMStoreFloat3(&this->forward, forwardVec);

    XMVECTOR rightVec = XMLoadFloat3(&this->right);
    rightVec = XMVector3TransformNormal(rightVec, rotationMatrix);
    XMStoreFloat3(&this->right, rightVec);

    XMVECTOR upVec = XMLoadFloat3(&this->up);
    upVec = XMVector3TransformNormal(upVec, rotationMatrix);
    XMStoreFloat3(&this->up, upVec);
}

void CameraD3D11::MoveForward(float amount) { MoveInDirection(amount, this->forward); }

void CameraD3D11::MoveRight(float amount) { MoveInDirection(amount, this->right); }

void CameraD3D11::MoveUp(float amount) { MoveInDirection(amount, this->up); }

void CameraD3D11::RotateForward(float amount) { RotateAroundAxis(amount, this->forward); }

void CameraD3D11::RotateRight(float amount) { RotateAroundAxis(amount, this->right); }

void CameraD3D11::RotateUp(float amount) { RotateAroundAxis(amount, this->up); }

const XMFLOAT3& CameraD3D11::GetPosition() const { return this->position; }

const XMFLOAT3& CameraD3D11::GetForward() const { return this->forward; }

const XMFLOAT3& CameraD3D11::GetRight() const { return this->right; }

const XMFLOAT3& CameraD3D11::GetUp() const { return this->up; }

void CameraD3D11::ResetUp()
{
    this->forward = { 0, 0, 1 };
    this->right = { 1, 0, 0 };
    this->up = { 0, 1, 0 };
}

void CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
    XMMATRIX viewMatrix = XMMatrixLookAtLH(
        XMVectorSet(this->position.x, this->position.y, this->position.z, 0.0f), //EyePosition
        XMVectorSet(
            this->position.x + this->forward.x,
            this->position.y + this->forward.y,
            this->position.z + this->forward.z, 0.0f),							// Focus Position
        XMVectorSet(this->up.x, this->up.y, this->up.z, 0.0f));					 // Up
        
        
       // XMLoadFloat3(&this->position), XMLoadFloat3(&this->forward), XMLoadFloat3(&this->up));
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        this->projInfo.fovAngleY, this->projInfo.aspectRatio,
        this->projInfo.nearZ, this->projInfo.farZ);
        
        //XMMatrixPerspectiveFovLH(this->projInfo.fovAngleY, this->projInfo.aspectRatio, this->projInfo.nearZ, this->projInfo.farZ);
    XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
        
        //XMMatrixMultiply(viewMatrix, projectionMatrix);

    XMFLOAT4X4 viewProjectionMatrixFloat4x4;
    XMStoreFloat4x4(&viewProjectionMatrixFloat4x4, XMMatrixTranspose(viewProjectionMatrix));

    this->cameraBuffer.UpdateBuffer(context, &viewProjectionMatrixFloat4x4);
}

ID3D11Buffer* CameraD3D11::GetConstantBuffer() const { return this->cameraBuffer.GetBuffer(); }

XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
    XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&this->position), XMLoadFloat3(&this->forward), XMLoadFloat3(&this->up));
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(this->projInfo.fovAngleY, this->projInfo.aspectRatio, this->projInfo.nearZ, this->projInfo.farZ);

    XMFLOAT4X4 viewProjMatrix;
    XMStoreFloat4x4(&viewProjMatrix, XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));

    return viewProjMatrix;
}