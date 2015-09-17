/*
 * Dead.cpp
 *
 *  Created on: Jul 13, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UIEvents.h>

#include "Dead.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "SpriteSheetPlayer.h"

Dead::Dead(Context* context, Urho3DPlayer* main, float duration) :
	LogicComponent(context)
{
	main_ = main;
	duration_ = duration;
	elapsedTime_ = 0.0f;
}

Dead::~Dead()
{
}

void Dead::Start()
{
	SetUpdateEventMask(USE_UPDATE);
	//SubscribeToEvent(E_GETCLIENTGRAVITY, HANDLER(Dead, HandleGetDead));

	VariantMap vm;
	vm[AnimateSceneNode::P_NODE] = node_;

	if (node_->GetVar("sex").GetBool())
	{
		vm[AnimateSceneNode::P_ANIMATION] = "dieF";
	}
	else
	{
		vm[AnimateSceneNode::P_ANIMATION] = "dieM";
	}

	vm[AnimateSceneNode::P_LOOP] = false;
	vm[AnimateSceneNode::P_LAYER] = 0;

	SendEvent(E_ANIMATESCENENODE, vm);
}

void Dead::HandleGetDead(StringHash eventType, VariantMap& eventData)
{/*
	Node* clientNode = (Node*)(eventData[GetClientDead::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		VariantMap vm;
		vm[SetClientDead::P_NODE] = clientNode;
		vm[SetClientDead::P_GRAVITY] = gravity_;
		SendEvent(E_SETCLIENTGRAVITY, vm);
	}*/
}

void Dead::Update(float timeStep)
{
	elapsedTime_ += timeStep;

	if (elapsedTime_ >= duration_ && duration_ > -1)
	{
		node_->RemoveComponent(this);
	}
}
