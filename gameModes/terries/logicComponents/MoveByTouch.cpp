/*
 * MoveByTouch.cpp
 *
 *  Created on: Jul 12, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "MoveByTouch.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"
#include "Speed.h"
#include "Gravity.h"
#include "Stunned.h"
#include "Dead.h"

MoveByTouch::MoveByTouch(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
	network_ = GetSubsystem<Network>();
	touchSubscriberCount_ = 0;
	isMoving_ = false;
	modelNode_ = node_;
	cameraNode_ = NULL;
	clientID_ = -1;
	lagTime_ = 0;
	isServer_ = false;
	gravity_ = 0.0f;
	gravityRamp_ = 0.0f;
	moveToSpeed_ = 0.0f;
	speedRamp_ = 0.0f;
	touchSubscribed_ = false;
	curDir_ = 0;
	//SetUpdateEventMask(USE_FIXEDUPDATE);
}

MoveByTouch::~MoveByTouch()
{
	/*if (clientExecuting_)
	{
		VariantMap vm;
		SendEvent(E_TOUCHUNSUBSCRIBE, vm);
	}*/
}

void MoveByTouch::Start()
{
	scene_ = node_->GetScene();
	ssp_ = main_->mySceneNode_->GetComponent<SpriteSheetPlayer>();

	//SubscribeToEvent(E_TOUCHSUBSCRIBE, HANDLER(MoveByTouch, HandleTouchSubscribe));
	//SubscribeToEvent(E_TOUCHUNSUBSCRIBE, HANDLER(MoveByTouch, HandleTouchUnSubscribe));
	//SubscribeToEvent(E_SETISSERVER, HANDLER(MoveByTouch, HandleSetIsServer));
	//SubscribeToEvent(E_MOVEMODELNODE, HANDLER(MoveByTouch, HandleMoveModelNode));

	VariantMap vm;
	//SendEvent(E_GETISSERVER, vm);

	SubscribeToEvent(E_SETCLIENTCAMERA, HANDLER(MoveByTouch, HandleSetCamera));

	vm.Clear();
	vm[GetClientCamera::P_NODE] = main_->mySceneNode_;
	SendEvent(E_GETCLIENTCAMERA, vm);

	radius_ = node_->GetComponent<CollisionShape>()->GetSize().x_;
	radius_ *= node_->GetWorldScale().x_;

	//SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(MoveByTouch, HandleNodeCollisionStart));
	//SubscribeToEvent(node_, E_NODECOLLISIONEND, HANDLER(MoveByTouch, HandleNodeCollisionEnd));
	//todo total rewrite for optimized multiplayer npc sync
	SubscribeToEvent(E_HUDBUTT, HANDLER(MoveByTouch, HandleHudButt));
}

void MoveByTouch::HandleSetIsServer(StringHash eventType, VariantMap& eventData)
{
	isServer_ = eventData[SetIsServer::P_ISSERVER].GetBool();
	UnsubscribeFromEvent(E_SETISSERVER);
}

void MoveByTouch::HandleSetCamera(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetClientCamera::P_NODE].GetPtr());

	if (clientNode == main_->mySceneNode_)
	{
		cameraNode_ = (Node*)(eventData[SetClientCamera::P_CAMERANODE].GetPtr());

		UnsubscribeFromEvent(E_SETCLIENTCAMERA);
		//SubscribeToEvent(E_SETCLIENTMODELNODE, HANDLER(MoveByTouch, HandleSetClientModelNode));

		//VariantMap vm;
		//vm[GetClientModelNode::P_NODE] = node_;
		//SendEvent(E_GETCLIENTMODELNODE, vm);

	}
}

void MoveByTouch::HandleSetClientModelNode(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetClientModelNode::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		modelNode_ = (Node*)(eventData[SetClientModelNode::P_MODELNODE].GetPtr());

		//beeBox_ = modelNode_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
		radius_ = modelNode_->GetComponent<CollisionShape>()->GetSize().x_;
		radius_ *= modelNode_->GetWorldScale().x_;

		UnsubscribeFromEvent(E_SETCLIENTMODELNODE);
		SubscribeToEvent(E_SETCLIENTID, HANDLER(MoveByTouch, HandleSetClientID));

		VariantMap vm;
		vm[GetClientID::P_NODE] = main_->GetRootNode(node_);
		SendEvent(E_GETCLIENTID, vm);
	}
}

void MoveByTouch::HandleSetClientID(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetClientID::P_NODE].GetPtr());

	if (main_->GetSceneNode(clientNode) == node_)
	{
		clientID_ = eventData[SetClientID::P_CLIENTID].GetInt();

		UnsubscribeFromEvent(E_SETCLIENTID);
		SubscribeToEvent(E_SETCONNECTION, HANDLER(MoveByTouch, HandleSetConnection));

		VariantMap vm;
		vm[GetConnection::P_NODE] = main_->GetRootNode(node_);
		SendEvent(E_GETCONNECTION, vm);

	}
}

void MoveByTouch::HandleSetConnection(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetConnection::P_NODE].GetPtr());

	if (main_->GetSceneNode(clientNode) == node_)
	{
		conn_ = (Connection*)(eventData[SetConnection::P_CONNECTION].GetPtr());

		UnsubscribeFromEvent(E_SETCONNECTION);

		//SubscribeToEvent(modelNode_, E_NODECOLLISION, HANDLER(MoveByTouch, HandleNodeCollision));
		SubscribeToEvent(E_LCMSG, HANDLER(MoveByTouch, HandleLCMSG));
		SubscribeToEvent(E_GETLC, HANDLER(MoveByTouch, HandleGetLc));

		if (main_->IsLocalClient(node_))
		{
			SubscribeToEvent(E_HUDBUTT, HANDLER(MoveByTouch, HandleHudButt));
		}
	}
}

void MoveByTouch::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
{
	if (main_->ui_->GetFocusElement() )//|| touchSubscriberCount_)
	{
		return;
	}

	if (!scene_)
	{
		return;
	}

	if (!scene_->GetComponent<PhysicsWorld>())
	{
		return;
	}

	/*if (!modelNode_)
	{
		return;
	}*/

	using namespace TouchEnd;

	Ray cameraRay = cameraNode_->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	PhysicsRaycastResult raeResult_;

	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, cameraRay, 10000.0f, 1);//todo define masks.

	if (raeResult_.body_)
	{
		moveToVector_ = raeResult_.position_;

		/*SubscribeToEvent(E_SETCLIENTSPEED, HANDLER(MoveByTouch, HandleSetSpeed));

		VariantMap vm;
		vm[GetClientSpeed::P_NODE] = node_;
		SendEvent(E_GETCLIENTSPEED, vm);*/

		MoveTo(moveToVector_, node_->GetComponent<Speed>()->speed_,
				node_->GetComponent<Speed>()->speed_,
				node_->GetComponent<Gravity>()->gravity_,
				node_->GetComponent<Gravity>()->gravity_, true, true, true);
	}

	VariantMap vm;
	SendEvent(E_TOUCHUNSUBSCRIBE, vm);

	touchSubscribed_ = false;

	UnsubscribeFromEvent(E_TOUCHEND);
}

void MoveByTouch::HandleTouchSubscribe(StringHash eventType, VariantMap& eventData)
{
	touchSubscriberCount_++;//Hazardous if a component gets deleted and it doesn't unsubscribe.
}

void MoveByTouch::HandleTouchUnSubscribe(StringHash eventType, VariantMap& eventData)
{
	touchSubscriberCount_--;
	if (touchSubscriberCount_ < 0)
	{
		touchSubscriberCount_ = 0;
	}
}

void MoveByTouch::HandleSetSpeed(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetClientSpeed::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		clientSpeed_ = eventData[SetClientSpeed::P_SPEED].GetFloat();

		UnsubscribeFromEvent(E_SETCLIENTSPEED);
		SubscribeToEvent(E_SETCLIENTGRAVITY, HANDLER(MoveByTouch, HandleSetGravity));

		VariantMap vm;
		vm[GetClientGravity::P_NODE] = node_;
		SendEvent(E_GETCLIENTGRAVITY, vm);

	}
}

void MoveByTouch::HandleSetGravity(StringHash eventType, VariantMap& eventData)
{
	Node* clientNode = (Node*)(eventData[SetClientGravity::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		clientGravity_ = eventData[SetClientGravity::P_GRAVITY].GetFloat();

		UnsubscribeFromEvent(E_SETCLIENTGRAVITY);

		MoveTo(moveToVector_, clientSpeed_, clientSpeed_, clientGravity_, clientGravity_, true, true, true);
	}
}

void MoveByTouch::FixedUpdate(float timeStep)
{
	/*if (!scene_)
	{
		return;
	}*/

	if (node_->GetComponent<Stunned>())
	{
		return;
	}

	if (node_->HasComponent<Dead>())
	{
		return;
	}

	if (isMoving_ == true)
	{
		moveToLoc_ =  node_->GetPosition();

		remainingDist_ = (moveToLoc_ - moveToDest_).Length();
		if (remainingDist_ <= 0.3f)
		{
			if (moveToStopOnTime_)
			{
				isMoving_ = false;
				OnMoveToComplete();
			}
			return;
		}

		Vector3 victoria = (moveToDest_ - moveToLoc_).Normalized();
		//victoria *= moveToSpeed_;
		victoria *= node_->GetComponent<Speed>()->speed_;

		//node_->GetComponent<RigidBody>()->SetLinearVelocity(victoria);
		node_->GetComponent<RigidBody>()->ApplyImpulse(victoria - node_->GetComponent<RigidBody>()->GetLinearVelocity());

		if (node_->GetVar("collisionCount").GetInt() == 0)
		{
			if (moveToLoc_.x_ < moveToDest_.x_)
			{
				if (curDir_ != 1)
				{
					curDir_ = 1;

					VariantMap vm;
					vm[AnimateSceneNode::P_NODE] = node_;

					if (node_->GetVar("sex").GetBool())
					{
						vm[AnimateSceneNode::P_ANIMATION] = "runF";
					}
					else
					{
						vm[AnimateSceneNode::P_ANIMATION] = "runM";
					}
					vm[AnimateSceneNode::P_LAYER] = 1;//right
					vm[AnimateSceneNode::P_LOOP] = true;
					SendEvent(E_ANIMATESCENENODE, vm);
				}
			}
			else
			{
				if (curDir_ != -1)
				{
					curDir_ = -1;

					VariantMap vm;
					vm[AnimateSceneNode::P_NODE] = node_;

					if (node_->GetVar("sex").GetBool())
					{
						vm[AnimateSceneNode::P_ANIMATION] = "runF";
					}
					else
					{
						vm[AnimateSceneNode::P_ANIMATION] = "runM";
					}
					vm[AnimateSceneNode::P_LAYER] = -1;//left
					vm[AnimateSceneNode::P_LOOP] = true;
					SendEvent(E_ANIMATESCENENODE, vm);
				}
			}
		}
	}
	return;

	if (isMoving_ == true)
	{
		if (speedRamp_ > moveToSpeed_)
		{
			speedRamp_ -= timeStep;
			if (speedRamp_ < moveToSpeed_)
			{
				speedRamp_ = moveToSpeed_;
			}
		}

		if (gravityRamp_ > gravity_)
		{
			gravityRamp_ -= timeStep;
			if (gravityRamp_ < gravity_)
			{
				gravityRamp_ = gravity_;
			}
		}

		inderp_ = speedRamp_ * timeStep;
		moveToLoc_ =  node_->GetPosition();
		if (rotate_)
		{
			moveToDest_.y_ = moveToLoc_.y_;
		}
		remainingDist_ = (moveToLoc_ - moveToDest_).Length();

		victoria_ = moveToLoc_.Lerp(moveToDest_, inderp_ / remainingDist_);

		node_->SetPosition(victoria_);

		if (remainingDist_ <= 0.1f)
		{
			isMoving_ = false;
			OnMoveToComplete();
		}
	}

	//Gravity.
	moveToLoc_ = node_->GetPosition();

	raeResult_.body_ = NULL;
	rae_.origin_ = node_->LocalToWorld(node_->GetComponent<CollisionShape>()->GetPosition());
	rae_.direction_ = Vector3::DOWN;//todo change for games that might behave differently.

	scene_->GetComponent<PhysicsWorld>()->SphereCast(raeResult_, rae_, radius_, Abs(gravityRamp_ * timeStep), 1);//todo define masks.

	if (!raeResult_.body_)
	{
		node_->SetPosition(moveToLoc_ - (rae_.direction_ * (gravityRamp_ * timeStep) ) );
	}
}

void MoveByTouch::MoveTo(Vector3 dest, float speed, float speedRamp, float gravity, float gravityRamp, bool stopOnCompletion, bool rotate, bool sendToServer)
{
	if (node_->GetComponent<Stunned>())
	{
		return;
	}

	if (node_->HasComponent<Dead>())
	{
		return;
	}

	node_->GetComponent<RigidBody>()->SetFriction(0.0f);

	moveToLoc_ = node_->GetPosition();

	if (node_->GetVar("collisionCount").GetInt() == 0)
	{
		VariantMap vm;
		vm[AnimateSceneNode::P_NODE] = node_;

		if (node_->GetVar("sex").GetBool())
		{
			vm[AnimateSceneNode::P_ANIMATION] = "runF";
		}
		else
		{
			vm[AnimateSceneNode::P_ANIMATION] = "runM";
		}

		vm[AnimateSceneNode::P_LOOP] = true;
		if (moveToLoc_.x_ < dest.x_)
		{
			vm[AnimateSceneNode::P_LAYER] = 1;//right
			curDir_ = 1;
		}
		else
		{
			vm[AnimateSceneNode::P_LAYER] = -1;//left
			curDir_ = -1;
		}
		SendEvent(E_ANIMATESCENENODE, vm);
	}

	moveToSpeed_ = speed;
	speedRamp_ = speedRamp;
	gravity_ = gravity;
	gravityRamp_ = gravityRamp;
	//moveToLoc_ = node_->GetPosition();
	moveToDest_ = dest;
	rotate_ = rotate;
	if (rotate)
	{
		//moveToDest_.y_ = moveToLoc_.y_;
	}
	moveToDir_ = dest - moveToLoc_;
	moveToDir_.Normalize();
	moveToTravelTime_ = (moveToDest_ - moveToLoc_).Length() / speed;
	moveToElapsedTime_ = 0;
	moveToStopOnTime_ = stopOnCompletion;
	radius_ = node_->GetComponent<CollisionShape>()->GetSize().x_;
	radius_ *= node_->GetWorldScale().x_;
	//beeBox_ = modelNode_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
	isMoving_ = true;

	if (speedRamp_ < moveToSpeed_)
	{
		speedRamp_ = moveToSpeed_;
	}

	if (gravityRamp_ < gravity_)
	{
		gravityRamp_ = gravity_;
	}

	//RotateTo
	quarterOnion_ = node_->GetRotation();//Temp storage.
	node_->LookAt(moveToDest_);//Hack rotation calculation :d
	quarterPounder_ = node_->GetRotation();
	node_->SetRotation(quarterOnion_);

	/*if (rotate)
	{
		vm.Clear();
		vm[RotateModelNode::P_NODE] = node_;
		vm[RotateModelNode::P_ROTATION] = quarterPounder_;
		vm[RotateModelNode::P_SPEED] = 4.0f;
		vm[RotateModelNode::P_SPEEDRAMP] = 4.0f;
		vm[RotateModelNode::P_STOPONCOMPLETION] = true;
		SendEvent(E_ROTATEMODELNODE, vm);
	}*/

	if (sendToServer)
	{return;
		msg_.Clear();
		msg_.WriteInt(clientID_);
		msg_.WriteString("MoveByTouch");
		msg_.WriteVector3(moveToLoc_);
		msg_.WriteVector3(dest);
		msg_.WriteFloat(moveToSpeed_);
		msg_.WriteFloat(speedRamp_);
		msg_.WriteFloat(gravity_);
		msg_.WriteFloat(gravityRamp_);
		msg_.WriteBool(moveToStopOnTime_);
		if (!isServer_)
		{
			network_->GetServerConnection()->SendMessage(MSG_LCMSG, true, true, msg_);
		}
		else
		{
			network_->BroadcastMessage(MSG_LCMSG, true, true, msg_);
		}
	}
}

void MoveByTouch::OnMoveToComplete()
{
	/*VariantMap vm;
	vm[SceneObjectMoveToComplete::P_NODE] = node_;
	SendEvent(E_SCENEOBJECTMOVETOCOMPLETE,vm);*/
	//todo implement stopOnCompletion = false
	//calculate direction based on previous location and keep going

	node_->GetComponent<RigidBody>()->SetFriction(10.0f);
	node_->GetComponent<RigidBody>()->ResetForces();
	//node_->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);

	VariantMap vm;
	vm[AnimateSceneNode::P_NODE] = node_;

	if (node_->GetVar("sex").GetBool())
	{
		vm[AnimateSceneNode::P_ANIMATION] = "idleF";
	}
	else
	{
		vm[AnimateSceneNode::P_ANIMATION] = "idleM";
	}

	vm[AnimateSceneNode::P_LOOP] = true;
	vm[AnimateSceneNode::P_LAYER] = 0;
	SendEvent(E_ANIMATESCENENODE, vm);

	/*VariantMap vm;
	vm[AnimateSceneNode::P_NODE] = node_;
	vm[AnimateSceneNode::P_ANIMATION] = "idleF";
	vm[AnimateSceneNode::P_LOOP] = true;
	vm[AnimateSceneNode::P_LAYER] = 0;
	SendEvent(E_ANIMATESCENENODE, vm);*/
}

void MoveByTouch::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	if (!scene_)
	{
		return;
	}

/*	if (!modelNode_)
	{
		return;
	}
*/
	using namespace NodeCollision;

	MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	//vectoria_ = modelNode_->GetPosition();
	vectoria_ = node_->LocalToWorld(node_->GetComponent<CollisionShape>()->GetPosition());
	vectoria_.y_ -= radius_;

	Vector3 finalcontactPosition = Vector3::ZERO;
	Vector3 finalcontactNormal = Vector3::ZERO;
	float finalcontactDistance = 0.0f;
	float finalcontactImpulse = 0.0f;

	while (!contacts.IsEof())
	{
		Vector3 contactPosition = contacts.ReadVector3();
		Vector3 contactNormal = contacts.ReadVector3();
		float contactDistance = contacts.ReadFloat();
		float contactImpulse = contacts.ReadFloat();

		if (contactPosition.y_ > finalcontactPosition.y_)
		{
			finalcontactPosition = contactPosition;
			finalcontactNormal = contactNormal;
			finalcontactDistance = contactDistance;
			finalcontactImpulse = contactImpulse;
		}
	}

	//float level = Abs(finalcontactNormal.y_);
	//if (level > 0.1f)
	{//todo check if above height, if so it's a ceiling, don't raise
		vectoria_.y_ += (finalcontactPosition.y_ - vectoria_.y_) + radius_;
		node_->SetPosition(vectoria_);
	}
	/*else//wall
	{
		vectoria_ = modelNode_->GetPosition();
		vectoria_ -= -modelNode_->GetDirection() * (finalcontactDistance * 1.01f);
		modelNode_->SetPosition(vectoria_);
		isMoving_ = false;
		OnMoveToComplete();
	}*/
}

void MoveByTouch::HandleLCMSG(StringHash eventType, VariantMap& eventData)
{
	const PODVector<unsigned char>& data = eventData[LcMsg::P_DATA].GetBuffer();
	MemoryBuffer msg(data);
	int clientID = msg.ReadInt();
	String lc = msg.ReadString();

	if (lc == "MoveByTouch")
	{
		if (clientID_ == clientID)
		{
			Vector3 loc = msg.ReadVector3();
			Vector3 dest = msg.ReadVector3();
			float speed = msg.ReadFloat();
			float speedRamp = msg.ReadFloat();
			float gravity = msg.ReadFloat();
			float gravityRamp = msg.ReadFloat();
			bool stopOnTime = msg.ReadBool();

			SubscribeToEvent(E_SETLAGTIME, HANDLER(MoveByTouch, HandleSetLagTime));

			VariantMap vm;
			vm[GetLagTime::P_CONNECTION] = conn_;
			SendEvent(E_GETLAGTIME, vm);

			//todo change to be asynchronous
			speedRamp += speed * lagTime_;
			gravityRamp += gravity * lagTime_;

			modelNode_->SetPosition(loc);

			MoveTo(dest, speed, speedRamp, gravity, gravityRamp, stopOnTime, true, false);

			if (isServer_)
			{
				//todo asynchronous messaging
				msg_.Clear();
				msg_.WriteInt(clientID_);
				msg_.WriteString("MoveByTouch");
				msg_.WriteVector3(moveToLoc_);
				msg_.WriteVector3(dest);
				msg_.WriteFloat(moveToSpeed_);
				msg_.WriteFloat(speedRamp_);
				msg_.WriteFloat(gravity_);
				msg_.WriteFloat(gravityRamp_);
				msg_.WriteBool(moveToStopOnTime_);

				vm.Clear();
				vm[ExclusiveNetBroadcast::P_EXCLUDEDCONNECTION] = conn_;
				vm[ExclusiveNetBroadcast::P_MSG] = msg_.GetBuffer();
				SendEvent(E_EXCLUSIVENETBROADCAST, vm);
			}
		}
	}
}

void MoveByTouch::HandleSetLagTime(StringHash eventType, VariantMap& eventData)
{
	Connection* sender = (Connection*)(eventData[SetLagTime::P_CONNECTION].GetPtr());
	if (sender == conn_)
	{
		lagTime_ = eventData[SetLagTime::P_LAGTIME].GetFloat();

		UnsubscribeFromEvent(E_SETLAGTIME);
	}
}

void MoveByTouch::HandleGetLc(StringHash eventType, VariantMap& eventData)
{
	if (!isMoving_)
	{
		return;
	}

	Node* clientNode = (Node*)(eventData[GetLc::P_NODE].GetPtr());

	if (clientNode == node_)
	{
		Connection* conn = (Connection*)(eventData[GetLc::P_CONNECTION].GetPtr());

		SubscribeToEvent(E_SETLAGTIME, HANDLER(MoveByTouch, HandleSetLagTime));

		VariantMap vm;
		vm[GetLagTime::P_CONNECTION] = conn;
		SendEvent(E_GETLAGTIME, vm);

		//todo change to be asynchronous
		float speedRamp = speedRamp_ + (moveToSpeed_ * lagTime_);
		float gravityRamp = gravityRamp_ + (gravity_ * lagTime_);

		msg_.Clear();
		msg_.WriteInt(clientID_);
		msg_.WriteString("MoveByTouch");
		msg_.WriteVector3(moveToLoc_);
		msg_.WriteVector3(moveToDest_);
		msg_.WriteFloat(moveToSpeed_);
		msg_.WriteFloat(speedRamp);
		msg_.WriteFloat(gravity_);
		msg_.WriteFloat(gravityRamp);
		msg_.WriteBool(moveToStopOnTime_);
		conn->SendMessage(MSG_LCMSG, true, true, msg_);
	}
}

void MoveByTouch::HandleMoveModelNode(StringHash eventType, VariantMap& eventData)
{
	Node* modelNode = (Node*)(eventData[MoveModelNode::P_NODE].GetPtr());

	if (modelNode == modelNode_)
	{
		Vector3 dest = eventData[MoveModelNode::P_DEST].GetVector3();
		float speed = eventData[MoveModelNode::P_SPEED].GetFloat();
		float speedRamp = eventData[MoveModelNode::P_SPEEDRAMP].GetFloat();
		float gravity = eventData[MoveModelNode::P_GRAVITY].GetFloat();
		float gravityRamp = eventData[MoveModelNode::P_GRAVITYRAMP].GetFloat();
		bool stop = eventData[MoveModelNode::P_STOPONCOMPLETION].GetBool();
		bool rotate = eventData[MoveModelNode::P_ROTATE].GetBool();
		bool sendToServer = eventData[MoveModelNode::P_SENDTOSERVER].GetBool();

		MoveTo(dest, speed, speedRamp, gravity, gravityRamp, stop, rotate, sendToServer);
	}
}

void MoveByTouch::HandleHudButt(StringHash eventType, VariantMap& eventData)
{
/*	Node* node = (Node*)(eventData[HudButt::P_NODE].GetPtr());

	if (node != node_)
	{
		return;
	}
*/
	UIElement* ele = static_cast<UIElement*>(eventData[HudButt::P_BUTT].GetPtr());

	String name = ele->GetName();

	if (name == "move")
	{
		focusedSprite_ = ssp_->focusedSprite_;

		if (focusedSprite_ != node_->GetName())
		{
			return;
		}

		if (!touchSubscribed_)
		{
			touchSubscribed_ = true;
			VariantMap vm;
			SendEvent(E_TOUCHSUBSCRIBE, vm);
		}

		SubscribeToEvent(E_TOUCHEND, HANDLER(MoveByTouch, HandleTouchEnd));
	}
}

void MoveByTouch::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	if (otherNode->GetName() == "sandDunes")
	{
		node_->GetComponent<RigidBody>()->SetGravityOverride(Vector3::ZERO);
	}
}

void MoveByTouch::HandleNodeCollisionEnd(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionEnd;

	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

	if (otherNode->GetName() == "sandDunes")
	{
		node_->GetComponent<RigidBody>()->SetGravityOverride(Vector3(0.0f, -9.81, 0.0f));
	}
}
