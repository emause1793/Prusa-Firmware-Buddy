// TODO: This is just copy of MK4 selftest
#pragma once

#include "i_selftest.hpp"
#include "super.hpp"
#include "selftest_part.hpp"
#include "selftest_result_type.hpp"
#include <selftest_types.hpp>

typedef enum {
    stsIdle,
    stsStart,
    stsPrologueAskRun,
    stsPrologueAskRun_wait_user,
    stsSelftestStart,
    stsPrologueInfo,
    stsPrologueInfo_wait_user,
    stsPrologueInfoDetailed,
    stsPrologueInfoDetailed_wait_user,
    stsFans,
    stsWait_fans,
    stsEnsureZAway,
    stsXAxis,
    stsYAxis,
    stsZcalib,
    stsDocks,
    stsLoadcell,
    stsWait_loadcell,
    stsToolOffsets,
    stsZAxis, // could not be first, printer can't home at front edges without steelsheet on
    stsMoveZup,
    stsWait_axes,
    stsHeaters_noz_ena,
    stsHeaters_bed_ena,
    stsHeaters,
    stsWait_heaters,
    stsFans_fine,
    stsFSensor_calibration,
    stsNet_status,
    stsSelftestStop,
    stsDidSelftestPass,
    stsEpilogue_nok,
    stsEpilogue_nok_wait_user,
    stsShow_result,
    stsResult_wait_user,
    stsEpilogue_ok,
    stsEpilogue_ok_wait_user,
    stsFinish,
    stsFinished,
    stsAborted,
} SelftestState_t;

consteval uint64_t to_one_hot(SelftestState_t state) {
    return static_cast<uint64_t>(1) << state;
}

enum SelftestMask_t : uint64_t {
    stmNone = 0,
    stmFans = to_one_hot(stsFans),
    stmWait_fans = to_one_hot(stsWait_fans),
    stmLoadcell = to_one_hot(stsLoadcell),
    stmWait_loadcell = to_one_hot(stsWait_loadcell),
    stmZcalib = to_one_hot(stsZcalib),
    stmEnsureZAway = to_one_hot(stsEnsureZAway),
    stmXAxis = to_one_hot(stsXAxis),
    stmYAxis = to_one_hot(stsYAxis),
    stmZAxis = to_one_hot(stsZAxis),
    stmMoveZup = to_one_hot(stsMoveZup),
    stmXYAxis = stmXAxis | stmYAxis,
    stmXYZAxis = stmXAxis | stmYAxis | stmZAxis,
    stmWait_axes = to_one_hot(stsWait_axes),
    stmHeaters_noz = to_one_hot(stsHeaters) | to_one_hot(stsHeaters_noz_ena),
    stmHeaters_bed = to_one_hot(stsHeaters) | to_one_hot(stsHeaters_bed_ena),
    stmHeaters = stmHeaters_bed | stmHeaters_noz,
    stmWait_heaters = to_one_hot(stsWait_heaters),
    stmFSensor = to_one_hot(stsFSensor_calibration),
    stmSelftestStart = to_one_hot(stsSelftestStart),
    stmSelftestStop = to_one_hot(stsSelftestStop),
    stmNet_status = to_one_hot(stsNet_status),
    stmDocks = to_one_hot(stsDocks),
    stmToolOffsets = to_one_hot(stsToolOffsets),
    stmShow_result = to_one_hot(stsShow_result) | to_one_hot(stsResult_wait_user),
    stmFullSelftest = stmFans | stmLoadcell | stmXYZAxis | stmHeaters | stmNet_status | stmShow_result | stmDocks | stmFSensor | to_one_hot(stsDidSelftestPass),
    stmWizardPrologue = to_one_hot(stsPrologueAskRun) | to_one_hot(stsPrologueAskRun_wait_user) | to_one_hot(stsPrologueInfo) | to_one_hot(stsPrologueInfo_wait_user) | to_one_hot(stsPrologueInfoDetailed) | to_one_hot(stsPrologueInfoDetailed_wait_user),
    stmEpilogue = to_one_hot(stsEpilogue_nok) | to_one_hot(stsEpilogue_nok_wait_user) | to_one_hot(stsEpilogue_ok) | to_one_hot(stsEpilogue_ok_wait_user),
    stmWizard = stmFullSelftest | stmWizardPrologue | stmEpilogue,
    stmFans_fine = to_one_hot(stsFans_fine),
};

// class representing whole self-test
class CSelftest : public AddSuper<ISelftest> {
public:
    CSelftest();

public:
    virtual bool IsInProgress() const override;
    virtual bool Start(const uint64_t test_mask, const uint8_t tool_mask) override; // parent has no clue about SelftestMask_t
    virtual void Loop() override;
    virtual bool Abort() override;

protected:
    void phaseSelftestStart();
    void restoreAfterSelftest();
    virtual void next() override;
    virtual const char *get_log_suffix() override;
    void phaseShowResult();
    bool phaseWaitUser(PhasesSelftest phase);
    void phaseDidSelftestPass();

protected:
    SelftestState_t m_State;
    SelftestMask_t m_Mask;
    ToolMask tool_mask = ToolMask::AllTools;
    std::array<selftest::IPartHandler *, HOTENDS * 2> pFans;
    selftest::IPartHandler *pXAxis;
    selftest::IPartHandler *pYAxis;
    selftest::IPartHandler *pZAxis;
    std::array<selftest::IPartHandler *, HOTENDS> pNozzles;
    selftest::IPartHandler *pBed;
    std::array<selftest::IPartHandler *, HOTENDS> m_pLoadcell;
    std::array<selftest::IPartHandler *, HOTENDS> pDocks;
    selftest::IPartHandler *pToolOffsets;
    std::array<selftest::IPartHandler *, HOTENDS> pFSensor;

    SelftestResult m_result;
};
