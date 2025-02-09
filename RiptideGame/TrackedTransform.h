#pragma once
#include "TransformInstance.h"

enum TrackedPoseObjectRole
{
	TRACKEDPOSEROLE_LEFT_CONTROLLER,
	TRACKEDPOSEROLE_RIGHT_CONTROLLER,

	TRACKEDPOSEROLE_HMD
};

class TrackedTransform : public TransformInstance
{
	// a tracked transform instance that is posed by the owners OpenVRInterface
public:
	TrackedPoseObjectRole PoseRole = TRACKEDPOSEROLE_HMD;
	// called BEFORE the packets are sent, all instances update is called by the owner
	void Update(float dt) override;

	std::string GetClassName() const override { return "TrackedTransform"; }
};