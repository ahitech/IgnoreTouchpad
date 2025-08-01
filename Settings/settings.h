/*
	Copyright 2025, Alexey "Hitech" Burshtein.   All Rights Reserved.
	This file may be used under the terms of the MIT License.
*/

/**
 * @file settings.h
 * @brief Settings manager for Ignore Touchpad project.
 *
 * This module is responsible for storing, loading, and monitoring user
 * settings (ignored input devices) using Haiku's flattened BMessage format.
 *
 * @defgroup SettingsModule settings
 * @brief Classes and functions for reading/writing Ignore Touchpad settings.
 *
 * This group contains all classes related to the configuration of ignored
 * input devices. It includes the Settings manager and its helpers.
 * @{
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Errors.h>
#include <Locker.h>
#include <Messenger.h>
#include <String.h>
#include <iostream>
#include <unordered_map>


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
	
	void Save() const;		
	void Load();			
	
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
	
	//! File name of the settings file
	const char* fFileName = "IgnoreTouchpad";
};

#endif // _SETTINGS_H_
/** @} */ // end of SettingsModule
