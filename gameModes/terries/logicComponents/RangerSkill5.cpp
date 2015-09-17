/*
 * RangerSkill5.cpp
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

#include "RangerSkill5.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "TerrySpawner.h"
#include "Speed.h"
#include "KinematicMoveTo.h"
#include "RangerSkill0.h"
#include "RangerSkill1.h"
#include "Armor.h"
#include "Dead.h"

RangerSkill5::RangerSkill5(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	enabled_ = false;
}

RangerSkill5::~RangerSkill5()
{
}

void RangerSkill5::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill5", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(RangerSkill5, HandleHudButt));
}

void RangerSkill5::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "ranger")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/ranger/S_Sword03.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill5")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "ranger")
		{
			if (enabled_)
			{
				enabled_ = false;
				node_->GetComponent<Armor>()->ModifyArmor(1, -1, false);
				node_->GetComponent<Speed>()->speed_ -= 3.0f;

				node_->RemoveChild(particleNode_);
				return;
			}

			enabled_ = true;

			if (node_->GetComponent<RangerSkill1>()->enabled_)
			{
				node_->GetComponent<RangerSkill1>()->enabled_ = false;
			}

			if (node_->GetComponent<RangerSkill0>()->enabled_)
			{
				node_->GetComponent<RangerSkill0>()->enabled_ = false;
				node_->RemoveComponent<Stunned>();
			}

			node_->GetComponent<Armor>()->ModifyArmor(1, 1, false);
			node_->GetComponent<Speed>()->speed_ += 3.0f;

			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesWhirlwind.xml"));
			emitterStartFX_->SetViewMask(1);
			emitterStartFX_->SetEmitting(true);

			node_->AddChild(particleStartNode_);
			particleStartNode_->SetWorldPosition(node_->GetWorldPosition());
			particleStartNode_->SetScale(0.5f);

			particleNode_ = particleStartNode_;

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.5f);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/106125__j1987__clickselect3.ogg"));

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
		}
	}
}
