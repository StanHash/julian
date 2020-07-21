
#pragma once

#include "common.hh"

#include <array>

enum struct Am : std::uint8_t
{
    IMP,
    ACC,
    IMM,
    ZRP,
    ZRX,
    ZRY,
    ABS,
    ABX,
    ABY,
    IAB,
    INX,
    INY,
    REL,
};

enum struct Mnem
{
    ADC,
    AND,
    ASL,
    BCC,
    BCS,
    BEQ,
    BIT,
    BMI,
    BNE,
    BPL,
    BRK,
    BVC,
    BVS,
    CLC,
    CLD,
    CLI,
    CLV,
    CMP,
    CPX,
    CPY,
    DEC,
    DEX,
    DEY,
    EOR,
    INC,
    INX,
    INY,
    JMP,
    JSR,
    LDA,
    LDX,
    LDY,
    LSR,
    NOP,
    ORA,
    PHA,
    PHP,
    PLA,
    PLP,
    ROL,
    ROR,
    RTI,
    RTS,
    SBC,
    SEC,
    SED,
    SEI,
    STA,
    STX,
    STY,
    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA,
};

struct OpInfo
{
    enum
    {
        FLAG_JUMP  = (1 << 0),
        FLAG_END   = (1 << 1),
        FLAG_CALL  = (1 << 2),
        FLAG_WRITE = (1 << 3),
    };

    char const* name;
    std::uint8_t opcode;
    Mnem mnemonic;
    Am addressing_mode;
    std::uint8_t flags;
};

struct Instr
{
    std::uint8_t opcode = 0;
    std::uint16_t operand = 0;
};

struct OpInfoCache
{
    OpInfo const* get_opcode_info(byte_type opcode) const;

private:
    mutable std::array<OpInfo const*, 0x100> m_lut {};
};

OpInfo const* find_opcode_info(byte_type opcode);
std::size_t get_addressing_mode_operand_size(Am am);

OpInfo const* get_instr_info(Instr const& instr);
OpInfo const* get_instr_info(Instr const& instr, OpInfoCache const& cache);
std::size_t get_instr_size(Instr const& instr);
std::size_t get_instr_size(Instr const& instr, OpInfoCache const& cache);
