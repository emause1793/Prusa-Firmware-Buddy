#include "ModbusRegisters.hpp"
#include "hal/HAL_OTP.hpp"
#include "PuppyConfig.hpp"
#include "otp.h"
#include "utility_extensions.hpp"
#include <cstring>
#include <cstdlib>
#include <array>

namespace modularbed::ModbusRegisters {

const uint32_t MIN_SystemDiscreteInput = (ftrstd::to_underlying(SystemDiscreteInput::power_painc_status));
const uint32_t MAX_SystemDiscreteInput = (ftrstd::to_underlying(SystemDiscreteInput::current_fault_status));

const uint32_t MIN_HBDiscreteInput = (ftrstd::to_underlying(HBDiscreteInput::is_ready));
const uint32_t MAX_HBDiscreteInput = ((ftrstd::to_underlying(HBDiscreteInput::is_ready)) + HEATBEDLET_COUNT - 1);

const uint32_t MIN_SystemCoil = (ftrstd::to_underlying(SystemCoil::clear_fault_status));
const uint32_t MAX_SystemCoil = (ftrstd::to_underlying(SystemCoil::print_fan_active));

const uint32_t MIN_HBCoil = (ftrstd::to_underlying(HBCoil::clear_fault_status));
const uint32_t MAX_HBCoil = ((ftrstd::to_underlying(HBCoil::clear_fault_status)) + HEATBEDLET_COUNT - 1);

const uint32_t MIN_SystemInputRegister = (ftrstd::to_underlying(SystemInputRegister::fault_status));
const uint32_t MAX_SystemInputRegister = (ftrstd::to_underlying(SystemInputRegister::mcu_temperature));

const uint32_t MIN_HBInputRegister = (ftrstd::to_underlying(HBInputRegister::fault_status));
const uint32_t MAX_HBInputRegister = ((ftrstd::to_underlying(HBInputRegister::pid_tc_control_action)) + HEATBEDLET_COUNT - 1);

const uint32_t MIN_SystemHoldingRegister = (ftrstd::to_underlying(SystemHoldingRegister::chamber_temperature));
const uint32_t MAX_SystemHoldingRegister = (ftrstd::to_underlying(SystemHoldingRegister::clear_system_fault_bits));

const uint32_t MIN_HBHoldingRegister = (ftrstd::to_underlying(HBHoldingRegister::max_allowed_current));
const uint32_t MAX_HBHoldingRegister = ((ftrstd::to_underlying(HBHoldingRegister::clear_hb_fault_bits)) + HEATBEDLET_COUNT - 1);

static uint16_t s_SystemDiscreteInputs[MAX_SystemDiscreteInput - MIN_SystemDiscreteInput + 1];
static uint16_t s_HBDiscreteInputs[MAX_HBDiscreteInput - MIN_HBDiscreteInput + 1];
static uint16_t s_SystemCoils[MAX_SystemCoil - MIN_SystemCoil + 1];
static uint16_t s_HBCoils[MAX_HBCoil - MIN_HBCoil + 1];
static uint16_t s_SystemInputRegisters[MAX_SystemInputRegister - MIN_SystemInputRegister + 1];
static uint16_t s_HBInputRegisters[MAX_HBInputRegister - MIN_HBInputRegister + 1];
static uint16_t s_SystemHoldingRegisters[MAX_SystemHoldingRegister - MIN_SystemHoldingRegister + 1];
static uint16_t s_HBHoldingRegisters[MAX_HBHoldingRegister - MIN_HBHoldingRegister + 1];

void Init() {
    //clear registers
    memset(s_SystemDiscreteInputs, 0, sizeof(s_SystemDiscreteInputs));
    memset(s_HBDiscreteInputs, 0, sizeof(s_HBDiscreteInputs));
    memset(s_SystemCoils, 0, sizeof(s_SystemCoils));
    memset(s_HBCoils, 0, sizeof(s_HBCoils));
    memset(s_SystemInputRegisters, 0, sizeof(s_SystemInputRegisters));
    memset(s_HBInputRegisters, 0, sizeof(s_HBInputRegisters));
    memset(s_SystemHoldingRegisters, 0, sizeof(s_SystemHoldingRegisters));
    memset(s_HBHoldingRegisters, 0, sizeof(s_HBHoldingRegisters));

    //add register blocks
    AddBlock(BlockType::DiscreteInput, s_SystemDiscreteInputs, MIN_SystemDiscreteInput, MAX_SystemDiscreteInput - MIN_SystemDiscreteInput + 1);
    AddBlock(BlockType::DiscreteInput, s_HBDiscreteInputs, MIN_HBDiscreteInput, MAX_HBDiscreteInput - MIN_HBDiscreteInput + 1);

    AddBlock(BlockType::Coil, s_SystemCoils, MIN_SystemCoil, MAX_SystemCoil - MIN_SystemCoil + 1);
    AddBlock(BlockType::Coil, s_HBCoils, MIN_HBCoil, MAX_HBCoil - MIN_HBCoil + 1);

    AddBlock(BlockType::InputRegister, s_SystemInputRegisters, MIN_SystemInputRegister, MAX_SystemInputRegister - MIN_SystemInputRegister + 1);
    AddBlock(BlockType::InputRegister, s_HBInputRegisters, MIN_HBInputRegister, MAX_HBInputRegister - MIN_HBInputRegister + 1);

    AddBlock(BlockType::HoldingRegister, s_SystemHoldingRegisters, MIN_SystemHoldingRegister, MAX_SystemHoldingRegister - MIN_SystemHoldingRegister + 1);
    AddBlock(BlockType::HoldingRegister, s_HBHoldingRegisters, MIN_HBHoldingRegister, MAX_HBHoldingRegister - MIN_HBHoldingRegister + 1);

    //init default register values

    //basic HW info
    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::heatbedlet_count), HEATBEDLET_COUNT);

    for (uint16_t i = 0; i < HEATBEDLET_COUNT; i++) {
        SetRegValue(HBHoldingRegister::max_allowed_current, i, static_cast<uint16_t>(HEATBEDLET_DEFAULT_MAX_ALLOWED_CURRENT * MODBUS_CURRENT_REGISTERS_SCALE));
    }

    //system registers
    SetRegValue(SystemHoldingRegister::chamber_temperature, ((float)DEFAULT_CHAMBER_TEMP) * MODBUS_TEMPERATURE_REGISTERS_SCALE);

    //init registers from OTP
    serial_nr_t sn;     // Serial number = raw datamatrix
    uint32_t timestamp; // Unix timestamp, seconds since 1970
    uint8_t bom_id;
    timestamp = otp_get_timestamp();
    if ((otp_get_serial_nr(&sn) != sizeof(sn.txt)) || (otp_get_bom_id(&bom_id) == false)) {

        // Default to zero
        timestamp = 0;
        bom_id = 0;
        memset(sn.txt, 0, sizeof(sn.txt));
    }

    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_bom_id), bom_id);
    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_otp_timestamp_0), timestamp & 0xFFFF);
    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_otp_timestamp_1), timestamp >> 16);

    static constexpr uint16_t raw_datamatrix_regsize = ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_last)
        - ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_first) + 1;
    // Check size of text -1 as the terminating \0 is not sent
    static_assert((raw_datamatrix_regsize * sizeof(uint16_t)) == (sizeof(sn.txt) - 1), "Size of raw datamatrix doesn't fit modbus registers");

    for (uint16_t i = 0; i < raw_datamatrix_regsize; i++) {
        uint16_t word = sn.txt[2 * i] | (sn.txt[2 * i + 1] << 8);
        SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_first) + i, word);
    }
}

void SetBitValue(SystemDiscreteInput reg, bool value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetDiscreteInputValue(regAddress, value);
}

void SetBitValue(HBDiscreteInput reg, uint16_t heatbedletIndex, bool value) {
    uint16_t regAddress = ftrstd::to_underlying(reg) + heatbedletIndex;
    SetDiscreteInputValue(regAddress, value);
}

void SetBitValue(SystemCoil reg, bool value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetCoilValue(regAddress, value);
}

bool GetBitValue(SystemCoil reg) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    bool value = 0;
    GetCoilValue(regAddress, &value);
    return value;
}

void SetRegValue(SystemInputRegister reg, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetInputRegisterValue(regAddress, value);
}

void SetRegValue(HBInputRegister reg, uint16_t heatbedletIndex, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg) + heatbedletIndex;
    SetInputRegisterValue(regAddress, value);
}

void SetRegValue(SystemHoldingRegister reg, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetHoldingRegisterValue(regAddress, value);
}

void SetRegValue(HBHoldingRegister reg, uint16_t heatbedletIndex, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg) + heatbedletIndex;
    SetHoldingRegisterValue(regAddress, value);
}

uint16_t GetRegValue(SystemInputRegister reg) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    uint16_t value = 0;
    GetInputRegisterValue(regAddress, &value);
    return value;
}

uint16_t GetRegValue(HBInputRegister reg, uint16_t heatbedletIndex) {
    uint16_t regAddress = ftrstd::to_underlying(reg) + heatbedletIndex;
    uint16_t value = 0;
    GetInputRegisterValue(regAddress, &value);
    return value;
}

uint16_t GetRegValue(SystemHoldingRegister reg) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    uint16_t value = 0;
    GetHoldingRegisterValue(regAddress, &value);
    return value;
}

uint16_t GetRegValue(HBHoldingRegister reg, uint16_t heatbedletIndex) {
    uint16_t regAddress = ftrstd::to_underlying(reg) + heatbedletIndex;
    uint16_t value = 0;
    GetHoldingRegisterValue(regAddress, &value);
    return value;
}

} //namespace
