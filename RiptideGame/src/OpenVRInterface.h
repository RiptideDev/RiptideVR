#include "RenderDevice.h"
#include "DeviceContext.h"
#include "openvr.h"
#include <stdexcept>
#include "RefCntAutoPtr.hpp"
#include "BasicMath.hpp"
#include <cassert>
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TexturedCube.hpp"

using namespace Diligent;

class OpenVRInterface
{
public:
    OpenVRInterface(IRenderDevice* pDevice, IDeviceContext* pContext) :
        m_pDevice(pDevice),
        m_pImmediateContext(pContext)
    {
    }

    void Initialize();

    void RenderFrame();

private:
    struct RenderTarget
    {
        RefCntAutoPtr<ITexture> Color;
        RefCntAutoPtr<ITexture> Depth;
    };

    struct ModelConstants
    {
        float4x4 WorldViewProj;
        float4x4 NormalTransform;
        float4   Color;
    };

    vr::IVRSystem*  m_pHMD = nullptr;
    IRenderDevice*  m_pDevice;
    IDeviceContext* m_pImmediateContext;
    RenderTarget    m_EyeTargets[2];

    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_Constants;
    RefCntAutoPtr<IPipelineState>         m_PSO;
    RefCntAutoPtr<IShaderResourceBinding> m_SRB;

    float4x4 m_HMDMatrix             = float4x4::Identity();
    float4x4 m_LeftControllerMatrix  = float4x4::Identity();
    float4x4 m_RightControllerMatrix = float4x4::Identity();

    void CreateEyeResources(uint32_t width, uint32_t height);

    void CreateCubeResources();

    void UpdateDevicePoses(vr::TrackedDevicePose_t* poses);

    void RenderEye(vr::EVREye eye);

    void RenderController(const float4x4& controllerMatrix, const float4x4& projectionMatrix);

    void RenderModel(const float4x4& modelMat, const float4x4& viewProj);


    void SubmitTextures();

    static float4x4 ConvertSteamVRMatrix(const vr::HmdMatrix34_t& mat);

    static float4x4 ConvertProjectionMatrix(const vr::HmdMatrix44_t& mat);

    const char* VSSource = R"(
struct VSInput
{
    float3 Pos   : ATTRIB0;
    float3 Norm  : ATTRIB1;
};

struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Norm  : NORMAL;
};

cbuffer Constants
{
    float4x4 WorldViewProj;
    float4x4 NormalTransform;
    float4 Color;
};

void main(in VSInput VSIn, out PSInput PSOut)
{
    PSOut.Pos = mul(float4(VSIn.Pos, 1.0), WorldViewProj);
    PSOut.Norm = mul(VSIn.Norm, (float3x3)NormalTransform);
}
)";

    const char* PSSource = R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Norm  : NORMAL;
};

cbuffer Constants
{
    float4 Color;
};

float4 main(in PSInput PSIn) : SV_TARGET
{
    float3 lightDir = normalize(float3(0.5, -1.0, 0.25));
    float NdL = max(dot(normalize(PSIn.Norm), -lightDir), 0.1);
    return float4(Color.rgb * NdL, Color.a);
}
)";
};