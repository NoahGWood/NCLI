/**
 * @file NCLI.h
 * @author NoahGWood
 * @brief Single-header include for the NCLI library
 * @version 0.1
 * @date 2025-4-7
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef NCLI_H
#define NCLI_H

#include <cstdlib>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
// --- Colors.h --- //
namespace NCLI::Color
{
    inline static bool terminal_supports_colors() {
        const char* term_type = std::getenv("TERM");
        // Check if we're on a Unix-like system with a terminal and color support
        if (term_type && (std::getenv("TERM") != nullptr)) {
            // Check if the terminal supports color (e.g., "xterm", "xterm-256color", etc.)
            return std::string(term_type) != "dumb" && isatty(fileno(stdout));
        }
#ifdef _WIN32
        // For Windows, check if virtual terminal processing is enabled
        DWORD dwMode = 0;
        if (GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode)) {
            return (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        return false;
#else
        return true;  // Assume color support for other systems
#endif
    }
    // Determine if we support 256 colors
    inline static bool terminal_supports_256_colors(){
        const char* term_type = std::getenv("TERM");
        // Check for 256 color support
        if (term_type && (std::getenv("TERM") != nullptr)) {
            return std::string(term_type) != "dumb" && isatty(fileno(stdout));
        }
        return false;
    }
    // Detect if the terminal supports true colors (24-bit)
    inline static bool terminal_supports_true_color() {
        const char* term_type = std::getenv("TERM");
        // Check for true color support
        return std::getenv("COLORTERM") != nullptr && std::string(std::getenv("COLORTERM")) == "truecolor";
    }
    // Determine if terminal supports styling
    inline static bool terminal_supports_styles(){
        return terminal_supports_colors(); 
    }
    inline static bool terminal_supports_italic() {
        const char* term_type = std::getenv("TERM");
        // Check if the terminal supports italic (not all terminals do)
        // This is a simple check, some terminals use different values for italic support
        return term_type && (std::string(term_type).find("xterm") != std::string::npos || 
                             std::string(term_type).find("screen") != std::string::npos);
    }
    inline static int rgb_to_256_color(int r, int g, int b){
        // First handle basic grayscale
        if(r == g && g == b){
            if(r < 8) return 0; // Black
            if(r > 248) return 15; // White
            return 232 + (r / 10); // Grayscale (range 232-253)
        }
        // Colors in the 216 color cube
        int red = (r > 248) ? 5 : r / 51;
        int green = (g > 248) ? 5 : g / 51;
        int blue = (b > 248) ? 5 : b / 51;
        // Return the 216 color value
        return 16 + (red * 36) + (green * 6) + blue;
    }
    inline static int rgb_to_ansi_color(int r, int g, int b){
        int color_code = 0;
        if (r > 200 && g < 100 && b < 100) color_code = 1; // Red
        else if (r < 100 && g > 200 && b < 100) color_code = 2; // Green
        else if (r > 200 && g > 200 && b < 100) color_code = 3; // Yellow
        else if (r < 100 && g < 100 && b > 200) color_code = 4; // Blue
        else if (r > 200 && g < 100 && b > 200) color_code = 5; // Magenta
        else if (r < 100 && g > 200 && b > 200) color_code = 6; // Cyan
        else if (r > 200 && g > 200 && b > 200) color_code = 7; // White
        else color_code = 0; // Black (default)
        return color_code;
    }
    inline static void set_terminal_background_color(int r, int g, int b)
    {
        if(terminal_supports_true_color()){
            std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";  // True-color background
        } else if(terminal_supports_256_colors()){
            // Convert RGB to 256 color index            
            int color_code = rgb_to_256_color(r,g,b);
            std::cout << "\033[48;5;" << color_code << "m";  // 256-color background    
        } else if(terminal_supports_colors()){
            std::cout << "\033[48;5;0m";  // Default to black background if no advanced support
        }
    }
    // Color application based on the terminal's capabilities
    inline static std::string apply_color(int r, int g, int b, const std::string& text){
        std::stringstream buffer;
        if (terminal_supports_true_color()) {
            buffer << "\033[38;2;" << r << ';' << g << ';' << b << "m" << text << "\033[0m";  // Applying true color (example orange)
        }
        else if (terminal_supports_256_colors()) {
            int color_code = rgb_to_256_color(r,g,b);
            buffer << "\033[38;5;" << color_code << "m" << text << "\033[0m";    
        }
        else if (terminal_supports_colors()) {
            int color_code = rgb_to_ansi_color(r,g,b);
            buffer << "\033[38;5;" << color_code << "m" << text << "\033[0m";
        } else {
            return text;  // No color, return plain text
        }
        return buffer.str();
    }
    // Apply styles dynamically
    inline static std::string apply_style(const std::string& style_code, const std::string& text) {
        if (terminal_supports_styles()) {
            return style_code + text + "\033[0m";  // Apply style if terminal supports it
        }
        return text;  // Return plain text if no styles are supported
    }
    // 256 color mode
    inline static std::string color256(int code, const std::string& s){
        return "\033[38;5;" + std::to_string(code) + "m" + s + "\033[0m"; // Foreground
    }
    
    inline static std::string bg_color256(int code, const std::string& s){
        return "\033[48;5;" + std::to_string(code) + "m" + s + "\033[0m"; // Foreground
    }
    // True-color (24-bit) mode
    inline static std::string true_color(int r, int g, int b, const std::string& s) {
        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + s + "\033[0m";  // foreground
    }
    inline static std::string bg_true_color(int r, int g, int b, const std::string& s) {
        return "\033[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + s + "\033[0m";  // background
    }
    // Basic color modes
    inline static std::string red(const std::string& s)    { return apply_color(255, 0, 0, s); }
    inline static std::string green(const std::string& s)  { return apply_color(0, 255, 0, s); }
    inline static std::string yellow(const std::string& s) { return apply_color(255, 255, 0, s); }
    inline static std::string cyan(const std::string& s)   { return apply_color(0, 255, 255, s); }
    // Styling
    inline static std::string bold(const std::string& s)   { return apply_style("\033[1m", s); }
    inline static std::string underline(const std::string& s) { return apply_style("\033[4m", s); }
    inline static std::string italic(const std::string& s) { return terminal_supports_italic() ? apply_style("\033[3m", s) : s; }
    inline static std::string blink(const std::string& s)  { return apply_style("\033[5m", s); }
    inline static std::string reverse(const std::string& s) { return apply_style("\033[7m", s); }
    inline static std::string hidden(const std::string& s) { return apply_style("\033[8m", s); }
} // namespace NCLI::Color
// --- Colors.h --- //

// --- Config.h --- //
namespace NCLI
{
    namespace CFG
    {
        inline static std::string promptChar = "> ";
    } // namespace CFG
} // namespace NCLI
// --- Config.h --- //

// --- Prompt.h --- //
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
namespace NCLI
{
    inline static void cls()
    {
        std::cout << "\033[H\033[J"; // Clear the screen
    }
    inline static std::string prompt(const std::string &message)
    {
        std::cout << NCLI::Color::cyan(NCLI::CFG::promptChar) << NCLI::Color::bold(message) << ": ";
        std::string input;
        std::getline(std::cin, input);
        return input;
    }
    // Platform independent function to capture a single key press
    inline static char getch()
    {
        char ch;
#ifdef _WIN32
        ch = _getch()
#else
        struct termios old = {0};
        if (tcgetattr(STDIN_FILENO, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON; // Disable canonical mode
        old.c_lflag &= ~ECHO;   // Disable echo
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        ch = getchar();
        old.c_lflag |= ICANON; // Enable canonical mode
        old.c_lflag |= ECHO;   // Enable echo
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
            return ch;
    }
    inline static void hide_cursor(){
        std::cout << "\033[?25l";  // Hide the cursor
    }
    inline static void show_cursor(){
        std::cout << "\033[?25h";  // Show the cursor
    }
    inline static std::string list(const std::string &message, const std::vector<std::string> &options)
    {
        int selected = 0;
        char key = 0;
        hide_cursor();
        cls(); // Clear screen first to prevent ugly jumping
        while (true)
        {
            std::cout << message << "\n";
            // Display the options with the currently selected highlighted
            for (int i = 0; i < options.size(); ++i)
            {
                if (i == selected)
                {
                    std::cout << "\033[1;32m> " << options[i] << " \033[0m\n"; // Green for the selected option
                }
                else
                {
                    std::cout << "  " << options[i] << "\n";
                }
            }
            key = getch();
            if (key == 27)
            {            // Escape sequence (arrow key)
                getch(); // Skip '['
                key = getch();
                if (key == 'A')
                {
                    // Up arrow
                    selected = (selected == 0) ? options.size() - 1 : selected - 1;
                }
                else if (key == 'B')
                {
                    selected = (selected + 1) % options.size();
                }
            }
            else if (key == '\n')
            {
                show_cursor();
                return options[selected];
            }
            cls();
        }
    }
    inline static std::set<std::string> checkbox(const std::string &message, const std::vector<std::string> &options)
    {
        hide_cursor();
        std::set<std::string> selected;
        int selected_index = 0;
        char key = 0;
        cls(); // Clear screen first to prevent ugly jumping
        while (true)
        {
            std::cout << message << "\n";
            // Display the options with checkboxes
            for (int i = 0; i < options.size(); ++i)
            {
                if(i == selected_index){
                    // Highlight selected region
                    std::cout << "\033[1;32m> \033[0m";  // Green for the selected option
                } else {
                    std::cout << " "; // no highlighting for other options;
                }
                std::cout << (selected.count(options[i]) ? Color::green("[x] " + options[i]) : "[ ] " + options[i]) << "\n";
            }
            key = getch();
            if (key == 27)
            {            // Escape sequence (arrow key)
                getch(); // Skip '['
                key = getch();
                if (key == 'A')
                { // Up arrow
                    selected_index = (selected_index == 0) ? options.size() - 1 : selected_index - 1;
                }
                else if (key == 'B')
                { // Down arrow
                    selected_index = (selected_index + 1) % options.size();
                }
            }
            else if (key == ' ')
            { // Space to toggle checkbox
                if (selected.count(options[selected_index]))
                {
                    selected.erase(options[selected_index]); // Deselect
                }
                else
                {
                    selected.insert(options[selected_index]); // Select
                }
            }
            else if (key == '\n')
            { // Enter to finalize selection
                break;
            }
            std::cout << "\033[H\033[J"; // Clear the screen (optional)
        }
        show_cursor();
        return selected;
    }
} // namespace NCLI
// --- Prompt.h --- //

// --- Command.h --- //
namespace NCLI
{
    struct Flag{
        std::string Name;
        std::string Help;
        bool FlagSet=false;
        Flag() = default;
        Flag(const std::string& name)
            : Name(name){}
        Flag(const std::string& name, const std::string& help)
            : Name(name), Help(help) {}
        Flag(const std::string& name, const std::string& help, bool set)
            : Name(name), Help(help), FlagSet(set){}
    };
    struct Option{
        std::string Name;
        std::string Help;
        std::string Value;
        std::vector<std::string> Values;
        bool Set=false;
        Option() = default;
        Option(const std::string& name)
            : Name(name){}
        Option(const std::string& name, const std::string& help)
            : Name(name), Help(help){}
        Option(const std::string& name, const std::string& help, bool set)
            : Name(name), Help(help), Set(set){}
        Option(const std::string& name, const std::string& help, const std::string& default_value)
            : Name(name), Help(help), Value(default_value){}
        Option(const std::string& name, const std::string& help, const std::string& default_value, bool set)
            : Name(name), Help(help), Value(default_value), Set(set) {}
    };
    using CommandFunc = std::function<bool(const std::vector<Flag> flags, const std::vector<Option> options)>;
    struct Command {
        std::string Name;
        std::string Help;
        CommandFunc Function;
        std::vector<Flag> Flags;
        std::vector<Option> Options;
        Command() = default;
        Command(const std::string& name)
            : Name(name){}
        Command(const std::string& name, const std::string& help)
            : Name(name), Help(help) {}
        Command(const std::string& name, const std::string& help, CommandFunc func)
            : Name(name), Help(help), Function(func) {}
        Command(const std::string& name, const std::string& help, CommandFunc func, std::vector<Flag> flags)
            : Name(name), Help(help), Function(func), Flags(flags) {}
        Command(const std::string& name, const std::string& help, CommandFunc func, std::vector<Flag> flags, std::vector<Option> options)
            : Name(name), Help(help), Function(func), Flags(flags), Options(options) {}
        
        Command& add_flag(const std::string& flag_name, const std::string& help=""){
            Flags.emplace_back(flag_name, help, false);
            return *this;
        }
        Command& add_option(const std::string& option_name, const std::string& help=""){
            Options.emplace_back(option_name, help, false);
            return *this;
        }
        Command& option(const std::string& name, const std::string& help=""){
            return add_option(name, help);
        }
        Command& flag(const std::string& name, const std::string& help=""){
            return add_flag(name, help);
        }
        Command& action(CommandFunc action){
            Function = action;
            return *this;
        }
        bool set_flag(const std::string& name){
            for(auto& flag : Flags){
                if(flag.Name == name){
                    flag.FlagSet = true;
                    return true;
                }
            }
            return false;
        }
        bool set_option(const std::string& name, const std::string& value){
            for(auto& option : Options){
                if(option.Name == name){
                    option.Value = value;
                    option.Set = true;
                    return true;
                }
            }
            return false;
        }
        bool set_option_vector(const std::string& name, const std::vector<std::string> values)
        {
            for(auto& option : Options) {
                if(option.Name == name) {
                    if(!option.Value.empty()){
                        if (std::find(option.Values.begin(), option.Values.end(), option.Value) != option.Values.end())
                        {
                            option.Values.insert(option.Values.begin(), option.Value);
                        }
                    } else if(!values.empty()) {
                        option.Value = values.front();
                    }
                    for(const auto& value : values)
                    {
                        option.Values.push_back(value);
                    }
                    option.Set = true;
                    return true;
                }
            }
            return false;
        }
        bool execute(){
            // Collect flags that are set
            std::vector<Flag> flags;
            if(!Flags.empty()){
                for(auto& flag : Flags){
                    if(flag.FlagSet){
                        std::cout << "Flag: " << flag.Name << "\n"; 
                        flags.push_back(flag);
                    }
                }
            }
            // Collect options that are set
            std::vector<Option> options = { Option{} };
            if(!Options.empty()){
                for(auto& opt : Options){
                    if(opt.Set){
                        std::cout << "Option: " << opt.Name << "\n";
                        options.push_back(opt);
                    }
                }
            }
            return Function(flags, options);
        }
        
        void display_help(){
            std::cout << Color::cyan(Color::bold("Command: ")) << Color::green(Name) << "\n";
            std::cout << Color::cyan(Color::bold("  Usage: ")) << Color::italic(Help) << "\n";
            std::cout << Color::cyan(Color::bold("  Options:\n"));
            for(const auto& option : Options){
                if(option.Set){
                    std::cout << Color::green("    [x] --" + option.Name) << ": " << Color::italic(option.Help) << "\n";
                    std::cout << Color::green("            Current Value: ") << Color::bold(option.Value) << "\n";
                } else {
                    std::cout << Color::yellow("    [ ] --" + option.Name) << ": " << Color::italic(option.Help) << "\n";
                }
            }
            std::cout << Color::cyan(Color::bold("  Flags:\n"));
            for(const auto& flag : Flags){
                if(flag.FlagSet){
                    std::cout << Color::green("    [x] -" + flag.Name) << ": " << Color::italic(flag.Help) << "\n";
                } else {
                    std::cout << Color::yellow("    [ ] -" + flag.Name) << ": " << Color::italic(flag.Help) << "\n";
                }
            }
        }
    };
} // namespace NCLI
// --- Command.h --- //

// --- CLI.h --- //
namespace NCLI
{
    class CLI {
        public:
            Command& command(const std::string& name){
                m_Commands.push_back(Command(name));
                return m_Commands.back();
            }
            Command& command(const std::string& name, const std::string& help){
                m_Commands.push_back(Command(name, help));
                return m_Commands.back();
            }
            Command& command(const std::string& name, const std::string& help, CommandFunc& func){
                m_Commands.push_back(Command(name, help, func));
                return m_Commands.back();
            }
            Command* get_command(const std::string& name){
                auto it = std::find_if(m_Commands.begin(), m_Commands.end(),
                    [&name](const Command& cmd) {
                        return cmd.Name == name;
                    });
                return it != m_Commands.end() ? &(*it) : nullptr;
            }
            bool has_command(){
                return !m_CurrentCommand.empty();
            }
            Option& add_option(const std::string& name) {
                m_Options.push_back({name, "", "", false});
                return m_Options.back();
            }
            Option& add_option(const std::string& name, const std::string& help) {
                m_Options.push_back({name, help, "", false});
                return m_Options.back();
            }
            Option* get_option(const std::string& name){
                auto it = std::find_if(m_Options.begin(), m_Options.end(),
                [&name](const Option& opt){
                    return opt.Name == name;
                });
                return it != m_Options.end() ? &(*it) : nullptr;
            }
        
            Flag& add_flag(const std::string& name) {
                m_Flags.push_back({name, "", false});
                return m_Flags.back();
            }
            Flag& add_flag(const std::string& name, const std::string& help) {
                m_Flags.push_back({name, help, false});
                return m_Flags.back();
            }
            Flag* get_flag(const std::string& name){
                auto it = std::find_if(m_Flags.begin(), m_Flags.end(),
                [&name](const Flag& flag){
                    return flag.Name == name;
                });
                return it != m_Flags.end() ? &(*it) : nullptr;
            }
            void display_help(){
                std::cout << Color::cyan(Color::bold("Available commands:\n"));
                for(const auto& cmd: m_Commands){
                    std::cout << "  " << Color::cyan(Color::bold(cmd.Name)) << ": " << Color::italic(cmd.Help) << "\n";
                }
                exit(0);
            }
            void parse(int argc, char* argv[]) {
                if(argc == 1 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"){
                    display_help();
                }
                for(int i=1; i<argc; ++i){
                    std::string arg = argv[i];
                    // Handle flags first
                    if(arg[0] == '-'){
                        if(arg[1] == '-'){ // Long option (e.g. --name)
                            std::string option_name = arg.substr(2);
                            std::vector<std::string> values;
                            while(i+1 < argc && argv[i+1][0] != '-')
                            {
                                values.push_back(argv[++i]);
                            }
                            if(!values.empty()){
                                set_option_vector(option_name, values);
                            } else {
                                std::cerr << "Error: Missing value for option " << option_name << std::endl;
                                exit(1);
                            }
                        } else {
                            // Short flag (e.g. -v)
                            set_flag(arg.substr(1));
                        }
                    } else {
                        // Command (first non-option argument)
                        m_CurrentCommand = arg;
                    }
                }
            }
            void run(const std::string& name=""){
                bool ran=false;
                Command* cmd = nullptr;
                if(!name.empty()){
                    cmd = get_command(name);
                } else if(!m_CurrentCommand.empty()) {
                    cmd = get_command(m_CurrentCommand);
                }
                if(cmd != nullptr){
                    std::cout << "Executing: " << cmd->Name << "\n";
                    ran = cmd->execute();
                    std::cout << "Done Executing\n";
                    if(!ran){
                        std::cout << "Displaying Help\n";
                        cmd->display_help();
                    }
                } else {
                    if(name.empty()){
                        std::cout << Color::red("Command not found: ") << Color::bold(m_CurrentCommand) << Color::red("!\n");
                    } else {
                        std::cout << Color::red("Command not found: ") << Color::bold(name) << Color::red("!\n");
                    }
                    display_help();
                }
            }
            std::string get(const std::string& name) const {
                for(const auto& option : m_Options){
                    if(option.Name == name){
                        return option.Value;
                    }
                }
                return "";
            }
            bool is_set(const std::string& name) const {
                for(const auto& flag : m_Flags){
                    if(flag.Name == name){
                        return flag.FlagSet;
                    }
                }
            }
        private:
            std::vector<Command> m_Commands;
            std::vector<Option> m_Options; // Global Options
            std::vector<Flag> m_Flags; // Global Flags
            std::string m_CurrentCommand;
        private:
            void set_option(const std::string& name, const std::string& value){
                // First check if global options are set
                for(auto& option : m_Options){
                    if(option.Name == name){
                        option.Value = value;
                        option.Set = true;
                        return;
                    }
                }
                // Then check if options are set on current command
                if(!m_CurrentCommand.empty()){
                    Command* current = get_command(m_CurrentCommand);
                    if(current != nullptr){
                        if(current->set_option(name, value)){
                            return;
                        } else {
                            std::cout << Color::red(Color::bold("Unknown option: ")) << name << "\n";
                            current->display_help();
                            exit(0);
                        }
                    }
                }
                // Nothing left to check, throw an error and show help
                throw std::invalid_argument("Unknown option: " + name);
            }
            void set_option_vector(const std::string& name, const std::vector<std::string>& values)
            {
                // First check if global options are set
                for(auto& option : m_Options){
                    if(option.Name == name){
                        // Check if option value is already set, if so make it first
                        if(!option.Value.empty()){
                            if (std::find(option.Values.begin(), option.Values.end(), option.Value) != option.Values.end())
                            {
                                option.Values.insert(option.Values.begin(), option.Value);
                            }
                        }
                        option.Set = true;
                        for(auto value : values)
                        {
                            option.Values.push_back(value);
                        }
                        return;
                    }
                }
                // Then check if options are set on current command
                if(!m_CurrentCommand.empty()){
                    Command* current = get_command(m_CurrentCommand);
                    if(current != nullptr){
                        if(current->set_option_vector(name, values)){
                            return;
                        } else {
                            std::cout << Color::red(Color::bold("Unknown option: ")) << name << "\n";
                            current->display_help();
                            exit(0);
                        }
                    }
                }
                // Nothing left to check, throw an error and show help
                throw std::invalid_argument("Unknown option: " + name);
            }
            void set_flag(const std::string& name){
                // First check if global flags are set
                for(auto& flag : m_Flags){
                    if(flag.Name == name){
                        flag.FlagSet = true;
                        return;
                    }
                }
                // Then check if flags are set on current command
                if(!m_CurrentCommand.empty()){
                    Command* current = get_command(m_CurrentCommand);
                    if(current != nullptr){
                        if(current->set_flag(name)){
                            return;
                        } else {
                            std::cout << Color::red(Color::bold("Unknown flag: ")) << name << "\n";
                            current->display_help();
                            exit(0);
                        }
                    }
                }
                // Nothing left to check, throw an error and show help
                throw std::invalid_argument("Unknown flag: " + name);
            }
    };
} // namespace NCLI
// --- CLI.h --- //
#endif