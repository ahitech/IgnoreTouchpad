/*
 * Copyright 2025, Alex Hitech <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "CLI.h"
#include <Catalog.h>
#include <Input.h>
#include <List.h>
#include <stdio.h>
#include <string.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Ignore Touchpad CLI"

int main(int argc, char *argv[])
{
	CommandType returnValue = CommandType::Undefined;
	
	CommandType = ParseInputOptions(argc, argv);
	
	if (returnValue == CommandType::Undefined) {
		PrintUsage();
		return 1;
	}
	
	return 0;
}


void Clean(BList* inputDevices) {
	uint count = inputDevices.CountItems();
	for (int32 i = 0; i < count; i++) {
        delete ((BInputDevice*)(inputDevices.ItemAt(i)));
	}
	list.MakeEmpty();	
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
    } else if (action == "enable" && args.size() == 2) {
        cmd.type = CommandType::kEnable;
        cmd.deviceNumber = std::stoi(args[1]);
    } else if (action == "disable" && args.size() == 2) {
        cmd.type = CommandType::kDisable;
        cmd.deviceNumber = std::stoi(args[1]);
    } else if (action == "enable_all") {
        cmd.type = CommandType::kEnableAll;
        cmd.deviceNumber = 0;
    } else if (action == "help") {
        cmd.type = CommandType::kHelp;
    } else if (action == "interactive") {
        cmd.type = CommandType::kInteractive;
    } else if (action == "quit") {
        cmd.type = CommandType::kQuit;
    } else {
        cmd.type = CommandType::kUnknown;
    }

    return cmd;
}


void ListDevices() {
    int32 count = 0;
	BList list;
    status_t err = get_input_devices(&list);
    if (err != B_OK) {
        fprintf(stderr, "Failed to get input devices: %s\n", strerror(err));
        return 1;
    }

    printf("Connected pointing devices:\n");
    count = list.CountItems();

    for (int32 i = 0; i < count; i++) {
        BInputDevice* info = list.ItemAt(i);
        if (info && info->Type() == B_POINTING_DEVICE)
        {
            printf(" %d - %s\n", i, info->Name(),
            		(info->IsRunning() ? "enabled" : "disabled");
        }
    }
}



void PrintUsage() {
	printf(B_TRANSLATE("This utility disables or enables a pointing device (mouse or touchpad). "
		   "Its aim is to ignore accidental clicks on the touchpad when an external pointing "
		   "device is connected to a notebook.\n"));
	printf(B_TRANSLATE("It can run in either interactive or non-interactive mode.\n\n"));
	printf(B_TRANSLATE("Supported options (in both modes, unless stated othwerwise):\n"));
	printf(B_TRANSLATE("  list        - Print a numbered list of the pointing input devices.\n"));
	printf(B_TRANSLATE("  enable #    - Enable a device number #. The number you take from the \"list\" command.\n"));
	printf(B_TRANSLATE("                If a device is already enabled, or if the number is wrong, nothing happens.\n"));
	printf(B_TRANSLATE("  disable #   - Disable a device number #. The number you take from the \"list\" command.\n"));
	printf(B_TRANSLATE("                If a device is already disabled, or if the number is wrong, nothing happens.\n"));
	printf(B_TRANSLATE("  enable_all  - Immediately enable all devices. If you accidentally disabled the last\n"
					   "                mouse, you can enable it.\nDefault shortcut: Ctrl + Alt + Win + E.\n"
					   "                (You can change in \'Shortcuts\', if you want, but this text won't be updated.\n"));
	printf(B_TRANSLATE("  help        - Display list of the available commands.\n"));
	printf(B_TRANSLATE("  interactive - (Command line option only) Enter interactive mode.\n"));
	printf(B_TRANSLATE("  quit        - (Interactive mode only) Quit interactive mode.\n"));
}
