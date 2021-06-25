// Dear ImGui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include <windows.h>
#include <vector>
#include <string>
#include "common.h"
#include "cpu.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/glfw3.h"


// const ImVec4 COLOR_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); 
// const ImVec4 COLOR_GREY = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); 
// const ImVec4 COLOR_RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); 
// const ImVec4 COLOR_YELLOW = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); 
// const ImVec4 COLOR_MODIFIED = COLOR_RED; 

#define IM_COLOR(r,g,b) ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f)

const ImVec4 COLOR_WHITE = IM_COLOR(0xFF, 0xFF, 0xFF); 
const ImVec4 COLOR_GREY = IM_COLOR(0x80, 0x80, 0x80); 
const ImVec4 COLOR_RED = IM_COLOR(0xFF, 0x00, 0x00); 
const ImVec4 COLOR_ORANGE = IM_COLOR(0xFF, 0x80, 0x00); 
const ImVec4 COLOR_YELLOW = IM_COLOR(0xFF, 0xFF, 0x00); 
const ImVec4 COLOR_MODIFIED = COLOR_ORANGE; 

ImGuiWindowFlags WINFLAGS_LOCKED =  ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar;
                              
const float BORDER_TOP_MENU_YPOS = 28.0f;
static float CHAR_HEIGHT;
static float CHAR_WIDTH;

struct GuiMemRegion
{
    uint16 offset;  // offset from base address passed in
    const char *name;
};

struct ScopedWindow
{
    bool collapsed;
    ScopedWindow(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
    {
        collapsed = ImGui::Begin(name, p_open, flags);
    }
    ~ScopedWindow() { ImGui::End(); }
};



const int FILE_BUF_LEN = MAX_PATH + 1;
struct FileWindowResult
{
    bool selected;
    char file[FILE_BUF_LEN]; 
};

enum FileWindowMode
{
    FileWindowMode_SelectFile,
    FileWindowMode_SelectDirectory,
    FileWindowMode_WriteFile,
};

// rudimentary file picker for win32
// param[inout] res: 
//     res.selected: submitted? true, canceled? false
//     res.file: file that was submitted from the window
// param[in] mode:
//     how the window is presenting the files
// param[in] directory:
//     directory to start in
// param[in] filters:
//     comma separated list of extensions to filter for, "*" for no filtering
//     (only used with FileWindowMode_SelectFile)
// return: bool (window closed)
//     -if submit or cancel has been clicked, return true
//     -else, return false
static bool 
draw_file_window(FileWindowResult *res, FileWindowMode mode, 
                 const char *directory = ".", const char *filters = "*")
{
    // TODO
    // displaying the current filters for open file
    // top region that is fixed and doesn't scroll with rest of window
    // different file window modes:
    //  1. write file
    //      -block writing readonly files, warn on overwriting files
    //  2. select existing file
    //  3. select existing directory

    static const auto PushDisabled = [](bool disable)
    {
        if (disable)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
    };

    static const auto PopDisabled = [](bool disable)
    {
        if (disable)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
    };

    if (!res || !directory || !filters) return true; // force a window close status

    const char *window_name =
        (mode == FileWindowMode_WriteFile) ? "Write File" :
        (mode == FileWindowMode_SelectDirectory) ? "Open Directory" :
        (mode == FileWindowMode_SelectFile) ? "Open File" : "???";

    ScopedWindow win(window_name);

    const int TMP_LEN = 512;
    const int INVALID_FILE_INDEX = -1;
    const int USER_INPUT_LEN = 128;
    int uid = 0; // prevent duplicate names from messing up ImGui objects

    struct FileWindowContext 
    {
        int select_index = INVALID_FILE_INDEX;
        bool window_opened = true;
        bool query_directory = true;
        char tmp[TMP_LEN] = {};
        char user_input[USER_INPUT_LEN] = {};
        std::string path = "";
        std::vector<std::string> dirs = {};
        std::vector<std::string> files = {};
    } static ctx;

    if (ctx.window_opened) 
    {
        ctx.window_opened = false;
        ctx.query_directory = true;

        // ensure path is canonical
        GetFullPathNameA(directory, TMP_LEN, &ctx.tmp[0], NULL);
        ctx.path = std::string(ctx.tmp);
    }

    static const auto FilterWindowsFilename = [](ImGuiInputTextCallbackData *data) -> int
    {
        if (data->BufTextLen == 0) return 0;

        char c = data->Buf[data->BufTextLen - 1];
        bool valid_char = 
            c >= 32 && c <= 126 && // not control or DEL
            c != '<' && c != '>' && c != ':' && c != '"' && // invalid windows file chars
            c != '/' && c != '\\' && c != '|' && c != '?' && c != '*';
        
        if (!valid_char)
            data->DeleteChars(data->BufTextLen - 1, 1);

        // TODO: pass in context and check if the input fits the path max length

        return 0;
    };

    bool submit_disabled;
    if (mode == FileWindowMode_WriteFile)
    {
        // compare the entered extension against the filters list
        // implemented again later in query_directory if branch
        submit_disabled = true;
        const char *ext = strrchr(ctx.user_input, '.');

        if (!ext) 
        {
            // set false earlier
        }
        else if (filters && filters[0] == '*')
        {
            // if there is no save filter, just check for anything past '.'
            submit_disabled = (ext[1] == '\0');
        }
        else 
        {
            ext += 1; // offset '.'

            // parse the comma separated filters for acceptable file extensions
            char filter_buf[128]; 
            strcpy(filter_buf, filters);

            for (char *iter = strtok(filter_buf, ","); 
                 iter != NULL; 
                 iter = strtok(NULL, ","))
            {
                if (0 == strcmp(iter, ext)) 
                {
                    submit_disabled = false;
                    break;
                }
            }
        }
    }
    else 
    {
        // enable submit once the user has selected a file
        submit_disabled = (ctx.select_index == INVALID_FILE_INDEX);
    }

    PushDisabled(submit_disabled);

    if (ImGui::Button("Submit")) 
    {
        // check if there are any files with the same name in the directory
        if (mode != FileWindowMode_SelectDirectory) 
        {
            bool dupe = false;
            for (const auto &filename : ctx.files)
            {
                if (0 == strcmp(ctx.user_input, filename.c_str()))
                {
                    dupe = true;  
                    break;
                }
            }

            // TODO: popup warning for overwriting file
        }

        // setting the return file 
        ctx.path += "\\" + std::string(ctx.user_input);
        snprintf(res->file, ArrayCount(res->file), "%s", ctx.path.c_str());

        // prepare context for next window call
        ctx.select_index = INVALID_FILE_INDEX;
        ctx.window_opened = true;
        res->selected = true;
        return true;
    }
    PopDisabled(submit_disabled);

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) 
    {
        // prepare context for next window call
        ctx.select_index = INVALID_FILE_INDEX;
        ctx.window_opened = true;

        res->selected = false;
        for (char &c : res->file) c = '\0';
        return true;
    }

    bool disable_input = mode == FileWindowMode_SelectDirectory ||
                         mode == FileWindowMode_SelectFile;

    PushDisabled(disable_input);

    ImGui::InputText("", ctx.user_input, USER_INPUT_LEN, 
                     ImGuiInputTextFlags_CallbackAlways, 
                     FilterWindowsFilename, NULL);

    PopDisabled(disable_input);

    // list current directory as a row of buttons
    size_t offset = 0;
    strcpy(ctx.tmp, ctx.path.c_str());
    const float WIN_WIDTH = ImGui::GetWindowWidth();

    ImGui::Text(""); // pad before listing current directory

    // list the current directory path as a series of buttons 
    // "C:/Windows/System32" displays as [C:] [Windows] [System32]
    for (char *iter = strtok(ctx.tmp, "\\"); 
         iter != NULL; 
         iter = strtok(NULL, "\\"))
    {
        size_t iter_len = strlen(iter) + 1; // offset path separator
        offset += iter_len;

        // check if the next button fits on line
        auto text_size = ImGui::CalcTextSize(iter, NULL, true);
        auto next_line = ImGui::GetCursorPos();
        ImGui::SameLine();
        auto same_line = ImGui::GetCursorPos();

        if (same_line.x + text_size.x > WIN_WIDTH) {
            // jump to next line
            ImGui::SetCursorPos(next_line);
        }

        if (ImGui::Button( tempf("%s##%d", iter, uid++) ) && offset <= ctx.path.size()) {
            ctx.path.erase(offset - 1); // offset path separator to not have the separator at end
            ctx.query_directory = true;
        }

    }
    
    ImGui::Text(""); // pad between directory and its contents

    // record all the files and folders of the current directory
    if (ctx.query_directory)
    {
        ctx.query_directory = false;
        ctx.dirs.clear();
        ctx.files.clear();

        // reset user input for entering a new directory
        for (char &c : ctx.user_input) c = '\0';
        ctx.select_index = INVALID_FILE_INDEX; 

        // query current directory for all its contents
        // trailing '*' in FindFirstFileA means "Find all files of the directory"
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA( (ctx.path + "\\*").c_str(), &find_data);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do 
            {
                const char *ptr = find_data.cFileName;
                if (ptr[0] == '.' && ptr[1] == '\0')
                    continue; // skip current directory
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                    continue; // skip hidden files

                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    ctx.dirs.push_back(ptr);
                }
                else 
                {
                    // check the file extension against the filters 
                    const char *ext = strrchr(ptr, '.');
                    if (!ext) continue;
                    ext += 1; // offset '.'

                    bool push_file = false;
                    if (filters[0] == '*') push_file = true;

                    // parse the comma separated filters for acceptable file extensions
                    char filter_buf[64]; 
                    strcpy(filter_buf, filters);

                    for (char *iter = strtok(filter_buf, ","); 
                         !push_file && iter != NULL; 
                         iter = strtok(NULL, ","))
                    {
                        if (0 == strcmp(iter, ext)) push_file = true;
                    }

                    if (push_file)
                        ctx.files.push_back(ptr);
                }

            } while (FindNextFileA(hFind, &find_data));
        }
    }

    // draw the directories as buttons 
    int dir_idx = 0;
    for (const auto &dirname : ctx.dirs) 
    {
        // additional radio button for selecting directories
        if (mode == FileWindowMode_SelectDirectory && dirname != "..")
        {
            // radio button for the directory (skip parent directory entry)
            if (ImGui::RadioButton( tempf("##%d", uid++), &ctx.select_index, dir_idx))
            {
                strcpy(ctx.user_input, dirname.c_str());
            }
            ImGui::SameLine();
        } 

        if (ImGui::Button( tempf("%s##%d", dirname.c_str(), uid++)) ) 
        {
            // navigate to the selected button directory
            ctx.path += "\\" + dirname;
            GetFullPathNameA(ctx.path.c_str(), TMP_LEN, &ctx.tmp[0], NULL);
            ctx.path = std::string(ctx.tmp);
            ctx.select_index = INVALID_FILE_INDEX;
            ctx.query_directory = true;
        }
        dir_idx++;
    }

    // draw the files as radio buttons
    if (mode != FileWindowMode_SelectDirectory)
    {
        int idx = 0;
        for (const auto &filename : ctx.files)
        {
            if (ImGui::RadioButton(filename.c_str(), &ctx.select_index, idx))
            {
                strcpy(ctx.user_input, filename.c_str());
            } 
            idx += 1;
        }
    } 

    return false; // actual return is higher up with Cancel/Submit buttons
}

static void 
glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static bool
is_modified(uint32 location, const Sim6502 &sim)
{
    for (const Mutation &m : sim.history.changelog) 
    {
        if (m.instruction_number == sim.instruction_number &&
            m.location == location) 
        {
            return true;
        }
    }

    return false;
}

// draw 128 bytes starting at [offset] addr in 16-bit memory space
// TODO: how to pass in address that have changed data
static void 
draw_mem_region(const Sim6502 &sim, uint8 *base, GuiMemRegion &region)
{
    const int BYTES = 256;
    const int COLUMN_COUNT = 16;
    const int LINE_COUNT = BYTES / COLUMN_COUNT; 
    uint16 offset = region.offset;

    static const auto hex = [](int nibble) -> char
    {
        return "0123456789ABCDEF"[ nibble & 0xF ];
    };

    {
        float char_width = 12.1f;                  // We assume the font is mono-space
         
                                                                            // input hex 
        ScopedWindow win(tempf("Memory Region %s", region.name), NULL,
                        ImGuiWindowFlags_AlwaysAutoResize); // Create a window called 
        //ImGui::SetWindowPos({ 920.0f, 408.0f });

        // ImGuiListClipper to separate scrolling / memory selection regions
        float select_width = (5 * char_width);
        char addr[5] = 
        { 
            hex(region.offset >> 12), 
            hex(region.offset >> 8), 
            hex(region.offset >> 4), 
            hex(region.offset), 
            '\0' 
        }; 

        ImGui::PushItemWidth(select_width);
        ImGui::Text("ADDR:"); ImGui::SameLine();
        ImGui::InputText("##foobar", addr, 5, 
                         ImGuiInputTextFlags_CharsHexadecimal | 
                         ImGuiInputTextFlags_CharsUppercase);
        ImGui::PopItemWidth();

        // header row, first bytes
        char max_header[] = "     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";
        max_header[4 + COLUMN_COUNT * 3] = '\0';

        // quickjump locations
        ImGui::SameLine();
        if (ImGui::Button("Zero Page")) 
        {
            addr[0] = addr[1] = addr[2] = addr[3] = '0';
        }

        ImGui::SameLine();
        if (ImGui::Button("Stack")) 
        {
            addr[0] = addr[2] = addr[3] = '0';
            addr[1] = '1';
        }

        if (*addr != '\0') 
        {
            int select_offset = 0;
            sscanf_s(addr, "%X", &select_offset); 
            region.offset = select_offset & 0xFFFF;
        }

        ImGui::Separator();
        //ImGui::SameLine(select_width);
        ImGui::TextColored(COLOR_WHITE, max_header);
        for (int i = 0; i < LINE_COUNT; i++) {
            // starting address of the row
            ImGui::TextColored(COLOR_WHITE, "%04X ", offset);

            for (int z = 0; z < COLUMN_COUNT; z++) 
            {
                ImGui::SameLine( 68.0f + ((3 * char_width) * z) );
                bool mod = is_modified(offset, sim);

                // wrapping uint16 add
                uint b = base[offset++];

                if (mod)
                    ImGui::TextColored(COLOR_MODIFIED, "%02X", b);
                else if (b == 0)
                    ImGui::TextDisabled("%02X", b);
                else
                    ImGui::TextColored(COLOR_WHITE, "%02X", b);
            }
        }
    }
}

static void
draw_instruction_expander(const Sim6502 &sim)
{
    // debug arithmetic in zero page / absolute x y 
    const auto rd = [&](uint16 addr) -> uint { // uint for printf formatting
        return sim.mem[addr];
    };

    ScopedWindow win("Expander", NULL, 0 /*WINFLAGS_LOCKED*/);
    ImGui::SetWindowPos({ 200, BORDER_TOP_MENU_YPOS });
    ImGui::SetWindowSize({ 409, 59 });

    uint16 target = sim.pc - sim.code_segment;
    const AssemblyLine *line = nullptr;

    for (size_t i = 0; i < sim.lines.size(); i++) {
        if (sim.lines[i].offset == target) {
            line = &sim.lines[i];
        }
    }

    char out[64] = "???";
    if (line != nullptr) 
    {
        uint8 lo = (uint8)rd(sim.pc + 1);
        uint8 hi = (uint8)rd(sim.pc + 2);
        uint16 full = ((uint16)(hi << 8)) | lo;

        switch (line->mode) 
        {
            case AddrMode_AbsX: 
            {
                sprintf_s(out, "$%04X,X => $%04X => %02X", 
                          full,
                          full + sim.x,
                          rd(full + sim.x));
            } break;
            case AddrMode_AbsY: 
            {
                sprintf_s(out, "$%04X,Y => $%04X => %02X", 
                          full,
                          full + sim.y,
                          rd(full + sim.y));
            } break;
            case AddrMode_Idr: 
            {
                full = (uint16)(rd(full + 0) | (rd(full + 1) << 8));
                sprintf_s(out, "($%04X) => $%04X", 
                          full, rd(full));
            } break;
            case AddrMode_Abs: 
            {
                sprintf_s(out, "$%04X => %02X", 
                          full, rd(full));
            } break;
            case AddrMode_Izx: 
            {
                full = (lo + sim.x) & 0xFF;
                A16 dst = A16(rd(full) | (rd(full + 1) << 8));
                sprintf_s(out, "($%02X,X) => (%02X) => (%04X) => %02X", 
                          lo, full, (uint)dst, rd(dst) );
            } break;
            case AddrMode_Izy: 
            {
                full = uint16(rd(lo)) | uint16(rd(hi));
                sprintf_s(out, "($%02X),Y => %04X,Y => $%04X => %02X", 
                          lo, full, full + sim.y, rd(full + sim.y));
            } break;
            case AddrMode_Rel: 
            {
                sprintf_s(out, "$%04X %c %d + 2 => $%04X", 
                          sim.pc, 
                          ((int8)lo) >=0 ? '+' : '-',
                          (int)abs((int8)lo),
                          sim.pc + (int8)lo + 2);
            } break;
            case AddrMode_ZpX: 
            {
                full = (lo + sim.x) & 0xFF;
                sprintf_s(out, "$%02X,X => %02X => %02X", 
                          lo, full, rd(full));
            } break;
            case AddrMode_ZpY: 
            {
                full = (lo + sim.y) & 0xFF;
                sprintf_s(out, "$%02X,Y => %02X => %02X", 
                          lo, full, rd(full));
            } break;
            case AddrMode_Zp: 
            {
                full = lo;
                sprintf_s(out, "$%02X => %02X", 
                          lo, rd(lo));
            } break;
            default:
            {
                sprintf(out, "???");
            } break;
        }
    }

    ImGui::Text(out);
}

static void
draw_menu_bar(Sim6502 &sim)
{
    if (ImGui::BeginMainMenuBar())
    {
        static bool file_open_state, file_save_state, file_open_binary;
        static FileWindowResult res = {};
        if (ImGui::BeginMenu("File"))
        {
            file_open_binary |= ImGui::MenuItem("Open Binary");
            file_open_state |= ImGui::MenuItem("Open Simulator State");
            file_save_state |= ImGui::MenuItem("Save Simulator State");
            ImGui::EndMenu();
        }

        if (file_open_binary) 
        {
            if (draw_file_window(&res, FileWindowMode_SelectFile, ".", "bin"))
            {
                // TODO: load binary (64k, need load address for partial)
                file_open_binary = false;
            }
        }

        if (file_open_state) 
        {
            if (draw_file_window(&res, FileWindowMode_SelectFile, ".", "save"))
            {
                if (res.selected)
                    sim_process_savestate(sim, res.file, true);
                file_open_state = false;
            }
        }

        if (file_save_state) 
        {
            if (draw_file_window(&res, FileWindowMode_WriteFile, ".", "save"))
            {
                if (res.selected)
                    sim_process_savestate(sim, res.file, false);
                file_save_state = false;
            }
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

static void
draw_assembly(const Sim6502 &sim)
{
    ScopedWindow win("Assembly", NULL, WINFLAGS_LOCKED); // Create a window called "Hello, world!" and append into it.
    ImGui::SetWindowPos({ 0.0f, BORDER_TOP_MENU_YPOS });
    ImGui::SetWindowSize({ 200.0f, 714.0f });

    // find the assembly line chunk the program counter is in
    // (binary search)
    size_t a = 0;
    size_t z = sim.lines.size();
    size_t pc_offset = sim.pc - sim.code_segment;
    const size_t LINE_COUNT = 25;

    for (size_t lines = z;
         lines > LINE_COUNT;
         lines /= 2) 
    {
        size_t mid = (a + z) / 2;
        if (sim.lines[mid].offset > pc_offset)
        {
            z = mid;
        }
        else
        {
            a = mid; 
        }
    }

    size_t rem = sim.lines.size() - a;
    size_t nlines = (rem < LINE_COUNT) ? rem : LINE_COUNT;
    for (size_t i = 0; i < nlines; i++) 
    {
        // draw the program counter instruction as white
        // draw the other instructions as grey
        const AssemblyLine &line = sim.lines[a + i];
        ImVec4 col;
        if (sim.exec_mode == ExecMode_Continuous || 
            sim.pc != (sim.code_segment + line.offset))
        {
            col = COLOR_GREY;
        } 
        else 
        {
            col = COLOR_WHITE;
        }

        ImGui::TextColored(col, "%04X %s", sim.code_segment + line.offset, line.text);
    }
}

static void 
draw_program_context(Sim6502 &sim)
{
    ScopedWindow win("Program Context", NULL, WINFLAGS_LOCKED); // Create a window called "Hello, world!" and append into it.
    ImGui::SetWindowPos({ 325.0f, 408.0f });

    ImGui::TextColored(COLOR_WHITE, "ACC: %02X", sim.acc);
    ImGui::TextColored(COLOR_WHITE, "X: %02X", sim.x);
    ImGui::TextColored(COLOR_WHITE, "Y: %02X", sim.y);
    ImGui::TextColored(COLOR_WHITE, "SP: %02X", sim.sp);
    ImGui::TextColored(COLOR_WHITE, "PC: %04X", sim.pc);
    static const auto draw_flag = [](const char *flag, uint8 state)
    {
        // idea from javidx9 emulator video
        const ImVec4 COLOR_GREYER = IM_COLOR(0x60, 0x60, 0x60); 
        ImGui::SameLine();
        ImVec4 color = (state) ? COLOR_WHITE : COLOR_GREYER;
        ImGui::TextColored(color, flag);
    };
    ImGui::Text("");
    draw_flag("N", sim.pflags.neg);
    draw_flag("V", sim.pflags.ovrf);
    draw_flag("B", sim.pflags.brk);
    draw_flag("P", sim.pflags.pad);
    draw_flag("D", sim.pflags.deci);
    draw_flag("I", sim.pflags.intdis);
    draw_flag("Z", sim.pflags.zero);
    draw_flag("C", sim.pflags.carry);

    if (ImGui::Button("Step Forward")) {
        sim.exec_mode = ExecMode_StepForward;
    }
    if (ImGui::Button("Step Backward")) {
        sim.exec_mode = ExecMode_StepBackward;
    }
    if (ImGui::Button("Run")) {
        sim.exec_mode = ExecMode_Continuous; 
    }
    if (ImGui::Button("Pause")) {
        sim.exec_mode = ExecMode_Pause; 
    }
}

GLFWwindow *g_window;

void 
init_imgui()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        BRK;

    g_window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
    if (g_window == NULL)
        BRK;

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    io.Fonts->AddFontFromFileTTF(R"str(C:\Users\Kyle\Documents\Visual Studio 2017\Projects\SiggyFyeOtuu\misc\fonts\consola.ttf)str", 
                                 22.0f);
}

void 
shutdown_imgui()
{
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// return if close has been signaled
bool 
step_frame_imgui(Sim6502 &sim)
{
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static GuiMemRegion zp = { 0x0000, "A" };

    draw_mem_region(sim, (uint8 *)&sim.mem[0], zp);
    draw_assembly(sim);
    draw_program_context(sim);
    draw_instruction_expander(sim);
    draw_menu_bar(sim);

    //ImGui::ShowDemoWindow();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    ImVec4 clear_color = IM_COLOR(0xD6, 0xDA, 0xF0);
    glClearColor(clear_color.x * clear_color.w, 
                 clear_color.y * clear_color.w, 
                 clear_color.z * clear_color.w, 
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
    // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
    //GLint last_program;
    //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    //glUseProgram(0);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    //glUseProgram(last_program);

    glfwMakeContextCurrent(g_window);
    glfwSwapBuffers(g_window);

    ImGui::EndFrame();

    return !glfwWindowShouldClose(g_window);
}

