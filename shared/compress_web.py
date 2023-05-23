#!/usr/bin/python

Import ("env")

import gzip
import os
import os.path as path

# Unquoted spaces will be removed from these, that is why not_found.html would be listed as a binary.
input_text_files = [ 'src/html/main.css', 'src/html/index.js' ]

input_binary_files = [ 'images/favicon.ico', 'images/favicon.svg', 'images/favicon.png' ]

# The javascript keywords that require a space after them.
js_keywords = [ 'await', 'case', 'class', 'const', 'delete', 'export', 'extends', 'function', 'import', 'in', 'instanceof', 'let', 'new', 'return', 'static', 'throw', 'typeof', 'var', 'void', 'yield' ]


def remove_whitespaces(lines_in):
    """Removes whitespaces and comments from text.
    
    Can only handle /* ... */ comments.
    
    Parameters
    ----------
    lines_in: list
        A list of strings containing the lines to edit.
    
    Returns
    -------
    list
        A list of strings containing the modified text.
    """

    lines_out = []
    current_quote = None
    for line in lines_in:
        last_char = None
        line_out = ""
        since_space = ""
        for char in line:
            if current_quote == None and (char == ' ' or char == '\t'):
                if since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                continue
            elif current_quote == None and (char == "'" or char == '"'):
                current_quote = char
            elif char == current_quote:
                current_quote = None
            elif char == '/' and current_quote == "/*" and last_char == '*':
                current_quote = None
                last_char = None
                continue
            elif current_quote == None and char == '*' and last_char == '/':
                if since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                current_quote = "/*"

            if last_char != None and current_quote != "/*":
                line_out += last_char

            since_space += char
            last_char = char

        if last_char != None:
            line_out += last_char

        lines_out.append(line_out)

    return lines_out


def gzip_file(input, output):
    """GZIP compresses the file from the given input path to the given output path.
    
    Parameters
    ----------
    input: str
        The path of the input file to compress.
    output: str
        The target path to write the compressed file to.
    """

    with open(input, 'rb') as src, gzip.GzipFile(filename=path.basename(output), mode='wb', compresslevel=9, fileobj=open(output, 'wb'), mtime=0) as dst:
        for chunk in iter(lambda: src.read(4096), b""):
            dst.write(chunk)


def compress_file(input, text):
    """Compresses the input file to data/gzip/filename.
    
    If text is True the file will first by copied to data.
    If run as a release build it will also me minifyed during this step.
    Then the file, the original for binaries and the copy for text, will be gzipped to data/gzip.
    
    Parameters
    ----------
    input: str
        The path of the input file to compress.
    text: bool
        Whether the input file is a text file.
    """

    filename = path.basename(file)
    minify = text

    if env.get('BUILD_TYPE') == "debug":
        print("Debug mode detected, not minifying text files.")
        minify = False

    data_dir = env.get('PROJECT_DATA_DIR')
    if not path.exists(data_dir):
        os.mkdir(data_dir)

    if not path.exists(input):
        print(f"File \"{input}\" doesn't exist.")
        env.Exit(1)
        return

    if text:
        with open(input) as src:
            output = path.join(data_dir, path.basename(file))
            if path.exists(output):
                os.remove(output)

            lines = src.readlines()
            if minify:
                lines = remove_whitespaces(lines)

            with open(output, "w") as dst:
                dst.writelines(lines)

            input = output

    gzip_dir = path.join(data_dir, "gzip")
    if not path.exists(gzip_dir):
        os.mkdir(gzip_dir)

    gzip_file(input, path.join(gzip_dir, filename + ".gz"))


for file in input_text_files:
    filename = path.basename(file) + ".gz"
    env.AlwaysBuild(f"$BUILD_DIR/{filename}.txt.o")
    compress_file(file, True)

for file in input_binary_files:
    filename = path.basename(file) + ".gz"
    env.AlwaysBuild(f"$BUILD_DIR/{filename}.txt.o")
    compress_file(file, False)
