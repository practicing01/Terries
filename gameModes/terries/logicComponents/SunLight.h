/*
 * SunLight.h
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

class SunLight : public LogicComponent
{
	OBJECT(SunLight);
public:
	SunLight(Context* context, Urho3DPlayer* main);
	~SunLight();
	virtual void Start();
	Urho3DPlayer* main_;
	virtual void Update(float timeStep);

	float speed_;
	bool dir_;
	Node* sun_;
};
