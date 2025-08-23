/*
 * Copyright 2025, Alex Hitech <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "CLI.h"
#include <Catalog.h>
#include <Input.h>
#include <List.h>
#include <stdio.h>
#include <iostream>
#include <sstream>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Ignore Touchpad CLI"


DeviceStructure::~DeviceStructure() {
	if (device) { delete device; device = NULL; }
}


int main(int argc, char** argv) {
	if (argc < 2) {
		PrintUsage();
		return 0;
	}

	std::vector<std::string> args(argv + 1, argv + argc);
	ParsedCommand command = ParseCommand(args);

	Clean(&gDevices, true);
	BuildListOfDevices();

	if (command.type == CommandType::kInteractive) {
		RunInteractiveLoop();
		Clean(&gDevices, true);
		return 0;
	}

	Clean(&gDevices, true);
	return ExecuteCommand(command);
}


void Clean(BList *in, bool cleaningGDevices = false) {
	if (!in) return;
	uint count = in->CountItems();
	for (int32 i = count-1; i >=0; i--) {
		if (cleaningGDevices) {
			DeviceStructure* delStr = static_cast<DeviceStructure*>(in->ItemAt(i));
//			delete delStr;
		} else {
			BInputDevice* delDev = static_cast<BInputDevice*>(in->ItemAt(i));
//			delete delDev;
		}
	}
	in->MakeEmpty();
}


void RunInteractiveLoop() {
	std::string input;
	while (true) {
		std::cout << "> " << std::flush;
		if (!std::getline(std::cin, input)) break;

		std::istringstream iss(input);
		std::vector<std::string> args;
		std::string token;
		while (iss >> token) args.push_back(token);

		if (args.empty()) continue;

		ParsedCommand command = ParseCommand(args);
		if (command.type == CommandType::kQuit)
			break;

		ExecuteCommand(command);
	}
}


ParsedCommand ParseCommand(const std::vector<std::string>& args) {
    ParsedCommand cmd;

    if (args.empty()) {
        cmd.type = CommandType::kHelp;
        return cmd;
    }

    const std::string& action = args[0];

    if (action == "list") {
        cmd.type = CommandType::kList;
    } else if ((action == "enable" || action == "e" || action == "E")
               && args.size() == 2) {
        cmd.type = CommandType::kEnable;
        cmd.deviceNumber = std::stoi(args[1]);
    } else if ((action == "disable" || action == "d" || action == "D")
               && args.size() == 2) {
        cmd.type = CommandType::kDisable;
        cmd.deviceNumber = std::stoi(args[1]);
    } else if (action == "enable_all" || action == "ea" || action == "EA") {
        cmd.type = CommandType::kEnableAll;
        cmd.deviceNumber = 0;
    } else if (action == "help" || action == "?") {
        cmd.type = CommandType::kHelp;
    } else if (action == "interactive") {
        cmd.type = CommandType::kInteractive;
    } else if (action == "refresh") {
    	cmd.type = CommandType::kList;
    } else if (action == "quit" || action == "exit") {
        cmd.type = CommandType::kQuit;
    } else {
        cmd.type = CommandType::kUnknown;
    }

    return cmd;
}


void BuildListOfDevices() {
	Clean(&gDevices, true);
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
    	device->enabled = device->device->IsRunning();
    	device->number = i;
    	gDevices.AddItem(device, i);
    }
    
    Clean(&devices, false);
}

void ListDevices() {
	BuildListOfDevices();
	
    int32 count = 0;
    count = gDevices.CountItems();

	if (count) printf("Connected pointing devices:\n");
    for (int32 i = 0; i < count; i++) {
    	DeviceStructure* dev = (DeviceStructure*)gDevices.ItemAt(i);
    	
        printf(" %d. %s - %s\n", dev->number,
        			 dev->device->Name(),
            		(dev->enabled ? "enabled" : "disabled"));
    }
}


status_t EnableDevice(BInputDevice* dev) {
	status_t toReturn = B_OK;
	if (dev) {
		toReturn = dev->Start();
		if (B_OK != toReturn) {
			fprintf(stderr, B_TRANSLATE("[EnableDevice] Error enabling device \'%s\': %s\n"),
					dev->Name(), strerror(toReturn));
		}
	}
	return toReturn;
}


status_t DisableDevice(BInputDevice* dev) {
	status_t toReturn = B_OK;
	if (dev) {
		toReturn = dev->Stop();
		if (B_OK != toReturn) {
			fprintf(stderr, B_TRANSLATE("[DisableDevice] Error disabling device \'%s\': %s\n"),
					dev->Name(), strerror(toReturn));
		}
	}
	return toReturn;
}



status_t EnableAll() {
	status_t toReturn = BInputDevice::Start(B_POINTING_DEVICE);
	if (B_OK != toReturn) {
		fprintf(stderr, B_TRANSLATE("[EnableAll] Error enabling all devices: %s\n"),
					strerror(toReturn));
	}
	return toReturn;
}


void PrintUsage() {
	printf(B_TRANSLATE("This utility disables or enables a pointing device (mouse or touchpad). "
		   "Its aim is to ignore accidental clicks on the touchpad when an external pointing "
		   "device is connected to a notebook.\n"));
	printf(B_TRANSLATE("It can run in either interactive or non-interactive mode.\n\n"));
	printf(B_TRANSLATE("Supported options (in both modes, unless stated othwerwise):\n"));
	printf(B_TRANSLATE("  list         - Build and print a numbered list of the pointing input devices.\n"
					   "                 Note: this option recreates the list of devices and updates it.\n"));
	printf(B_TRANSLATE("  refresh      - Equals to \"list\".\n"));
	printf(B_TRANSLATE("  enable #     - Enable a device number #. The number you take from the \"list\" command.\n"));
	printf(B_TRANSLATE("                 If a device is already enabled, or if the number is wrong, nothing happens.\n"));
	printf(B_TRANSLATE("  e # or E #   - Equals to \"enable #\", just fewer symbols to type. :) \n"));
	printf(B_TRANSLATE("  disable #    - Disable a device number #. The number you take from the \"list\" command.\n"));
	printf(B_TRANSLATE("                 If a device is already disabled, or if the number is wrong, nothing happens.\n"));
	printf(B_TRANSLATE("  d # or D #   - Equals to \"disable #\", just fewer symbols to type. :) \n"));
	printf(B_TRANSLATE("  enable_all   - Immediately enable all devices. If you accidentally disabled the last\n"
					   "                 mouse, you can enable it.\n\tDefault shortcut: Ctrl + Alt + Win + E.\n"
					   "                 (You can change in \'Shortcuts\', if you want, but this text won't be updated.\n"));
	printf(B_TRANSLATE("  EA or ea     - Equals to \"enable all\", just fewer symbols to type. :) \n"));
	printf(B_TRANSLATE("  help or ?    - Display list of the available commands.\n"));
	printf(B_TRANSLATE("  interactive  - (Command line option only) Enter interactive mode.\n"));
	printf(B_TRANSLATE("  quit or exit - (Interactive mode only) Quit interactive mode.\n"));
}


status_t ExecuteCommand(const ParsedCommand& command) {
	BuildListOfDevices();
	
	switch (command.type) {
		case CommandType::kList:
			ListDevices();
			return B_OK;

		case CommandType::kEnable:
			if (command.deviceNumber >= 0) {
				uint count = gDevices.CountItems();
				for (uint i = 0; i < count; i++) {
					DeviceStructure* dev = (DeviceStructure*)gDevices.ItemAt(i);
					if (dev->number == i) {
						return EnableDevice(dev->device);
					}	
				}
			}
			return B_OK;

		case CommandType::kDisable:
			if (command.deviceNumber >= 0) {
				uint count = gDevices.CountItems();
				uint enabledDevices = 0;
				
				// Can't disable last pointing device!
				for (uint i = 0; i < count; i++) {
					DeviceStructure* dev = (DeviceStructure*)gDevices.ItemAt(i);
					if (dev->enabled) {
						enabledDevices++;
					}	
				}
				
				if (enabledDevices == 1) {
					fprintf (stderr, "[Disable Device]: Can't disable last active pointing device!\n");
					return B_OK;
				}
				for (uint i = 0; i < count; i++) {
					DeviceStructure* dev = (DeviceStructure*)gDevices.ItemAt(i);
					if (dev->number == i) {
						return DisableDevice(dev->device);
					}	
				}
			}
			return B_OK;

		case CommandType::kEnableAll:			
			return EnableAll();

		case CommandType::kHelp:
			PrintUsage();
			return B_OK;

		case CommandType::kQuit:
			// Exit interactive mode
			return B_OK;
		
		case CommandType::kUnknown:
			PrintUsage();
			return B_OK;

		default:
			fprintf(stderr, "Unknown command. Type 'help'.\n");
			return B_ERROR;
	}
}
