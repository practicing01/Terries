/*
 * WarriorSkill5.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: practicing01
 */
#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
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

#include "WarriorSkill5.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "HUD.h"
#include "SpriteSheetPlayer.h"
#include "Stunned.h"
#include "TimedRemove.h"
#include "Health.h"
#include "TerrySpawner.h"
#include "Dead.h"

WarriorSkill5::WarriorSkill5(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	cooldown_ = 1.0f;
	radius_ = 1.0f;
	cooling_ = false;
	processing_ = false;
}

WarriorSkill5::~WarriorSkill5()
{
}

void WarriorSkill5::Start()
{
	skillSprite_ = (Sprite*)(main_->mySceneNode_->GetComponent<HUD>()->
			hud_->GetChild("skill5", false)->GetChild(0)->GetChild(0));
	SubscribeToEvent(E_HUDBUTT, HANDLER(WarriorSkill5, HandleHudButt));
	SetUpdateEventMask(USE_UPDATE);
}

void WarriorSkill5::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "warrior")
	{
		skillSprite_->SetTexture(main_->cache_->
				GetResource<Texture2D>("Textures/terriesHud/icons/warrior/W_Gold_Mace.png"));
		skillSprite_->SetOpacity(1.0f);
		return;
	}

	if (name == "skill5")
	{
		if (node_->HasComponent<Dead>())
		{
			return;
		}

		if (cooling_){return;}

		if (main_->mySceneNode_->GetComponent<SpriteSheetPlayer>()->focusedSprite_ == "warrior")
		{

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
			vm[AnimateSceneNode::P_LAYER] = 0;
			SendEvent(E_ANIMATESCENENODE, vm);


			Node* particleStartNode_ = node_->GetScene()->CreateChild(0,LOCAL);
			ParticleEmitter* emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
			emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dustCloud.xml"));
			emitterStartFX_->SetViewMask(1);
			particleStartNode_->SetPosition(node_->GetPosition());
			emitterStartFX_->SetEmitting(true);

			particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

			particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
			particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/273473__n-audioman__randomize23.ogg"));

			node_->GetScene()->GetComponent<PhysicsWorld>()->GetRigidBodies(rigidBodies_, Sphere(node_->GetPosition(), radius_ * node_->GetScale().y_), 2);

			//if (!node_->GetComponent<Blind>()->blinded_)
			{
				if (rigidBodies_.Size())
				{
					for (int x = 0; x < rigidBodies_.Size(); x++)
					{
						Node* noed = rigidBodies_[x]->GetNode();

						if (noed != node_)
						{
							if (noed->GetVar("npcType").GetInt() == 1)//terry
							{
								if (noed->HasComponent<Health>())
								{
									noed->GetComponent<Health>()->ModifyHealth(10, -1, false);

									if (noed->GetComponent<Health>()->health_ <= 0)
									{
										noed->GetComponent<Health>()->ModifyHealth(100, 0, false);
										noed->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(noed);
										continue;
									}
								}

								Vector3 victoria = noed->GetPosition();
								Vector3 dir = (victoria - node_->GetPosition()).Normalized();
								dir.y_ = 1.0f;
								dir *= 5.0f;

								noed->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);

								rigidBodies_[x]->ApplyImpulse(dir);
							}
						}
					}
				}

				elapsedTime_ = 0.0f;
				processing_ = true;
				cooling_ = true;
			}
		}
	}
}

void WarriorSkill5::Update(float timeStep)
{
	if (processing_)
	{
		elapsedTime_ += timeStep;
		if (elapsedTime_ >= cooldown_)
		{
			cooling_ = false;
			processing_ = false;
		}
	}
}
