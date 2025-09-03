/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _IGNORE_TOUCHPAD_SETTINGS_H_
#define _IGNORE_TOUCHPAD_SETTINGS_H_

#include <SupportDefs.h>
#include "common.h"

// make adding configuration fields easier
#define CONF_ADDPROP(_type, _name) \
protected:\
		_type _conf##_name;\
public:\
		void Set##_name(_type value);\
		_type _name();

class IgnoreTouchpadSettings
{
	protected:
		BFile _settingsFile;
		BMessage _settingsMessage;
		

			
		BMessage openSettingsFile();
		void closeSettingsFile();
	
	public:
		IgnoreTouchpadSettings();
		~IgnoreTouchpadSettings();

CONF_ADDPROP(bool, Active)
CONF_ADDPROP(bigtime_t, Delay)
CONF_ADDPROP(int32, Mode)

};

#undef CONF_ADDPROP

#endif



#endif		// _IGNORE_TOUCHPAD_SETTINGS_H_
