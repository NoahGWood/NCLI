#pragma once

#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

namespace NCLI::Color
{
    bool terminal_supports_colors() {
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
    bool terminal_supports_256_colors(){
        const char* term_type = std::getenv("TERM");
        // Check for 256 color support
        if (term_type && (std::getenv("TERM") != nullptr)) {
            return std::string(term_type) != "dumb" && isatty(fileno(stdout));
        }

        return false;
    }

    // Detect if the terminal supports true colors (24-bit)
    bool terminal_supports_true_color() {
        const char* term_type = std::getenv("TERM");

        // Check for true color support
        return std::getenv("COLORTERM") != nullptr && std::string(std::getenv("COLORTERM")) == "truecolor";
    }

    // Determine if terminal supports styling
    bool terminal_supports_styles(){
        return terminal_supports_colors(); 
    }

    bool terminal_supports_italic() {
        const char* term_type = std::getenv("TERM");
        // Check if the terminal supports italic (not all terminals do)
        // This is a simple check, some terminals use different values for italic support
        return term_type && (std::string(term_type).find("xterm") != std::string::npos || 
                             std::string(term_type).find("screen") != std::string::npos);
    }

    int rgb_to_256_color(int r, int g, int b){
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

    int rgb_to_ansi_color(int r, int g, int b){
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

    inline void set_terminal_background_color(int r, int g, int b)
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
    inline std::string apply_color(int r, int g, int b, const std::string& text){
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
    inline std::string apply_style(const std::string& style_code, const std::string& text) {
        if (terminal_supports_styles()) {
            return style_code + text + "\033[0m";  // Apply style if terminal supports it
        }
        return text;  // Return plain text if no styles are supported
    }
    // 256 color mode
    inline std::string color256(int code, const std::string& s){
        return "\033[38;5;" + std::to_string(code) + "m" + s + "\033[0m"; // Foreground
    }
    
    inline std::string bg_color256(int code, const std::string& s){
        return "\033[48;5;" + std::to_string(code) + "m" + s + "\033[0m"; // Foreground
    }

    // True-color (24-bit) mode
    inline std::string true_color(int r, int g, int b, const std::string& s) {
        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + s + "\033[0m";  // foreground
    }
    inline std::string bg_true_color(int r, int g, int b, const std::string& s) {
        return "\033[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + s + "\033[0m";  // background
    }


    // Basic color modes
    inline std::string red(const std::string& s)    { return apply_color(255, 0, 0, s); }
    inline std::string green(const std::string& s)  { return apply_color(0, 255, 0, s); }
    inline std::string yellow(const std::string& s) { return apply_color(255, 255, 0, s); }
    inline std::string cyan(const std::string& s)   { return apply_color(0, 255, 255, s); }
    // Styling
    inline std::string bold(const std::string& s)   { return apply_style("\033[1m", s); }
    inline std::string underline(const std::string& s) { return apply_style("\033[4m", s); }
    inline std::string italic(const std::string& s) { return terminal_supports_italic() ? apply_style("\033[3m", s) : s; }
    inline std::string blink(const std::string& s)  { return apply_style("\033[5m", s); }
    inline std::string reverse(const std::string& s) { return apply_style("\033[7m", s); }
    inline std::string hidden(const std::string& s) { return apply_style("\033[8m", s); }
} // namespace NCLI::Color
