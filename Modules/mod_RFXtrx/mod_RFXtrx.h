/* mod_RFXtrx
 *
 * Handle RFXtrx devices (like the RFXCom)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 30/04/2016 - LF - First version
 * 20/08/2016 - LF - handles Disabled
 * 14/05/2023 - LF - Switch to V8's module
 */

#ifndef MOD_RFXTRX_H
#define MOD_RFXTRX_H

/* Include shared modules definitions and utilities */
#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_RFXtrx {
	struct Module module;

	const char *RFXdevice;
};

struct section_RFXCom {
	struct Section section;

	uint32_t did;	/* Device ID */

	bool inerror;
};

#endif
