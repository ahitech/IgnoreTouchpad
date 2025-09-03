/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GUIApp.h"
#include "GUIView.h"

#include <Catalog.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IgnoreTouchpadApp"


IgnoreTouchpadApp::IgnoreTouchpadApp()
	: BApplication( APP_SIG )
{
	B_TRANSLATE_MARK_SYSTEM_NAME_VOID("AutoRaise");

	removeFromDeskbar(NULL);
	fPersist = true;
	fDone = false;
}

IgnoreTouchpadApp::~IgnoreTouchpadApp()
{
	return;
}

void IgnoreTouchpadApp::ArgvReceived(int32 argc, char ** argv)
{
	BString option;

	for (int32 i = 1; i < argc; i++)
	{
		option = argv[i];
		if (option.IFindFirst("deskbar") != B_ERROR)
			fPersist = false;

		if (option.IFindFirst("persist") != B_ERROR)
			fPersist = true;

		if (option.IFindFirst("-h") != B_ERROR
			|| option.IFindFirst("help") != B_ERROR) {
			BString usageNote = 
				"\nUsage: " APP_NAME " [options]\n\t--deskbar\twill not open "
				"window, will just put " APP_NAME " into tray\n\t--persist (default) will put "
				APP_NAME " into tray such that it remains across reboots";
			puts(usageNote.String());
			fDone = true;
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		}
	}
}

void IgnoreTouchpadApp::ReadyToRun()
{
	if (!fDone) {
		PutInTray(fPersist);
	}
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
}

void IgnoreTouchpadApp::PutInTray(bool persist)
{
	BDeskbar db;

	if (!persist)
		db.AddItem(new GUIView);
	else {
		BRoster roster;
		entry_ref ref;
		roster.FindApp(APP_SIG, &ref);
		int32 id;
		db.AddItem(&ref, &id);
	}
}

int main()
{
	IgnoreTouchpadApp *app = new IgnoreTouchpadApp();
	app->Run();
}
