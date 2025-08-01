/*
	Copyright 2025, Alexey "Hitech" Burshtein.   All Rights Reserved.
	This file may be used under the terms of the MIT License.
*/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Errors.h>
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


/**	\class 		Settings
 *	\brief		The main purpose of the library
 *	\details	Provides interface for reading and writing the settings file.
 *				Allows the add-on to receive live updates if monitoring is active.
 */
class Settings {
public:
	//!	\copydoc	Settings::Settings
	Settings(BMessenger* target = NULL, bool startMonitoring = false);
	//!	\copydoc	Settings::~Settings
	~Settings();
	
	void Save() const;		//!<	\copydoc	Settings::Save
	void Load();			//!<	\copydoc	Settings::Load
	
	//!	\copydoc	Settings::GetStatus
	bool GetStatus(BString deviceName);
	
	status_t StartMonitoring();	//!<	\copydoc	Settings::StartMonitoring
	void StopMonitoring();		//!<	\copydoc	Settings::StopMonitoring
	
	//!	\copydoc	Settings::SetNotifyTarget
	void SetNotifyTarget(BMessenger* target);
	
protected:
	/**
	 *	\brief		Structure that holds status of individual devices.
	 *	\li **key** (`BString`) — the device name as reported by input_server.
	 *	\li **value** (`bool`) — `true` if the device is ignored, `false` otherwise.
	 */
	std::unordered_map<BString, bool> fDevicesStatus;
	
	BMessenger*	fTarget;	//!<	What BMessenger should be notified? Can be `NULL`.
	bool	fMonitoringActive;	//!< `true` if monitoring is currently active, `false` otherwise.
	//!	Used for updating the settings. Probably overkill, since I use BFile::Lock() as well.
	BLocker	fLock;			
	
	//! Service function for creating a default file with settings. All devices are enabled.
	status_t	CreateSettingsFile() const;
	//! Service function that builds path to the settings file.
	BPath* GetPathToSettingsFile();
}

#endif // _SETTINGS_H_
