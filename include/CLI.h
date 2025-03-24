#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cstdlib>

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
                            if(i+1 < argc && argv[i+1][0] != '-'){
                                set_option(option_name, argv[++i]);
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