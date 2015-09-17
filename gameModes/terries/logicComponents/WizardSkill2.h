/*
 * WizardSkill2.h
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

class WizardSkill2 : public LogicComponent
{
	OBJECT(WizardSkill2);
public:
	WizardSkill2(Context* context, Urho3DPlayer* main);
	~WizardSkill2();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleHudButt(StringHash eventType, VariantMap& eventData);
	void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);
	virtual void FixedUpdate(float timeStep);

	Sprite* skillSprite_;
	Vector3 dest_;
	bool touchSubscribed_;
	float elapsedTime_;
	float cooldown_;
	bool cooling_;
	float attackInterval_;
	float attackElapsedTime_;
	bool canAttack_;

	Node* dot_;
};
