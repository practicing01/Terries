/*
 * Blind.h
 *
 *  Created on: Jul 13, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include "../../../Urho3DPlayer.h"
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/Connection.h>

using namespace Urho3D;

class Blind : public LogicComponent
{
	OBJECT(Blind);
public:
	Blind(Context* context, Urho3DPlayer* main, float duration);
	~Blind();
	virtual void Start();
	void HandleGetBlind(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);

	Urho3DPlayer* main_;

	float duration_;
	float elapsedTime_;
};
