#!/usr/bin/env python3

"""A custom gzip compressing stream using a configurable window size.

Also contains a simple command line interface."""

import errno
import io
import os
import struct
import sys
import traceback
import zlib

__all__ = ["GzipCompressingStream"]


class GzipCompressingStream(io.RawIOBase):
    """A write-only stream deflate compressing the written data into a gzip file.

    Always opens files in binary mode.
    Unlike the python GzipFile this stream uses a smaller window size.
    It also cannot handle an already opened file as its filename parameter.
    """

    fileobj = None

    size = 0

    def __init__(self, filename, compresslevel=9, wsize=-10):
        """Constructor for the GzipCompressingStream class.

        Opens the file represented by filename if it ends with .gz,
        or the filename + .gz if it doesn't, in binary write mode.

        Then writes the gzip header to said file, and initializes
        the variables required for data compression.
        
        Parameters
        ----------
        filename: str
            The path of the output file to write to.
            If '-' is given, this stream will write to the standard output stream.
        compresslevel: int
            The zlib compression level to use.
            Valid values are from -8 to -15.
        wsize: int
            The zlib window size to use. See zlib docs.
        """

        if wsize > -8 or wsize < -15:
            raise ValueError("Window size out of range")

        if filename != "-" and not filename.endswith(".gz"):
            filename += ".gz"

        if filename == "-":
            self.fileobj = sys.stdout.buffer
            filename = ""
        else:
            self.fileobj = open(filename, 'wb')

        self.crc = zlib.crc32(b'')
        self.compress = zlib.compressobj(compresslevel, zlib.DEFLATED, wsize, zlib.DEF_MEM_LEVEL, 0)

        self._write_gzip_header(filename, compresslevel)

    def _write_gzip_header(self, filename, compresslevel=9):
        """Writes a gzip file header to the output file of this stream.

        Always uses deflate compression, an unknown os, and a last modified time of 0.
        Writes the filename if possible.

        Parameters
        ----------
        compresslevel: int
            The gzip compression level used.
        """

        self.fileobj.write(b'\x1f\x8b\x08')  # magic numbers + deflate compression
        try:
            # filename should always end with .gz
            filename = os.path.basename(filename)[:-3]
            if not isinstance(filename, bytes):
                filename = filename.encode('latin-1')
        except UnicodeEncodeError:
            print(f"Encoding the filename \"{self.name[:-3]}\" as latin-1 failed with the following exception.", file=sys.stderr)
            traceback.print_exc()
            filename = None

        if filename:
            self.fileobj.write(b'\x08')
        else:
            self.fileobj.write(b'\x00')

        self.fileobj.write(b'\x00\x00\x00\x00')  # don't write last modified time

        if compresslevel == 9:
            self.fileobj.write(b'\x02')
        elif compresslevel == 1:
            self.fileobj.write(b'\x04')
        else:
            self.fileobj.write(b'\x00')

        self.fileobj.write(b'\xff')  # unknown os

        if filename:
            self.fileobj.write(filename + b'\x00')

    def write(self, data):
        """Compresses the given data and writes it to the output file.

        Writes the compressed form of the given bytes, bytearray, or str(utf-8 encoded)
        to the output file.
        Also updates the crc.

        Parameters
        ----------
        data: bytes | bytearray | str
            The data to compress and write.

        Returns
        -------
        int
            The number of bytes in the input data.
        """

        if self.closed:
            raise ValueError("Cannot write to closed file")

        if isinstance(data, str):
            data = data.encode("utf-8")
        elif not isinstance(data, (bytes, bytearray)):
            raise ValueError("Can only write bytes, bytearray, or str")

        length = len(data)
        if length > 0:
            self.fileobj.write(self.compress.compress(data))
            self.size += length
            self.crc = zlib.crc32(data, self.crc)

        return length

    def close(self):
        """Flushes and closes this object and its underlying file object.

        Also writes the crc and size to the output file.
        """

        if self.closed:
            return

        fileobj = self.fileobj
        self.fileobj = None

        fileobj.write(self.compress.flush())
        fileobj.write(struct.pack("<L", self.crc))
        fileobj.write(struct.pack("<L", self.size & 0xffffffff))

        if fileobj != sys.stdout.buffer:
            fileobj.close()

    def flush(self):
        """Flushes the current compression cache and file object.
        
        Can degrade compression and should only be called if necessary.
        """

        self.fileobj.write(self.compress.flush(zlib.Z_SYNC_FLUSH))
        self.fileobj.flush()

    @property
    def closed(self):
        return self.fileobj == None

    def fileno(self):
        return self.fileobj.fileno()

    def isatty(self):
        return False

    def readable(self):
        return False

    def seekable(self):
        return False

    def writable(self):
        return True

    def readlines(self):
        raise OSError(errno.EBADF, "Cannot read write-only file.")

    def readline(self):
        raise OSError(errno.EBADF, "Cannot read write-only file.")

    def readinto(self, _):
        raise OSError(errno.EBADF, "Cannot read write-only file.")


def main():
    """The entrypoint for the command line interface.
    
    This function is only intended to be called if this file is run in module mode.
    It parses the command line arguments, compresses one or more files, and returns the process exit code.
    """

    from argparse import ArgumentParser
    parser = ArgumentParser(description=
        "A simple command line interface for the gzip_compressing_stream module, "
        "compresses a file and leaves it untouched.")
    parser.add_argument("--window-size", dest="wsize", metavar='SIZE', type=int,
        default=-10, help="The deflate window size to use.")
    parser.add_argument('-c', "--compression-level", dest="clevel", metavar='LEVEL',
        type=int, default=9, help="The level of compression to use. 1 is fastest, 9 is best.")
    parser.add_argument("args", nargs="*", default=["-"], metavar='FILE',
        help="A file to compress. Use '-' to read from standard input and write to standard output.")
    args = parser.parse_args()

    for arg in args.args:
        if arg == "-":
            fin = sys.stdin.buffer
        else:
            try:
                fin = open(arg, "rb")
            except OSError as e:
                if e.errno == errno.ENOENT:
                    print(f"File or directory {arg} not found!", file=sys.stderr)
                    return 1
                elif e.errno == errno.EISDIR:
                    print(f"File {arg} is a directory!", file=sys.stderr)
                    return 1
                else:
                    traceback.print_exc()
                    return 1

        fout = GzipCompressingStream(arg, args.clevel, args.wsize)
        while True:
            chunk = fin.read(128 * 1024)
            if not chunk:
                break
            fout.write(chunk)

        fout.close()
        if fin != sys.stdin.buffer:
            fin.close()

    return 0


if __name__ == '__main__':
    sys.exit(main())
