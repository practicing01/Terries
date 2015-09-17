/*
 * RogueSkill1.cpp
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

#include "RogueSkill1.h"
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

RogueSkill1::RogueSkill1(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	touchSubscribed_ = false;
	cooldown_ = 0.0f;
	cooling_ = false;
}

RogueSkill1::~RogueSkill1()
{
}

void RogueSkill1::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill1", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(RogueSkill1, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void RogueSkill1::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "rogue")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/rogue/S_Shadow05.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill1")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "rogue")
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

			SubscribeToEvent(E_TOUCHEND, HANDLER(RogueSkill1, HandleTouchEnd));
		}
	}
}

void RogueSkill1::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
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
		dest_ = raeResult_.position_ + (Vector3::UP * 0.5f);

		Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesTeleport.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(node_->GetPosition());
		emitterStartFX_->SetEmitting(true);
		particleStartNode_->SetScale(0.5f);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 0.5f), 0, LOCAL);

		node_->SetPosition(dest_);

		Node* particleEndNode_ = node_->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterEndFX_ = particleEndNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterEndFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesTeleport.xml"));
		emitterEndFX_->SetViewMask(1);
		particleEndNode_->SetPosition(node_->GetPosition());
		emitterEndFX_->SetEmitting(true);
		particleEndNode_->SetScale(0.5f);

		particleEndNode_->AddComponent(new TimedRemove(context_, main_, 0.5f), 0, LOCAL);

		particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
		particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/89170__unklekarma__nintendo-percystart.ogg"));

		VariantMap vm;
		vm[AnimateSceneNode::P_NODE] = node_;

		if (node_->GetVar("sex").GetBool())
		{
			vm[AnimateSceneNode::P_ANIMATION] = "gestureF";
		}
		else
		{
			vm[AnimateSceneNode::P_ANIMATION] = "gestureM";
		}

		vm[AnimateSceneNode::P_LOOP] = false;

		if (node_->GetPosition().x_ < dest_.x_)
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
	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}

void RogueSkill1::Update(float timeStep)
{
	if (cooling_)
	{
		elapsedTime_ += timeStep;
		if (elapsedTime_ >= cooldown_)
		{
			cooling_ = false;
		}
	}
}
