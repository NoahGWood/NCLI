#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>

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
    inline std::string prompt(const std::string &message)
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

    static void hide_cursor(){
        std::cout << "\033[?25l";  // Hide the cursor
    }

    static void show_cursor(){
        std::cout << "\033[?25h";  // Show the cursor
    }

    static std::string list(const std::string &message, const std::vector<std::string> &options)
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

    static std::set<std::string> checkbox(const std::string &message, const std::vector<std::string> &options)
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
