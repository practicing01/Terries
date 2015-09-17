/*
 * DotsNetCrits.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "Terries.h"
#include "../../network/NetworkConstants.h"
#include "../../Constants.h"
#include "logicComponents/RTSCamera.h"
#include "logicComponents/SpriteSheetPlayer.h"
#include "logicComponents/Speed.h"
#include "logicComponents/Gravity.h"
#include "logicComponents/HUD.h"
#include "logicComponents/MoveByTouch.h"
#include "logicComponents/SunLight.h"
#include "logicComponents/TerrySpawner.h"

Terries::Terries(Context* context, Urho3DPlayer* main, bool isServer) :
	LogicComponent(context)
{
	main_ = main;
	network_ = GetSubsystem<Network>();
	isServer_ = isServer;

	//SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(Terries, HandlePostRenderUpdate));
	SubscribeToEvent(E_GETISSERVER, HANDLER(Terries, HandleGetIsServer));
}

Terries::~Terries()
{
	scene_->RemoveAllChildren();
	scene_->RemoveAllComponents();
	scene_->Remove();
	main_->ClearRootNodes();
}

void Terries::Start()
{
	main_->ClearRootNodes();

	scene_ = new Scene(context_);

	File loadFile(context_,main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/terriesDunes.xml", FILE_READ);
	scene_->LoadXML(loadFile);

	main_->mySceneNode_ = SharedPtr<Node>( new Node(context_) );
	main_->sceneNodes_.Push(main_->mySceneNode_);

	for (int x = 0; x < main_->sceneNodes_.Size(); x++)
	{
		scene_->AddChild(main_->sceneNodes_[x]);
	}

	cameraNode_ = new Node(context_);
	cameraNode_ = scene_->GetChild("camera");

	Node* spawns = scene_->GetChild("spawns");

	for (int x = 0; x < spawns->GetNumChildren(false); x++)
	{
		spawnPoints_.Push( SharedPtr<Node>(spawns->GetChild(x) ) );
	}

	if (!main_->engine_->IsHeadless())
	{
		main_->viewport_->SetScene(scene_);
		main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());
		main_->renderer_->SetViewport(0, main_->viewport_);

		if (GetPlatform() == "Android")
		{
			//main_->renderer_->SetReuseShadowMaps(false);
			//main_->renderer_->SetShadowQuality(SHADOWQUALITY_LOW_16BIT);
			//main_->renderer_->SetMobileShadowBiasMul(2.0f);
			//main_->renderer_->SetMobileShadowBiasAdd(0.001);
		}

		scene_->AddComponent(new SunLight(context_, main_), 0, LOCAL);

		//if (isServer_)
		{
			RespawnNode(main_->mySceneNode_, -1);
		}

		AttachLogicComponents(main_->mySceneNode_);

		//if (isServer_)
		{
			RespawnNode(main_->mySceneNode_, -1);
		}
	}
	else
	{
		scene_->GetChild("bgm")->GetComponent<SoundSource>()->Stop();
	}

	SubscribeToEvent(E_NETWORKMESSAGE, HANDLER(Terries, HandleNetworkMessage));
	SubscribeToEvent(E_GAMEMENUDISPLAY, HANDLER(Terries, HandleDisplayMenu));
	SubscribeToEvent(E_NEWCLIENTID, HANDLER(Terries, HandleNewClientID));
	//SubscribeToEvent(E_CLIENTHEALTHSET, HANDLER(Terries, HandleClientHealthSet));

	scene_->AddComponent(new TerrySpawner(context_, main_), 0, LOCAL);
	/*
	XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");

	// Create console
	Console* console = main_->engine_->CreateConsole();
	console->SetDefaultStyle(xmlFile);
	console->GetBackground()->SetOpacity(0.8f);

	// Create debug HUD.
	DebugHud* debugHud = main_->engine_->CreateDebugHud();
	debugHud->SetDefaultStyle(xmlFile);

	GetSubsystem<DebugHud>()->ToggleAll();
	//debugHud->SetMode(DEBUGHUD_SHOW_ALL);
	 */
}

void Terries::HandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
	using namespace NetworkMessage;

	int msgID = eventData[P_MESSAGEID].GetInt();

    if (msgID == MSG_GAMEMODEMSG)
    {
    	Connection* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

        const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
        MemoryBuffer msg(data);

        int gmMSG = msg.ReadInt();

        if (gmMSG == GAMEMODEMSG_RESPAWNNODE)
        {
        	int clientID = msg.ReadInt();
        	int index = msg.ReadInt();

        	SharedPtr<Node> sceneNode = SharedPtr<Node>( main_->GetSceneNode(clientID) );

        	if (sceneNode != NULL)
        	{
        		RespawnNode(sceneNode, index);
        	}
        }
        else if (gmMSG == GAMEMODEMSG_GETLC)
        {
        	int clientID = msg.ReadInt();
        	SharedPtr<Node> sceneNode = SharedPtr<Node>( main_->GetSceneNode(clientID) );

        	VariantMap vm;
        	vm[GetLc::P_NODE] = sceneNode;
        	vm[GetLc::P_CONNECTION] = sender;
        	SendEvent(E_GETLC, vm);
        }
    }
}

void Terries::HandleDisplayMenu(StringHash eventType, VariantMap& eventData)
{
	bool state = eventData[GameMenuDisplay::P_STATE].GetBool();

	if (state)
	{
		VariantMap vm;
		SendEvent(E_GAMEMODEREMOVED, vm);

		Remove();//buhbye.
	}
}

void Terries::HandleNewClientID(StringHash eventType, VariantMap& eventData)
{
	int clientID = eventData[NewClientID::P_CLIENTID].GetInt();

	SharedPtr<Node> sceneNode = SharedPtr<Node>( main_->GetSceneNode(clientID) );

	if (sceneNode != NULL)
	{
		scene_->AddChild(sceneNode);

		AttachLogicComponents(sceneNode);

		if (isServer_)
		{
			int index = Random( 0, spawnPoints_.Size() );
			RespawnNode(sceneNode, index);

			msg_.Clear();
			msg_.WriteInt(GAMEMODEMSG_RESPAWNNODE);
			msg_.WriteInt(clientID);
			msg_.WriteInt(index);
			network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
		}
		else
		{
			msg_.Clear();
			msg_.WriteInt(GAMEMODEMSG_GETLC);
			msg_.WriteInt(clientID);
			network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
		}
	}
}

void Terries::RespawnNode(SharedPtr<Node> sceneNode, int index)
{
	if (spawnPoints_.Size() == 0)
	{
		return;
	}

	if (index == -1)
	{
		index = Random( 0, spawnPoints_.Size() );
	}

	Vector3 victoria = spawnPoints_[index]->GetPosition();
	Quaternion quarterOnion = spawnPoints_[index]->GetRotation();

	sceneNode->SetPosition(victoria);
	sceneNode->SetRotation(quarterOnion);

	VariantMap vm;
	vm[RespawnSceneNode::P_NODE] = sceneNode;
	vm[RespawnSceneNode::P_SPAWNNODE] = spawnPoints_[index];
	//vm[RespawnSceneNode::P_POSITION] = victoria;
	//vm[RespawnSceneNode::P_ROTATION] = quarterOnion;
	SendEvent(E_RESPAWNSCENENODE, vm);
}

void Terries::AttachLogicComponents(SharedPtr<Node> sceneNode)
{
	sceneNode->AddComponent(new HUD(context_, main_), 0, LOCAL);

	sceneNode->AddComponent(new SpriteSheetPlayer(context_, main_), 0, LOCAL);

	sceneNode->AddComponent(new Speed(context_, main_), 0, LOCAL);

	sceneNode->AddComponent(new Gravity(context_, main_), 0, LOCAL);

	sceneNode->AddComponent(new RTSCamera(context_, main_), 0, LOCAL);
}

void Terries::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	GetSubsystem<Renderer>()->DrawDebugGeometry(true);
	if (scene_)
	{
		scene_->GetComponent<DebugRenderer>()->SetView(cameraNode_->GetComponent<Camera>());
		if (scene_->GetComponent<PhysicsWorld>())
		{
			scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
		}
	}
}

void Terries::HandleGetIsServer(StringHash eventType, VariantMap& eventData)
{
	VariantMap vm;
	vm[SetIsServer::P_ISSERVER] = isServer_;
	SendEvent(E_SETISSERVER, vm);
}

void Terries::HandleClientHealthSet(StringHash eventType, VariantMap& eventData)
{
	if (!isServer_)
	{
		return;
	}

	targetSceneNode_ = (Node*)(eventData[ModifyClientHealth::P_NODE].GetPtr());
	int health = eventData[ClientHealthSet::P_HEALTH].GetInt();

	if (health <= 0)
	{
		SubscribeToEvent(E_SETSCENENODECLIENTID, HANDLER(Terries, HandleSetSceneNodeClientID));

		VariantMap vm0;
		vm0[GetSceneNodeClientID::P_NODE] = targetSceneNode_;
		SendEvent(E_GETSCENENODECLIENTID, vm0);

		UnsubscribeFromEvent(E_SETSCENENODECLIENTID);

		int index = Random( 0, spawnPoints_.Size() );
		RespawnNode(SharedPtr<Node>(targetSceneNode_), index);

		msg_.Clear();
		msg_.WriteInt(GAMEMODEMSG_RESPAWNNODE);
		msg_.WriteInt(targetSceneNodeClientID_);
		msg_.WriteInt(index);
		network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);

		VariantMap vm1;
		vm1[ModifyClientHealth::P_NODE] = targetSceneNode_;
		vm1[ModifyClientHealth::P_HEALTH] = 100;
		vm1[ModifyClientHealth::P_OPERATION] = 0;
		vm1[ModifyClientHealth::P_SENDTOSERVER] = false;
		SendEvent(E_MODIFYCLIENTHEALTH, vm1);

		msg_.Clear();//todo better way of doing this stuff
		msg_.WriteInt(targetSceneNodeClientID_);
		msg_.WriteString("Health");
		msg_.WriteInt(100);
		msg_.WriteInt(0);
		network_->BroadcastMessage(MSG_LCMSG, true, true, msg_);
	}
}

void Terries::HandleSetSceneNodeClientID(StringHash eventType, VariantMap& eventData)
{
	Node* sceneNode = (Node*)(eventData[SetSceneNodeClientID::P_NODE].GetPtr());

	if (sceneNode == targetSceneNode_)
	{
		targetSceneNodeClientID_ = eventData[SetSceneNodeClientID::P_CLIENTID].GetInt();
	}
}
