/*
 * KinematicMoveTo.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: practicing01
 */
#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
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

#include "KinematicMoveTo.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "TimedRemove.h"

KinematicMoveTo::KinematicMoveTo(Context* context, Urho3DPlayer* main, Vector3 dest, float speed) :
LogicComponent(context)
{
	main_ = main;
	dest_ = dest;
	speed_ = speed;
	percent_ = 0.0f;
}

KinematicMoveTo::~KinematicMoveTo()
{
}

void KinematicMoveTo::Start()
{
	origin_ = node_->GetPosition();
	SetUpdateEventMask(USE_FIXEDUPDATE);
}

void KinematicMoveTo::FixedUpdate(float timeStep)
{
	percent_ += speed_ * timeStep;

	if (percent_ > 1.0f)
	{
		if (node_->GetName() == "blindArrow")
		{
			/*Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
			emitterStartFX_->SetViewMask(1);
			particleStartNode_->SetPosition(node_->GetPosition() + (Vector3::UP));
			emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);*/

			Node* lightNode = node_->GetScene()->CreateChild(0,LOCAL);
			lightNode->SetPosition(node_->GetPosition() + (Vector3::UP));
			Light* light = lightNode->CreateComponent<Light>(LOCAL);
			light->SetBrightness(2.0f);
			light->SetPerVertex(true);
			light->SetRange(5.0f);
			lightNode->AddComponent(new TimedRemove(context_, main_, 0.1f), 0, LOCAL);

		}
		//node_->RemoveAllChildren();
		//node_->RemoveAllComponents();//crash
		node_->Remove();
		return;
	}

	node_->SetPosition(origin_.Lerp(dest_, percent_));
}
