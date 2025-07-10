# cat gtp made this scirpt

import json
import os
import argparse

def convert_arguments(msvc_args):
    clang_args = ['clang++']  # or 'clang' for C files
    skip = False

    for i, arg in enumerate(msvc_args):
        if skip:
            skip = False
            continue

        if arg.lower().endswith('cl.exe'):
            continue

        if arg.startswith('/I'):
            if arg == '/I':
                clang_args.append('-I' + msvc_args[i + 1])
                skip = True
            else:
                clang_args.append('-I' + arg[2:])

        elif arg.startswith('/D'):
            if arg == '/D':
                clang_args.append('-D' + msvc_args[i + 1])
                skip = True
            else:
                clang_args.append('-D' + arg[2:])

        elif arg == '/c':
            continue  # clang assumes -c by default

        elif arg == '/nologo' or arg == '/Zi' or arg == '/FS':
            continue  # ignore MSVC-only flags

        elif arg.startswith('/Fd') or arg.startswith('/Fo') or arg.startswith('/Fe'):
            continue  # ignore output file flags

        elif arg == '/EHsc':
            clang_args.append('-fexceptions')
            clang_args.append('-fcxx-exceptions')

        elif arg.startswith('/std:'):
            std = arg.split(':')[1]
            if std == 'c++latest':
                clang_args.append('-std=c++23')
            elif std == 'c11':
                clang_args.append('-std=c11')
            elif std == 'c99':
                clang_args.append('-std=c99')
            else:
                # basic fallback
                clang_args.append('-std=' + std.replace('c++', 'c++').replace('c', 'c'))

        elif arg.startswith('/'):
            continue  # unrecognized MSVC flag

        else:
            clang_args.append(arg)

    # Add MSVC compatibility if needed
    clang_args.append('-fms-extensions')
    clang_args.append('-fms-compatibility')

    return clang_args

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Original compile_commands.json')
    parser.add_argument('output', help='Clang-compatible compile_commands.json')
    args = parser.parse_args()

    with open(args.input, 'r') as f:
        data = json.load(f)

    new_data = []

    for entry in data:
        original_args = entry['arguments']
        new_args = convert_arguments(original_args)

        # Detect source file type
        file_ext = os.path.splitext(entry['file'])[1]
        if file_ext == '.c':
            new_args[0] = 'clang'  # use clang for C, not clang++

        new_entry = {
            'directory': entry['directory'],
            'file': entry['file'],
            'arguments': new_args
        }

        new_data.append(new_entry)

    with open(args.output, 'w') as f:
        json.dump(new_data, f, indent=2)

    print(f"Converted compile_commands.json written to: {args.output}")

if __name__ == '__main__':
    main()
