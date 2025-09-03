/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _GUI_VIEW_H_
#define _GUI_VIEW_H_

#include "common.h"
#include "GUISettings.h"

#include <InterfaceDefs.h>
#include <TranslationKit.h>
#include <Deskbar.h>
#include <List.h>
#include <InputDevice.h>
#include <OS.h>


#include <Roster.h>
#include <SupportDefs.h>

#include <stdio.h>

#include <AppKit.h>
#include <InterfaceKit.h>
#include <String.h>
#include <StorageKit.h>
#include <WindowInfo.h>



//exported instantiator function
extern "C" _EXPORT BView* instantiate_deskbar_item();


//since we can't remove the view from the deskbar from within the same thread
//as tray view, a thread will be spawned and this function called. It removed
//TrayView from the Deskbar
status_t removeFromDeskbar(void *);

class _EXPORT TrayView;


struct DeviceStructure {
	BInputDevice* device;
	bool		  enabled;
	uint		  number;
	
	virtual ~DeviceStructure() {if (device) { delete device; device = NULL; }};
};




class TrayView : 
	public BView
{
	protected:

		BBitmap *_activeIcon, *_inactiveIcon;
		bool fWatching;

		void _init(void); //initialization common to all constructors
		
		BList fInputDevices;
		void ListDevices();
		void BuildDevicesList(BList& );
		void Clean(BList*, bool cleaningGlobalDevices = false);

	public:
		volatile thread_id last_raiser_thread;
		team_id fDeskbarTeam;

		bigtime_t polling_delay;
		sem_id fPollerSem;
		thread_id poller_thread;

		TrayView();
		TrayView(BMessage *mdArchive);
		virtual ~TrayView();

		virtual status_t Archive(BMessage *data, bool deep = true) const;
		static TrayView *Instantiate(BMessage *data);

		virtual void Draw(BRect updateRect );
		virtual void AttachedToWindow();
		virtual void MouseDown(BPoint where);
		virtual void MessageReceived(BMessage* message);
		virtual void GetPreferredSize(float *w, float *h);

		void SetActive(bool);
};

int32 fronter(void *);
int32 poller(void *);

/*********************************************
	ConfigMenu derived from BPopUpMenu
	Provides the contextual left-click menu for the
	TrayView. Fires it's messages at the TrayView specified
	in it's constructor;
	Also, it's by default set to asynchronously destruct,
	so it's basically a fire & forget kinda fella.
*********************************************/

class ConfigMenu: public BPopUpMenu{
	private:

	public:
		ConfigMenu(TrayView *tv, bool useMag);
		virtual ~ConfigMenu();
};



#endif // _GUI_VIEW_H_
