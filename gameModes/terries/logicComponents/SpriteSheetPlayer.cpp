/*
 * SpriteSheetPlayer.cpp
 *
 *  Created on: Aug 8, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "SpriteSheetPlayer.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "MoveByTouch.h"
#include "Speed.h"
#include "Gravity.h"
#include "RotateTo.h"
#include "WarriorSkill0.h"
#include "WarriorSkill1.h"
#include "WarriorSkill2.h"
#include "WarriorSkill3.h"
#include "WarriorSkill4.h"
#include "WarriorSkill5.h"
#include "RangerSkill0.h"
#include "RangerSkill1.h"
#include "RangerSkill2.h"
#include "RangerSkill3.h"
#include "RangerSkill4.h"
#include "RangerSkill5.h"
#include "WizardSkill0.h"
#include "WizardSkill1.h"
#include "WizardSkill2.h"
#include "WizardSkill3.h"
#include "WizardSkill4.h"
#include "WizardSkill5.h"
#include "ClericSkill0.h"
#include "ClericSkill1.h"
#include "ClericSkill2.h"
#include "ClericSkill3.h"
#include "ClericSkill4.h"
#include "ClericSkill5.h"
#include "RogueSkill0.h"
#include "RogueSkill1.h"
#include "RogueSkill2.h"
#include "RogueSkill3.h"
#include "RogueSkill4.h"
#include "RogueSkill5.h"
#include "Health.h"
#include "Armor.h"
#include "TerrySpawner.h"
#include "Dead.h"

SpriteSheetPlayer::SpriteSheetPlayer(Context* context, Urho3DPlayer* main) :
	LogicComponent(context)
{
	main_ = main;
	spriteIDCount_ = 0;
	focusedSprite_ = NULL;
}

SpriteSheetPlayer::~SpriteSheetPlayer()
{
	RemoveModelNode();
}

void SpriteSheetPlayer::Start()
{
	modelNode_ = NULL;
	scene_ = node_->GetScene();
	LoadDefaultPlayer();
	//RecursiveSetAnimation(modelNode_, "idle1", true, 0);

	SetUpdateEventMask(USE_UPDATE);
	SubscribeToEvent(E_GETCLIENTCAMERA, HANDLER(SpriteSheetPlayer, HandleGetCamera));
	SubscribeToEvent(E_GETCLIENTMODELNODE, HANDLER(SpriteSheetPlayer, HandleGetClientModelNode));
	SubscribeToEvent(E_ANIMATESCENENODE, HANDLER(SpriteSheetPlayer, HandleAnimateSceneNode));
	SubscribeToEvent(E_GETSCENENODEBYMODELNODE, HANDLER(SpriteSheetPlayer, HandleGetSceneNodeByModelNode));
	SubscribeToEvent(E_GETMODELNODEBYSCENENODE, HANDLER(SpriteSheetPlayer, HandleGetModelNodeBySceneNode));
}

void SpriteSheetPlayer::RecursiveSetAnimation(Node* noed, String ani, bool loop, unsigned char layer)//todo check if animation exists, if not set default.
{
	if (noed->HasComponent<AnimationController>())
	{
		noed->GetComponent<AnimationController>()->PlayExclusive("Models/Witch3/" + ani + ".ani", layer, loop, 0.25f);//reduce archive size by reusing animations
		//noed->GetComponent<AnimationController>()->PlayExclusive("Models/" + noed->GetName() + "/" + ani + ".ani", layer, loop, 0.25f);//proper way
		if (!loop)
		{
			noed->GetComponent<AnimationController>()->SetAutoFade(ani, 0.25f);
		}
	}

	for (int x = 0; x < noed->GetNumChildren(); x++)
	{
		RecursiveSetAnimation(noed->GetChild(x), ani, loop, layer);
	}
}

void SpriteSheetPlayer::LoadDefaultPlayer()
{
	LoadPlayer("");
}

void SpriteSheetPlayer::LoadPlayer(String modelFilename)
{
	RemoveModelNode();

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesNode.xml");
	modelNode_ = scene_->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

    modelNode_->SetPosition(main_->mySceneNode_->GetPosition());
    modelNode_->SetRotation(main_->mySceneNode_->GetRotation());
    //modelNode_->GetComponent<RigidBody>()->SetTrigger(true);
    //modelNode_->GetComponent<RigidBody>()->SetUseGravity(false);
    modelNode_->GetComponent<RigidBody>()->SetEnabled(false);

	main_->viewport_->SetCamera(modelNode_->GetChild("camera")->GetComponent<Camera>());

	main_->audio_->SetListener(modelNode_->GetComponent<SoundListener>());

	SubscribeToEvent(E_RESPAWNSCENENODE, HANDLER(SpriteSheetPlayer, HandleRespawnSceneNode));
	SubscribeToEvent(E_ANIMATESPRITESHEET, HANDLER(SpriteSheetPlayer, HandleAnimateSpriteSheet));

	LoadSprite("cleric");
	LoadSprite("ranger");
	LoadSprite("warrior");
	LoadSprite("wizard");
	LoadSprite("rogue");

	SubscribeToEvent(E_HUDBUTT, HANDLER(SpriteSheetPlayer, HandleHudButt));
}

void SpriteSheetPlayer::LoadSprite(String name)
{
	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesNode.xml");
	Node* spriteNode = scene_->InstantiateXML(xmlFile->GetRoot(),
			Vector3::ZERO, Quaternion::IDENTITY, LOCAL);

	spriteNode->RemoveChild(spriteNode->GetChild("camera"));

	spriteNode->SetName(name);
	spriteNode->AddComponent(new Gravity(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new Speed(context_, main_), 0, LOCAL);
	//spriteNode->AddComponent(new RotateTo(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new MoveByTouch(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new Health(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new Armor(context_, main_), 0, LOCAL);

	if (name == "warrior")
	{
		spriteNode->AddComponent(new WarriorSkill0(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WarriorSkill1(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WarriorSkill2(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WarriorSkill3(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WarriorSkill4(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WarriorSkill5(context_, main_), 0, LOCAL);
	}
	else if (name == "ranger")
	{
		spriteNode->AddComponent(new RangerSkill0(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RangerSkill1(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RangerSkill2(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RangerSkill3(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RangerSkill4(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RangerSkill5(context_, main_), 0, LOCAL);
	}
	else if (name == "wizard")
	{
		spriteNode->AddComponent(new WizardSkill0(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WizardSkill1(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WizardSkill2(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WizardSkill3(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WizardSkill4(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new WizardSkill5(context_, main_), 0, LOCAL);
	}
	else if (name == "cleric")
	{
		spriteNode->AddComponent(new ClericSkill0(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new ClericSkill1(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new ClericSkill2(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new ClericSkill3(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new ClericSkill4(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new ClericSkill5(context_, main_), 0, LOCAL);
	}
	else if (name == "rogue")
	{
		spriteNode->AddComponent(new RogueSkill0(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RogueSkill1(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RogueSkill2(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RogueSkill3(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RogueSkill4(context_, main_), 0, LOCAL);
		spriteNode->AddComponent(new RogueSkill5(context_, main_), 0, LOCAL);
	}

	StaticSprite2D* sprite = spriteNode->CreateComponent<StaticSprite2D>();

	AnimatedSpriteSheet* animatedSpriteSheet = new AnimatedSpriteSheet();
	animatedSpriteSheet->sheet_ = main_->cache_->GetResource<SpriteSheet2D>("Urho2D/" + name + "/" + name + "Sheet.xml");
	animatedSpriteSheet->staticSprite_ = sprite;
	animatedSpriteSheet->playing_ = false;
	animatedSpriteSheet->spriteID_ = spriteIDCount_;
	animatedSpriteSheet->noed_ = spriteNode;
	animatedSpriteSheet->flipX_ = false;
	animatedSpriteSheet->customMat_ = SharedPtr<Material>(main_->cache_->GetResource<Material>("Materials/" + name + "Mat.xml"));

	sprite->SetCustomMaterial(animatedSpriteSheet->customMat_);

	sprites_.Push(animatedSpriteSheet);

	spriteIDCount_++;

	Vector<String> files;
	files.Push("attackF.xml");
	files.Push("attackM.xml");
	files.Push("dieF.xml");
	files.Push("dieM.xml");
	files.Push("gestureF.xml");
	files.Push("gestureM.xml");
	files.Push("idleF.xml");
	files.Push("idleM.xml");
	files.Push("runF.xml");
	files.Push("runM.xml");
	files.Push("attackLoopF.xml");
	files.Push("attackLoopM.xml");

	/*main_->filesystem_->ScanDir(files,
			main_->filesystem_->GetProgramDir() + "Data/Urho2D/" + name + "/animations/",
			"*", SCAN_FILES, true);*/

	for (int x = 0; x < files.Size(); x++)
	{
		XMLElement ani = main_->cache_->GetResource<XMLFile>("Urho2D/" + name + "/animations/" + files[x])->GetRoot();

		SpriteSheetAnimation* spriteSheetAni = new SpriteSheetAnimation();
		animatedSpriteSheet->animations_.Push(spriteSheetAni);

		spriteSheetAni->name_ = ani.GetChild("Name").GetAttribute("name");
		spriteSheetAni->loop_ = ani.GetChild("Loop").GetBool("loop");

		int frameCount = ani.GetChild("FrameCount").GetInt("frameCount");

		for (int x = 0; x < frameCount; x++)
		{
			SpriteSheetAnimationFrame* frame = new SpriteSheetAnimationFrame();
			spriteSheetAni->frames_.Push(frame);

			String child = "Frame" + String(x);

			frame->duration_ = ani.GetChild(child).GetFloat("duration");
			frame->sprite_ = ani.GetChild(child).GetAttribute("sprite");
		}
	}

	bool sex = Random(0,2);
	animatedSpriteSheet->noed_->SetVar("sex",sex);

	VariantMap vm;
	vm[AnimateSpriteSheet::P_NODE] = node_;
	vm[AnimateSpriteSheet::P_SPRITEID] = animatedSpriteSheet->spriteID_;

	if (sex)
	{
		vm[AnimateSpriteSheet::P_ANIMATION] = "idleF";
	}
	else
	{
		vm[AnimateSpriteSheet::P_ANIMATION] = "idleM";
	}

	vm[AnimateSpriteSheet::P_FLIPX] = 0;
	SendEvent(E_ANIMATESPRITESHEET, vm);

	animatedSpriteSheet->noed_->SetVar("collisionCount",0);

	animatedSpriteSheet->noed_->SetVar("attack",1);

	if (name == "warrior")
	{
		animatedSpriteSheet->noed_->SetVar("attack",50);
	}

	animatedSpriteSheet->noed_->SetVar("attackInterval",1.0f);
	animatedSpriteSheet->noed_->SetVar("attackElapsedTime",0.0f);
	animatedSpriteSheet->noed_->SetVar("canAttack",false);

	animatedSpriteSheet->noed_->SetVar("npcType",0);//0 = hero, 1 = terry

	SubscribeToEvent(spriteNode, E_NODECOLLISIONSTART, HANDLER(SpriteSheetPlayer, HandleNodeCollisionStart));
	SubscribeToEvent(spriteNode, E_NODECOLLISIONEND, HANDLER(SpriteSheetPlayer, HandleNodeCollisionEnd));
	SubscribeToEvent(spriteNode, E_NODECOLLISION, HANDLER(SpriteSheetPlayer, HandleNodeCollision));
}

void SpriteSheetPlayer::RemoveModelNode()
{
	for (int x = 0; x < sprites_.Size(); x++)
	{
		for (int y = 0; y < sprites_[x]->animations_.Size(); y++)
		{
			for (int z = 0; z < sprites_[x]->animations_[y]->frames_.Size(); z++)
			{
				delete sprites_[x]->animations_[y]->frames_[z];
			}

			delete sprites_[x]->animations_[y];
		}

		sprites_[x]->noed_->RemoveAllChildren();
		sprites_[x]->noed_->RemoveAllComponents();
		sprites_[x]->noed_->Remove();

		delete sprites_[x];
	}
	sprites_.Clear();

	if (modelNode_ != NULL)
	{
		modelNode_->RemoveAllChildren();
		modelNode_->RemoveAllComponents();
		modelNode_->Remove();
	}
}

void SpriteSheetPlayer::HandleGetCamera(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[GetClientCamera::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		VariantMap vm;
		vm[SetClientCamera::P_NODE] = clientNode;
		vm[SetClientCamera::P_CAMERANODE] = modelNode_->GetChild("camera", true);
		SendEvent(E_SETCLIENTCAMERA, vm);
	}
}

void SpriteSheetPlayer::HandleGetClientModelNode(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[GetClientModelNode::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		VariantMap vm;
		vm[SetClientModelNode::P_NODE] = clientNode;
		vm[SetClientModelNode::P_MODELNODE] = modelNode_;
		SendEvent(E_SETCLIENTMODELNODE, vm);
	}
}

void SpriteSheetPlayer::HandleRespawnSceneNode(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[RespawnSceneNode::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		Node* spawnNode = (Node*)(eventData[RespawnSceneNode::P_SPAWNNODE].GetPtr());

		modelNode_->SetPosition(spawnNode->GetPosition());
		modelNode_->SetRotation(spawnNode->GetRotation());

		for (int x = 0; x < sprites_.Size(); x++)
		{
			Node* sprite = sprites_[x]->noed_;
			Node* spawnChild = spawnNode->GetChild(sprite->GetName());
			if (spawnChild)
			{
				sprite->SetWorldPosition(spawnChild->GetWorldPosition());
				sprite->SetWorldRotation(spawnChild->GetWorldRotation());
			}
		}
	}
}

void SpriteSheetPlayer::HandleAnimateSceneNode(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[AnimateSceneNode::P_NODE].GetPtr());

	String ani = eventData[AnimateSceneNode::P_ANIMATION].GetString();
	bool loop = eventData[AnimateSceneNode::P_LOOP].GetBool();
	int layer = eventData[AnimateSceneNode::P_LAYER].GetInt();

	for (int x = 0; x < sprites_.Size(); x++)
	{
		if (sprites_[x]->noed_ == clientNode)
		{
			VariantMap vm;
			vm[AnimateSpriteSheet::P_NODE] = node_;
			vm[AnimateSpriteSheet::P_SPRITEID] = sprites_[x]->spriteID_;
			vm[AnimateSpriteSheet::P_ANIMATION] = ani;
			vm[AnimateSpriteSheet::P_FLIPX] = layer;//too many haxolutions >:0
			SendEvent(E_ANIMATESPRITESHEET, vm);
			break;
		}
	}

	return;
	if (clientNode == node_)
	{
		String ani = eventData[AnimateSceneNode::P_ANIMATION].GetString();
		bool loop = eventData[AnimateSceneNode::P_LOOP].GetBool();
		unsigned char layer = (unsigned char)(eventData[AnimateSceneNode::P_LAYER].GetUInt());
		RecursiveSetAnimation(modelNode_, ani, loop, layer);
	}
}

void SpriteSheetPlayer::HandleGetSceneNodeByModelNode(StringHash eventType, VariantMap& eventData)
{
	Node* modelNode = (Node*)(eventData[GetSceneNodeByModelNode::P_NODE].GetPtr());

	if (modelNode == modelNode_)
	{
		VariantMap vm;
		vm[SetSceneNodeByModelNode::P_MODELNODE] = modelNode;
		vm[SetSceneNodeByModelNode::P_SCENENODE] = node_;
		SendEvent(E_SETSCENENODEBYMODELNODE, vm);
	}
}

void SpriteSheetPlayer::HandleGetModelNodeBySceneNode(StringHash eventType, VariantMap& eventData)
{
	Node* sceneNode = (Node*)(eventData[GetModelNodeBySceneNode::P_NODE].GetPtr());

	if (sceneNode == node_)
	{
		VariantMap vm;
		vm[SetModelNodeBySceneNode::P_SCENENODE] = sceneNode;
		vm[SetModelNodeBySceneNode::P_MODELNODE] = modelNode_;
		SendEvent(E_SETMODELNODEBYSCENENODE, vm);
	}
}

void SpriteSheetPlayer::HandleAnimateSpriteSheet(StringHash eventType, VariantMap& eventData)
{
	Node* sceneNode = (Node*)(eventData[GetModelNodeBySceneNode::P_NODE].GetPtr());

	if (sceneNode == node_)
	{
		int spriteID = eventData[AnimateSpriteSheet::P_SPRITEID].GetInt();
		String ani = eventData[AnimateSpriteSheet::P_ANIMATION].GetString();
		int flipX = eventData[AnimateSpriteSheet::P_FLIPX].GetInt();

		for (int x = 0; x < sprites_.Size(); x++)
		{
			if (sprites_[x]->spriteID_ == spriteID)
			{
				for (int y = 0; y < sprites_[x]->animations_.Size(); y++)
				{
					if (sprites_[x]->animations_[y]->name_ == ani)
					{
						sprites_[x]->animation_ = sprites_[x]->animations_[y];
						sprites_[x]->elapsedTime_ = 0.0f;
						sprites_[x]->curFrame_ = 0;
						sprites_[x]->staticSprite_->SetSprite(
								sprites_[x]->sheet_->GetSprite(
										sprites_[x]->animations_[y]->frames_[0]->sprite_));
						sprites_[x]->playing_ = true;

						if (flipX == -1)
						{
							sprites_[x]->staticSprite_->SetFlipX(true);
						}
						else if (flipX == 1)
						{
							sprites_[x]->staticSprite_->SetFlipX(false);
						}
						break;
					}
				}
			}
		}
	}
}

void SpriteSheetPlayer::Update(float timeStep)
{
	for (int x = 0; x < sprites_.Size(); x++)
	{
		float attackElapsedTime = sprites_[x]->noed_->GetVar("attackElapsedTime").GetFloat();
		float attackInterval = sprites_[x]->noed_->GetVar("attackInterval").GetFloat();
		bool canAttack = sprites_[x]->noed_->GetVar("canAttack").GetBool();

		attackElapsedTime += timeStep;

		if (attackElapsedTime >= attackInterval)
		{
			attackElapsedTime = 0.0f;
			canAttack = true;
		}

		sprites_[x]->noed_->SetVar("attackElapsedTime", attackElapsedTime);
		sprites_[x]->noed_->SetVar("canAttack", canAttack);

		if (sprites_[x]->playing_)
		{
			sprites_[x]->elapsedTime_ += timeStep;

			if (sprites_[x]->elapsedTime_ >=
					sprites_[x]->animation_->frames_[sprites_[x]->curFrame_]->duration_)
			{
				sprites_[x]->curFrame_++;

				if (sprites_[x]->curFrame_ >= sprites_[x]->animation_->frames_.Size())
				{
					sprites_[x]->curFrame_ = 0;

					if (!sprites_[x]->animation_->loop_)
					{
						sprites_[x]->playing_ = false;

						if (sprites_[x]->noed_->HasComponent<Dead>())
						{
							continue;
						}

						if (sprites_[x]->noed_->GetComponent<MoveByTouch>()->isMoving_)
						{
							VariantMap vm;
							vm[AnimateSpriteSheet::P_NODE] = node_;
							vm[AnimateSpriteSheet::P_SPRITEID] = sprites_[x]->spriteID_;

							if (sprites_[x]->noed_->GetVar("sex").GetBool())
							{
								vm[AnimateSceneNode::P_ANIMATION] = "runF";
							}
							else
							{
								vm[AnimateSceneNode::P_ANIMATION] = "runM";
							}

							vm[AnimateSpriteSheet::P_FLIPX] = 0;
							SendEvent(E_ANIMATESPRITESHEET, vm);
						}
						else
						{
							VariantMap vm;
							vm[AnimateSpriteSheet::P_NODE] = node_;
							vm[AnimateSpriteSheet::P_SPRITEID] = sprites_[x]->spriteID_;

							if (sprites_[x]->noed_->GetVar("sex").GetBool())
							{
								vm[AnimateSceneNode::P_ANIMATION] = "idleF";
							}
							else
							{
								vm[AnimateSceneNode::P_ANIMATION] = "idleM";
							}

							vm[AnimateSpriteSheet::P_FLIPX] = 0;
							SendEvent(E_ANIMATESPRITESHEET, vm);
						}
					}
				}

				sprites_[x]->staticSprite_->SetSprite(
						sprites_[x]->sheet_->GetSprite(
								sprites_[x]->animation_->frames_[sprites_[x]->curFrame_]->sprite_));

				sprites_[x]->elapsedTime_ = 0.0f;
			}
		}
	}
}

void SpriteSheetPlayer::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
	Node* node = (Node*)(eventData[HudButt::P_NODE].GetPtr());

	if (node != node_)
	{
		return;
	}

	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	bool camFocus = false;

	if (focusedSprite_ == name)
	{
		camFocus = true;
	}
	else
	{
		if (name == "warrior" || name == "cleric" || name == "wizard" || name == "ranger" || name == "rogue")
		{
			focusedSprite_ = name;
		}
	}

	if (camFocus)
	{
		for (int x = 0; x < sprites_.Size(); x++)
		{
			if (sprites_[x]->noed_->GetName() == focusedSprite_)
			{
				Vector3 pos = modelNode_->GetPosition();
				Vector3 des = sprites_[x]->noed_->GetPosition();
				des.z_ -= 1.0f;
				des.y_ = pos.y_;
				modelNode_->SetPosition(des);
				break;
			}
		}
	}
}

void SpriteSheetPlayer::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));
	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		int collisionCount = noed->GetVar("collisionCount").GetInt();
		collisionCount++;
		noed->SetVar("collisionCount", collisionCount);

		if (noed->HasComponent<Dead>())
		{
			return;
		}

		if (collisionCount == 1)
		{
			VariantMap vm;
			vm[AnimateSceneNode::P_NODE] = noed;

			if (noed->GetVar("sex").GetBool())
			{
				vm[AnimateSceneNode::P_ANIMATION] = "attackLoopF";
			}
			else
			{
				vm[AnimateSceneNode::P_ANIMATION] = "attackLoopM";
			}

			vm[AnimateSceneNode::P_LOOP] = false;
			vm[AnimateSceneNode::P_LAYER] = 0;
			SendEvent(E_ANIMATESCENENODE, vm);
		}

		int healthMod = noed->GetVar("attack").GetInt();

		if (otherNode->HasComponent<Armor>())
		{
			healthMod = noed->GetVar("attack").GetInt() - otherNode->GetComponent<Armor>()->armor_;

			if (healthMod < 0)
			{
				healthMod = 0;
			}
		}

		otherNode->GetComponent<Health>()->ModifyHealth(healthMod, -1, false);

		if (otherNode->GetComponent<Health>()->health_ <= 0)
		{
			otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
			otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
			return;
		}

		/*if (!noed->HasComponent<Stunned>())
		{
			noed->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
		}*/
	}
}

void SpriteSheetPlayer::HandleNodeCollisionEnd(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionEnd;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		int collisionCount = noed->GetVar("collisionCount").GetInt();
		collisionCount--;
		noed->SetVar("collisionCount", collisionCount);

		if (noed->HasComponent<Dead>())
		{
			return;
		}

		if (collisionCount == 0)
		{
			VariantMap vm;
			vm[AnimateSceneNode::P_NODE] = noed;

			if (noed->GetVar("sex").GetBool())
			{
				vm[AnimateSceneNode::P_ANIMATION] = "idleF";
			}
			else
			{
				vm[AnimateSceneNode::P_ANIMATION] = "idleM";
			}

			vm[AnimateSceneNode::P_LOOP] = false;
			vm[AnimateSceneNode::P_LAYER] = 0;
			SendEvent(E_ANIMATESCENENODE, vm);
		}
	}
}

void SpriteSheetPlayer::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollision;

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	if (!noed->GetVar("canAttack").GetBool())
	{
		return;
	}

	noed->SetVar("canAttack", false);

	if (noed->HasComponent<Dead>())
	{
		return;
	}

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 1)//terry
	{
		int healthMod = noed->GetVar("attack").GetInt();

		if (otherNode->HasComponent<Armor>())
		{
			healthMod = noed->GetVar("attack").GetInt() - otherNode->GetComponent<Armor>()->armor_;

			if (healthMod < 0)
			{
				healthMod = 0;
			}
		}

		otherNode->GetComponent<Health>()->ModifyHealth(healthMod, -1, false);

		if (otherNode->GetComponent<Health>()->health_ <= 0)
		{
			otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
			otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
			return;
		}

		/*if (!noed->HasComponent<Stunned>())
		{
			noed->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
		}*/
	}
}
