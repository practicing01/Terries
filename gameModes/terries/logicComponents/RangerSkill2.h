/*
 * RangerSkill2.h
 *
 *  Created on: Aug 14, 2015
 *      Author: practicing01
 */
#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include "../../../Urho3DPlayer.h"
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/UI/Sprite.h>

using namespace Urho3D;

class RangerSkill2 : public LogicComponent
{
	OBJECT(RangerSkill2);
public:
	RangerSkill2(Context* context, Urho3DPlayer* main);
	~RangerSkill2();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleHudButt(StringHash eventType, VariantMap& eventData);
	void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);

	Sprite* skillSprite_;

	bool enabled_;
	Vector3 dest_;
	bool touchSubscribed_;
	float attackElapsedTime_;
	float attackInterval_;
	float arrowSpeed_;
	float radius_;

	Node* arrow_;

	PODVector<RigidBody*> rigidBodies_;
};
