/*
 * Enchanted.cpp
 *
 *  Created on: Jul 13, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "Enchanted.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"

Enchanted::Enchanted(Context* context, Urho3DPlayer* main, float duration) :
	LogicComponent(context)
{
	main_ = main;
	duration_ = duration;
	elapsedTime_ = 0.0f;
}

Enchanted::~Enchanted()
{
}

void Enchanted::Start()
{
	SetUpdateEventMask(USE_UPDATE);
	//SubscribeToEvent(E_GETCLIENTGRAVITY, HANDLER(Enchanted, HandleGetEnchanted));
}

void Enchanted::HandleGetEnchanted(StringHash eventType, VariantMap& eventData)
{/*
	Node* clientNode = (Node*)(eventData[GetClientEnchanted::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		VariantMap vm;
		vm[SetClientEnchanted::P_NODE] = clientNode;
		vm[SetClientEnchanted::P_GRAVITY] = gravity_;
		SendEvent(E_SETCLIENTGRAVITY, vm);
	}*/
}

void Enchanted::Update(float timeStep)
{
	elapsedTime_ += timeStep;

	if (elapsedTime_ >= duration_ && duration_ > -1)
	{
		node_->RemoveComponent(this);
	}
}
