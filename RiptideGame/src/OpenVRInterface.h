#include "RenderDevice.h"
#include "DeviceContext.h"
#include "C:\Users\Blue\OculusWRP\openvr\headers\openvr.h"
#include <stdexcept>
#include "RefCntAutoPtr.hpp"
#include "BasicMath.hpp"
#include <cassert>
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "C:\Users\Blue\RiptideVR\DiligentSamples\Tutorials\Common\src\TexturedCube.hpp"

using namespace Diligent;

class OpenVRInterface
{
public:
    OpenVRInterface(IRenderDevice* pDevice, IDeviceContext* pContext) :
        m_pDevice(pDevice),
        m_pImmediateContext(pContext)
    {
    }

    void Initialize()
    {
        vr::EVRInitError eError = vr::VRInitError_None;
        m_pHMD                  = vr::VR_Init(&eError, vr::VRApplication_Scene);
        if (eError != vr::VRInitError_None)
            throw std::runtime_error("VR_Init failed");

        uint32_t renderWidth, renderHeight;
        m_pHMD->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);

        CreateEyeResources(renderWidth, renderHeight);
        CreateCubeResources();
    }

    void RenderFrame()
    {
        vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
        vr::VRCompositor()->WaitGetPoses(trackedDevicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

        UpdateDevicePoses(trackedDevicePoses);

        for (int eye = 0; eye < 2; ++eye)
        {
            RenderEye(static_cast<vr::EVREye>(eye));
        }

        SubmitTextures();
    }

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

    void CreateEyeResources(uint32_t width, uint32_t height)
    {
        TextureDesc eyeTexDesc;
        eyeTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        eyeTexDesc.Width     = width;
        eyeTexDesc.Height    = height;
        eyeTexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
        eyeTexDesc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

        TextureDesc depthDesc = eyeTexDesc;
        depthDesc.Format      = TEX_FORMAT_D32_FLOAT;
        depthDesc.BindFlags   = BIND_DEPTH_STENCIL;

        for (int eye = 0; eye < 2; ++eye)
        {
            m_pDevice->CreateTexture(eyeTexDesc, nullptr, &m_EyeTargets[eye].Color);
            m_pDevice->CreateTexture(depthDesc, nullptr, &m_EyeTargets[eye].Depth);
        }
    }

    void CreateCubeResources()
    {
        // vertex/index buffer/normals
        GEOMETRY_PRIMITIVE_VERTEX_FLAGS VertexComponents =
            GEOMETRY_PRIMITIVE_VERTEX_FLAG_POSITION |
            GEOMETRY_PRIMITIVE_VERTEX_FLAG_NORMAL;

        m_CubeVertexBuffer = TexturedCube::CreateVertexBuffer(m_pDevice, VertexComponents);
        m_CubeIndexBuffer  = TexturedCube::CreateIndexBuffer(m_pDevice);

        // constant buffer
        BufferDesc CBDesc;
        CBDesc.Name           = "Cube Constants CB";
        CBDesc.Size           = sizeof(ModelConstants);
        CBDesc.Usage          = USAGE_DYNAMIC;
        CBDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(CBDesc, nullptr, &m_Constants);

        // pipeline state
        GraphicsPipelineStateCreateInfo PSOCreateInfo;
        PSOCreateInfo.PSODesc.Name = "VR Cube PSO";

        // shaders
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pDevice->GetEngineFactory()->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        // vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Cube VS";
            ShaderCI.Source          = VSSource;
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Cube PS";
            ShaderCI.Source          = PSSource;
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // pipeline setup
        PSOCreateInfo.PSODesc.PipelineType               = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]     = TEX_FORMAT_RGBA8_UNORM;
        PSOCreateInfo.GraphicsPipeline.DSVFormat         = TEX_FORMAT_D32_FLOAT;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // depth test
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = True;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = True;

        // input layout
        LayoutElement LayoutElems[] =
            {
                {0, 0, 3, VT_FLOAT32, False}, // Position
                {1, 0, 3, VT_FLOAT32, False}  // Normal
            };
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // shaders
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // resource layout
        ShaderResourceVariableDesc Variables[] =
            {
                {SHADER_TYPE_VERTEX, "Constants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Variables;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Variables);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_PSO);
        m_PSO->CreateShaderResourceBinding(&m_SRB, true);
        m_SRB->GetVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_Constants);
    }

    void UpdateDevicePoses(vr::TrackedDevicePose_t* poses)
    {
        m_LeftControllerMatrix  = float4x4::Identity();
        m_RightControllerMatrix = float4x4::Identity();

        for (uint32_t deviceIdx = 0; deviceIdx < vr::k_unMaxTrackedDeviceCount; ++deviceIdx)
        {
            if (!poses[deviceIdx].bPoseIsValid)
                continue;

            switch (m_pHMD->GetTrackedDeviceClass(deviceIdx))
            {
                case vr::TrackedDeviceClass_Controller:
                {
                    vr::ETrackedControllerRole role = m_pHMD->GetControllerRoleForTrackedDeviceIndex(deviceIdx);
                    if (role == vr::TrackedControllerRole_LeftHand)
                        m_LeftControllerMatrix = ConvertSteamVRMatrix(poses[deviceIdx].mDeviceToAbsoluteTracking);
                    else if (role == vr::TrackedControllerRole_RightHand)
                        m_RightControllerMatrix = ConvertSteamVRMatrix(poses[deviceIdx].mDeviceToAbsoluteTracking);
                    break;
                }
                case vr::TrackedDeviceClass_HMD:
                    m_HMDMatrix = ConvertSteamVRMatrix(poses[deviceIdx].mDeviceToAbsoluteTracking);
                    break;
            }
        }
    }

    void RenderEye(vr::EVREye eye)
    {
        const int eyeIdx = (eye == vr::Eye_Left) ? 0 : 1;
        auto      pRTV   = m_EyeTargets[eyeIdx].Color->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        auto      pDSV   = m_EyeTargets[eyeIdx].Depth->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float clearColor[] = {0.17f, 0.17f, 0.17f, 1.0f};
        m_pImmediateContext->ClearRenderTarget(pRTV, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // get eye matrices
        auto     eyeToHead     = m_pHMD->GetEyeToHeadTransform(eye);
        float4x4 matEyeToHead  = ConvertSteamVRMatrix(eyeToHead);
        float4x4 matProjection = ConvertProjectionMatrix(m_pHMD->GetProjectionMatrix(eye, 0.1f, 1000.0f));

        // correct view matrix with inverse: headPose^-1 * eyeOffset^-1
        float4x4 matView = (m_HMDMatrix * matEyeToHead).Inverse();

        // only render controllers, not HMD
        RenderController(m_LeftControllerMatrix, matView, matProjection);
        RenderController(m_RightControllerMatrix, matView, matProjection);
    }

    void RenderController(const float4x4& controllerMat, const float4x4& view, const float4x4& proj)
    {
        // add scale matrix (0.1m size)
        float4x4 scaleMat = float4x4::Scale(0.1f, 0.1f, 0.1f);

        // offset cube to appear in front of controller
        float4x4 offsetMat = float4x4::Translation(0, 0, -0.1f);

        float4x4 modelMat = scaleMat * offsetMat * controllerMat;

        ModelConstants constants;
        constants.WorldViewProj   = modelMat * view * proj;
        constants.NormalTransform = (modelMat * view).Inverse().Transpose();
        constants.Color           = float4(0.2f, 0.8f, 0.2f, 1.0f); // Green color

        {
            MapHelper<ModelConstants> CBConstants(m_pImmediateContext, m_Constants, MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = constants;
        }

        IBuffer* pVBs[] = {m_CubeVertexBuffer};
        m_pImmediateContext->SetVertexBuffers(0, 1, pVBs, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pImmediateContext->SetPipelineState(m_PSO);
        m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs drawAttrs{36, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
        m_pImmediateContext->DrawIndexed(drawAttrs);
    }

    void RenderModel(const float4x4& modelMat, const float4x4& viewProj)
    {
        ModelConstants constants;
        constants.WorldViewProj   = modelMat * viewProj;
        constants.NormalTransform = (modelMat * viewProj).Inverse().Transpose();
        constants.Color           = float4(0.5f, 0.8f, 0.3f, 1.0f);

        {
            MapHelper<ModelConstants> CBConstants(m_pImmediateContext, m_Constants, MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = constants;
        }

        IBuffer* pVBs[] = {m_CubeVertexBuffer};
        m_pImmediateContext->SetVertexBuffers(0, 1, pVBs, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pImmediateContext->SetPipelineState(m_PSO);
        m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs drawAttrs{36, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
        m_pImmediateContext->DrawIndexed(drawAttrs);
    }


    void SubmitTextures()
    {
        vr::Texture_t tex[2];
        tex[0].eType = tex[1].eType = vr::TextureType_DirectX;
        tex[0].eColorSpace = tex[1].eColorSpace = vr::ColorSpace_Gamma;

        for (int eye = 0; eye < 2; ++eye)
        {
            tex[eye].handle = reinterpret_cast<void*>(m_EyeTargets[eye].Color->GetNativeHandle());
            vr::VRCompositor()->Submit(static_cast<vr::EVREye>(eye), &tex[eye]);
        }
    }

    static float4x4 ConvertSteamVRMatrix(const vr::HmdMatrix34_t& mat)
    {
        return float4x4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,
            mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,
            -mat.m[0][2], -mat.m[1][2], -mat.m[2][2], 0.0f,
            mat.m[0][3], mat.m[1][3], -mat.m[2][3], 1.0f);
    }


    static float4x4 ConvertProjectionMatrix(const vr::HmdMatrix44_t& mat)
    {
        return float4x4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    }

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