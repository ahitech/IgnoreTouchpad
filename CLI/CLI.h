/*
 * Copyright 2025, Alex Hitech <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IGNORE_TOUCHPAD_CLI_H
#define IGNORE_TOUCHPAD_CLI_H

#include <List.h>
#include <SupportDefs.h>
#include <InputDevice.h>

typedef enum class InputOptions {
	Undefined,
	List,
	Disable,
	Enable,
	EnableAll
} InputOptions;

enum class InputOptions ParseInputOptions(int argc, char *argv[]);
void ListDevices();
void PrintUsage();
status_t DisableDevice(BInputDevice*);
status_t EnableDevice(BInputDevice*);
void Clean(BList*);

#endif // IGNORE_TOUCHPAD_CLI_H
