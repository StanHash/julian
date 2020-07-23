
#include "6502.hh"

OpInfo const g_opcode_info[] =
{
    // adc
    { "adc", 0x69, Mnem::ADC, Am::IMM, 0 },
    { "adc", 0x65, Mnem::ADC, Am::ZRP, 0 },
    { "adc", 0x75, Mnem::ADC, Am::ZRX, 0 },
    { "adc", 0x6D, Mnem::ADC, Am::ABS, 0 },
    { "adc", 0x7D, Mnem::ADC, Am::ABX, 0 },
    { "adc", 0x79, Mnem::ADC, Am::ABY, 0 },
    { "adc", 0x61, Mnem::ADC, Am::INX, 0 },
    { "adc", 0x71, Mnem::ADC, Am::INY, 0 },

    // and
    { "and", 0x29, Mnem::AND, Am::IMM, 0 },
    { "and", 0x25, Mnem::AND, Am::ZRP, 0 },
    { "and", 0x35, Mnem::AND, Am::ZRX, 0 },
    { "and", 0x2D, Mnem::AND, Am::ABS, 0 },
    { "and", 0x3D, Mnem::AND, Am::ABX, 0 },
    { "and", 0x39, Mnem::AND, Am::ABY, 0 },
    { "and", 0x21, Mnem::AND, Am::INX, 0 },
    { "and", 0x31, Mnem::AND, Am::INY, 0 },

    // asl
    { "asl", 0x0A, Mnem::ASL, Am::ACC, 0 },
    { "asl", 0x06, Mnem::ASL, Am::ZRP, OpInfo::FLAG_WRITE },
    { "asl", 0x16, Mnem::ASL, Am::ZRX, OpInfo::FLAG_WRITE },
    { "asl", 0x0E, Mnem::ASL, Am::ABS, OpInfo::FLAG_WRITE },
    { "asl", 0x1E, Mnem::ASL, Am::ABX, OpInfo::FLAG_WRITE },

    // bcc
    { "bcc", 0x90, Mnem::BCC, Am::REL, OpInfo::FLAG_JUMP },

    // bcs
    { "bcs", 0xB0, Mnem::BCS, Am::REL, OpInfo::FLAG_JUMP },

    // beq
    { "beq", 0xF0, Mnem::BEQ, Am::REL, OpInfo::FLAG_JUMP },

    // bit
    { "bit", 0x24, Mnem::BIT, Am::ZRP, 0 },
    { "bit", 0x2C, Mnem::BIT, Am::ABS, 0 },

    // bmi
    { "bmi", 0x30, Mnem::BMI, Am::REL, OpInfo::FLAG_JUMP },

    // bne
    { "bne", 0xD0, Mnem::BNE, Am::REL, OpInfo::FLAG_JUMP },

    // bpl
    { "bpl", 0x10, Mnem::BPL, Am::REL, OpInfo::FLAG_JUMP },

    // brk
    { "brk", 0x00, Mnem::BRK, Am::IMP, OpInfo::FLAG_JUMP },

    // bvc
    { "bvc", 0x50, Mnem::BVC, Am::REL, OpInfo::FLAG_JUMP },

    // bvs
    { "bvs", 0x70, Mnem::BVS, Am::REL, OpInfo::FLAG_JUMP },

    // clc
    { "clc", 0x18, Mnem::CLC, Am::IMP, 0 },

    // cld
    { "cld", 0xD8, Mnem::CLD, Am::IMP, 0 },

    // cli
    { "cli", 0x58, Mnem::CLI, Am::IMP, 0 },

    // clv
    { "clv", 0xB8, Mnem::CLV, Am::IMP, 0 },

    // cmp
    { "cmp", 0xC9, Mnem::CMP, Am::IMM, 0 },
    { "cmp", 0xC5, Mnem::CMP, Am::ZRP, 0 },
    { "cmp", 0xD5, Mnem::CMP, Am::ZRX, 0 },
    { "cmp", 0xCD, Mnem::CMP, Am::ABS, 0 },
    { "cmp", 0xDD, Mnem::CMP, Am::ABX, 0 },
    { "cmp", 0xD9, Mnem::CMP, Am::ABY, 0 },
    { "cmp", 0xC1, Mnem::CMP, Am::INX, 0 },
    { "cmp", 0xD1, Mnem::CMP, Am::INY, 0 },

    // cpx
    { "cpx", 0xE0, Mnem::CPX, Am::IMM, 0 },
    { "cpx", 0xE4, Mnem::CPX, Am::ZRP, 0 },
    { "cpx", 0xEC, Mnem::CPX, Am::ABS, 0 },

    // cpy
    { "cpy", 0xC0, Mnem::CPY, Am::IMM, 0 },
    { "cpy", 0xC4, Mnem::CPY, Am::ZRP, 0 },
    { "cpy", 0xCC, Mnem::CPY, Am::ABS, 0 },

    // dec
    { "dec", 0xC6, Mnem::DEC, Am::ZRP, OpInfo::FLAG_WRITE },
    { "dec", 0xD6, Mnem::DEC, Am::ZRX, OpInfo::FLAG_WRITE },
    { "dec", 0xCE, Mnem::DEC, Am::ABS, OpInfo::FLAG_WRITE },
    { "dec", 0xDE, Mnem::DEC, Am::ABX, OpInfo::FLAG_WRITE },

    // dex
    { "dex", 0xCA, Mnem::DEX, Am::IMP, 0 },

    // dey
    { "dey", 0x88, Mnem::DEY, Am::IMP, 0 },

    // eor
    { "eor", 0x49, Mnem::EOR, Am::IMM, 0 },
    { "eor", 0x45, Mnem::EOR, Am::ZRP, 0 },
    { "eor", 0x55, Mnem::EOR, Am::ZRX, 0 },
    { "eor", 0x4D, Mnem::EOR, Am::ABS, 0 },
    { "eor", 0x5D, Mnem::EOR, Am::ABX, 0 },
    { "eor", 0x59, Mnem::EOR, Am::ABY, 0 },
    { "eor", 0x41, Mnem::EOR, Am::INX, 0 },
    { "eor", 0x51, Mnem::EOR, Am::INY, 0 },

    // inc
    { "inc", 0xE6, Mnem::INC, Am::ZRP, OpInfo::FLAG_WRITE },
    { "inc", 0xF6, Mnem::INC, Am::ZRX, OpInfo::FLAG_WRITE },
    { "inc", 0xEE, Mnem::INC, Am::ABS, OpInfo::FLAG_WRITE },
    { "inc", 0xFE, Mnem::INC, Am::ABX, OpInfo::FLAG_WRITE },

    // inx
    { "inx", 0xE8, Mnem::INX, Am::IMP, 0 },

    // iny
    { "iny", 0xC8, Mnem::INY, Am::IMP, 0 },

    // jmp
    { "jmp", 0x4C, Mnem::JMP, Am::ABS, OpInfo::FLAG_JUMP | OpInfo::FLAG_END },
    { "jmp", 0x6C, Mnem::JMP, Am::IAB, OpInfo::FLAG_JUMP | OpInfo::FLAG_END },

    // jsr
    { "jsr", 0x20, Mnem::JSR, Am::ABS, OpInfo::FLAG_JUMP | OpInfo::FLAG_CALL },

    // lda
    { "lda", 0xA9, Mnem::LDA, Am::IMM, 0 },
    { "lda", 0xA5, Mnem::LDA, Am::ZRP, 0 },
    { "lda", 0xB5, Mnem::LDA, Am::ZRX, 0 },
    { "lda", 0xAD, Mnem::LDA, Am::ABS, 0 },
    { "lda", 0xBD, Mnem::LDA, Am::ABX, 0 },
    { "lda", 0xB9, Mnem::LDA, Am::ABY, 0 },
    { "lda", 0xA1, Mnem::LDA, Am::INX, 0 },
    { "lda", 0xB1, Mnem::LDA, Am::INY, 0 },

    // ldx
    { "ldx", 0xA2, Mnem::LDX, Am::IMM, 0 },
    { "ldx", 0xA6, Mnem::LDX, Am::ZRP, 0 },
    { "ldx", 0xB6, Mnem::LDX, Am::ZRY, 0 },
    { "ldx", 0xAE, Mnem::LDX, Am::ABS, 0 },
    { "ldx", 0xBE, Mnem::LDX, Am::ABY, 0 },

    // ldy
    { "ldy", 0xA0, Mnem::LDY, Am::IMM, 0 },
    { "ldy", 0xA4, Mnem::LDY, Am::ZRP, 0 },
    { "ldy", 0xB4, Mnem::LDY, Am::ZRX, 0 },
    { "ldy", 0xAC, Mnem::LDY, Am::ABS, 0 },
    { "ldy", 0xBC, Mnem::LDY, Am::ABX, 0 },

    // lsr
    { "lsr", 0x4A, Mnem::LSR, Am::ACC, 0 },
    { "lsr", 0x46, Mnem::LSR, Am::ZRP, OpInfo::FLAG_WRITE },
    { "lsr", 0x56, Mnem::LSR, Am::ZRX, OpInfo::FLAG_WRITE },
    { "lsr", 0x4E, Mnem::LSR, Am::ABS, OpInfo::FLAG_WRITE },
    { "lsr", 0x5E, Mnem::LSR, Am::ABX, OpInfo::FLAG_WRITE },

    // nop
    { "nop", 0xEA, Mnem::NOP, Am::IMP, 0 },

    // ora
    { "ora", 0x09, Mnem::ORA, Am::IMM, 0 },
    { "ora", 0x05, Mnem::ORA, Am::ZRP, 0 },
    { "ora", 0x15, Mnem::ORA, Am::ZRX, 0 },
    { "ora", 0x0D, Mnem::ORA, Am::ABS, 0 },
    { "ora", 0x1D, Mnem::ORA, Am::ABX, 0 },
    { "ora", 0x19, Mnem::ORA, Am::ABY, 0 },
    { "ora", 0x01, Mnem::ORA, Am::INX, 0 },
    { "ora", 0x11, Mnem::ORA, Am::INY, 0 },

    // pha
    { "pha", 0x48, Mnem::PHA, Am::IMP, 0 },

    // php
    { "php", 0x08, Mnem::PHP, Am::IMP, 0 },

    // pla
    { "pla", 0x68, Mnem::PLA, Am::IMP, 0 },

    // plp
    { "plp", 0x28, Mnem::PLP, Am::IMP, 0 },

    // rol
    { "rol", 0x2A, Mnem::ROL, Am::ACC, 0 },
    { "rol", 0x26, Mnem::ROL, Am::ZRP, OpInfo::FLAG_WRITE },
    { "rol", 0x36, Mnem::ROL, Am::ZRX, OpInfo::FLAG_WRITE },
    { "rol", 0x2E, Mnem::ROL, Am::ABS, OpInfo::FLAG_WRITE },
    { "rol", 0x3E, Mnem::ROL, Am::ABX, OpInfo::FLAG_WRITE },

    // ror
    { "ror", 0x6A, Mnem::ROR, Am::ACC, 0 },
    { "ror", 0x66, Mnem::ROR, Am::ZRP, OpInfo::FLAG_WRITE },
    { "ror", 0x76, Mnem::ROR, Am::ZRX, OpInfo::FLAG_WRITE },
    { "ror", 0x6E, Mnem::ROR, Am::ABS, OpInfo::FLAG_WRITE },
    { "ror", 0x7E, Mnem::ROR, Am::ABX, OpInfo::FLAG_WRITE },

    // rti
    { "rti", 0x40, Mnem::RTI, Am::IMP, OpInfo::FLAG_JUMP | OpInfo::FLAG_END },

    // rts
    { "rts", 0x60, Mnem::RTS, Am::IMP, OpInfo::FLAG_JUMP | OpInfo::FLAG_END },

    // sbc
    { "sbc", 0xE9, Mnem::SBC, Am::IMM, 0 },
    { "sbc", 0xE5, Mnem::SBC, Am::ZRP, 0 },
    { "sbc", 0xF5, Mnem::SBC, Am::ZRX, 0 },
    { "sbc", 0xED, Mnem::SBC, Am::ABS, 0 },
    { "sbc", 0xFD, Mnem::SBC, Am::ABX, 0 },
    { "sbc", 0xF9, Mnem::SBC, Am::ABY, 0 },
    { "sbc", 0xE1, Mnem::SBC, Am::INX, 0 },
    { "sbc", 0xF1, Mnem::SBC, Am::INY, 0 },

    // sec
    { "sec", 0x38, Mnem::SEC, Am::IMP, 0 },

    // sed
    { "sed", 0xF8, Mnem::SED, Am::IMP, 0 },

    // sei
    { "sei", 0x78, Mnem::SEI, Am::IMP, 0 },

    // sta
    { "sta", 0x85, Mnem::STA, Am::ZRP, OpInfo::FLAG_WRITE },
    { "sta", 0x95, Mnem::STA, Am::ZRX, OpInfo::FLAG_WRITE },
    { "sta", 0x8D, Mnem::STA, Am::ABS, OpInfo::FLAG_WRITE },
    { "sta", 0x9D, Mnem::STA, Am::ABX, OpInfo::FLAG_WRITE },
    { "sta", 0x99, Mnem::STA, Am::ABY, OpInfo::FLAG_WRITE },
    { "sta", 0x81, Mnem::STA, Am::INX, OpInfo::FLAG_WRITE },
    { "sta", 0x91, Mnem::STA, Am::INY, OpInfo::FLAG_WRITE },

    // stx
    { "stx", 0x86, Mnem::STX, Am::ZRP, OpInfo::FLAG_WRITE },
    { "stx", 0x96, Mnem::STX, Am::ZRY, OpInfo::FLAG_WRITE },
    { "stx", 0x8E, Mnem::STX, Am::ABS, OpInfo::FLAG_WRITE },

    // sty
    { "sty", 0x84, Mnem::STY, Am::ZRP, OpInfo::FLAG_WRITE },
    { "sty", 0x94, Mnem::STY, Am::ZRX, OpInfo::FLAG_WRITE },
    { "sty", 0x8C, Mnem::STY, Am::ABS, OpInfo::FLAG_WRITE },

    // tax
    { "tax", 0xAA, Mnem::TAX, Am::IMP, 0 },

    // tay
    { "tay", 0xA8, Mnem::TAY, Am::IMP, 0 },

    // tsx
    { "tsx", 0xBA, Mnem::TSX, Am::IMP, 0 },

    // txa
    { "txa", 0x8A, Mnem::TXA, Am::IMP, 0 },

    // txs
    { "txs", 0x9A, Mnem::TXS, Am::IMP, 0 },

    // tya
    { "tya", 0x98, Mnem::TYA, Am::IMP, 0 },
};

OpInfo const* find_opcode_info(byte_type opcode)
{
    for (OpInfo const& info : g_opcode_info)
    {
        if (opcode == info.opcode)
            return &info;
    }

    return nullptr;
}

std::size_t get_addressing_mode_operand_size(Am am)
{
    switch (am)
    {

    case Am::IMP:
        return 0;

    case Am::ACC:
        return 0;

    case Am::IMM:
        return 1;

    case Am::ZRP:
    case Am::ZRX:
    case Am::ZRY:
        return 1;

    case Am::ABS:
    case Am::ABX:
    case Am::ABY:
        return 2;

    case Am::IAB:
        return 2;

    case Am::INX:
    case Am::INY:
        return 1;

    case Am::REL:
        return 1;

    default:
        return 0;

    }
}

OpInfo const* OpInfoCache::get_opcode_info(byte_type opcode) const
{
    if (m_lut[opcode] == nullptr)
        m_lut[opcode] = find_opcode_info(opcode);

    return m_lut[opcode];
}

OpInfo const* get_instr_info(Instr const& instr)
{
    return find_opcode_info(instr.opcode);
}

OpInfo const* get_instr_info(Instr const& instr, OpInfoCache const& cache)
{
    return cache.get_opcode_info(instr.opcode);
}

std::size_t get_instr_size(Instr const& instr)
{
    OpInfo const* const info = get_instr_info(instr);

    if (info == nullptr)
        return 1;

    return 1 + get_addressing_mode_operand_size(info->addressing_mode);
}

std::size_t get_instr_size(Instr const& instr, OpInfoCache const& cache)
{
    OpInfo const* const info = get_instr_info(instr, cache);

    if (info == nullptr)
        return 1;

    return 1 + get_addressing_mode_operand_size(info->addressing_mode);
}
