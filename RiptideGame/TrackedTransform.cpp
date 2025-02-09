#include "TrackedTransform.h"
#include "Riptide.h"

void TrackedTransform::Update(float dt)
{
	switch (PoseRole)
	{
	case TRACKEDPOSEROLE_LEFT_CONTROLLER:
		ModelMatrix = Riptide::VRInterface->GetControllerPoseMatrix(vr::TrackedControllerRole_LeftHand);
		break;
	case TRACKEDPOSEROLE_RIGHT_CONTROLLER:
		ModelMatrix = Riptide::VRInterface->GetControllerPoseMatrix(vr::TrackedControllerRole_RightHand);
		break;
	default:
		ModelMatrix = Riptide::VRInterface->GetHMDPoseMatrix();
		break;
	}
}
