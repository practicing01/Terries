/*
 * HUD.h
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

using namespace Urho3D;

class HUD : public LogicComponent
{
	OBJECT(HUD);
public:
	HUD(Context* context, Urho3DPlayer* main);
	~HUD();
	virtual void Start();
	Urho3DPlayer* main_;

	void HandleButtonRelease(StringHash eventType, VariantMap& eventData);

	SharedPtr<UIElement> hud_;
};
