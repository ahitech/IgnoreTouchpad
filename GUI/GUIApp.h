/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once
 
#ifndef _IGNORE_TOUCHPAD_APP_H_
#define _IGNORE_TOUCHPAD_APP_H_

#include <Application.h>
#include <Archivable.h>

#include "common.h"
#include "GUISettings.h"


class IgnoreTouchpadApp: public BApplication{

	public:
		IgnoreTouchpadApp();
		virtual ~IgnoreTouchpadApp();
		virtual bool QuitRequested() { return true; }
		virtual void ArgvReceived(int32 argc, char ** args);		
		virtual void ReadyToRun();
		
		void PutInTray(bool);

	private:
		bool fPersist;
		bool fDone;
};

#endif		// _IGNORE_TOUCHPAD_APP_H_
