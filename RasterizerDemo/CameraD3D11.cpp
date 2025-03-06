#include "CameraD3D11.h"
#include <DirectXMath.h>

using namespace DirectX;

CameraD3D11::CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo, const XMFLOAT3& initialPosition)
{
    Initialize(device, projectionInfo, initialPosition);
}

void CameraD3D11::Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo, const XMFLOAT3& initialPosition)
{
    position = initialPosition;
    projInfo = projectionInfo;

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(projInfo.fovAngleY, projInfo.aspectRatio, projInfo.nearZ, projInfo.farZ);
    XMFLOAT4X4 projectionMatrixFloat4x4;
    XMStoreFloat4x4(&projectionMatrixFloat4x4, projectionMatrix);

    cameraBuffer.Initialize(device, sizeof(XMFLOAT4X4), &projectionMatrixFloat4x4);
}

void CameraD3D11::MoveInDirection(float amount, const XMFLOAT3& direction)
{
    XMVECTOR dir = XMLoadFloat3(&direction);
    XMVECTOR pos = XMLoadFloat3(&position);
    pos = XMVectorAdd(pos, XMVectorScale(dir, amount));
    XMStoreFloat3(&position, pos);
}

void CameraD3D11::RotateAroundAxis(float amount, const XMFLOAT3& axis)
{
    XMVECTOR ax = XMLoadFloat3(&axis);
    XMMATRIX rotationMatrix = XMMatrixRotationAxis(ax, amount);

    XMVECTOR forwardVec = XMLoadFloat3(&forward);
    forwardVec = XMVector3TransformNormal(forwardVec, rotationMatrix);
    XMStoreFloat3(&forward, forwardVec);

    XMVECTOR rightVec = XMLoadFloat3(&right);
    rightVec = XMVector3TransformNormal(rightVec, rotationMatrix);
    XMStoreFloat3(&right, rightVec);

    XMVECTOR upVec = XMLoadFloat3(&up);
    upVec = XMVector3TransformNormal(upVec, rotationMatrix);
    XMStoreFloat3(&up, upVec);
}

void CameraD3D11::MoveForward(float amount)
{
    MoveInDirection(amount, forward);
}

void CameraD3D11::MoveRight(float amount)
{
    MoveInDirection(amount, right);
}

void CameraD3D11::MoveUp(float amount)
{
    MoveInDirection(amount, up);
}

void CameraD3D11::RotateForward(float amount)
{
    RotateAroundAxis(amount, forward);
}

void CameraD3D11::RotateRight(float amount)
{
    RotateAroundAxis(amount, right);
}

void CameraD3D11::RotateUp(float amount)
{
    RotateAroundAxis(amount, up);
}

const XMFLOAT3& CameraD3D11::GetPosition() const
{
    return position;
}

const XMFLOAT3& CameraD3D11::GetForward() const
{
    return forward;
}

const XMFLOAT3& CameraD3D11::GetRight() const
{
    return right;
}

const XMFLOAT3& CameraD3D11::GetUp() const
{
    return up;
}

void CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
    XMMATRIX viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMLoadFloat3(&up));
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(projInfo.fovAngleY, projInfo.aspectRatio, projInfo.nearZ, projInfo.farZ);
    XMMATRIX viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    XMFLOAT4X4 viewProjectionMatrixFloat4x4;
    XMStoreFloat4x4(&viewProjectionMatrixFloat4x4, viewProjectionMatrix);

    cameraBuffer.UpdateBuffer(context, &viewProjectionMatrixFloat4x4);
}

ID3D11Buffer* CameraD3D11::GetConstantBuffer() const
{
    return cameraBuffer.GetBuffer();
}

XMMATRIX CameraD3D11::GetViewProjectionMatrix() const
{
    XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMLoadFloat3(&up));
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(projInfo.fovAngleY, projInfo.aspectRatio, projInfo.nearZ, projInfo.farZ);
    
    return XMMatrixMultiply(viewMatrix, projectionMatrix);
}