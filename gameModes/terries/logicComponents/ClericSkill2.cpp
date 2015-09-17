/*
 * ClericSkill2.cpp
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

#include "ClericSkill2.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "Armor.h"
#include "SpriteSheetPlayer.h"
#include "TerrySpawner.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "Speed.h"
#include "Dead.h"

ClericSkill2::ClericSkill2(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	cooldown_ = 10.0f;
	cooling_ = false;
}

ClericSkill2::~ClericSkill2()
{
}

void ClericSkill2::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill2", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(ClericSkill2, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void ClericSkill2::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "cleric")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/cleric/S_Holy10.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill2")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (cooling_){return;}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "cleric")
		{
			TerrySpawner* ts = node_->GetScene()->GetComponent<TerrySpawner>();

			ts->tent_->GetComponent<Health>()->ModifyHealth(100, 0, false);

			elapsedTime_ = 0.0f;
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
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesTentHeal.xml"));
			emitterStartFX_->SetViewMask(1);
			particleStartNode_->SetPosition(ts->tent_->GetPosition() + Vector3::UP);
			emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 4.0f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/248050__nicolasdrweski__arpege-haut.ogg"));

		}
	}
}

void ClericSkill2::Update(float timeStep)
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
