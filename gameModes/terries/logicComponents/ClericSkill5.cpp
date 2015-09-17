/*
 * ClericSkill5.cpp
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
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "ClericSkill5.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "TerrySpawner.h"
#include "KinematicMoveTo.h"
#include "Dead.h"

ClericSkill5::ClericSkill5(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	touchSubscribed_ = false;
	cooldown_ = 0.0f;
	cooling_ = false;
	processing_ = false;
	duration_ = 1.0f;
	durationElapsedTime_ = 0.0f;
}

ClericSkill5::~ClericSkill5()
{
}

void ClericSkill5::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill5", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(ClericSkill5, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesBeam.xml");
	beam_ = node_->GetScene()->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

	SubscribeToEvent(beam_, E_NODECOLLISIONSTART, HANDLER(ClericSkill5, HandleNodeCollisionStart));

	beam_->SetEnabled(false);
}

void ClericSkill5::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "cleric")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/cleric/S_Light04.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill5")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "cleric")
		{
			if (touchSubscribed_ || cooling_)
			{
				return;
			}

			if (!touchSubscribed_)
			{
				touchSubscribed_ = true;
				VariantMap vm;
				SendEvent(E_TOUCHSUBSCRIBE, vm);
			}

			SubscribeToEvent(E_TOUCHEND, HANDLER(ClericSkill5, HandleTouchEnd));
		}
	}
}

void ClericSkill5::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
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
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/beam.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(node_->GetPosition());
		particleStartNode_->LookAt(raeResult_.position_);
		emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 1.0f), 0, LOCAL);

		Node* particleStartNode0_ = node_->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX0_ = particleStartNode0_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX0_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/beamSpark.xml"));
		emitterStartFX0_->SetViewMask(1);
		particleStartNode0_->SetPosition(node_->GetPosition());
		particleStartNode0_->LookAt(raeResult_.position_);
		emitterStartFX0_->SetEmitting(true);

		particleStartNode0_->AddComponent(new TimedRemove(context_, main_, 1.0f), 0, LOCAL);

		particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
		particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/276331__n-audioman__low-pulse-ambulance.ogg"));

		beam_->SetPosition(node_->GetPosition());
		beam_->LookAt(raeResult_.position_);
		beam_->SetEnabled(true);

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
			vm[AnimateSceneNode::P_LAYER] = 1;
		}
		else
		{
			vm[AnimateSceneNode::P_LAYER] = -1;
		}

		SendEvent(E_ANIMATESCENENODE, vm);

		elapsedTime_ = 0.0f;
		cooling_ = true;
		processing_ = true;
		durationElapsedTime_ = 0.0f;
	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}

void ClericSkill5::Update(float timeStep)
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
			beam_->SetEnabled(false);
		}
	}
}

void ClericSkill5::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
		otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
	}
}
