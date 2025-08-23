/*
 * Copyright 2025, Alex Hitech <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IGNORE_TOUCHPAD_CLI_H
#define IGNORE_TOUCHPAD_CLI_H

#include <List.h>
#include <SupportDefs.h>
#include <Input.h>
#include <vector>
#include <string>

enum class CommandType {
    kUnknown,
    kList,
    kEnable,
    kDisable,
    kEnableAll,
    kHelp,
    kInteractive,
    kQuit
};

struct ParsedCommand {
    CommandType type;
    int deviceNumber = -1; // By default, no device is affected
};

struct DeviceStructure {
	BInputDevice* device;
	bool		  enabled;
	uint		  number;
	
	virtual ~DeviceStructure();
};

BList gDevices;

ParsedCommand ParseCommand(const std::vector<std::string>& args);
void BuildListOfDevices();
void ListDevices();
void PrintUsage();
status_t DisableDevice(BInputDevice*);
status_t EnableDevice(BInputDevice*);
status_t EnableAll();
void Clean(BList*, bool);
status_t ExecuteCommand(const ParsedCommand& command);
void RunInteractiveLoop();

#endif // IGNORE_TOUCHPAD_CLI_H
