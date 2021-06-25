#include "cpu.h"
#include "imgui/imgui.h"

#include <Windows.h> // GetKeyState
#include <map>

//
// Functions
//

const char *
get_opcode_mode_str(AddressingMode mode)
{
    switch (mode) {
        case AddrMode_Abs:  return "abs";
        case AddrMode_AbsX: return "absx";
        case AddrMode_AbsY: return "absy";
        case AddrMode_Idr:  return "idr";
        case AddrMode_Zp:   return "zp";
        case AddrMode_ZpX:  return "zpx";
        case AddrMode_ZpY:  return "zpy";
        case AddrMode_Rel:  return "rel";
        case AddrMode_Izx:  return "izx";
        case AddrMode_Izy:  return "izy";
        case AddrMode_Imm:  return "imm";
        case AddrMode_Acc:  return "acc";
        case AddrMode_Imp:  return "   ";
        default:            return "???";
    }
}

int 
get_opcode_len(AddressingMode mode)
{
    switch (mode) {
        case AddrMode_Abs: 
        case AddrMode_AbsX: 
        case AddrMode_AbsY: 
        case AddrMode_Idr: 
            return 3;

        case AddrMode_Zp: 
        case AddrMode_ZpX: 
        case AddrMode_ZpY: 
        case AddrMode_Rel: 
        case AddrMode_Izx: 
        case AddrMode_Izy: 
        case AddrMode_Imm: 
            return 2;

        case AddrMode_Acc: 
        case AddrMode_Imp: 
        case AddrMode_Invalid: 
            return 1;

        DEFAULT_INVALID 
    }
}

Vector<AssemblyLine> 
disasm(const Sim6502 &sim, const uint8 *data, size_t len)
{
    Vector<AssemblyLine> result;
    const uint8 *end = data + len;

    // need sim for opcode definitions
    const auto pop8 = [&]() -> uint8 {
        return *data++;
    };
    const auto pop16 = [&]() -> uint16 {
        uint16 u16 = data[0] | ((uint16)data[1] << 8);
        data += 2;
        return u16;
    };
    const auto printlines = [&]() {
        for (auto x : result) 
            printf("%s\n", &x.text[0]);
    };
    
    A16 offset = 0;

    while (data < end) {
        uint8 opcode = pop8();
        Opcode op = sim.optable[ opcode ];

        AssemblyLine line = {};
        line.offset = offset;
        line.mode = (uint8)op.mode;
        char *out = &line.text[0];

        out += sprintf(out, op.name);
        int remain = int(end - data);

        int oplen = get_opcode_len(op.mode); 
        offset += oplen;
        if ((oplen - 1) > remain) { // offset the pop
            printlines();
            BRK; 
        }

        switch (op.mode) {

            // length 3
            case AddrMode_AbsX: 
                out += sprintf(out, " $%04X,X", (uint)pop16());
                break;
            case AddrMode_AbsY: 
                out += sprintf(out, " $%04X,Y", (uint)pop16());
                break;
            case AddrMode_Idr: 
                out += sprintf(out, " ($%04X)", (uint)pop16());
                break;
            case AddrMode_Abs: 
                out += sprintf(out, " $%04X", (uint)pop16());
                break;

            // length 2
            case AddrMode_Izx: 
                out += sprintf(out, " ($%02X,X)", (uint)pop8());
                break;
            case AddrMode_Izy: 
                out += sprintf(out, " ($%02X),Y", (uint)pop8());
                break;
            case AddrMode_Imm: 
                out += sprintf(out, " #$%02X", (uint)pop8());
                break;
            case AddrMode_ZpX: 
                out += sprintf(out, " $%02X,X", (uint)pop8());
                break;
            case AddrMode_ZpY: 
                out += sprintf(out, " $%02X,Y", (uint)pop8());
                break;
            case AddrMode_Zp: 
                out += sprintf(out, " $%02X", (uint)pop8());
                break;
            case AddrMode_Rel: 
                out += sprintf(out, " *%d", (int8)pop8());
                break;

            // length 1, only have opcode name 
            case AddrMode_Acc:   
            case AddrMode_Imp: 
                break;

            case AddrMode_Invalid:
                // dont add invalid lines
                //continue;
                out += sprintf(out, " Hex: %02X", (uint)opcode);
                break;

            default: { 
                printlines();
                BRK; 
            }
        }

        {
            // show addressing mode of instruction
            //for (; out < &line.text[ sizeof(line.text) ]; out++) 
            //    *out = ' ';
            sprintf(&line.text[16], "%s",
                    get_opcode_mode_str(op.mode));
        }

        result.push_back(line);
    }

    // TODO: remove this
    // cap the end for displaying the last line in write_disasm_to_file
    AssemblyLine pad;
    pad.offset = offset;
    strcpy(pad.text, "END");
    result.push_back(pad);

    return result;
}

size_t 
read_filesize(FILE *f)
{
    if (f == NULL) errorf("BAD PARAM");
    __int64 pos = _ftelli64(f);

    _fseeki64(f, 0, SEEK_END);
    size_t filesize = (size_t)_ftelli64(f);
    _fseeki64(f, pos, SEEK_SET);  

    return filesize;
}

// start up the GUI for debugging
void
init_gui_debugging(Sim6502 &sim)
{
    extern void init_imgui();

    if (!sim.debugger.enabled) 
    {
        init_imgui();
    }
    sim.exec_mode = ExecMode_Pause;
    sim.debugger.enabled = true;
}

void 
sim_process_savestate(Sim6502 &sim, const char *filename, bool read)
{
    const char *openflags = (read) ? "rb" : "wb";
    FILE *f = fopen(filename, openflags);
    if (f == NULL) errorf("error opening %s", filename);
    size_t filesize = read_filesize(f);

    struct Variable
    {
        void *address;
        size_t bytes;
    };

    Vector<Variable> vars;

#define VAR(data) vars.push_back( { &data, sizeof(data) } );

    VAR(sim.mem);
    VAR(sim.acc);
    VAR(sim.x);
    VAR(sim.y);
    VAR(sim.pflags);
    VAR(sim.sp);
    VAR(sim.pc);
    VAR(sim.cycle);
    VAR(sim.instruction_number);
    VAR(sim.history);
    VAR(sim.code_segment);
    VAR(sim.image.size);

    if (read)
    {
        // make sure the read size matches
        size_t expected_size = 0;
        for (Variable v : vars)
        {
            expected_size += v.bytes;
        }

        if (expected_size != filesize)
        {
            errorf("size mismatch\n"
                   "actual save size: %d\n"
                   "expected save size: %d\n",
                   (int)filesize, (int)expected_size);
        }
    }

    for (Variable v : vars)
    {
        if (read)
        {
            fread(v.address, v.bytes, 1, f);
        }
        else 
        {
            fwrite(v.address, v.bytes, 1, f);
        }
    }

    if (read)
    {
        size_t nbytes = (sim.image.size < CODE_SEGMENT_SIZE) ?
                            sim.image.size : CODE_SEGMENT_SIZE;
        sim.lines = disasm(sim, &sim.mem[CODE_SEGMENT_START], nbytes);
    }

#undef VAR

    fclose(f);
}

void
query_keyboard(Sim6502 &sim)
{
    // don't allow shortcuts while entering keystrokes 
    if (ImGui::GetIO().WantCaptureKeyboard) return; 
    
    // from Windows.h
    //short __stdcall GetKeyState(int nVirtKey);
    static const auto keydown = [&](int vk_code) -> bool {
        static Map<int, bool> key_map;
        if (key_map.find(vk_code) == key_map.end())
            key_map.insert({ vk_code, false });

        bool state = GetKeyState(vk_code) & 0x8000;
        bool fire = (state != key_map[vk_code] && state == true);
        key_map[vk_code] = state;

        return fire;
    };

    if (keydown('F')) sim.exec_mode = ExecMode_StepForward;
    if (keydown('B')) sim.exec_mode = ExecMode_StepBackward;
    if (keydown('P')) sim.exec_mode = ExecMode_Pause;
    if (keydown('R')) sim.exec_mode = ExecMode_Continuous;
    if (keydown('D')) init_gui_debugging(sim);
    if (keydown('O')) sim.debugger.break_once = true;

}

// load binary image into buf
void
load_image(Sim6502 &cpu, const char *filename, A16 offset)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        errorf("Error opening file: %s", filename);
    }

    size_t filesize = read_filesize(f);

    if ( filesize > (MEM_SIZE - offset) ) errorf("NOT ENOUGH ROOM");

    fread(&cpu.mem[offset], 1, filesize, f);
    cpu.image.size = filesize;
    cpu.image.filename = filename;

    fclose(f);
}

void
write_disasm_to_file(Sim6502 &sim)
{
    FILE *f = fopen("disasm.txt", "wb");
    for (size_t i = 0; i < sim.lines.size() - 1; i++) {
        AssemblyLine &line = sim.lines[i];
        fprintf(f, "%s\n", line.text);
        continue;

        fprintf(f, "%04X: %-16s", sim.code_segment + line.offset, line.text);

        size_t len = sim.lines[i + 1].offset - line.offset;
        for (size_t j = 0; j < len; j++) {
            fprintf(f, "%02X ", sim.mem[sim.code_segment + line.offset + j]);
        }

        fprintf(f, "\n");
    }
    fclose(f);
}

void
add_mutation(Sim6502 &sim, Mutation mut)
{
    sim.history.changelog[sim.history.mut_idx] = mut;
    sim.history.mut_idx = (sim.history.mut_idx + 1) % MUTATION_COUNT;
}

void
mem_write(Sim6502 &sim, A16 addr, uint8 data)
{
    add_mutation(sim, {addr, data, sim.mem[addr], sim.instruction_number} );  
    sim.mem[addr] = data;
}

uint8
mem_read(const Sim6502 &sim, A16 addr)
{
    return sim.mem[addr];
}

/// push byte onto the stack, different from x86 push
/// 1: assign byte, 2: dec SP
void
push_stack(Sim6502 &sim, uint8 data) 
{
    Mutation mut = 
    {
        (uint32)STACK_ADDR + sim.sp,
        data, sim.mem[STACK_ADDR + sim.sp], 
        sim.instruction_number
    };
    add_mutation(sim, mut);  
    sim.mem[STACK_ADDR + sim.sp] = data; 
    sim.sp -= 1;
}

// pop byte from stack, different from x86 pop
// 1: inc SP, 2: return [SP]
uint8
pop_stack(Sim6502 &sim)
{
    uint8 res;

    sim.sp += 1;
    res = sim.mem[STACK_ADDR + sim.sp];

    // DEBUG: set previous top of stack to zero for easier debugging
    //mem_write(sim, STACK_ADDR + sim.sp, 0);

    return res;
}

void 
setnz(Sim6502 &sim, uint8 data)
{
    sim.pflags.zero = (data == 0);
    sim.pflags.neg = ( (data & 0x80) != 0 );
}

// save sim data for modifiction checks after an instruction
struct CheckPoint
{
    A16 pc; Reg sp; Reg acc; Reg x; Reg y; uint8 pflags;
};

CheckPoint 
get_checkpoint(Sim6502 &sim)
{
    return {sim.pc, sim.sp, sim.acc, sim.x, sim.y, sim.pflags.data};
}

void
write_instruction_mutations(Sim6502 &sim, CheckPoint old)
{
    if (sim.acc != old.acc)
        add_mutation(sim, { OverMax_acc, sim.acc, old.acc, sim.instruction_number} );
    if (sim.x != old.x)
        add_mutation(sim, { OverMax_x, sim.x, old.x, sim.instruction_number} );
    if (sim.y != old.y)
        add_mutation(sim, { OverMax_y, sim.y, old.y, sim.instruction_number} );
    if (sim.sp != old.sp)
        add_mutation(sim, { OverMax_sp, sim.sp, old.sp, sim.instruction_number} );
    if (sim.pflags.data != old.pflags)
        add_mutation(sim, { OverMax_p, sim.pflags.data, old.pflags, sim.instruction_number} );
    if (READ_HI(sim.pc) != READ_HI(old.pc))
        add_mutation(sim, { OverMax_pch, READ_HI(sim.pc), READ_HI(old.pc), sim.instruction_number} );
    if (READ_LO(sim.pc) != READ_LO(old.pc))
        add_mutation(sim, { OverMax_pcl, READ_LO(sim.pc), READ_LO(old.pc), sim.instruction_number} );
}

void 
revert_last_instruction_mutations(Sim6502 &sim)
{
    // revert the latest instruction number mutations of the changelog
    Sim6502::History &hist = sim.history;
    const auto decrement_index = [](uint8 &idx) 
    {
        idx = (MUTATION_COUNT - 1 + idx) % MUTATION_COUNT;
    };

    // decrement the mutation index and point to the last 
    // mutation in the changelog (TODO might change this behavior)
    uint8 start_idx = hist.mut_idx;
    decrement_index(start_idx);
    int32 start_instruction = hist.changelog[start_idx].instruction_number;
    if (start_instruction == 0) return; // ran out of mutations to revert

    for (uint8 i = start_idx; 
         hist.changelog[i].instruction_number == start_instruction; 
         decrement_index(i)) 
    {
        Mutation &mut = hist.changelog[i];
        uint8 value = mut.old_value;

        hist.mut_idx = i;
        sim.instruction_number = mut.instruction_number;

        if (!is_register_location(mut)) 
        {
            sim.mem[mut.location & 0xFFFF] = value;
        } 
        else 
        {
            switch(mut.location) {
            case OverMax_acc:
                sim.acc = value;
                break;
            case OverMax_x:
                sim.x = value;
                break;
            case OverMax_y:
                sim.y = value;
                break;
            case OverMax_sp:
                sim.sp = value;
                break;
            case OverMax_pch:
                WRITE_HI(sim.pc, value);
                break;
            case OverMax_pcl:
                WRITE_LO(sim.pc, value);
                break;
            case OverMax_p:
                sim.pflags.data = value;
                break;
            }
        }
        mut = { 0 };
    }
}



// ADDRESSES:
// Start    End
// 0xFFFE   0xFFFF IRQ/BRK
// 0xFFFC   0xFFFD RESET
// 0xFFFA   0xFFFB NMI

void 
dev_interrupt(Sim6502 &sim) 
{
    // from https://www.pagetable.com/?p=410
    // All interrupts reuse the BRK opcode with different attributes set 
    // IRQ, RESET, NMI, and default BRK
    (void)sim;
}


// struct Err6502 {
//     // Stack Underflow (error?)
//     // Stack Overflow (error?)
//     // Access Violation
//     // Invalid Instruction
// }

void
load_program(Sim6502 &sim, A16 code_segment, const char *filename)
{
    load_image(sim, filename, code_segment);
    sim.code_segment = code_segment;
    sim.pc = code_segment;
    sim.lines = disasm(sim, &sim.mem[code_segment], sim.image.size);    
}

void
load_program_64k(Sim6502 &sim, A16 code_segment, const char *filename)
{
    load_image(sim, filename, 0);

    // load an image that takes up entire memory space
    if (sim.image.size != MEM_SIZE) errorf("NOT 64k");

    sim.code_segment = code_segment;
    sim.pc = code_segment;
    sim.lines = disasm(sim, &sim.mem[code_segment], 
                       ROM_SEGMENT_START - code_segment);    
}

void 
dev_init()
{
    // opcodes.h
    extern void init_opcodes(Opcode (&table)[OPCODE_COUNT]);

    // imgui_impl.cpp
    extern bool step_frame_imgui(Sim6502 &sim);
    extern void shutdown_imgui();

    Sim6502 sim = { 0 };
    init_opcodes(sim.optable);
    load_program_64k(sim, CODE_SEGMENT_START, "6502_functional_test.bin");
    //load_program(sim, 0x4000, "AllSuiteA.bin");
    //write_disasm_to_file(sim);

    bool run_program = true;
    uint64 all_cycles = 0;

    sim.exec_mode = ExecMode_Continuous;
    sim.cycle = 1;
    sim.instruction_number = 0;
    sim.sp = 0xFF;
    //init_gui_debugging(sim);

    {
        // system loop
        CheckPoint chk = {};
        write_instruction_mutations(sim, chk);

        //sim_process_savestate(sim, "6502_functional_test.save", true);
        while (run_program) 
        {
            if (sim.exec_mode == ExecMode_StepBackward) 
            {
                revert_last_instruction_mutations(sim);
                sim.exec_mode = ExecMode_Pause;
            }

            all_cycles += 1;
            {
                // process opcodes in main memory
                if (sim.cycle == 1) 
                {
                    // first cycle is always reading the opcode
                    if (sim.exec_mode != ExecMode_Pause) 
                    {
                        sim.ophex = mem_read(sim, sim.pc);
                        sim.instruction_number += 1;
                        chk = get_checkpoint(sim);
                        sim.cycle += 1;
                    }
                }
                else 
                {
                    // invoke a programatic breakpoint for investigating functions below
                    if (sim.debugger.break_once) {
                        BRK; sim.debugger.break_once = false;
                    }

                    sim.optable[sim.ophex].func(sim);
                    if (sim.cycle == 1) 
                    {
                        // function reset the cycle for the next instruction
                        write_instruction_mutations(sim, chk);
                        if (sim.exec_mode == ExecMode_StepForward)
                            sim.exec_mode = ExecMode_Pause;
                    } 
                    else 
                    {
                        sim.cycle += 1;
                    }                
                }
            }

            // do ticks for all other subsystems (Sound, IO, GUI)
            if (sim.debugger.enabled) query_keyboard(sim);
            if (sim.exec_mode == ExecMode_Pause && sim.debugger.enabled) run_program = step_frame_imgui(sim);
            if (all_cycles % 1'000'000 == 0) 
                printf("\rinstruction: %llu", all_cycles);
        }

    }

    if (sim.debugger.enabled) 
        shutdown_imgui();
}


#if 0

// (VERY) simple test functions
// TODO LIST: 
// 1. calling add_test without making dummy variable
// 2. how to call run_tests (post-build, preprocessor conditional)
// 3. timing the tests
//      -testing a condition to take place within a timespan
//      -displaying the time it took to execute all tests
// 4. test.cpp and test.h files

struct TestContext;
typedef void(*test_fn)(TestContext&);

struct Test
{
    test_fn func;
    const char *name;
};

struct TestContext
{
    // result, lines describing how it failed
    bool passed = true;
    const char *text = nullptr;
};

std::vector<Test> tests;

void run_tests()
{
    int pass_count = 0;
    int fail_count = 0;

    for (Test &test : tests) 
    {
        TestContext ctx;
        test.func(ctx);

        if (ctx.passed) 
        {
            printf("[PASS    ] %s\n", test.name);
            pass_count++;
        }
        else 
        {
            printf("[    FAIL] %s\n", test.name);
            printf("           :%s\n", ctx.text);
            fail_count++;
        }
    }

    printf("\n"
           "Tests Passed: %d\n"
           "Tests Failed: %d\n", pass_count, fail_count);
}

char add_test(test_fn func, const char *name)
{
    // hack to add tests in the global scope
    tests.push_back( { func, name } );
    return 0;
}

// shenanigans to get c preprocessor to tokenize __LINE__ and __COUNTER__
#define CONCAT(x,y) x##y
#define CONCAT2(x,y) CONCAT(x,y)

// declare a test function and add it to the global tests collection
#define DECLARE_TEST(function_name) \
extern void function_name(TestContext &ctx); \
static char CONCAT2(__test_,__COUNTER__) = add_test(function_name, #function_name); \
void function_name(TestContext &ctx)

// testing macros
#define GENERIC_TEST(cond) do { \
    if ( !(cond) ) {            \
        ctx.passed = false;     \
        ctx.text = tempf("condition failed: %s", #cond); \
    }                           \
} while(0)                      

#define TEST_ASSERT_EQ(a,b) GENERIC_TEST(a == b)
#define TEST_ASSERT_NEQ(a,b) GENERIC_TEST(a != b)
#define TEST_ASSERT_LT(a,b) GENERIC_TEST(a < b)
#define TEST_ASSERT_LTE(a,b) GENERIC_TEST(a <= b)
#define TEST_ASSERT_GT(a,b) GENERIC_TEST(a > b)
#define TEST_ASSERT_GTE(a,b) GENERIC_TEST(a >= b)
#define TEST_ASSERT GENERIC_TEST(a)

#define TEST_ASSERT_STREQ(a,b) GENERIC_TEST( (a && b) && strlen(a) == strlen(b))
#define TEST_ASSERT_STRNEQ(a,b) GENERIC_TEST( (a && b) && strlen(a) != strlen(b))


DECLARE_TEST(OpcodeADC)
{

}

#endif



void 
string_program_testing()
{
    #define TEST(str) disasm(sim, str, sizeof(str) - 1);
    const char hex[] = 
        "\x20\x06\x06\x20\x38\x06\x20\x0D\x06\x20\x2A\x06\x60\xA9\x02\x85"
        "\x02\xA9\x04\x85\x03\xA9\x11\x85\x10\xA9\x10\x85\x12\xA9\x0F\x85"
        "\x14\xA9\x04\x85\x11\x85\x13\x85\x15\x60\xA5\xFE\x85\x00\xA5\xFE"
        "\x29\x03\x18\x69\x02\x85\x01\x60\x20\x4D\x06\x20\x8D\x06\x20\xC3"
        "\x06\x20\x19\x07\x20\x20\x07\x20\x2D\x07\x4C\x38\x06\xA5\xFF\xC9"
        "\x77\xF0\x0D\xC9\x64\xF0\x14\xC9\x73\xF0\x1B\xC9\x61\xF0\x22\x60"
        "\xA9\x04\x24\x02\xD0\x26\xA9\x01\x85\x02\x60\xA9\x08\x24\x02\xD0"
        "\x1B\xA9\x02\x85\x02\x60\xA9\x01\x24\x02\xD0\x10\xA9\x04\x85\x02"
        "\x60\xA9\x02\x24\x02\xD0\x05\xA9\x08\x85\x02\x60\x60\x20\x94\x06"
        "\x20\xA8\x06\x60\xA5\x00\xC5\x10\xD0\x0D\xA5\x01\xC5\x11\xD0\x07"
        "\xE6\x03\xE6\x03\x20\x2A\x06\x60\xA2\x02\xB5\x10\xC5\x10\xD0\x06"
        "\xB5\x11\xC5\x11\xF0\x09\xE8\xE8\xE4\x03\xF0\x06\x4C\xAA\x06\x4C"
        "\x35\x07\x60\xA6\x03\xCA\x8A\xB5\x10\x95\x12\xCA\x10\xF9\xA5\x02"
        "\x4A\xB0\x09\x4A\xB0\x19\x4A\xB0\x1F\x4A\xB0\x2F\xA5\x10\x38\xE9"
        "\x20\x85\x10\x90\x01\x60\xC6\x11\xA9\x01\xC5\x11\xF0\x28\x60\xE6"
        "\x10\xA9\x1F\x24\x10\xF0\x1F\x60\xA5\x10\x18\x69\x20\x85\x10\xB0"
        "\x01\x60\xE6\x11\xA9\x06\xC5\x11\xF0\x0C\x60\xC6\x10\xA5\x10\x29"
        "\x1F\xC9\x1F\xF0\x01\x60\x4C\x35\x07\xA0\x00\xA5\xFE\x91\x00\x60"
        "\xA6\x03\xA9\x00\x81\x10\xA2\x00\xA9\x01\x81\x10\x60\xA2\x00\xEA"
        "\xEA\xCA\xD0\xFB\x60";
}

// generated opcode functions
// Info from MCS6500 Microcomputer Family Hardware Manual January 1976
// Single Cycle Execution timing in Appendix A
#include "opcode.h"
