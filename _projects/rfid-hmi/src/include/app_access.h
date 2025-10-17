/**
 * @file app_access.h
 * @brief Access control logic header.
 * @details Defines functions to validate RFID UIDs against a predefined whitelist
 *          and manage access decisions.
 */

#ifndef APP_ACCESS_H_
#define APP_ACCESS_H_

#include <stdint.h>

/**
 * @brief Initializes access control system.
 */
void app_access_init(void);

/**
 * @brief Verifies an RFID UID against the whitelist.
 * @param uid Pointer to the 4-byte UID array.
 */
void app_access_verify_uid(uint8_t *uid);

#endif /* APP_ACCESS_H_ */