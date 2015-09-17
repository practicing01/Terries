/*
 * TerrySpawner.cpp
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
#include <Urho3D/Engine/Engine.h>
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
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "TerrySpawner.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "MoveByTouch.h"
#include "Speed.h"
#include "Gravity.h"
#include "RotateTo.h"
#include "Health.h"
#include "Stunned.h"
#include "Armor.h"
#include "Blind.h"
#include "Enchanted.h"
#include "Dead.h"
#include "SpriteSheetPlayer.h"
#include "HUD.h"
#include "TimedRemove.h"

TerrySpawner::TerrySpawner(Context* context, Urho3DPlayer* main) :
	LogicComponent(context)
{
	main_ = main;
	spriteIDCount_ = 0;
	elapsedTime_ = 0.0f;
	spawnInterval_ = 1.0f;
	maxTerries_ = 100;
	terryCount_ = 0;
	terrySpeed_ = 1.0f;
	score_ = 0;

	terriesNames_.Push("goblin");
	terriesNames_.Push("orc");
	terriesNames_.Push("ratbat");
	terriesNames_.Push("skeleton");
	terriesNames_.Push("slime");
	terriesNames_.Push("snake");
}

TerrySpawner::~TerrySpawner()
{
	RemoveModelNode();
}

void TerrySpawner::Start()
{
	scene_ = node_->GetScene();

	terrySpawns_ = scene_->GetChild("terrySpawns");
	tent_ = scene_->GetChild("tent");

	tent_->AddComponent(new Health(context_, main_), 0, LOCAL);
	tent_->AddComponent(new Armor(context_, main_), 0, LOCAL);

	tent_->SetVar("npcType",0);//0 = hero, 1 = terry

	spawnPoint_ = terrySpawns_->GetChild(Random(0,terrySpawns_->GetNumChildren()))->GetPosition();

	SubscribeToEvent(E_ANIMATESCENENODE, HANDLER(TerrySpawner, HandleAnimateSceneNode));
	SubscribeToEvent(E_ANIMATESPRITESHEET, HANDLER(TerrySpawner, HandleAnimateSpriteSheet));

	/*for (int x = 0; x < 50; x ++)
	{
		Node* terry = LoadSprite( terriesNames_[ Random( 0, terriesNames_.Size() ) ] );

		terry->GetComponent<Speed>()->speed_ = 1.0f;
		float speed = terry->GetComponent<Speed>()->speed_;
		float gravity = terry->GetComponent<Gravity>()->gravity_;

		terry->GetComponent<MoveByTouch>()->MoveTo(tent_->GetPosition(),
				speed,speed,gravity,gravity,true,true,true);
	}*/
}

Node* TerrySpawner::LoadSprite(String name)
{
	//Vector3 startPos = terrySpawns_->GetChild(Random(0,terrySpawns_->GetNumChildren()))->GetPosition();
	Vector3 startPos = spawnPoint_;

	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Objects/terriesNode.xml");
	Node* spriteNode = scene_->InstantiateXML(xmlFile->GetRoot(),
			startPos, Quaternion::IDENTITY, LOCAL);

	spriteNode->RemoveChild(spriteNode->GetChild("camera"));

	spriteNode->SetName(name);
	spriteNode->AddComponent(new Gravity(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new Speed(context_, main_), 0, LOCAL);
	//spriteNode->AddComponent(new RotateTo(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new MoveByTouch(context_, main_), 0, LOCAL);
	spriteNode->AddComponent(new Health(context_, main_), 0, LOCAL);

	spriteNode->GetComponent<MoveByTouch>()->UnsubscribeFromEvent(E_HUDBUTT);

	StaticSprite2D* sprite = spriteNode->CreateComponent<StaticSprite2D>();

	sprite->SetCustomMaterial(SharedPtr<Material>(main_->cache_->GetResource<Material>("Materials/" + name + "Mat.xml")));

	AnimatedSpriteSheet* animatedSpriteSheet = new AnimatedSpriteSheet();
	animatedSpriteSheet->sheet_ = main_->cache_->GetResource<SpriteSheet2D>("Urho2D/" + name + "/" + name + "Sheet.xml");
	animatedSpriteSheet->staticSprite_ = sprite;
	animatedSpriteSheet->playing_ = false;
	animatedSpriteSheet->spriteID_ = spriteIDCount_;
	animatedSpriteSheet->noed_ = spriteNode;
	animatedSpriteSheet->flipX_ = false;

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

	/*main_->filesystem_->ScanDir(files,
			main_->filesystem_->GetProgramDir() + "Data/Urho2D/" + name + "/animations/",
			"*.xml", SCAN_FILES, false);*/

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
	animatedSpriteSheet->noed_->SetVar("attackInterval",1.0f);
	animatedSpriteSheet->noed_->SetVar("attackElapsedTime",0.0f);
	animatedSpriteSheet->noed_->SetVar("canAttack",false);

	animatedSpriteSheet->noed_->SetVar("npcType",1);//0 = hero, 1 = terry

	return spriteNode;
}

void TerrySpawner::HandleAnimateSpriteSheet(StringHash eventType, VariantMap& eventData)
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

void TerrySpawner::Update(float timeStep)
{
	/*elapsedTime_ += timeStep;

	if (elapsedTime_ >= spawnInterval_)
	{
		elapsedTime_ = 0.0f;
		Node* terry = LoadSprite( terriesNames_[ Random( 0, terriesNames_.Size() ) ] );

		terry->GetComponent<Speed>()->speed_ = 0.5f;
		float speed = terry->GetComponent<Speed>()->speed_;
		float gravity = terry->GetComponent<Gravity>()->gravity_;

		terry->GetComponent<MoveByTouch>()->MoveTo(tent_->GetPosition(),
				speed,speed,gravity,gravity,true,true,true);
	}*/

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

void TerrySpawner::FixedPostUpdate(float timeStep)
{
	elapsedTime_ += timeStep;

	if (elapsedTime_ >= spawnInterval_)
	{
		elapsedTime_ = 0.0f;

		Node* terry = LoadSprite( terriesNames_[ Random( 0, terriesNames_.Size() ) ] );

		terry->GetComponent<Speed>()->speed_ = terrySpeed_;
		float speed = terry->GetComponent<Speed>()->speed_;
		float gravity = terry->GetComponent<Gravity>()->gravity_;

		terry->GetComponent<MoveByTouch>()->MoveTo(tent_->GetPosition(),
				speed,speed,gravity,gravity,false,true,true);

		SubscribeToEvent(terry, E_NODECOLLISIONSTART, HANDLER(TerrySpawner, HandleNodeCollisionStart));
		SubscribeToEvent(terry, E_NODECOLLISIONEND, HANDLER(TerrySpawner, HandleNodeCollisionEnd));
		SubscribeToEvent(terry, E_NODECOLLISION, HANDLER(TerrySpawner, HandleNodeCollision));

		terryCount_++;
	}

	if (terryCount_ >= maxTerries_)
	{
		SetUpdateEventMask(USE_UPDATE);
	}
}

void TerrySpawner::HandleAnimateSceneNode(StringHash eventType, VariantMap& eventData)
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
}

void TerrySpawner::RemoveModelNode()
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
}

void TerrySpawner::RespawnTerry(Node* terry)
{
	Node* particleStartNode_ = terry->GetScene()->CreateChild(0,LOCAL);
	particleStartNode_->SetPosition(terry->GetPosition());

	particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

	particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
	particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.025f);
	particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/275895__n-audioman__coin04.ogg"));

	Vector3 startPos = terrySpawns_->GetChild(Random(0,terrySpawns_->GetNumChildren()))->GetPosition();

	//Vector3 startPos = spawnPoint_;

	terry->GetComponent<RigidBody>()->SetPosition(startPos);

	terry->SetVar("collisionCount", 0);

	score_++;

	HUD* hud = main_->mySceneNode_->GetComponent<HUD>();

	UIElement* score = hud->hud_->GetChild("score", true);

	((Text*)score)->SetText("Score: " + String(score_));
}

void TerrySpawner::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));
	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	if (otherNode->GetName() == "safetyNet")
	{
		RespawnTerry(noed);
		return;
	}

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 0)//hero
	{
		MoveByTouch* mbt = noed->GetComponent<MoveByTouch>();
		mbt->MoveTo(otherNode->GetWorldPosition(),
				mbt->moveToSpeed_,mbt->speedRamp_,mbt->gravity_,mbt->gravityRamp_,false,true,true);

		int collisionCount = noed->GetVar("collisionCount").GetInt();
		collisionCount++;
		noed->SetVar("collisionCount", collisionCount);

		if (collisionCount == 1)
		{
			VariantMap vm;
			vm[AnimateSceneNode::P_NODE] = noed;

			if (noed->GetVar("sex").GetBool())
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
		}

		if (noed->GetComponent<Blind>() || noed->GetComponent<Stunned>())
		{
			return;
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
			if (otherNode->GetName() != "tent")
			{
				if (!otherNode->HasComponent<Dead>())
				{
					otherNode->AddComponent(new Dead(context_, main_, -1.0f), 0, LOCAL);
				}
			}
			else
			{
				SpriteSheetPlayer* ssp = main_->mySceneNode_->GetComponent<SpriteSheetPlayer>();
				for (int x = 0; x < ssp->sprites_.Size(); x++)
				{
					if (!ssp->sprites_[x]->noed_->HasComponent<Dead>())
					{
						ssp->sprites_[x]->noed_->AddComponent(new Dead(context_, main_, -1.0f), 0, LOCAL);

						Node* particleStartNode_ = ssp->sprites_[x]->noed_->GetScene()->CreateChild(0,LOCAL);
						particleStartNode_->SetPosition(ssp->sprites_[x]->noed_->GetPosition());

						particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

						particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
						particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.1f);
						particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/319071__mishicu__v8.ogg"));

					}
				}
			}

			//otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
			//otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
			return;
		}

		/*if (!noed->HasComponent<Stunned>())
		{
			noed->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
		}*/
	}
}

void TerrySpawner::HandleNodeCollisionEnd(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionEnd;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	MoveByTouch* mbt = noed->GetComponent<MoveByTouch>();
	mbt->MoveTo(tent_->GetWorldPosition(),
			mbt->moveToSpeed_,mbt->speedRamp_,mbt->gravity_,mbt->gravityRamp_,false,true,true);

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	if (otherNode->GetVar("npcType").GetInt() == 0)//hero
	{
		int collisionCount = noed->GetVar("collisionCount").GetInt();
		collisionCount--;
		noed->SetVar("collisionCount", collisionCount);

		if (collisionCount == 0)
		{
			VariantMap vm;
			vm[AnimateSceneNode::P_NODE] = noed;

			if (noed->GetVar("sex").GetBool())
			{
				vm[AnimateSceneNode::P_ANIMATION] = "runF";
			}
			else
			{
				vm[AnimateSceneNode::P_ANIMATION] = "runM";
			}

			vm[AnimateSceneNode::P_LOOP] = false;
			vm[AnimateSceneNode::P_LAYER] = 0;
			SendEvent(E_ANIMATESCENENODE, vm);
		}
	}
}

void TerrySpawner::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollision;

	RigidBody* rb = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());

	Node* noed = rb->GetNode();

	if (!noed->GetVar("canAttack").GetBool())
	{
		return;
	}

	if (noed->GetComponent<Blind>())
	{
		return;
	}

	noed->SetVar("canAttack", false);

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	if (!otherNode->HasComponent<Health>())
	{
		return;
	}

	bool enemy = false;

	if (otherNode->GetVar("npcType").GetInt() == 0)//hero
	{
		enemy = true;
	}

	if (enemy || noed->HasComponent<Enchanted>())
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
			if (!enemy && noed->HasComponent<Enchanted>())
			{
				otherNode->GetComponent<Health>()->ModifyHealth(100, 0, false);
				otherNode->GetScene()->GetComponent<TerrySpawner>()->RespawnTerry(otherNode);
			}
			else if (otherNode->GetName() != "tent")
			{
				if (!otherNode->HasComponent<Dead>())
				{
					otherNode->AddComponent(new Dead(context_, main_, -1.0f), 0, LOCAL);
				}
			}
			else
			{
				SpriteSheetPlayer* ssp = main_->mySceneNode_->GetComponent<SpriteSheetPlayer>();
				for (int x = 0; x < ssp->sprites_.Size(); x++)
				{
					if (!ssp->sprites_[x]->noed_->HasComponent<Dead>())
					{
						ssp->sprites_[x]->noed_->AddComponent(new Dead(context_, main_, -1.0f), 0, LOCAL);

						Node* particleStartNode_ = ssp->sprites_[x]->noed_->GetScene()->CreateChild(0,LOCAL);
						particleStartNode_->SetPosition(ssp->sprites_[x]->noed_->GetPosition());

						particleStartNode_->AddComponent(new TimedRemove(context_, main_, 2.0f), 0, LOCAL);

						particleStartNode_->AddComponent(new SoundSource3D(context_), 0, LOCAL);
						particleStartNode_->GetComponent<SoundSource3D>()->SetGain(0.1f);
						particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/319071__mishicu__v8.ogg"));

					}
				}
			}

			return;
		}

		/*if (!noed->HasComponent<Stunned>())
		{
			noed->AddComponent(new Stunned(context_, main_, 2.0f), 0, LOCAL);
		}*/
	}
}
