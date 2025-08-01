/*
	Copyright 2025, Alexey "Hitech" Burshtein.   All Rights Reserved.
	This file may be used under the terms of the MIT License.
*/

#include "settings.h"

#include <FindDirectory.h>
#include <InputDevice.h>
#include <List.h>

#include <stdio.h>

static bool del_InputDevice(void *ptr)
{
	if (ptr) {
		BInputDevice *dev = (BInputDevice *)ptr;
		delete dev;
		return false;
	}
	return true;
}



void SomeFunc( void )
{
   // Get a list of all input devices.
   BList list_o_devices;

   status_t retval = get_input_devices( &list_o_devices );
   if( retval != B_OK ) return;

   // Do something with the input devices.
   ...

   // Dispose of the device list.
   list_o_devices.DoForEach( del_InputDevice );
   list_o_devices.MakeEmpty();
}


Settings::Settings(BMessenger* target = NULL, bool startMonitoring = false) :
		fLock("Monitoring")
{
	fDevicesStatus.clear();
	this->SetNotifyTarget(target);
	if (startMonitoring) {
		StartMonitoring();	
	}
}


Settings::~Settings() {
	fDevicesStatus.clear();
}



void Settings::SetNotifyTarget(BMessenger* in) {
	fTarget = in;
}


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
	node_ref nref;
	status_t status = entry.GetNodeRef(&nref);
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


BPath*	Settings::GetPathToSettingsFile() {
	BPath* pathToSettingsFile = new BPath();
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, pathToSettingsFile)) {
	if (B_OK != status) { return status; }
	pathToSettingsFile.Append(fileName);
	return pathToSettingsFile;
}


status_t	Settings::CreateSettingsFile() {
	BInputDevice* device = NULL;
	BMessage toSave = new BMessage('CONF');
	BPath* pathToSettingsFile = GetPathToSettingsFile();
	if (! pathToSettingsFile) { return B_BAD_VALUE; }

	// Initialize the settings file
	BFile file(fPath.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK) { return status; }
	file.Lock();		// <==== LOCK THE FILE FROM ALL OTHER ACCESSES!
	
	// Obtain the list of all devices
	BList devices;
	status = get_input_devices(&devices);
	if (B_OK != status) { file.Unlock(); return status; }
	
	// Populate the BMessage toSave with pointing devices
	int32 listSize = devices.CountItems();
	int32 i = 0;
	while (i < listSize) {
		device = devices.ItemAt(i);
		if (!device ||
			device->Type() != B_POINTING_DEVICE)
		{ i++; continue; }
		
		BMessage singleDevice(i);		// The "what" field will be the number of device
		singleDevice.AddString("name", device->Name());
		singleDevice.AddBool("ignored", false);
		toSave->AddMessage(&singleDevice);
	}	
	
	// Dispose of the device list.
	list_o_devices.DoForEach( del_InputDevice );
	list_o_devices.MakeEmpty();

	// Save the BMessage with settings
	status = toSave.Flatten(&file);
	file.Unlock();		// <==== UNLOCK THE FILE, enable access
	if (status != B_OK) { return status; }
	
	printf("[SettingsMonitor] Created new settings file: %s\n", fPath.Path());
	return B_OK;
}



// Получаем список устройств ввода
    input_device_ref devices[32];
    int32 count = 0;
    status_t status = 
    if (status != B_OK)
        return status;
