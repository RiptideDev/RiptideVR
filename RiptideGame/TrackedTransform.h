#pragma once
#include "TransformInstance.h"
class TrackedTransform : public TransformInstance
{
	// a tracked transform instance that is posed by the owners OpenVRInterface
public:
	// called BEFORE the packets are sent, all instances update is called by the owner
	void Update(float dt) override;
};