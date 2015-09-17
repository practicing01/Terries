/*
 * WizardSkill4.cpp
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

#include "WizardSkill4.h"
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

WizardSkill4::WizardSkill4(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	touchSubscribed_ = false;
	cooldown_ = 0.0f;
	cooling_ = false;
}

WizardSkill4::~WizardSkill4()
{
}

void WizardSkill4::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill4", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(WizardSkill4, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesOrb.xml");
	orb_ = node_->GetScene()->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

	orb_->SetPosition(node_->GetScene()->GetChild("tent")->GetPosition() + (Vector3::UP * 2.0f));

	SubscribeToEvent(orb_, E_NODECOLLISIONSTART, HANDLER(WizardSkill4, HandleNodeCollisionStart));

	//orb_->SetEnabled(false);
}

void WizardSkill4::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "wizard")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/wizard/S_Magic01.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill4")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "wizard")
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

			SubscribeToEvent(E_TOUCHEND, HANDLER(WizardSkill4, HandleTouchEnd));
		}
	}
}

void WizardSkill4::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
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
		dest_ = raeResult_.position_;

		orb_->GetComponent<RigidBody>()->ResetForces();

		orb_->SetPosition(dest_ + (Vector3::UP));

		/*Node* orb = orb_->Clone(LOCAL);

		orb->SetEnabled(true);

		orb->SetPosition(dest_ + (Vector3::UP));

		SubscribeToEvent(orb, E_NODECOLLISIONSTART, HANDLER(WizardSkill4, HandleNodeCollisionStart));

		orb->AddComponent(new TimedRemove(context_, main_, 10.0f), 0, LOCAL);*/

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

		if (orb_->GetPosition().x_ < dest_.x_)
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

		Node* particleStartNode_ = orb_->GetScene()->CreateChild(0,LOCAL);
		particleStartNode_->SetPosition(orb_->GetPosition());

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

		particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
		particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.5f);
		particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/31128__calethos__super-lead-cut-17.ogg"));

	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}

void WizardSkill4::Update(float timeStep)
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

void WizardSkill4::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));
	RigidBody* otherRB = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		/*Node* particleStartNode_ = noed->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(noed->GetPosition() + (Vector3::UP));
		emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);*/

		/*Node* lightNode = noed->GetScene()->CreateChild(0,LOCAL);
		lightNode->SetPosition(noed->GetPosition() + (Vector3::UP));
		Light* light = lightNode->CreateComponent<Light>(LOCAL);
		light->SetBrightness(2.0f);
		light->SetPerVertex(true);
		light->SetRange(5.0f);
		lightNode->AddComponent(new TimedRemove(context_, main_, 0.1f), 0, LOCAL);*/

		//noed->Remove();
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);

		/*Vector3 victoria = otherNode->GetPosition();
		Vector3 dir = (victoria - noed->GetPosition()).Normalized();
		dir.y_ = 1.0f;
		dir *= 5.0f;

		otherNode->AddComponent(new Stunned(context_, main_, 4.0f), 0, LOCAL);

		otherRB->ApplyImpulse(dir);*/

		/*Node* particleStartNode_ = noed->GetScene()->CreateChild(0,LOCAL);
		ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
		emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
		emitterStartFX_->SetViewMask(1);
		particleStartNode_->SetPosition(noed->GetPosition() + (Vector3::UP));
		emitterStartFX_->SetEmitting(true);

		particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);*/

		/*Node* lightNode = noed->GetScene()->CreateChild(0,LOCAL);
		lightNode->SetPosition(noed->GetPosition() + (Vector3::UP));
		Light* light = lightNode->CreateComponent<Light>(LOCAL);
		light->SetBrightness(2.0f);
		light->SetPerVertex(true);
		light->SetRange(5.0f);
		lightNode->AddComponent(new TimedRemove(context_, main_, 0.1f), 0, LOCAL);*/

		//noed->Remove();
	}
}
