#!/usr/bin/env python3

from enum import Enum
import os
from os import path
import sys

from gzip_compressing_stream import GzipCompressingStream

try:
    Import ("env") # type: ignore[name-defined]
except:
    print("Failed to load platformio environment!", file=sys.stderr)
    exit(1)

# Text files to potentially remove the spaces from, and compress.
input_text_files = [ 'src/html/index.html', 'src/html/error.html', 'src/html/main.css', 'src/html/index.js', 'src/html/manifest.json', 'images/favicon.svg' ]

# Binary files to compress without modifying.
input_binary_files = [ 'images/favicon.ico', 'images/favicon.png' ]

# These files will not be gzip compressed, just copied and potentially stripped of spaces.
input_gzip_blacklist = [ 'src/html/index.html', 'src/html/error.html' ]

# The javascript keywords that require a space after them.
js_keywords = [ 'await', 'case', 'class', 'const', 'delete', 'export', 'extends', 'function', 'import', 'in', 'instanceof', 'let', 'new', 'return', 'static', 'throw', 'typeof', 'var', 'void', 'yield' ]

# The characters before/after which a space doesn't make a difference in default mode.
separator_chars = [ '(', ')', '[', ']', '{', '}', '<', '>', ':', ';', ',', '+', '-', '*', '/' ]

# The window size parameter to use for gzip compression.
# The actual window size used by zlib is pow(2, -gzip_windowsize).
# The window size used for decompression(on the esp) has to be
# at least as much as the window size used for compression.
# This requires a pow(2, -gzip_windowsize) byte buffer on the esp.
gzip_windowsize = -10

MinifyMode = Enum('MinifyMode', [ 'Default', 'HTML', 'CSS', 'JavaScript' ])


def remove_whitespaces(lines_in, mode):
    """Removes whitespaces and comments from text.
    
    Can only handle /* ... */ comments.
    Can not handle html pre tags.
    Removes empty lines if they follow another empty line.
    
    Parameters
    ----------
    lines_in: list
        A list of strings containing the lines to edit.
    mode: MinifyMode
        The minifying mode to use.
        There are currently four modes.
        In all modes spaces are removed if they are the first or last character in a line.
        In Default, HTML, and CSS mode multiple spaces in a row are always collapsed to a single one.
        In JavaScript mode multiple spaces in a row are collapsed, unless they are inside of a quote.
        In all modes except HTML mode /* ... */ comments are removed entirely.
        In Default mode spaces are removed if they are before or after a character from the separator_chars list.
        In CSS mode spaces are removed if they are before or after a character from the separator_chars list,
        except if they are before an opening bracket.
        In all modes empty lines at the beginning of the file are removed.
        In all modes multiple empty lines in a row a collapsed to a single one.
        In CSS mode linebreaks after commas are removed.
    
    Returns
    -------
    list
        A list of strings containing the modified text.
    """

    if not isinstance(mode, MinifyMode):
        raise ValueError("mode has to be of type MinifyMode")

    lines_out = []
    current_quote = None
    for line in lines_in:
        last_char = None
        line_out = ""
        since_space = ""
        for char in line:
            if not current_quote and (char == ' ' or char == '\t'):
                if mode == MinifyMode.JavaScript and since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                if mode == MinifyMode.JavaScript or not last_char or last_char == ' ':
                    continue
                elif (mode == MinifyMode.Default or mode == MinifyMode.CSS) and last_char in separator_chars:
                    continue
            elif mode == MinifyMode.JavaScript and not current_quote and (char == "'" or char == '"'):
                current_quote = char
            elif char == current_quote:
                current_quote = None
            elif mode != MinifyMode.HTML and char == '/' and current_quote == "/*" and last_char == '*':
                current_quote = None
                if line_out and line_out[-1] != ' ':
                    # Comments separate values, so replace them with a single space.
                    last_char = ' '
                else:
                    last_char = None
                continue
            elif mode != MinifyMode.HTML and not current_quote and char == '*' and last_char == '/':
                if since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                current_quote = "/*"
            elif mode == MinifyMode.Default and char in separator_chars and last_char == ' ':
                last_char = None
            elif mode == MinifyMode.CSS and char != '(' and char in separator_chars and last_char == ' ':
                last_char = None

            if last_char and current_quote != "/*":
                line_out += last_char

            since_space += char
            if char != '\t':
                last_char = char
            else:
                last_char = ' '

        if last_char and last_char != ' ':
            line_out += last_char

        # Remove trailing whitespace
        if len(line_out) > len(os.linesep) and line_out[-len(os.linesep) - 1] == ' ':
            line_out = line_out[:-len(os.linesep) - 2] + os.linesep

        # Remove linebreaks after lines ending with a comma
        if mode == MinifyMode.CSS and lines_out and lines_out[-1].strip()[-1:] == ',':
            lines_out[-1] = lines_out[-1][:-len(os.linesep)] + line_out
        elif (lines_out and lines_out[-1].strip()) or line_out.strip():
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

    with open(input, 'rb') as src, GzipCompressingStream(filename=output, compresslevel=9, wsize=gzip_windowsize) as dst:
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

    do_gzip = not input in input_gzip_blacklist
    input = path.join(env.subst('$PROJECT_DIR'), input)
    filename = path.basename(input)
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
                mode = MinifyMode.Default
                if input.endswith(".html") or input.endswith(".htm"):
                    mode = MinifyMode.HTML
                elif input.endswith(".css"):
                    mode = MinifyMode.CSS
                elif input.endswith(".js") or input.endswith(".jsm") or input.endswith(".mjs"):
                    mode = MinifyMode.JavaScript

                lines = remove_whitespaces(lines, mode)

            with open(output, "w") as dst:
                dst.writelines(lines)

            input = output

    gzip_dir = path.join(data_dir, "gzip")
    if not path.exists(gzip_dir):
        os.mkdir(gzip_dir)

    if do_gzip:
        gzip_file(input, path.join(gzip_dir, filename + ".gz"))


for file in input_text_files:
    if file in input_gzip_blacklist:
        filename = path.basename(file)
    else:
        filename = path.basename(file) + ".gz"
    compress_file(file, True)

for file in input_binary_files:
    filename = path.basename(file) + ".gz"
    compress_file(file, False)
