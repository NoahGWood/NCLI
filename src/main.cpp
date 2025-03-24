/**
 * @file main.cpp
 * @author @NoahGWood
 * @brief This is a simple test CLI to demonstrate how to use NCLI
 * @version 0.1
 * @date 2025-03-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "NCLI.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <set>

using namespace NCLI;

bool show_selected_options(const std::vector<Flag> flags, const std::vector<Option> options)
{
    std::set<std::string> selected = NCLI::checkbox("Select your favorite colors", {"Red", "Green", "Blue", "Yellow"});

    std::cout << "You selected.\n";
    for(const auto& color: selected){
        std::cout << color << "\n";
    }
    return true;    
}

std::vector<std::string> LIST_CHOICES = { "Choice 1", "Choice 2", "Choice 3" };

int main(int argc, char* argv[]) {
    CLI cli;


    // Register commands
    cli.command("colors", "Select Colors")
        .action(show_selected_options);

    cli.command("greet", "Greet a user")
        .option("name", "Specify the user's name")
        .flag("blink", "Blink user name")
        .flag("verbose", "Enable verbose output")
        .flag("formal", "User formal greeting")
        .action([&cli](auto flags, auto options) {
            bool formal = false;
            bool blink = false;
            for(const auto& flag : flags){
                if(flag.Name == "formal"){
                    formal = true;
                }
                else if(flag.Name == "blink"){
                    blink=true;
                }
            }
            std::string name;
            for(const auto& option : options){
                if(option.Name == "name"){
                    name = option.Value;
                }
            }
            if(name.empty()){
                name = prompt("What is your name?");
            }
            std::stringstream buf;
            if(formal){
                buf << Color::green("Good evening, ");
            } else {
                buf << Color::green("Hello, ");
            }
            if(blink){
                buf << Color::blink(Color::bold(name));
            } else {
                buf << Color::bold(name);
            }
            buf <<"!\n";
            std::cout << buf.str();
            return true;
        });

    // cli.command("add", "Add numbers together")

    // Parse the arguments
    cli.parse(argc, argv);

    // Run command if provided
    cli.run();
    // if(cli.has_command()){
    //     cli.run();
    // } else {
    //     cli.run("colors");
    // }

    // std::string selected = list("Select one", LIST_CHOICES);
    // std::cout << "You selected: " << selected << "\n";
    return 0;
}