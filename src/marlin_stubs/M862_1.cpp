#include "../common/sound.hpp"
#include "PrusaGcodeSuite.hpp"
#include <Marlin/src/gcode/parser.h>
#include <Marlin/src/module/motion.h>
#include "eeprom.h"

#ifdef PRINT_CHECKING_Q_CMDS
/**
 * M862.1: Check nozzle diameter
 *
 *  Q          - get machine value
 *  T<number>  - specific tool, default to currently active nozzle
 */
void PrusaGcodeSuite::M862_1() {
    // Handle only Q
    // P is ignored when printing (it is handled before printing by GCodeInfo.*)
    if (!parser.boolval('Q')) {
        return;
    }

    // Get for which tool
    #if HAS_TOOLCHANGER()
    const uint8_t default_tool = active_extruder;
    #else  /*HAS_TOOLCHANGER()*/
    const uint8_t default_tool = 0;
    #endif /*HAS_TOOLCHANGER()*/
    uint8_t tool = parser.byteval('T', default_tool);

    // Fetch diameter from EEPROM
    if (tool < EEPROM_MAX_TOOL_COUNT) {
        SERIAL_ECHO_START();
        char temp_buf[sizeof("  M862.1 T0 P0.00")];
        snprintf(temp_buf, sizeof(temp_buf), PSTR("  M862.1 T%u P%.2f"), tool, static_cast<double>(eeprom_get_nozzle_dia(tool)));
        SERIAL_ECHO(temp_buf);
        SERIAL_EOL();
    }
}
#endif
