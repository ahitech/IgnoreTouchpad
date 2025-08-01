/*
	Copyright 2025, Alexey "Hitech" Burshtein.   All Rights Reserved.
	This file may be used under the terms of the MIT License.
*/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Errors.h>
#include <FindDirectory.h>
#include <Locker.h>
#include <Messenger.h>
#include <String.h>
#include <iostream>
#include <unordered_map>

const char* fileName = "IgnoreTouchpad";

const 

struct DeviceInfo {
	BString name;
	bool disabled;
	
	DeviceInfo(BString in) {
		name = in;
		disabled = false;
	}
}


class Settings {
public:
	Settings(BMessenger* target = NULL, bool startMonitoring = false);
	~Settings();
	
	void Save() const;
	void Load();
	
	bool GetStatus(BString deviceName);
	
	status_t StartMonitoring();
	void StopMonitoring();
	void SetNotifyTarget(BMessenger* target);
	
protected:
	std::unordered_map<BString, bool> fDevicesStatus;
	BMessenger*	fTarget;
	bool	fMonitoringActive;
	BLocker	fLock;
	
	status_t	CreateSettingsFile() const;
	BPath* GetPathToSettingsFile();
}

#endif // _SETTINGS_H_
