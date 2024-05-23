 /**
 **************************************************************
 * @file mylib/s4375116_cli_mnemonic.h
 * @author Sami Kaab - s4375116 
 * @date 22052022
 * @brief cli command functions used to control the cag simulation
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_cli_mnemonic_init() - registers the cli commands
 ***************************************************************
 */

#ifndef S4375116_CLI_MNEMONIC_H
#define S4375116_CLI_MNEMONIC_H

#include "event_groups.h"

EventBits_t uxBits;

void s4375116_cli_mnemonic_init(void);

#endif