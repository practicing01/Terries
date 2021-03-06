/*
 * Constants.h
 *
 *  Created on: Jul 6, 2015
 *      Author: practicing01
 */
#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>

#include "gameModes/terries/Terries.h"

using namespace Urho3D;

EVENT(E_GAMEMENUDISPLAY, GameMenuDisplay)
{
	PARAM(P_STATE, State);// bool
}

EVENT(E_NEWCLIENTID, NewClientID)
{
	PARAM(P_CLIENTID, ClientID);// int
}

EVENT(E_GAMEMODEREMOVED, GameModeRemoved)
{
}

EVENT(E_GETCLIENTCAMERA, GetClientCamera)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTCAMERA, SetClientCamera)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_CAMERANODE, CameraNode);// node pointer
}

EVENT(E_TOUCHSUBSCRIBE, TouchSubscribe)
{
}

EVENT(E_TOUCHUNSUBSCRIBE, TouchUnSubscribe)
{
}

EVENT(E_GETCLIENTSPEED, GetClientSpeed)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTSPEED, SetClientSpeed)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_SPEED, Speed);// float
}

EVENT(E_GETCLIENTGRAVITY, GetClientGravity)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTGRAVITY, SetClientGravity)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_GRAVITY, Gravity);// float
}

EVENT(E_GETCLIENTMODELNODE, GetClientModelNode)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTMODELNODE, SetClientModelNode)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_MODELNODE, ModelNode);// node pointer
}

EVENT(E_RESPAWNSCENENODE, RespawnSceneNode)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_SPAWNNODE, SpawnNode);// SharedPtr<Node>
	//PARAM(P_POSITION, Position);// Vector3
	//PARAM(P_ROTATION, Rotation);// Quaternion
}

EVENT(E_ROTATEMODELNODE, RotateModelNode)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_ROTATION, Rotation);// Quaternion
	PARAM(P_SPEED, Speed);// float
	PARAM(P_SPEEDRAMP, SpeedRamp);// float
	PARAM(P_STOPONCOMPLETION, StopOnCompletion);// bool
}

EVENT(E_ANIMATESCENENODE, AnimateSceneNode)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_ANIMATION, Animation);// string
	PARAM(P_LOOP, Loop);// bool
	PARAM(P_LAYER, Layer);// unsigned char
}

EVENT(E_GETCLIENTID, GetClientID)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTID, SetClientID)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_CLIENTID, ClientID);// int
}

EVENT(E_LCMSG, LcMsg)
{
	PARAM(P_DATA, Data);// Buffer (PODVector)
}

EVENT(E_GETLAGTIME, GetLagTime)
{
	PARAM(P_CONNECTION, Connection);// Connection pointer
}

EVENT(E_SETLAGTIME, SetLagTime)
{
	PARAM(P_CONNECTION, Connection);// Connection pointer
	PARAM(P_LAGTIME, LagTime);// float
}

EVENT(E_GETCONNECTION, GetConnection)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCONNECTION, SetConnection)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_CONNECTION, Connection);// connection pointer
}

EVENT(E_GETISSERVER, GetIsServer)
{
}

EVENT(E_SETISSERVER, SetIsServer)
{
	PARAM(P_ISSERVER, IsServer);// bool
}

EVENT(E_EXCLUSIVENETBROADCAST, ExclusiveNetBroadcast)
{
	PARAM(P_EXCLUDEDCONNECTION, ExcludedConnection);// connection pointer
	PARAM(P_MSG, Msg);// Buffer
}

EVENT(E_GETLC, GetLc)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_CONNECTION, Connection);// connection pointer
}

EVENT(E_MECHANICREQUEST, MechanicRequest)
{
	PARAM(P_MECHANICID, MechanicID);// string
}

EVENT(E_GETCLIENTSILENCE, GetClientSilence)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTSILENCE, SetClientSilence)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_SILENCE, Silence);// bool
}

EVENT(E_GETSCENENODEBYMODELNODE, GetSceneNodeByModelNode)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETSCENENODEBYMODELNODE, SetSceneNodeByModelNode)
{
	PARAM(P_MODELNODE, ModelNode);// node pointer
	PARAM(P_SCENENODE, SceneNode);// node pointer
}

EVENT(E_GETSCENENODECLIENTID, GetSceneNodeClientID)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETSCENENODECLIENTID, SetSceneNodeClientID)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_CLIENTID, ClientID);// int
}

EVENT(E_GETMODELNODEBYSCENENODE, GetModelNodeBySceneNode)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETMODELNODEBYSCENENODE, SetModelNodeBySceneNode)
{
	PARAM(P_SCENENODE, SceneNode);// node pointer
	PARAM(P_MODELNODE, ModelNode);// node pointer
}

EVENT(E_MODIFYCLIENTSPEED, ModifyClientSpeed)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_SPEED, Speed);// float
	PARAM(P_OPERATION, Operation);// char
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_GETCLIENTBLIND, GetClientBlind)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTBLIND, SetClientBlind)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_BLIND, Blind);// bool
}

EVENT(E_GETCLIENTHEALTH, GetClientHealth)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTHEALTH, SetClientHealth)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_HEALTH, Health);// float
}

EVENT(E_MODIFYCLIENTHEALTH, ModifyClientHealth)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_HEALTH, Health);// int
	PARAM(P_OPERATION, Operation);// char
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_CLIENTHEALTHSET, ClientHealthSet)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_HEALTH, Health);// int
}

EVENT(E_MODIFYCLIENTSILENCE, ModifyClientSilence)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_STATE, State);// bool
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_MODIFYCLIENTBLIND, ModifyClientBlind)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_STATE, State);// bool
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_GETCLIENTARMOR, GetClientArmor)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_SETCLIENTARMOR, SetClientArmor)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_ARMOR, Armor);// int
}

EVENT(E_MODIFYCLIENTARMOR, ModifyClientArmor)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_ARMOR, Armor);// int
	PARAM(P_OPERATION, Operation);// char
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_CLEANSE, CleanseStatus)
{
	PARAM(P_NODE, Node);// node pointer
}

EVENT(E_MOVEMODELNODE, MoveModelNode)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_DEST, Dest);// Vector3
	PARAM(P_SPEED, Speed);// float
	PARAM(P_SPEEDRAMP, SpeedRamp);// float
	PARAM(P_GRAVITY, Gravity);// float
	PARAM(P_GRAVITYRAMP, GravityRamp);// float
	PARAM(P_STOPONCOMPLETION, StopOnCompletion);// bool
	PARAM(P_ROTATE, Rotate);// bool
	PARAM(P_SENDTOSERVER, SendToServer);// bool
}

EVENT(E_SOUNDREQUEST, SoundRequest)
{
   PARAM(P_NODE, Node);
   PARAM(P_SOUNDTYPE, SoundType);
}

EVENT(E_ANIMATESPRITESHEET, AnimateSpriteSheet)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_SPRITEID, SpriteID);// int
	PARAM(P_ANIMATION, Animation);// string
	PARAM(P_FLIPX, FlipX);// int
}

EVENT(E_HUDBUTT, HudButt)
{
	PARAM(P_NODE, Node);// node pointer
	PARAM(P_BUTT, Butt);// xml element pointer
}

extern const int GAMEMODEMSG_RESPAWNNODE;
extern const int GAMEMODEMSG_GETLC;
extern const int SOUNDTYPE_CAST;
extern const int SOUNDTYPE_MELEE;
extern const int SOUNDTYPE_HURT;
