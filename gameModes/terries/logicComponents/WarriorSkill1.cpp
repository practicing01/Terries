/*
 * WarriorSkill1.cpp
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

#include "WarriorSkill1.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "Armor.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "Speed.h"
#include "Dead.h"

WarriorSkill1::WarriorSkill1(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	cooldown_ = 5.0f;
	duration_ = 5.0f;
	cooling_ = false;
	processing_ = false;
}

WarriorSkill1::~WarriorSkill1()
{
}

void WarriorSkill1::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill1", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(WarriorSkill1, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void WarriorSkill1::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "warrior")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/warrior/S_Buff14.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill1")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (cooling_){return;}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "warrior")
		{
			node_->GetComponent<Armor>()->ModifyArmor(1, -1, false);
			int attack = node_->GetVar("attack").GetInt();
			node_->SetVar("attack", attack + 50);
			float attackInterval = node_->GetVar("attackInterval").GetFloat();
			node_->SetVar("attackInterval", attackInterval * 0.5f);

			elapsedTime_ = 0.0f;
			durationElapsedTime_ = 0.0f;
			processing_ = true;
			cooling_ = true;

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
			vm[AnimateSceneNode::P_LAYER] = 0;

			SendEvent(E_ANIMATESCENENODE, vm);

			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesFury.xml"));
			emitterStartFX_->SetViewMask(1);
			emitterStartFX_->SetEmitting(true);

			node_->AddChild(particleStartNode_);
			particleStartNode_->SetWorldPosition(node_->GetWorldPosition());
			particleStartNode_->SetScale(0.5f);

			particleNode_ = particleStartNode_;

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/93828__cgeffex__alien-voice8.ogg"));

		}
	}
}

void WarriorSkill1::Update(float timeStep)
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
			node_->GetComponent<Armor>()->ModifyArmor(1, 1, false);
			int attack = node_->GetVar("attack").GetInt();
			node_->SetVar("attack", attack - 50);
			float attackInterval = node_->GetVar("attackInterval").GetFloat();
			node_->SetVar("attackInterval", attackInterval * 1.5f);

			node_->RemoveChild(particleNode_);

			return;
		}
	}
}
