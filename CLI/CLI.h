/*
 * Copyright 2025, Alex Hitech <ahitech@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IGNORE_TOUCHPAD_CLI_H
#define IGNORE_TOUCHPAD_CLI_H

#include <List.h>
#include <SupportDefs.h>
#include <InputDevice.h>

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
    int deviceNumber = -1; // по умолчанию недействителен
};

enum class CommandType ParseInputOptions(int argc, char *argv[]);
void ListDevices();
void PrintUsage();
status_t DisableDevice(BInputDevice*);
status_t EnableDevice(BInputDevice*);
void Clean(BList*);

#endif // IGNORE_TOUCHPAD_CLI_H
