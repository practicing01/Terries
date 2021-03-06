/*
 * Server.h
 *
 *  Created on: Jun 29, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include "../Urho3DPlayer.h"
#include <Urho3D/Network/Network.h>

using namespace Urho3D;

class Server : public LogicComponent
{
	OBJECT(Server);
public:
	Server(Context* context, Urho3DPlayer* main);
	~Server();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	virtual void Start();
	void HandleServerConnect(StringHash eventType, VariantMap& eventData);
	void HandleServerDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleConnectFailed(StringHash eventType, VariantMap& eventData);
	void HandleClientDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleClientConnect(StringHash eventType, VariantMap& eventData);
    void HandleNetworkMessage(StringHash eventType, VariantMap& eventData);
    void HandleExclusiveNetBroadcast(StringHash eventType, VariantMap& eventData);
    void LoadGameMode(String gameMode);

	Urho3DPlayer* main_;
	float elapsedTime_;

	Network* network_;
	VectorBuffer msg_;

	String masterServerIP_;
	String serverName_;
	String gameMode_;

	int clientIDCount_;
};
