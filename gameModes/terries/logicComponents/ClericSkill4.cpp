/*
 * ClericSkill4.cpp
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

#include "ClericSkill4.h"
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

ClericSkill4::ClericSkill4(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	cooldown_ = 10.0f;
	cooling_ = false;
	processing_ = false;
	processingInterval_ = 1.0f;
	processingElapsedTime_ = 1.0f;
	processCount_ = 0;
	processMax_ = 10;
}

ClericSkill4::~ClericSkill4()
{
}

void ClericSkill4::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill4", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(ClericSkill4, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void ClericSkill4::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "cleric")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/cleric/S_Earth06.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill4")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (cooling_){return;}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "cleric")
		{
			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesEarthquake.xml"));
			emitterStartFX_->SetViewMask(1);
			particleStartNode_->SetPosition(node_->GetPosition());
			emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/60365__noise-generator__nanonoise-012.ogg"));

			processing_ = true;
			processingElapsedTime_ = 1.0f;
			processCount_ = 0;

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
		}
	}
}

void ClericSkill4::Update(float timeStep)
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
		processingElapsedTime_ += timeStep;

		if (processingElapsedTime_ >= processingInterval_)
		{
			processingElapsedTime_ = 0.0f;

			processCount_++;
			if (processCount_ >= processMax_)
			{
				processing_ = false;
				return;
			}

			/*SpriteSheetPlayer* ssp = main_->mySceneNode_->GetComponent<SpriteSheetPlayer>();

			for (int x = 0; x < ssp->sprites_.Size(); x++)
			{
				RigidBody* rb = ssp->sprites_[x]->noed_->GetComponent<RigidBody>();
				ssp->sprites_[x]->noed_->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
				rb->ApplyImpulse(Vector3::UP * 4.0f);
			}*/

			TerrySpawner* ts = node_->GetScene()->GetComponent<TerrySpawner>();

			for (int x = 0; x < ts->sprites_.Size(); x++)
			{
				ts->sprites_[x]->noed_->GetComponent<Health>()->ModifyHealth(10, -1, false);

				if (ts->sprites_[x]->noed_->GetComponent<Health>()->health_ <= 0)
				{
					ts->sprites_[x]->noed_->GetComponent<Health>()->ModifyHealth(100, 0, false);
					ts->RespawnTerry(ts->sprites_[x]->noed_);
					continue;
				}

				RigidBody* rb = ts->sprites_[x]->noed_->GetComponent<RigidBody>();
				ts->sprites_[x]->noed_->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
				rb->ApplyImpulse(Vector3::UP * 4.0f);
			}

			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesEarthquake.xml"));
			emitterStartFX_->SetViewMask(1);
			particleStartNode_->SetPosition(node_->GetPosition());
			emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/60365__noise-generator__nanonoise-012.ogg"));

		}
	}
}
