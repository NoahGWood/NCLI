#pragma once

#include <unordered_map>
#include <functional>
#include <string>

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