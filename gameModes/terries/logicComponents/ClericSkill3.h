/*
 * ClericSkill3.h
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

class ClericSkill3 : public LogicComponent
{
	OBJECT(ClericSkill3);
public:
	ClericSkill3(Context* context, Urho3DPlayer* main);
	~ClericSkill3();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleHudButt(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);

	Sprite* skillSprite_;
	float elapsedTime_;
	float cooldown_;
	bool cooling_;
};
