/*
 * RangerSkill5.h
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

class RangerSkill5 : public LogicComponent
{
	OBJECT(RangerSkill5);
public:
	RangerSkill5(Context* context, Urho3DPlayer* main);
	~RangerSkill5();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleHudButt(StringHash eventType, VariantMap& eventData);

	Sprite* skillSprite_;

	bool enabled_;

	Node* particleNode_;
};
