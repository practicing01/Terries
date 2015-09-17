/*
 * RangerSkill3.cpp
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

#include "RangerSkill3.h"
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
#include "Blind.h"
#include "MoveByTouch.h"
#include "Dead.h"

RangerSkill3::RangerSkill3(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
}

RangerSkill3::~RangerSkill3()
{
}

void RangerSkill3::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill3", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(RangerSkill3, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesMeat.xml");
	meat_ = node_->GetScene()->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

	meat_->SetVar("npcType",0);//0 = hero, 1 = terry

	meat_->SetEnabled(false);
	SubscribeToEvent(E_NODEREMOVED, HANDLER(RangerSkill3, HandleNodeRemoved));
}

void RangerSkill3::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "ranger")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/ranger/I_C_Meat.png"));
		skillSprite_->SetOpacity(1.0f);
	}

	if (name == "skill3")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "ranger")
		{
			Node* meat = meat_->Clone(LOCAL);

			meat->AddComponent(new Health(context_, main_), 0, LOCAL);
			meat->GetComponent<Health>()->killOn0_ = true;

			meat->SetEnabled(true);

			meat->SetPosition(node_->GetPosition() + (Vector3::BACK * 0.5f));

			SubscribeToEvent(meat, E_NODECOLLISIONSTART, HANDLER(RangerSkill3, HandleNodeCollisionStart));
			//SubscribeToEvent(meat, E_NODECOLLISION, HANDLER(RangerSkill3, HandleNodeCollisionStart));
			SubscribeToEvent(meat->GetChild("trigger"), E_NODECOLLISIONSTART, HANDLER(RangerSkill3, HandleNodeCollisionStart));
			//SubscribeToEvent(meat->GetChild("trigger"), E_NODECOLLISION, HANDLER(RangerSkill3, HandleNodeCollisionStart));

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
			/*ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
					emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
					emitterStartFX_->SetViewMask(1);*/
			particleStartNode_->SetPosition(node_->GetPosition());
			//emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.1f);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/265385__b-lamerichs__sound-effects-01-03-2015-8-pops-2.ogg"));

		}
	}
}

void RangerSkill3::Update(float timeStep)
{
	//
}

void RangerSkill3::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;
	//using namespace NodeCollision;

	bool trigger = eventData[P_TRIGGER].GetBool();

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		if (trigger)
		{
			if (otherNode->HasComponent<MoveByTouch>())
			{
				MoveByTouch* mbt = otherNode->GetComponent<MoveByTouch>();
				mbt->MoveTo(noed->GetWorldPosition(),
						mbt->moveToSpeed_,mbt->speedRamp_,mbt->gravity_,mbt->gravityRamp_,false,true,true);
			}
			return;
		}

		/*otherNode->GetComponent<Health>()->ModifyHealth(10, -1, false);

		if (otherNode->GetComponent<Health>()->health_ <= 0)
		{
			otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
			otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
		}*/
	}
}

void RangerSkill3::HandleNodeRemoved(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeRemoved;

	Node* nohd = static_cast<Node*>(eventData[P_NODE].GetPtr());

	if (nohd->GetName() != "meat")
	{
		return;
	}

	if (!nohd->GetScene())
	{
		return;
	}

	PODVector<RigidBody*> rigidBodies;

	nohd->GetScene()->GetComponent<PhysicsWorld>()->GetRigidBodies(rigidBodies, Sphere(nohd->GetPosition(), 5.0f), 2);

	if (rigidBodies.Size())
	{
		for (int x = 0; x < rigidBodies.Size(); x++)
		{
			Node* noed = rigidBodies[x]->GetNode();

			if (!noed->HasComponent<Health>())
			{
				continue;
			}

			if (!noed)
			{
				continue;
			}

			if (noed != nohd)
			{
				if (noed->GetVar("npcType").GetInt() == 1)//terry
				{
					TerrySpawner* ts = noed->GetScene()->GetComponent<TerrySpawner>();

					if (!ts)
					{
						continue;
					}

					MoveByTouch* mbt = noed->GetComponent<MoveByTouch>();

					if (!mbt)
					{
						continue;
					}

					mbt->MoveTo(ts->tent_->GetPosition(),
							mbt->moveToSpeed_,mbt->speedRamp_,mbt->gravity_,mbt->gravityRamp_,false,true,true);
				}
			}
		}
	}
}
