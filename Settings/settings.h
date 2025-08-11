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
#include <Node.h>
#include <String.h>
#include <iostream>
#include <unordered_map>
#include <vector>


/**	\struct		DeviceInfo
 *	\brief		This struct holds a single device and its status (is it currently connected,
 *				is it currently ignored).
 *	\details	The purpose of this structure is holding status of a single device: its name,
 *				whether it's connected or not right now, whether its input is ignored.
 */
struct DeviceInfo {
	BString		DeviceName;		//!<	Name of the input device
	bool		IsConnected;	//!<	Is the device currently connected? Yes = "true".
	bool		IsIgnored;		//!<	Is the device's input ignored? Yes = "true".

	//!		copydoc	DeviceInfo::DeviceInfo	
	DeviceInfo(BString name, bool connected = true, bool ignored = false);
	
	//!		copydoc	DeviceInfo::ToBMessage
	status_t ToBMessage(BMessage* ) const;
	
	//!		copydoc	DeviceInfo::FromBMessage
	status_t FromBMessage(const BMessage* );
	
	//!		Printing debugging information
	void DebugPrint(void) const;
};


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
	
	status_t StartMonitoring();		//!<	\copydoc	Settings::StartMonitoring
	void StopMonitoring();			//!<	\copydoc	Settings::StopMonitoring
	
	//!	\copydoc	Settings::SetNotifyTarget
	void SetNotifyTarget(BMessenger* target);
	
	//!	\copydoc	Settings::GetCurrentlyAttachedDevices
	std::vector<DeviceInfo> GetCurrentlyAttachedDevices() const;
	
	//!	\copydoc	Settings::GetMergedListOfDevices
	std::vector<DeviceInfo> GetMergedListOfDevices() const;
	
protected:
	/**
	 *	\brief		Structure that holds status of individual devices.
	 *	\see		DeviceInfo
	 */
	std::vector<DeviceInfo> fDevicesStatus;
	
	BMessenger*	fTarget;	//!<	What BMessenger should be notified? Can be `NULL`.
	bool	fMonitoringActive;	//!< `true` if monitoring is currently active, `false` otherwise.
	//!	Used for updating the settings. Probably overkill, since I use BFile::Lock() as well.
	BLocker	fLock;			
	
	// Service function for creating a default file with settings. All devices are enabled.
	// status_t	CreateSettingsFile() const;
	
	BPath* GetPathToSettingsFile() const;	//!<	\copydoc	Settings::GetPathToSettingsFile
	
	//! File name of the settings file
	const char* fFileName = "IgnoreTouchpad";
	
	//! Node reference to the settings file. Used for monitoring the file.
	node_ref fNodeRef;
	
	
};

#endif // _SETTINGS_H_
/** @} */ // end of SettingsModule
