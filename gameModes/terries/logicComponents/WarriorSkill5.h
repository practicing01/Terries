/*
 * WarriorSkill5.h
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
#include <Urho3D/Physics/PhysicsWorld.h>

using namespace Urho3D;

class WarriorSkill5 : public LogicComponent
{
	OBJECT(WarriorSkill5);
public:
	WarriorSkill5(Context* context, Urho3DPlayer* main);
	~WarriorSkill5();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleHudButt(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);

	Sprite* skillSprite_;

	float radius_;
	float elapsedTime_;
	float cooldown_;
	bool cooling_;
	bool processing_;

	PODVector<RigidBody*> rigidBodies_;
};
