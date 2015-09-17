/*
 * SunLight.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: practicing01
 */
#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UIEvents.h>

#include "SunLight.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"

SunLight::SunLight(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	speed_ = 0.5f;
	dir_ = false;
}

SunLight::~SunLight()
{
}

void SunLight::Start()
{
	sun_ = node_->GetChild("sun");
	SetUpdateEventMask(USE_UPDATE);
}

void SunLight::Update(float timeStep)
{

	if (!dir_)
	{
		sun_->GetComponent<Light>()->SetBrightness(
				sun_->GetComponent<Light>()->GetBrightness() - (speed_ * timeStep));

		if (sun_->GetComponent<Light>()->GetBrightness() <= 0.0f)
		{
			dir_ = !dir_;
		}
	}
	else
	{
		sun_->GetComponent<Light>()->SetBrightness(
				sun_->GetComponent<Light>()->GetBrightness() + (speed_ * timeStep));

		if (sun_->GetComponent<Light>()->GetBrightness() >= 4.0f)
		{
			dir_ = !dir_;
		}
	}
}
