#!/usr/bin/env python3

import re
import mmap
import os
import sys


class Header:
    __slots__ = ['dir', 'file', 'map', 'data', 'comment', 'once',
                 'includes', 'local_includes', 'body']

    FIRST_COMMENT = re.compile(br'^\s*(/\*(?:[^*]|\*+[^*/])*\*+/)')
    SKIP = re.compile(br'\s*(?:(?://[^\n]*|/\*(?:[^*]|\*+[^*/])*\*+/)\s*)*')
    DIRECTIVE_SKIP = re.compile(
        br'(?:[ \t]|\\\n)*(?:/\*(?:[^*]|\*+[^*/])*\*+/(?:[ \t]|\\\n)*)*')
    DIRECTIVE_SEP = re.compile(
        br'[ \t]|\\\n|/\*(?:[^*]|\*+[^*/])*\*+/')
    DIRECTIVE_END = re.compile(
        br'(?:[ \t]|\\\n)*(?:/\*(?:[^*]|\*+[^*/])*\*+/(?:[ \t]|\\\n)*)*(?:$|\n)')
    PRAGMA = re.compile(b'pragma')
    ONCE = re.compile(b'once')
    INCLUDE = re.compile(b'include\b')
    INCLUDE_ARG = re.compile(br'<((?:[^>\n]|\\\n)+)>')
    LOCAL_INCLUDE_ARG = re.compile(br'"((?:[^"\n]|\\\n)+)"')

    def __init__(self, path):
        path = os.path.abspath(path)
        self.dir = os.path.dirname(path)
        self.file = open(path)

        try:
            self.map = mmap.mmap(self.file.fileno(), 0,
                                 access=mmap.ACCESS_READ)
            self.data = memoryview(self.map)
        except:
            self.file.close()
            raise

        pos = 0

        # Extract first comment
        m = self.FIRST_COMMENT.match(self.map)
        if m is not None:
            self.comment = self.data[m.start(1):m.end(1)]
            pos = m.end()

        self.once = False
        self.includes = []
        self.local_includes = []

        self.body = slice(pos, len(self.map))

        while pos < len(self.map):
            pos = self.SKIP.match(self.map, pos).end()
            if pos >= len(self.map):
                break

            if self.map[pos] == ord('#'):
                pos = self.DIRECTIVE_SKIP.match(self.map, pos + 1).end()
                if pos >= len(self.map):
                    break

                if self.PRAGMA.match(self.map, pos) is not None:
                    if pos + 6 >= len(self.map):
                        break

                    m = self.DIRECTIVE_SEP.match(self.map, pos + 6)
                    if m is None or m.end() >= len(self.map):
                        break

                    m = self.DIRECTIVE_SKIP.match(self.map, m.end())
                    if m.end() >= len(self.map):
                        break

                    m = self.ONCE.match(self.map, m.end())
                    if m is None:
                        break

                    m = self.DIRECTIVE_END.match(self.map, m.end())
                    if m is None:
                        break

                    pos = m.end()

                    self.body = slice(pos, len(self.map))
                    self.once = True
                elif self.INCLUDE.match(self.map, pos) != -1:
                    if pos + 7 >= len(self.map):
                        break

                    pos = self.DIRECTIVE_SKIP.match(self.map, pos + 7).end()
                    if pos >= len(self.map):
                        break

                    arg = self.INCLUDE_ARG.match(self.map, pos)
                    local = arg is None
                    if local:
                        arg = self.LOCAL_INCLUDE_ARG.match(self.map, pos)
                        if arg is None:
                            break

                    m = self.DIRECTIVE_END.match(self.map, arg.end())
                    if m is None:
                        break

                    (self.local_includes if local else self.includes).append(
                        arg.group(1).decode('charmap', errors='ignore'))

                    pos = m.end()
                    self.body = slice(pos, len(self.map))
                else:
                    break
            else:
                break

        self.body = self.data[self.body]

    def close(self):
        if self.comment is not None:
            self.comment.release()
        self.body.release()
        self.data.release()
        self.map.close()
        self.file.close()

    def __enter__(self):
        return self

    def __exit__(self, type, exc, tb):
        self.close()


if __name__ == '__main__':
    roots = []
    includes = []
    new_includes = []

    deps = {}

    for arg in sys.argv[1:]:
        hdr = Header(arg)

        roots.append(hdr)
        includes.append(hdr)
        new_includes.append(hdr)

        deps[hdr] = []

    if not len(roots):
        sys.exit(0)

    while len(new_includes):
        old_includes = new_includes
        new_includes = []

        for hdr in old_includes:
            for path in hdr.local_includes:
                if not os.path.isabs(path):
                    path = os.path.abspath(os.path.join(hdr.dir, path))

                for inc in includes:
                    if os.path.samefile(inc.file.name, path):
                        deps[hdr].append(inc)
                        break
                else:
                    inc = Header(path)

                    includes.append(inc)
                    new_includes.append(inc)

                    deps[hdr].append(inc)
                    deps[inc] = []

    # Sort dependencies by DFS
    sorted_local_includes = []

    discovered = set()
    stack = []

    for root in roots:
        if root not in discovered:
            discovered.add(root)
            stack.append((root, deps[root]))

        while len(stack):
            u, d = stack[-1]
            while len(d):
                v = d.pop(0)
                if v not in discovered:  # Ignore back/forward/cross edges
                    discovered.add(v)
                    stack.append((v, deps[v]))
                    break
            else:  # Finished
                sorted_local_includes.append(u)
                stack.pop()

    comment = None

    for hdr in roots:
        if hdr.comment is not None:
            comment = hdr.comment
            break
    else:
        for hdr in includes:
            if hdr.comment is not None:
                comment = hdr.comment
                break

    once = False
    sorted_includes = set()

    for hdr in includes:
        once = once or hdr.once
        sorted_includes = sorted_includes.union(hdr.includes)

    sorted_includes = sorted(sorted_includes)

    if comment is not None:
        sys.stdout.buffer.write(comment)
        sys.stdout.buffer.write(b'\n\n')

    if once:
        sys.stdout.buffer.write(b'#pragma once\n\n')

    for inc in sorted_includes:
        sys.stdout.buffer.write(b'#include <' + inc.encode('charmap') + b'>\n')

    sys.stdout.buffer.write(b'\n')

    for hdr in sorted_local_includes:
        sys.stdout.buffer.write(b'// ' + os.path.relpath(hdr.file.name).encode('charmap') + b'\n')
        sys.stdout.buffer.write(hdr.body)
        sys.stdout.buffer.write(b'\n\n')

    for hdr in includes:
        hdr.close()
