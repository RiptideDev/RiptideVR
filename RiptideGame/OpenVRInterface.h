#pragma once
#include "glad.h"
#include <openvr.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

class OpenVRInterface {
public:
    OpenVRInterface();
    ~OpenVRInterface();

    bool IsHMDConnected();

    bool Initialize();
    void Shutdown();

    glm::mat4 GetHMDPoseMatrix() const;
    glm::mat4 GetControllerPoseMatrix(vr::ETrackedControllerRole controllerRole) const;
    glm::mat4 GetEyeViewMatrix(vr::Hmd_Eye eye) const;
    glm::mat4 GetProjectionMatrix(vr::Hmd_Eye eye, float nearClip, float farClip) const;

    glm::vec3 GetHMDPosition() const;
    glm::vec3 GetControllerPosition(vr::ETrackedControllerRole controllerRole) const;

    glm::vec2 GetEyeRenderTargetSize() const;

    void SubmitOpenGLTextureToEye(vr::Hmd_Eye eye, GLuint textureID);
    void SubmitOpenGLTextures(GLuint leftEyeTextureID, GLuint rightEyeTextureID);

    void UpdateTracking();

private:
    vr::IVRSystem* m_pHMD;
    vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
    glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

    bool m_bInitialized;

    glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose) const;
    glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye eye, float nearClip, float farClip) const;
};