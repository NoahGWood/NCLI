import os
import datetime

parts = ['Colors.h', 'Config.h', 'Prompt.h', 'Command.h', 'CLI.h']
strip_any = ['// Forward declare']
strip_starts = ["#pragma once"]
strip_exact = ["\n", "\n\n"]
out_file = "../single-header/NCLI.h"
file = "NCLI.h"
file_guard = "NCLI_H"
author = "NoahGWood"
brief = "Single-header include for the NCLI library"
version = "0.1"
date = datetime.datetime.now()

def strip_line(line: str):
    for each in strip_any:
        if each in line:
            return True
    for each in strip_starts:
        if line.startswith(each):
            return True
    for each in strip_exact:
        if line == each:
            return True
    return False

with open(out_file, "w") as out:
    out.write(f"""/**
 * @file {file}
 * @author {author}
 * @brief {brief}
 * @version {version}
 * @date {date.year}-{date.month}-{date.day}
 * 
 * @copyright Copyright (c) {date.year}
 * 
 */\n\n""")
    out.write(f"#ifndef {file_guard}\n")
    out.write(f"#define {file_guard}\n\n")
    includes = []
    content = []
    indef = False
    for part in parts:
        with open(f"../include/{part}", 'r') as f:
            content.append(f"// --- {part} --- //")
            for line in f.readlines():
                line = line.strip('\n')
                if line.startswith("#include") and not indef:
                    includes.append(line)
                elif strip_line(line):
                    pass
                elif line.startswith("#ifdef"):
                    indef = True
                    content.append(line)
                elif line.startswith("#endif"):
                    indef = False
                    content.append(line)
                else:
                    if line != '':
                        content.append(line)
            content.append(f"// --- {part} --- //\n")
    includes = list(set(includes))
    includes.sort()
    out.write('\n'.join(includes))
    out.write("\n")
    out.write('\n'.join(content))
    out.write("#endif")

    print(content[8:20])