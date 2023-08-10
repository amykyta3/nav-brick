#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "utils/cli.h"
#include <stdint.h>

// If set to 1, command parsing recognizes multi-word arguments in quotes
#define PARSE_QUOTED_ARGS   1

// If set to 1, performs command lookup using a binary search instead of linear.
#define USE_BINARY_SEARCH   0

// maximum length of a command input line
#define CLI_STRBUF_SIZE    64

// Maximum number of arguments in a command (including command).
#define CLI_MAX_ARGC    5

// Table of commands: {"command_word" , function }
// If using binary search, command words MUST be in alphabetical (ascii) order (A-Z then a-z)
#define CMDTABLE    {"args"  , cmd_ArgList  },\
                    {"hi"    , cmd_Hello    },\
                    {"l"     , cmd_DisplayLeft  },\
                    {"r"     , cmd_DisplayRight  },\
                    {"b"     , cmd_SetLightness},\
                    {"p"     , cmd_SetLightness}

// Custom command function prototypes:
int cmd_ArgList(uint8_t argc, char *argv[]);
int cmd_Hello(uint8_t argc, char *argv[]);
int cmd_DisplayLeft(uint8_t argc, char *argv[]);
int cmd_DisplayRight(uint8_t argc, char *argv[]);
int cmd_SetLightness(uint8_t argc, char *argv[]);

#endif
