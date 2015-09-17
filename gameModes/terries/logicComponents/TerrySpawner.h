/*
 * TerrySpawner.h
 *
 *  Created on: Aug 8, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include "../../../Urho3DPlayer.h"
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

using namespace Urho3D;

class TerrySpawner : public LogicComponent
{
	OBJECT(TerrySpawner);
public:
	TerrySpawner(Context* context, Urho3DPlayer* main);
	~TerrySpawner();
	virtual void Start();
	virtual void FixedPostUpdate(float timeStep);
	virtual void Update(float timeStep);
	void HandleAnimateSceneNode(StringHash eventType, VariantMap& eventData);
	void HandleAnimateSpriteSheet(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollisionEnd(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	Node* LoadSprite(String name);
	void RemoveModelNode();
	void RespawnTerry(Node* terry);

	Urho3DPlayer* main_;

	SharedPtr<Scene> scene_;

	typedef struct
	{
		float duration_;
		String sprite_;
	}SpriteSheetAnimationFrame;

	typedef struct
	{
		String name_;
		bool loop_;
		Vector<SpriteSheetAnimationFrame*> frames_;
	}SpriteSheetAnimation;

	typedef struct
	{
		float elapsedTime_;
		int curFrame_;
		bool flipX_;
		bool playing_;
		Vector<SpriteSheetAnimation*> animations_;
		SpriteSheetAnimation* animation_;
		SpriteSheet2D* sheet_;
		StaticSprite2D* staticSprite_;
		int spriteID_;
		Node* noed_;
	}AnimatedSpriteSheet;

	Vector<AnimatedSpriteSheet*> sprites_;

	int spriteIDCount_;
	float elapsedTime_;
	float spawnInterval_;
	int maxTerries_;
	int terryCount_;
	float terrySpeed_;

	Node* terrySpawns_;
	Node* tent_;

	Vector3 spawnPoint_;

	Vector<String> terriesNames_;

	int score_;
};
