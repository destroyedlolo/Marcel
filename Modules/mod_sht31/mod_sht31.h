/* mod_sht31
 *
 * Exposes SHT31 (Temperature/humidity probe) figures.
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/10/2022 - LF - First version
 */

#ifndef MOD_SHT31_H
#define MOD_SHT31_H

/* Include shared modules definitions and utilities */
#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_sht31 {
	struct Module module;
};


/* Custom structure to store a section handled by this module.
 */
struct section_sht31 {
	struct Section section;

		/* Variables dedicated to this structure */
	int dummy;

	const char *device;	/* I2C device */
	uint8_t i2c_addr;	/* I2C address (default : 0x44) */
	float offset;		/* Offset to apply to the raw value */
};

#endif
