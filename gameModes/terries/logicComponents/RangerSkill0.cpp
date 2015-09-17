/*
 * RangerSkill0.cpp
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

#include "RangerSkill0.h"
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
#include "RangerSkill1.h"
#include "RangerSkill5.h"
#include "Armor.h"
#include "Dead.h"

RangerSkill0::RangerSkill0(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	touchSubscribed_ = false;
	attackElapsedTime_ = 0.25f;
	attackInterval_ = 0.25f;
	arrowSpeed_ = 1.0f;
	enabled_ = false;
}

RangerSkill0::~RangerSkill0()
{
}

void RangerSkill0::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill0", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(RangerSkill0, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesArrow.xml");
	arrow_ = node_->GetScene()->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

	arrow_->SetEnabled(false);
}

void RangerSkill0::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "ranger")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/ranger/S_Bow01.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill0")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "ranger")
		{
			if (enabled_)
			{
				node_->RemoveComponent<Stunned>();
				enabled_ = false;
				return;
			}

			if (!touchSubscribed_)
			{
				touchSubscribed_ = true;
				VariantMap vm;
				SendEvent(E_TOUCHSUBSCRIBE, vm);
			}

			SubscribeToEvent(E_TOUCHEND, HANDLER(RangerSkill0, HandleTouchEnd));
		}
	}
}

void RangerSkill0::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
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
		/*Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(node_->GetPosition());
		emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);*/

		//if (!node_->GetComponent<Blind>()->blinded_)
		{
			dest_ = raeResult_.position_;
			enabled_ = true;
			node_->AddComponent(new Stunned(context_, main_, -1.0f), 0, LOCAL);

			if (node_->GetComponent<RangerSkill1>()->enabled_)
			{
				node_->GetComponent<RangerSkill1>()->enabled_ = false;
			}

			if (node_->GetComponent<RangerSkill5>()->enabled_)
			{
				node_->GetComponent<RangerSkill5>()->enabled_ = false;
				node_->GetComponent<Armor>()->ModifyArmor(1, -1, false);
				node_->GetComponent<Speed>()->speed_ -= 3.0f;
				node_->RemoveChild(node_->GetComponent<RangerSkill5>()->particleNode_);
			}
		}
	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}


void RangerSkill0::Update(float timeStep)
{
	if (!enabled_)
	{
		return;
	}

	if (node_->HasComponent<Dead>())
	{
		return;
	}

	attackElapsedTime_ += timeStep;

	if (attackElapsedTime_ >= attackInterval_)
	{
		attackElapsedTime_ = 0.0f;

		Node* arrow = arrow_->Clone(LOCAL);

		arrow->SetEnabled(true);

		arrow->SetPosition(node_->GetPosition());

		arrow->LookAt(dest_);

		arrow->AddComponent(new KinematicMoveTo(context_, main_, dest_, arrowSpeed_), 0, LOCAL);

		SubscribeToEvent(arrow, E_NODECOLLISIONSTART, HANDLER(RangerSkill0, HandleNodeCollisionStart));

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

		if (arrow->GetPosition().x_ < dest_.x_)
		{
			vm[AnimateSceneNode::P_LAYER] = 1;
		}
		else
		{
			vm[AnimateSceneNode::P_LAYER] = -1;
		}

		SendEvent(E_ANIMATESCENENODE, vm);

		Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
		/*ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
		emitterStartFX_->SetViewMask(1);*/
		particleStartNode_->SetPosition(node_->GetPosition());
		//emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

		particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
		particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.1f);
		particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/shoot.ogg"));

	}
}

void RangerSkill0::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		noed->Remove();
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

		otherNode->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);

		noed->Remove();
	}
}
