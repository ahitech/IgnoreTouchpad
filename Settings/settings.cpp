/*
	Copyright 2025, Alexey "Hitech" Burshtein.   All Rights Reserved.
	This file may be used under the terms of the MIT License.
*/

/**
 * @file settings.cpp
 * @brief Implementation of the Settings class.
 * @ingroup SettingsModule
 *
 * This file contains the implementation of methods that handle
 * saving/loading settings and monitoring the settings file.
 */

#include "settings.h"

#include <FindDirectory.h>
#include <InputDevice.h>
#include <List.h>
#include <NodeMonitor.h>

#include <stdio.h>


/**	\brief		Constructor of the device information, for easier initialization.
 *	\param[in]	name		Name of the device.
 *	\param[in]	connected	"true" if the device is connected (default), "false" otherwise.
 *	\param[in]	ignored		"true" if the input from the device is ignored,
 *							"false" otherwise (default).
 */
DeviceInfo::DeviceInfo(BString name, bool connected = true, bool ignored = false) {
	DeviceName = name;
	IsConnected = connected;
	IsIgnored = ignored;
}


/**	\brief		Inserts the data from this structure into BMessage.
 *	\param[out]	out		Pointer to the BMessage which should be filled with data.
 *	\returns	B_OK			If everything went good
 *				B_BAD_VALUE		If the input pointer is NULL
 */
status_t	DeviceInfo::ToBMessage(BMessage* out) const {
	if (!out)	{ return B_BAD_VALUE; }
	
	out->what = 'DEVI';
	out->AddString("name", DeviceName);
	out->AddBool("ignored", IsIgnored);
	out->AddBool("connected", IsConnected);
	return	B_OK;
}


/**	\brief		Reads the data about a single input device from a BMessage.
 *	\param[in]	in		Pointer to the BMessage which should be parsed.
 *	\returns	B_OK			If everything went good
 *				B_BAD_VALUE		If the input pointer is NULL
 *				B_BAD_TYPE		If the input BMessage has incorrect "what" field
 *				B_NAME_NOT_FOUND	If the name of the device is not found in the BMessage
 *	\note		Both boolean values are not required for successful initialization,
 *				the convention is "IsConnected = false" and "IsIgnored = false".
 */
status_t	DeviceInfo::FromBMessage(const BMessage* in) {
	if (!in)	return	B_BAD_VALUE;
	if (in->what != 'DEVI')	return B_BAD_TYPE;
	
	if (B_OK != in->FindString("name", &DeviceName))	return B_NAME_NOT_FOUND;
	if (B_OK != in->FindBool("ignored", &IsIgnored))		IsIgnored = false;
	if (B_OK != in->FindBool("connected", &IsConnected))	IsConnected = false;
	return B_OK;
}


/**	\brief		This function was copied directly from the BeBook.
 *	\see		InputServer and InputDevice
 *	\returns	Whatever, no-one bothers to check the return value, even in the example :)
 */
static bool del_InputDevice(void *ptr)
{
	if (ptr) {
		BInputDevice *dev = (BInputDevice *)ptr;
		delete dev;
		return false;
	}
	return true;
}


/**	\brief		Constructor.
 *	\param[in]	target		BMessenger to be notified when the settings file is updated.
 *	\param[in]	startMonitoring		If `true`, the monitoring is started right away.
 *									Requires `target` to be not `NULL`.
 */
Settings::Settings(BMessenger* target = NULL, bool startMonitoring = false) :
		fLock("Monitoring")
{
	fDevicesStatus.clear();
	this->SetNotifyTarget(target);
	if (target && startMonitoring) {
		StartMonitoring();	
	}
}


/**	\brief		Destructor
 *	\details	Just clears some memory. Probably unnecessary, but a good practice anyway.
 */
Settings::~Settings() {
	if (fMonitoringActive)	{	StopMonitoring(), SetNotifyTarget(NULL); 	}
	fDevicesStatus.clear();
}


/**	\brief		Updates the target to send notifications to.
 *	\note		If monitoring was active, it is stopped <b><u>and not restarted</u></b>.
 */
void Settings::SetNotifyTarget(BMessenger* in) {
	if (fMonitoringActive) {
		StopMonitoring();
	}
	fTarget = in;
}


/**	\brief		Starts monitoring the settings file for changes.
 *	\details	When the settings file is updated, the target BMessenger
 *				receives a message from Storage Kit.
 *	\see		watch_node()
 *	\returns	B_OK 			If monitoring is already active or if it was started successfully.
 *				B_BAD_HANDLER	If the target BMessenger is `NULL`.
 *				B_BAD_VALUE		If Settings::GetPathToSettingsFile() returned NULL.
 *				B_ENTRY_NOT_FOUND	If the settings file couldn't be created for some reason.
 *				Some other error	If could not get node ref inside critical section
 *									or if monitoring could not be started.
 */
status_t Settings::StartMonitoring() {
	if (! fTarget) { return B_BAD_HANDLER; }
	if (fMonitoringActive) { return B_OK; }
	
	BPath* pathToSettingsFile = GetPathToSettingsFile();
	if (! pathToSettingsFile) { return B_BAD_VALUE; }
	
	// ---==< Entering critical section >==--- 
	fLock.Lock();
	
	// Find the settings file
	BEntry entry(pathToSettingsFile.Path(), true);
	if (!entry.Exists()) {
		// There's no settings file
		if ( B_OK != CreateSettingsFile()) {
			fLock.Unlock();
			return B_ENTRY_NOT_FOUND;
		}
	}
	
	// Get the node reference
	status_t status = entry.GetNodeRef(&fNodeRef);
    if (B_OK != status) { fLock.Unlock(); return status; }

    // Start monitoring
	status = watch_node(&fNodeRef, B_WATCH_STAT, target);
	if (status == B_OK) {
		fMonitoringActive = true;
    }

	fLock.Unlock();
	// ---==< Exitting critical section >==---

    return status;
}


/**	\brief		Stop watching the settings file for changes
 *	\note		This function does not check the return value of `watch_node()`,
 *				because, well, why bother?
 */
void Settings::StopMonitoring() {
	if (fTarget && fMonitoringActive) {
		watch_node(&fNodeRef, B_STOP_WATCHING, fTarget);
		fMonitoringActive = false;
	}
}


/**	\brief		Builds the path to the settings file.
 *	\returns	Pointer to BPath object, or `NULL` if something goes wrong.
 *	\note		The caller is responsible for freeing the returned object!
 *	\note		Nothing guarantees that the file exists. Check first!
 */
BPath*	Settings::GetPathToSettingsFile() {
	BPath* pathToSettingsFile = new BPath();
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, pathToSettingsFile)) {
	if (B_OK != status) { return NULL; }
	pathToSettingsFile.Append(fFileName);
	return pathToSettingsFile;
}


status_t	Settings::CreateSettingsFile() {
	BInputDevice* device = NULL;
	BMessage toSave = new BMessage('CONF');
	
	
	// Obtain the list of all devices
	BList devices;
	status = get_input_devices(&devices);
	if (B_OK != status) { file.Unlock(); return status; }
	
	device = devices.ItemAt(i);
		if (!device ||
			device->Type() != B_POINTING_DEVICE)
		{ i++; continue; }
	
	// Insert them into the unordered map
	
	
	// Dispose of the device list.
	list_o_devices.DoForEach (del_InputDevice);
	list_o_devices.MakeEmpty();

	
	
	printf("[SettingsMonitor] Created new settings file: %s\n", fPath.Path());
	return B_OK;
}


/**	\brief		Load current settings from the settings file
 *	\note		Settings include only names of the devices and boolean flags for
 *				them to be ignored or allowed.
 *	\note		The received list is always a merge of data stored in the settings
 *				file and of currently connected devices. Users, as peripherials of
 *				chaotic input, always connects and disconnects things, shame on them.
 *				Therefore the logic is as follows:
 *				  - If the device exists in the list of currently connected devices,
 *					it will appear in the output.
 *				  - If its name was stored in the settings, its "ignored" value will
 *					also be read, otherwise it will be "false".
 *				  - If it is not connected, but appears in the settings file, things
 *					are getting interesting. Current behavior is TBD. :) 
 */
void Settings::Load() {
	BMessage readFrom;
	BFile settingsFile;
	
	// Get the path
	BPath* pathToSettingsFile = this->GetPathToSettingsFile();
	if (! pathToSettingsFile) {
		fprintf(stderr, "[Settings Load] Could not build path to settings file.\n");
		return;
	}
	
	// Set settings file to that path
	settingsFile.SetTo(pathToSettingsFile->Path());
	if (settingsFile.InitCheck() != B_OK) {
		fprintf(stderr, "[Settings Load] Initialization of BFile failed.\n");
		delete pathToSettingsFile;
		return;
	}	
	delete pathToSettingsFile;
	
	// Unflatten the file into BMessage (under lock)
	settingsFile.Lock();	
	readFrom.Unflatten(&settingsFile);
	settingsFile.Unlock();
	
	// Sanity check
	if (readFrom.what() != 'CONF') {
		fprintf(stderr,
			"[Settings Load] The BMessage stored in the settings file has wrong 'what'.\n");
		return;
	}
	
	// Populate the devices map
	int32 i = 0;
	BMessage individualDevice;
	while (readFrom.FindMessage("device", i++, &individualDevice) == B_OK) {
		BString deviceName;
		bool deviceIgnored;
		individualDevice.FindString("name", &deviceName);
		individualDevice.FindBool("ignored", &deviceIgnored);
		fDevicesStatus.insert(std::makepair(deviceName, deviceIgnored));
	}
	fprintf(stdout, "[Settings Load] %d devices loaded from settings, "
			"size of map is %d", i, fDevicesStatus.size());
	
	//
	
	
}


/**	\brief		Save current settings into the settings file
 */
void Settings::Save() {
	// Get path to settings file
	BPath* pathToSettingsFile = GetPathToSettingsFile();
	if (! pathToSettingsFile) { 
		fprintf(stderr, "[Settings Create settings file] Could not get path to settings file\n");
		return;
	}

	// Initialize the settings file
	BFile file(fPath.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK) { 
		fprintf (stderr, "[Settings Save] Couldn't initialize settings file\n");
		delete pathToSettingsFile;
		return;
	}
	file.Lock();		// <==== LOCK THE FILE FROM ALL OTHER ACCESSES!
	
	// Populate the BMessage toSave with pointing devices
	BMessage* toSave = new BMessage('CONF');
	int32 listSize = devices.CountItems();
	int32 i = 0;
	for (const auto& pair : fDevicesStatus) {		
		BMessage singleDevice('DEVI');
		singleDevice.AddString("name", pair.first);
		singleDevice.AddBool("ignored", false);
		toSave->AddMessage("device", &singleDevice);
	}
	
	// Save the BMessage with settings
	status = toSave.Flatten(&file);
	file.Unlock();		// <==== UNLOCK THE FILE, enable access
	
	delete pathToSettingsFile;
}
