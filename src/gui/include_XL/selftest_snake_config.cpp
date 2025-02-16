#include "selftest_snake_config.hpp"
#include <selftest_types.hpp>
#include <screen_menu_selftest_snake_result_parsing.hpp>
#include <option/has_toolchanger.h>
#if HAS_TOOLCHANGER()
    #include <module/prusa/toolchanger.h>
#endif

namespace SelftestSnake {
TestResult get_test_result(Action action, Tool tool) {
    SelftestResult sr;
    eeprom_get_selftest_results(&sr);

    switch (action) {
    case Action::Fans:
        return merge_hotends_evaluations(
            [&](int8_t e) {
                return evaluate_results(sr.tools[e].printFan, sr.tools[e].heatBreakFan);
            });
    case Action::ZAlign:
        return evaluate_results(sr.zalign);
    case Action::XYCheck:
        return evaluate_results(sr.xaxis, sr.yaxis);
    case Action::DockCalibration:
        if (tool == Tool::_all_tools) {
            return merge_hotends_evaluations(
                [&](int8_t e) {
                    return evaluate_results(sr.tools[e].dockoffset);
                });
        } else {
            return evaluate_results(sr.tools[ftrstd::to_underlying(tool)].dockoffset);
        }
    case Action::Loadcell:
        if (tool == Tool::_all_tools) {
            return merge_hotends_evaluations(
                [&](int8_t e) {
                    return evaluate_results(sr.tools[e].loadcell);
                });
        } else {
            return evaluate_results(sr.tools[ftrstd::to_underlying(tool)].loadcell);
        }
    case Action::ToolOffsetsCalibration:
        return merge_hotends_evaluations(
            [&](int8_t e) {
                return evaluate_results(sr.tools[e].tooloffset);
            });
    case Action::ZCheck:
        return evaluate_results(sr.zaxis);
    case Action::BedHeaters:
        return evaluate_results(sr.bed);
    case Action::NozzleHeaters:
        return merge_hotends_evaluations([&](int8_t e) {
            return evaluate_results(sr.tools[e].nozzle);
        });
    case Action::Heaters:
        return evaluate_results(sr.bed, merge_hotends_evaluations([&](int8_t e) {
            return evaluate_results(sr.tools[e].nozzle);
        }));
    case Action::FilamentSensorCalibration:
        if (tool == Tool::_all_tools) {
            return merge_hotends_evaluations(
                [&](int8_t e) {
                    return evaluate_results(sr.tools[e].fsensor);
                });
        } else {
            return evaluate_results(sr.tools[ftrstd::to_underlying(tool)].fsensor);
        }
    case Action::_count:
        break;
    }
    return TestResult_Unknown;
}

uint8_t get_tool_mask(Tool tool) {
#if HAS_TOOLCHANGER()
    switch (tool) {
    case Tool::Tool1:
        return ToolMask::ToolO;
    case Tool::Tool2:
        return ToolMask::Tool1;
    case Tool::Tool3:
        return ToolMask::Tool2;
    case Tool::Tool4:
        return ToolMask::Tool3;
    case Tool::Tool5:
        return ToolMask::Tool4;
        break;
    default:
        assert(false);
        break;
    }
#endif
    return ToolMask::AllTools;
}

uint64_t get_test_mask(Action action) {
    switch (action) {
    case Action::Fans:
        return stmFans;
    case Action::XYCheck:
        return stmXYAxis;
    case Action::ZCheck:
        return stmZAxis;
    case Action::Heaters:
        return stmHeaters;
    case Action::BedHeaters:
        return stmHeaters_bed;
    case Action::NozzleHeaters:
        return stmHeaters_noz;
    case Action::FilamentSensorCalibration:
        return stmFSensor;
    case Action::Loadcell:
        return stmLoadcell;
    case Action::ZAlign:
        return stmZcalib;
    case Action::DockCalibration:
        return stmDocks;
    case Action::ToolOffsetsCalibration:
        return stmToolOffsets;
    case Action::_count:
        break;
    }
    assert(false);
    return stmNone;
}

Tool get_last_enabled_tool() {
#if HAS_TOOLCHANGER()
    return static_cast<Tool>(prusa_toolchanger.get_num_enabled_tools() - 1);
#else
    return Tool::Tool1;
#endif
}

}
