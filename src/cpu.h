#pragma once

#include "common.h"
#include <vector>
#include <map>


//
// Common CPU Definitions
//

struct Sim6502;

typedef uint16 A16;
typedef uint8 Reg;
typedef void (*opcode_fn)(Sim6502 &);
template<typename T> using Vector = std::vector<T>;
template<typename T, size_t S> using Array = std::array<T, S>;
template<typename K, typename V> using Map = std::map<K, V>;

// Constants
const uint MEM_SIZE = 64 * 1024;
const A16 ROM_SEGMENT_START = 0x8000;
const A16 CODE_SEGMENT_START = 0x0400;
const A16 CODE_SEGMENT_SIZE = ROM_SEGMENT_START - CODE_SEGMENT_START;
const uint16 PAGE_SIZE = 0x0100;
const uint16 STACK_ADDR = 0x0100;
const int OPCODE_COUNT = 256;
const int MUTATION_COUNT = 256;

struct Opcode
{
    const char *name;
    opcode_fn func;
    enum AddressingMode mode;
};

struct MemPage
{
    uint8 data[PAGE_SIZE];
};

// memory indices of registers
// (no memory places guaranteed to be unused)
enum OverMax
{
    OverMax_acc = MEM_SIZE,
    OverMax_x,
    OverMax_y,

    // TODO add variables for PC hi and lo
    OverMax_sp,
    OverMax_pch,
    OverMax_pcl,
    OverMax_p,
};

enum ExecMode
{
    ExecMode_Pause,
    ExecMode_StepForward,
    ExecMode_StepBackward,
    ExecMode_Continuous,
};

// record a mutation taking place
// execution order is old_value, OPERATION, new_value
struct Mutation
{
    // location of the data being mutated
    // TODO: uint32 for sim registers
    // could make this uint16 if there is a guaranteed unused location to use
    uint32 location;    

    // value being set at the location
    uint8 new_value;

    // old value being replaced at the location
    uint8 old_value;

    // instruction number of the mutation
    int32 instruction_number;
};

inline bool
is_register_location(const Mutation &mut)
{
    return (mut.location >= MEM_SIZE);
}

struct AssemblyLine
{
    // offset from load address
    uint16 offset;

    // addressing mode
    uint8 mode; 

    // disassembly of the instruction
    char text[32];
};

struct Sim6502
{
    // system memory
    uint8 mem[MEM_SIZE];

    // accumulator 
    Reg acc;

    // temporary scratch values
    uint16 tmp16;
    uint8 tmp8;

    // index registers
    Reg x;
    Reg y;

    // processor status flags
    union
    {
        struct 
        {
            uint8 carry     : 1;    // carry 
            uint8 zero      : 1;    // zero
            uint8 intdis    : 1;    // interrupt disable
            uint8 deci      : 1;    // decimal

            uint8 brk       : 1;    // break
            uint8 pad       : 1;    // pad
            uint8 ovrf      : 1;    // overflow
            uint8 neg       : 1;    // negative
        };
        uint8 data;
    } pflags;

    // stack pointer
    Reg sp;

    // program counter
    A16 pc;

    // cycle number for current instruction
    int cycle;

    // instruction number waiting to be executed
    int32 instruction_number;

    // last parsed opcode
    uint8 ophex;

    // all the opcode functions
    Opcode optable[OPCODE_COUNT];

    // starting address of program loaded in
    uint16 code_segment;



    // 
    // global state 
    //

    // assembly lines
    Vector<AssemblyLine> lines;

    // program execution state
    enum ExecMode exec_mode;

    // binary image loaded into [mem]
    struct BinaryImage
    {
        size_t size;
        const char *filename;
        // TODO A16 code_segment;
    } image;

    // variables related to time machine debugging
    struct History
    {
        // ring buffer of most recent changes
        Mutation changelog[MUTATION_COUNT];

        // current mutation ring buffer index
        uint8 mut_idx;

    } history;

    struct Debugger
    {
        // is the GUI up and running
        bool enabled;       

        // stop program once before opcode execution
        bool break_once;    
    } debugger;
};

enum AddressingMode {
    AddrMode_Invalid,// Any other mode not listed
    AddrMode_Imp,    // Implicit
    AddrMode_Acc,    // Accumulator
    AddrMode_Imm,    // Immediate
    AddrMode_Zp,     // ZeroPage
    AddrMode_ZpX,    // ZeroPageX
    AddrMode_ZpY,    // ZeroPageY
    AddrMode_Rel,    // Relative
    AddrMode_Abs,    // Absolute
    AddrMode_AbsX,   // AbsoluteX
    AddrMode_AbsY,   // AbsoluteY
    AddrMode_Idr,    // Indirect
    AddrMode_Izx, // Indexed-Indirect ADC ($AA, X)
    AddrMode_Izy, // Indirect-Indexed ADC ($AA), Y
};

// Function Declarations
inline void WRITE_LO(uint16 &dest16, uint8 byte) { dest16 = (dest16 & 0xFF00) | byte; }
inline void WRITE_HI(uint16 &dest16, uint8 byte) { dest16 = (dest16 & 0x00FF) | (byte << 8); }
inline uint8 READ_LO(uint16 src16) { return (src16 & 0xFF); }
inline uint8 READ_HI(uint16 src16) { return (src16 >> 8); }

void sim_process_savestate(Sim6502 &sim, const char *filename, bool read);

