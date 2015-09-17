/*
 * RogueSkill0.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: practicing01
 */
#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsDefs.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/Graphics/Material.h>
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
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "RogueSkill0.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "Armor.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "Speed.h"
#include "RogueSkill2.h"
#include "RogueSkill5.h"
#include "Dead.h"

RogueSkill0::RogueSkill0(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	/*cooldown_ = 0.0f;
	duration_ = 10.0f;
	cooling_ = false;
	processing_ = false;*/
	enabled_ = false;
}

RogueSkill0::~RogueSkill0()
{
}

void RogueSkill0::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill0", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(RogueSkill0, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void RogueSkill0::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "rogue")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/rogue/S_Shadow16.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill0")
	{
		//if (cooling_){return;}
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "rogue")
		{
			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/terriesInvisibility.xml"));
			emitterStartFX_->SetViewMask(1);
			emitterStartFX_->SetEmitting(true);

			node_->AddChild(particleStartNode_);
			particleStartNode_->SetWorldPosition(node_->GetWorldPosition());
			particleStartNode_->SetScale(0.5f);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 0.5f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/160756__cosmicembers__fast-swing-air-woosh.ogg"));

			if (!enabled_)
			{
				node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->SetShaderParameter(
						"MatDiffColor",Color(0.3f, 0.3f, 0.3f, 0.5f));
				node_->GetComponent<Armor>()->ModifyArmor(1, 1, false);
				node_->GetComponent<RigidBody>()->SetCollisionLayerAndMask(4,249);
				enabled_ = true;

				if (node_->GetComponent<RogueSkill2>()->enabled_)
				{
					node_->GetComponent<RogueSkill2>()->enabled_ = false;
				}

				if (node_->GetComponent<RogueSkill5>()->enabled_)
				{
					node_->GetComponent<RogueSkill5>()->enabled_ = false;
				}
			}
			else
			{
				node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->SetShaderParameter(
						"MatDiffColor",Color(0.3f, 0.3f, 0.3f, 1.0f));
				node_->GetComponent<Armor>()->ModifyArmor(1, -1, false);
				node_->GetComponent<RigidBody>()->SetCollisionLayerAndMask(2,-1);
				enabled_ = false;
			}

			//Variant v = node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->GetShaderParameter("MatDiffColor");
			//Color c = v.GetColor();LOGERRORF("%s",c.ToString().CString());
			//c.a_ = 0.5f;
			//node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->SetShaderParameter(
					//"MatDiffColor",Color(0.3f, 0.3f, 0.3f, 0.5f));

			/*elapsedTime_ = 0.0f;
			durationElapsedTime_ = 0.0f;
			processing_ = true;
			cooling_ = true;*/

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

void RogueSkill0::Update(float timeStep)
{
	/*if (cooling_)
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
			//Variant v = node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->GetShaderParameter("MatDiffColor");
			//Color c = v.GetColor();LOGERRORF("%s",c.ToString().CString());
			//c.a_ = 1.0f;
			node_->GetComponent<StaticSprite2D>()->GetCustomMaterial()->SetShaderParameter(
								"MatDiffColor",Color(0.3f, 0.3f, 0.3f, 1.0f));
			return;
		}
	}*/
}
