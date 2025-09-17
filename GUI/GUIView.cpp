/*
 * Copyright 2025, Alexey "Hitech" Burshtein <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GUIView.h"

//#define DEBUG 1
#include <BeBuild.h>

// #include "AutoRaiseIcon.h"

#include <stdlib.h>
#include <Catalog.h>
#include <DataIO.h>
#include <Screen.h>
#include <View.h>
#include <Debug.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IgnoreTouchpadIcon"


extern "C" _EXPORT BView *instantiate_deskbar_item(void)
{
	puts(B_TRANSLATE("Instanciating IgnoreTouchpad TrayView..."));
	return (new TrayView);
}


status_t removeFromDeskbar(void *)
{
	BDeskbar db;
	if (db.RemoveItem(APP_NAME) != B_OK) {
		fprintf(stderr, B_TRANSLATE("Unable to remove AutoRaise from BDeskbar\n"));
	}

	return B_OK;
}


static status_t
our_image(image_info& image)
{
	int32 cookie = 0;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &image) == B_OK) {
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}


//**************************************************

ConfigMenu::ConfigMenu(TrayView *tv, bool useMag)
	:BPopUpMenu("config_popup", false, false){

	BMenu *tmpm;
	BMenuItem *tmpi;
	BMessage *msg;

//	AutoRaiseSettings *s = tv->Settings();

	SetFont(be_plain_font);
	
	BList devicesList;
	BuildDevicesList(devicesList);
	int itemsCount = devicesList.Count();
	
	for (int i = 0; i < itemsCount; ++i) {
		msg = new BMessage('TOGL');
		msg->AddInt16("device", i);
		DeviceStructure* currentDevice = static_cast<DeviceStructure*>devicesList.ItemAt(i);
		if (!currentDevice) { continue; }
		tmpi = new BMenuItem(currentDevice->deviceName.String(), tmpm);
		
		// If the device is active, its item is checked
		if (currentDevice->enabled) {
			tmpi->SetMarked();
		}
		
		// Don't allow disabling the last active pointing device
		if (itemsCount == 1 && currentDevice->enabled) {
			tmpi->SetEnabled(false);
		}
		
		AddItem(tmpi);
	}
	
	AddSeparatorItem();
	
	msg = new BMessage('ENAA');
	tmpi = new BMenuItem(B_TRANSLATE("Enable all"), msg);
	AddItem(tmpi);

	AddSeparatorItem();

	AddItem(new BMenuItem(B_TRANSLATE("About " APP_NAME B_UTF8_ELLIPSIS),
		new BMessage(B_ABOUT_REQUESTED)));
	AddItem(new BMenuItem(B_TRANSLATE("Remove from tray"),
		new BMessage(REMOVE_FROM_TRAY)));

	SetTargetForItems(tv);
	SetAsyncAutoDestruct(true);
}

ConfigMenu::~ConfigMenu() {}

//************************************************

TrayView::TrayView()
	:BView(BRect(0, 0, B_MINI_ICON, B_MINI_ICON -1), "AutoRaise", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW){
	_init(); 	//Initialization common to both constructors
}

//Rehydratable constructor
TrayView::TrayView(BMessage *mdArchive):BView(mdArchive){
	_init();		//As above
}

void TrayView::GetPreferredSize(float *w, float *h)
{
	*w = B_MINI_ICON;
	*h = B_MINI_ICON - 1;
}

void TrayView::_init()
{
	thread_info ti;

	watching = false;
	_settings = new AutoRaiseSettings;

	raise_delay = _settings->Delay();
	current_window = 0;
	polling_delay = 100000;
	fPollerSem = create_sem(0, "AutoRaise poller sync");
	last_raiser_thread = 0;
	fNormalMM = mouse_mode();

	_activeIcon = NULL;
	_inactiveIcon = NULL;

	get_thread_info(find_thread(NULL), &ti);
	fDeskbarTeam = ti.team;


	image_info info;
	{
		status_t result = our_image(info);
		if (result != B_OK) {
			printf("Unable to lookup image_info for the AutoRaise image: %s\n",
				strerror(result));
			removeFromDeskbar(NULL);
			return;
		}
	}

	BFile file(info.name, B_READ_ONLY);
	if (file.InitCheck() != B_OK) {
		printf("Unable to access AutoRaise image file: %s\n",
			strerror(file.InitCheck()));
		removeFromDeskbar(NULL);
		return;
	}

	BResources res(&file);
	if (res.InitCheck() != B_OK) {
		printf("Unable to load image resources: %s\n",
			strerror(res.InitCheck()));
		removeFromDeskbar(NULL);
		return;
	}

	size_t bmsz;
	char *p;

	p = (char *)res.LoadResource('MICN', ACTIVE_ICON, &bmsz);
	_activeIcon = new BBitmap(BRect(0, 0, B_MINI_ICON-1, B_MINI_ICON -1),
		B_CMAP8);
	if (p == NULL) {
		puts("ERROR loading active icon");
		removeFromDeskbar(NULL);
		return;
	}
	_activeIcon->SetBits(p, B_MINI_ICON * B_MINI_ICON, 0, B_CMAP8);

	p = (char *)res.LoadResource('MICN', INACTIVE_ICON, &bmsz);
	_inactiveIcon = new BBitmap(BRect(0, 0, B_MINI_ICON-1, B_MINI_ICON -1),
		B_CMAP8);
	if (p == NULL) {
		puts("ERROR loading inactive icon");
		removeFromDeskbar(NULL);
		return;
	}
	_inactiveIcon->SetBits(p, B_MINI_ICON * B_MINI_ICON, 0, B_CMAP8);

	SetDrawingMode(B_OP_ALPHA);
	SetFlags(Flags() | B_WILL_DRAW);

	// begin watching if we want
	// (doesn't work here, better do it in AttachedToWindow())
}

TrayView::~TrayView(){
	status_t ret;

	if (watching) {
		set_mouse_mode(fNormalMM);
		watching = false;
	}
	delete_sem(fPollerSem);
	wait_for_thread(poller_thread, &ret);
	if (_activeIcon) delete _activeIcon;
	if (_inactiveIcon) delete _inactiveIcon;
	if (_settings) delete _settings;

	return;
}

// Dehydrate into a message (called by the DeskBar)
status_t TrayView::Archive(BMessage *data, bool deep) const {
	status_t error=BView::Archive(data, deep);
	data->AddString("add_on", APP_SIG);

	return error;
}

// Rehydrate the View from a given message (called by the DeskBar)
TrayView *TrayView::Instantiate(BMessage *data) {

	if (!validate_instantiation(data, "TrayView")) {
		return NULL;
	}

	return (new TrayView(data));
}

void TrayView::AttachedToWindow() {
	if(Parent())
		SetViewColor(Parent()->ViewColor());
	if (_settings->Active()) {
		fNormalMM = mouse_mode();
		set_mouse_mode(B_FOCUS_FOLLOWS_MOUSE);
		release_sem(fPollerSem);
		watching = true;
	}
}

void TrayView::Draw(BRect updaterect) {
	BRect bnds(Bounds());

	if (Parent()) SetHighColor(Parent()->ViewColor());
	else SetHighColor(189, 186, 189, 255);
	FillRect(bnds);

	if (_settings->Active())
	{
		if (_activeIcon) DrawBitmap(_activeIcon);
	}
	else
	{
		if (_inactiveIcon) DrawBitmap(_inactiveIcon);
	}
}

void TrayView::MouseDown(BPoint where) {
	BWindow *window = Window();	/*To handle the MouseDown message*/
	if (!window)	/*Check for proper instantiation*/
		return;

	BMessage *mouseMsg = window->CurrentMessage();
	if (!mouseMsg)	/*Check for existence*/
		return;

	if (mouseMsg->what == B_MOUSE_DOWN) {
		/*Variables for storing the button pressed / modifying key*/
		uint32 	buttons = 0;
		uint32  modifiers = 0;

		/*Get the button pressed*/
		mouseMsg->FindInt32("buttons", (int32 *) &buttons);
		/*Get modifier key (if any)*/
		mouseMsg->FindInt32("modifiers", (int32 *) &modifiers);

		/*Now perform action*/
		switch(buttons) {
			case B_PRIMARY_MOUSE_BUTTON:
			{
				ConvertToScreen(&where);

				//menu will delete itself (see constructor of ConfigMenu),
				//so all we're concerned about is calling Go() asynchronously
				ConfigMenu *menu = new ConfigMenu(this, false);
				menu->Go(where, true, true, ConvertToScreen(Bounds()), true);

				break;
			}
			case B_SECONDARY_MOUSE_BUTTON:
			{
				ConvertToScreen(&where);

				//menu will delete itself (see constructor of ConfigMenu),
				//so all we're concerned about is calling Go() asynchronously
				ConfigMenu *menu = new ConfigMenu(this, false);
				menu->Go(where, true, true, ConvertToScreen(Bounds()), true);

				break;
			}
		}
	}
}

void TrayView::MessageReceived(BMessage* message)
{
	BMessenger msgr;

	BAlert *alert;
	bigtime_t delay;
	int32 mode;

	switch(message->what)
	{
		case 'TOGL':
			int deviceId = 0;
			if (B_OK != message->FindInt16("device", &deviceId)) break;
			Toggle(deviceId);
			break;
		case 'ENAA':
			EnableAll();
			break;
		case REMOVE_FROM_TRAY:
		{
			thread_id tid = spawn_thread(removeFromDeskbar, "RemoveFromDeskbar", B_NORMAL_PRIORITY, (void*)this);
			if (tid != 0) resume_thread(tid);

			break;
		}
		case B_ABOUT_REQUESTED:
			alert = new BAlert("about box",
				B_TRANSLATE("Ignore Touchpad, © 2025, Alexey \"Hitech\" Burshtein\n"
							"Derived from \"AutoRaise\" by mmu_man\nEnjoy :-)"),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
			alert->SetShortcut(0, B_ENTER);
			alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
			alert->Go(NULL); // use asynchronous version
			break;
		case OPEN_SETTINGS:

			break;

		default:
			BView::MessageReceived(message);
	}
}

AutoRaiseSettings *TrayView::Settings() const
{
	return _settings;
}

void TrayView::Toggle(int deviceNo)
{
	BList devList;
	BuildDevicesList(devList);
	int totalItems = devList.CountItems();
	for (int i = 0; i < totalItems; i++) {
		if (i == deviceNo) {
			DeviceStructure* devStruct = static_cast<DeviceStructure>(devList.ItemAt(i));
			if (devStruct) {
				if (devStruct->enabled && totalItems > 1) {
					device->Stop();
				}
				if (!devStruct->enabled) {
					device->Start();
				}
			}
		}
	}
	Clean(&toReturn, true);
}


void TrayView::EnableAll() {
	BList devList;
	BuildDevicesList(devList);
	int totalItems = devList.CountItems();
	for (int i = 0; i < totalItems; i++) {
		DeviceStructure* devStruct = static_cast<DeviceStructure>(devList.ItemAt(i));
		if (devStruct) {
			if (!devStruct->enabled) {
				device->Start();
			}
		}
	}
	Clean(&toReturn, true);
}



void BuildDevicesList(BList& toReturn) {
	Clean(&toReturn, true);
	BList devices;
    status_t err = get_input_devices(&devices);
    if (err != B_OK) {
        fprintf(stderr, "[BuildListOfDevices] Failed to get input devices: %s\n", strerror(err));
    }
    
    int count = devices.CountItems();
    for (int32 i = count - 1; i >= 0; i--) {
        BInputDevice* info = static_cast<BInputDevice*>(devices.ItemAt(i));
        if (!info || info->Type() != B_POINTING_DEVICE) {
        	devices.RemoveItem(i);
        	delete info;
        }
    }
    
    count = devices.CountItems();
    for (uint i = 0; i < count; i++) {
    	DeviceStructure* device = new DeviceStructure();
    	device->device = static_cast<BInputDevice*>(devices.ItemAt(i));
    	device->name.SetTo(device->device->Name());
    	device->enabled = device->device->IsRunning();
    	device->number = i;
    	toReturn.AddItem(device, i);
    }
    
    this->Clean(&devices, false);
}


void Clean(BList *in, bool cleaningGlobalDevices = false) {
	if (!in) return;
	uint count = in->CountItems();
	for (int32 i = count-1; i >=0; i--) {
		if (cleaningGlobalDevices) {
			DeviceStructure* delStr = static_cast<DeviceStructure*>(in->ItemAt(i));
//			delete delStr;
		} else {
			BInputDevice* delDev = static_cast<BInputDevice*>(in->ItemAt(i));
//			delete delDev;
		}
	}
	in->MakeEmpty();
}


void ListDevices() {
	BList devices;
	BuildListOfDevices();
	
    int32 count = 0;
    count = fInputDevices.CountItems();

	if (count) printf("Connected pointing devices:\n");
    for (int32 i = 0; i < count; i++) {
    	DeviceStructure* dev = (DeviceStructure*)gDevices.ItemAt(i);
    	
        printf(" %d. %s - %s\n", dev->number,
        			 dev->device->Name(),
            		(dev->enabled ? "enabled" : "disabled"));
    }
}
