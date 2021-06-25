#include <stdio.h>
#include "common.h"

extern void dev_init(); // cpu.cpp
extern int imgui_main(int argc, char** argv); // imgui_impl.cpp

int main(int argc, char **argv)
{
    void(argc && argv);
    //imgui_main(0, NULL);
    dev_init();
}
