/*
 * WarriorSkill3.cpp
 *
 *  Created on: Aug 14, 2015
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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "WarriorSkill3.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "TerrySpawner.h"
#include "Speed.h"
#include "Dead.h"

WarriorSkill3::WarriorSkill3(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	cooldown_ = 1.0f;
	duration_ = 0.5f;
	cooling_ = false;
	processing_ = false;
	touchSubscribed_ = false;
	speed_ = 5.0f;
}

WarriorSkill3::~WarriorSkill3()
{
	if (processing_)
	{
		VariantMap vm;
		SendEvent(E_TOUCHUNSUBSCRIBE, vm);
	}
}

void WarriorSkill3::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill3", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(WarriorSkill3, HandleHudButt));
	SetUpdateEventMask(USE_FIXEDUPDATE);
}

void WarriorSkill3::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "warrior")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/warrior/I_Cannon04.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill3")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (cooling_){return;}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "warrior")
		{

			if (!touchSubscribed_)
			{
				touchSubscribed_ = true;
				VariantMap vm;
				SendEvent(E_TOUCHSUBSCRIBE, vm);
			}

			SubscribeToEvent(E_TOUCHEND, HANDLER(WarriorSkill3, HandleTouchEnd));
		}
	}
}

void WarriorSkill3::FixedUpdate(float timeStep)
{
	if (cooling_)
	{
		elapsedTime_ += timeStep;
		if (elapsedTime_ >= cooldown_)
		{
			cooling_ = false;
		}
	}

	if (processing_)
	{
		durationElapsedTime_ += timeStep;
		if (durationElapsedTime_ >= duration_)
		{
			processing_ = false;
			//node_->GetComponent<RigidBody>()->SetMass(1.0f);
			node_->GetComponent<RigidBody>()->SetFriction(10.0f);
			return;
		}

		Vector3 pos = node_->GetPosition();

		float remainingDist_ = (pos - dest_).Length();

		if (remainingDist_ <= 0.5f)
		{
			return;
		}

		Vector3 dir = (dest_ - pos).Normalized();
		dir *= speed_;

		node_->GetComponent<RigidBody>()->ApplyImpulse(dir - node_->GetComponent<RigidBody>()->GetLinearVelocity());
	}
}

void WarriorSkill3::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	if (!processing_)
	{
		UnsubscribeFromEvent(E_NODECOLLISIONSTART);
		return;
	}

	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		otherNode->GetComponent<Health>()->ModifyHealth(10, -1, false);

		if (otherNode->GetComponent<Health>()->health_ <= 0)
		{
			otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
			otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
			return;
		}

		Vector3 victoria = otherNode->GetPosition();
		Vector3 dir = (victoria - node_->GetPosition()).Normalized();
		dir.y_ = 10.0f;
		dir *= 10.0f;

		otherNode->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);

		otherNode->GetComponent<RigidBody>()->ApplyImpulse(dir);
	}
}

void WarriorSkill3::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
{
	if (main_->ui_->GetFocusElement() )//|| touchSubscriberCount_)
	{
		return;
	}

	using namespace TouchEnd;

	Ray cameraRay = main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->
			modelNode_->GetChild("camera")->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	PhysicsRaycastResult raeResult_;

	node_->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, cameraRay, 10000.0f, 1);//todo define masks.

	if (raeResult_.body_)
	{
		Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesCanon.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(node_->GetPosition());
		emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 1.0f), 0, LOCAL);

		particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
		particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.5f);
		particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/275897__n-audioman__blip.ogg"));

		//if (!node_->GetComponent<Blind>()->blinded_)
		{
			dest_ = raeResult_.position_;
			dest_.y_ += 10.0f;
			Vector3 dir = (raeResult_.position_ - node_->GetPosition()).Normalized();
			dir *= speed_;

			node_->GetComponent<RigidBody>()->SetFriction(0.0f);
			//node_->GetComponent<RigidBody>()->SetMass(5.0f);
			node_->GetComponent<RigidBody>()->ApplyImpulse(dir);

			elapsedTime_ = 0.0f;
			durationElapsedTime_ = 0.0f;
			processing_ = true;
			cooling_ = true;

			//SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(WarriorSkill3, HandleNodeCollisionStart));

			VariantMap vm;
			vm[AnimateSceneNode::P_NODE] = node_;

			if (node_->GetVar("sex").GetBool())
			{
				vm[AnimateSceneNode::P_ANIMATION] = "attackF";
			}
			else
			{
				vm[AnimateSceneNode::P_ANIMATION] = "attackM";
			}

			vm[AnimateSceneNode::P_LOOP] = false;

			if (node_->GetPosition().x_ < raeResult_.position_.x_)
			{
				vm[AnimateSceneNode::P_LAYER] = 1;//right
			}
			else
			{
				vm[AnimateSceneNode::P_LAYER] = -1;//left
			}

			SendEvent(E_ANIMATESCENENODE, vm);
		}
	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}
