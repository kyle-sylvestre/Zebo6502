#pragma once

enum ExecMode
{
    ExecMode_Pause,
    ExecMode_StepForward,
    ExecMode_StepBackward,
    ExecMode_Continuous,
};

struct Simulator
{
    Base6502 dev;

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

    struct Debugger
    {
        // is the GUI up and running
        bool enabled;       

        // stop program once before opcode execution
        bool break_once;    
    } debugger;
};
