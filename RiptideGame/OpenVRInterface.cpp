#include "OpenVRInterface.h"
#include <iostream>

OpenVRInterface::OpenVRInterface() : m_pHMD(nullptr), m_bInitialized(false) {
    for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
        m_rmat4DevicePose[i] = glm::mat4(1.0f);
    }
}

OpenVRInterface::~OpenVRInterface() {
    Shutdown();
}

bool OpenVRInterface::IsHMDConnected() {
    return vr::VR_IsHmdPresent();
}

bool OpenVRInterface::Initialize() {
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

    if (eError != vr::VRInitError_None) {
        m_pHMD = nullptr;
        std::cerr << "Unable to init VR runtime: " << vr::VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
        throw std::runtime_error(vr::VR_GetVRInitErrorAsEnglishDescription(eError));
    }

    // Initialize the VR compositor
    if (!vr::VRCompositor()) {
        std::cerr << "Failed to initialize VR compositor!" << std::endl;
        return false;
    }

    m_bInitialized = true;
    return true;
}

void OpenVRInterface::Shutdown() {
    if (m_pHMD) {
        vr::VR_Shutdown();
        m_pHMD = nullptr;
    }
    m_bInitialized = false;
}

glm::mat4 OpenVRInterface::GetHMDPoseMatrix() const {
    return m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
}

glm::mat4 OpenVRInterface::GetControllerPoseMatrix(vr::ETrackedControllerRole controllerRole) const {
    vr::TrackedDeviceIndex_t controllerIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(controllerRole);
    if (controllerIndex == vr::k_unTrackedDeviceIndexInvalid) {
        return glm::mat4(1.0f);
    }
    return m_rmat4DevicePose[controllerIndex];
}

glm::mat4 OpenVRInterface::GetEyeViewMatrix(vr::Hmd_Eye eye) const {
    if (!m_pHMD) {
        return glm::mat4(1.0f);
    }

    vr::HmdMatrix34_t matEye = m_pHMD->GetEyeToHeadTransform(eye);
    glm::mat4 matrix = ConvertSteamVRMatrixToMatrix4(matEye);
    return glm::inverse(matrix);
}

glm::mat4 OpenVRInterface::GetProjectionMatrix(vr::Hmd_Eye eye, float nearClip, float farClip) const {
    if (!m_pHMD) {
        return glm::mat4(1.0f);
    }

    vr::HmdMatrix44_t matProjection = m_pHMD->GetProjectionMatrix(eye, nearClip, farClip);
    return glm::mat4(
        matProjection.m[0][0], matProjection.m[1][0], matProjection.m[2][0], matProjection.m[3][0],
        matProjection.m[0][1], matProjection.m[1][1], matProjection.m[2][1], matProjection.m[3][1],
        matProjection.m[0][2], matProjection.m[1][2], matProjection.m[2][2], matProjection.m[3][2],
        matProjection.m[0][3], matProjection.m[1][3], matProjection.m[2][3], matProjection.m[3][3]
    );
}

glm::vec3 OpenVRInterface::GetHMDPosition() const {
    glm::mat4 hmdPose = GetHMDPoseMatrix();
    return glm::vec3(hmdPose[3][0], hmdPose[3][1], hmdPose[3][2]);
}

glm::vec3 OpenVRInterface::GetControllerPosition(vr::ETrackedControllerRole controllerRole) const {
    glm::mat4 controllerPose = GetControllerPoseMatrix(controllerRole);
    return glm::vec3(controllerPose[3][0], controllerPose[3][1], controllerPose[3][2]);
}

glm::vec2 OpenVRInterface::GetEyeRenderTargetSize() const {
    if (!m_pHMD) {
        return glm::vec2(0.0f, 0.0f);
    }

    uint32_t width, height;
    m_pHMD->GetRecommendedRenderTargetSize(&width, &height);
    return glm::vec2(width, height);
}

void OpenVRInterface::UpdateTracking() {
    if (!m_pHMD) {
        return;
    }

    vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

    for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
        if (m_rTrackedDevicePose[i].bPoseIsValid) {
            m_rmat4DevicePose[i] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[i].mDeviceToAbsoluteTracking);
        }
    }
}

glm::mat4 OpenVRInterface::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose) const {
    return glm::mat4(
        matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
        matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
        matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
        matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
    );
}

glm::mat4 OpenVRInterface::GetCurrentViewProjectionMatrix(vr::Hmd_Eye eye, float nearClip, float farClip) const {
    glm::mat4 viewMatrix = GetEyeViewMatrix(eye);
    glm::mat4 projectionMatrix = GetProjectionMatrix(eye, nearClip, farClip);
    return projectionMatrix * viewMatrix;
}

// Submit an OpenGL texture to a specific eye
void OpenVRInterface::SubmitOpenGLTextureToEye(vr::Hmd_Eye eye, GLuint textureID) {
    if (!m_pHMD || !vr::VRCompositor()) {
        return;
    }

    vr::Texture_t eyeTexture = { (void*)(uintptr_t)textureID, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
    vr::VRCompositor()->Submit(eye, &eyeTexture);
}

// Submit OpenGL textures to both eyes
void OpenVRInterface::SubmitOpenGLTextures(GLuint leftEyeTextureID, GLuint rightEyeTextureID) {
    SubmitOpenGLTextureToEye(vr::Eye_Left, leftEyeTextureID);
    SubmitOpenGLTextureToEye(vr::Eye_Right, rightEyeTextureID);
}