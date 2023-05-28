#!/usr/bin/python

Import ("env")

import os
import os.path as path

from gzip_compressing_stream import GzipCompressingStream

# Unquoted spaces will be removed from these.
input_text_files = [ 'src/html/index.html', 'src/html/not_found.html', 'src/html/main.css', 'src/html/index.js', 'images/favicon.svg' ]

input_binary_files = [ 'images/favicon.ico', 'images/favicon.png' ]

# These files will not be gzip compressed, just copied and potentially stripped of spaces.
input_gzip_blacklist = [ 'src/html/index.html' ]

# The javascript keywords that require a space after them.
js_keywords = [ 'await', 'case', 'class', 'const', 'delete', 'export', 'extends', 'function', 'import', 'in', 'instanceof', 'let', 'new', 'return', 'static', 'throw', 'typeof', 'var', 'void', 'yield' ]

# The characters before/after which a space doesn't make a different in default mode.
seperator_chars = [ '(', ')', '[', ']', '{', '}', '<', '>', ':', ';', ',' ]

# The window size parameter to use for gzip compression.
# The actual window size used by zlib is pow(2, -gzip_windowsize).
# The window size used for decompression(on the esp) has to be
# at least as much as the window size used for compression.
# This requires a pow(2, -gzip_windowsize) byte buffer on the esp.
gzip_windowsize = -10


def remove_whitespaces(lines_in, jsmode):
    """Removes whitespaces and comments from text.
    
    Can only handle /* ... */ comments.
    Can not handle html pre tags.
    In javascript mode spaces are not removed from inside quotes.
    
    Parameters
    ----------
    lines_in: list
        A list of strings containing the lines to edit.
    jsmode: bool
        Whether to remove spaces in javascript mode.
        The alternative is the "default" css/svg/html mode.
    
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
            if not current_quote and (char == ' ' or char == '\t'):
                if jsmode and since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                if jsmode or not last_char or last_char in seperator_chars:
                    continue
            elif jsmode and not current_quote and (char == "'" or char == '"'):
                current_quote = char
            elif char == current_quote:
                current_quote = None
            elif char == '/' and current_quote == "/*" and last_char == '*':
                current_quote = None
                last_char = None
                continue
            elif not current_quote and char == '*' and last_char == '/':
                if since_space in js_keywords:
                    line_out += last_char + ' '
                    last_char = None
                since_space = ""
                current_quote = "/*"
            elif not jsmode and char in seperator_chars and (last_char == ' ' or char == '\t'):
                last_char = None

            if last_char and current_quote != "/*":
                line_out += last_char

            since_space += char
            if last_char != '\t':
                last_char = char
            else:
                last_char = ' '

        if last_char:
            line_out += last_char

        if not lines_out or lines_out[-1].strip() or line_out.strip():
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

    filename = path.basename(input)
    minify = text

    do_gzip = not input in input_gzip_blacklist

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
                lines = remove_whitespaces(lines, input.endswith(".js"))

            with open(output, "w") as dst:
                dst.writelines(lines)

            input = output

    gzip_dir = path.join(data_dir, "gzip")
    if not path.exists(gzip_dir):
        os.mkdir(gzip_dir)

    if do_gzip:
        gzip_file(input, path.join(gzip_dir, filename + ".gz"))


for file in input_text_files:
    filename = path.basename(file) + ".gz"
    # Always build because the output will depend on whether we are building in debug mode.
    env.AlwaysBuild(f"$BUILD_DIR/{filename}.txt.o")
    compress_file(file, True)

for file in input_binary_files:
    filename = path.basename(file) + ".gz"
    # Always build these because I'm too lazy to check whether they actually changed.
    env.AlwaysBuild(f"$BUILD_DIR/{filename}.txt.o")
    compress_file(file, False)
