/* mod_axp20x
 *
 * Expose axp20x PMU figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 19/07/2025 - LF - First version
 */

#ifndef MOD_AXP20X_H
#define MOD_AXP20X_H

/* Include shared modules definitions and utilities */
#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_axp20x {
	struct Module module;
};

/* Custom structure to store a section handled by this module.
 * Usually, there is only one PMU per system, but it's harmless to
 * use sections.
 */

struct section_axp20x {
	struct Section section;

		/* Variables dedicated to this structure */
	const char *device;	/* I2C device */
	uint8_t i2c_addr;	/* I2C address (default : 0x34) */

		/* Figures to publish */
	bool ac;
	bool vbus;
	bool bat;
	bool ips;
	bool temperature;
};

#endif
