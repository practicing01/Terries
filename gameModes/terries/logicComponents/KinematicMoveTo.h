/*
 * KinematicMoveTo.h
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

class KinematicMoveTo : public LogicComponent
{
	OBJECT(KinematicMoveTo);
public:
	KinematicMoveTo(Context* context, Urho3DPlayer* main, Vector3 dest, float speed);
	~KinematicMoveTo();
	virtual void Start();
	Urho3DPlayer* main_;
	virtual void FixedUpdate(float timeStep);

	Vector3 origin_;
	Vector3 dest_;
	float speed_;
	float percent_;
};
