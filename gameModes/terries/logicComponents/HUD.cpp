/*
 * HUD.cpp
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
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

#include "HUD.h"
#include "../../../network/NetworkConstants.h"
#include "../../../Constants.h"

HUD::HUD(Context* context, Urho3DPlayer* main) :
LogicComponent(context)
{
	main_ = main;
}

HUD::~HUD()
{
	if (hud_)
	{
		main_->ui_->GetRoot()->RemoveChild(hud_);
	}
}

void HUD::Start()
{
	if (!main_->IsLocalClient(node_))
	{
		return;
	}

	XMLFile* style = main_->cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
	main_->ui_->GetRoot()->SetDefaultStyle(style);

	hud_ = main_->ui_->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/terriesHud.xml"));
	main_->ui_->GetRoot()->AddChild(hud_);
	main_->RecursiveAddGuiTargets(hud_);
	main_->ElementRecursiveResize(hud_);

	for (int x = 0; x < hud_->GetNumChildren(); x++)
	{
		if (hud_->GetChild(x)->GetTypeName() == "Button")
		{
			SubscribeToEvent(hud_->GetChild(x), E_RELEASED, HANDLER(HUD, HandleButtonRelease));
		}
	}
}

void HUD::HandleButtonRelease(StringHash eventType, VariantMap& eventData)
{
	using namespace Released;

	UIElement* ele = static_cast<UIElement*>(eventData[Released::P_ELEMENT].GetPtr());

	VariantMap vm;
	vm[HudButt::P_NODE] = node_;
	vm[HudButt::P_BUTT] = ele;
	SendEvent(E_HUDBUTT, vm);
}
