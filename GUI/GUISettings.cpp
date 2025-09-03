/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GUISettings.h"

#include <iostream>
#include <stdio.h>

#define CONF_ADDPROP(_type, _name) \
void IgnoreTouchpadSettings::Set##_name(_type value)\
{\
	_conf##_name = value;\
}\
\
_type IgnoreTouchpadSettings::_name()\
{\
	return _conf##_name;\
}\


CONF_ADDPROP(bool, Active)
CONF_ADDPROP(bigtime_t, Delay)
CONF_ADDPROP(int32, Mode)
#undef CONF_ADDPROP

IgnoreTouchpadSettings::IgnoreTouchpadSettings()
{
	_confActive = false;
	_confDelay = DEFAULT_DELAY;
	_confMode = Mode_All;
	BPath prefPath;

	find_directory(B_USER_SETTINGS_DIRECTORY, &prefPath);
	prefPath.Append(SETTINGS_FILE);
		
	_settingsFile.SetTo(prefPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	
	if (_settingsMessage.Unflatten(&_settingsFile) == B_OK){
	
		if (_settingsMessage.FindBool(AR_ACTIVE, &_confActive) != B_OK)
			fprintf(stderr, "IgnoreTouchpadSettings::IgnoreTouchpadSettings();\tFailed to load active boolean from settings file. Using default\n");
			
		if (_settingsMessage.FindInt64(AR_DELAY, &_confDelay) != B_OK)
			fprintf(stderr, "IgnoreTouchpadSettings::IgnoreTouchpadSettings();\tFailed to load delay from settings file. Using default\n");

		if (_settingsMessage.FindInt32(AR_MODE, &_confMode) != B_OK)
			fprintf(stderr, "IgnoreTouchpadSettings::IgnoreTouchpadSettings();\tFailed to load mode from settings file. Using default\n");
	}
	else
	{
		fprintf(stderr, "IgnoreTouchpadSettings::IgnoreTouchpadSettings();\nUnable to open settings file (either corrupted or doesn't exist), using defaults.\n");		
	}
	
	_settingsFile.Unset();
}

IgnoreTouchpadSettings::~IgnoreTouchpadSettings()
{
	BPath prefPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &prefPath);
	prefPath.Append(SETTINGS_FILE);
	
	//clobber existing settings and write in new ones
	_settingsFile.SetTo(prefPath.Path(), B_READ_WRITE | B_ERASE_FILE);


	//empty message and refill it with whatever has been set
	_settingsMessage.MakeEmpty();

	_settingsMessage.AddBool(AR_ACTIVE, _confActive);
	_settingsMessage.AddInt64(AR_DELAY, _confDelay);
	_settingsMessage.AddInt32(AR_MODE, _confMode);

	//write message to settings file
	if (_settingsMessage.Flatten(&_settingsFile) != B_OK)
		fprintf(stderr, "IgnoreTouchpadSettings::~IgnoreTouchpadSettings();\tError occurred writing settings\n");

	_settingsFile.Unset();	
}

