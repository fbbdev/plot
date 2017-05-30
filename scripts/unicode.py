#!/usr/bin/env python3

from collections import deque
import sys
import random
import urllib.request


class Interval:
    __slots__ = ('start', 'end')

    def __init__(self, start, end):
        self.start = start
        self.end = end

    @property
    def center(self):
        return (self.start + self.end) // 2

    def overlaps(self, other):
        return other.start in self or other.end in self or self in other

    def __len__(self):
        return self.end - self.start + 1

    def __eq__(self, other):
        return self.start == other.start and self.end == other.end

    def __contains__(self, item):
        if isinstance(item, Interval):
            return item.start in self and item.end in self

        return item >= self.start and item <= self.end

    def __repr__(self):
        return hex(self.start)[2:] + ".." + hex(self.end)[2:]


def remove_point(intervals, point):
    for i, interval in enumerate(intervals):
        if point in interval:
            if len(interval) == 1:
                del intervals[i]
            elif point == interval.start:
                interval.start += 1
            elif point == interval.end:
                interval.end -= 1
            else:
                intervals.insert(i, Interval(interval.start, point - 1))
                interval.start = point + 1

            return


def remove_interval(intervals, intv):
    start = None
    end = None

    for i, interval in enumerate(intervals):
        if not start:
            if intv.overlaps(interval):
                start = (i, interval)
        else:
            if not intv.overlaps(interval):
                end = (i - 1, intervals[i - 1])
                break

    if not start:
        return

    if end and end[1] is not start[1]:
        if end[1] not in intv:
            end[1].start = intv.end + 1
        del intervals[start[0] + 1, end[0] + (1 if end[1] in intv else 0)]

    if start[1] in intv:
        del intervals[start[0]]
    elif intv.start <= start[1].start:
        start[1].start = intv.end + 1
    elif intv.end >= start[1].end:
        start[1].end = intv.start - 1
    else:  # intv is contained in start[1]
        intervals.insert(start[0], Interval(start[1].start, intv.start-1))
        start[1].start = intv.end + 1


def build_tree(intervals, start=0, end=None, base=0):
    if end is None:
        end = len(intervals)

    tree = []

    if not intervals or end <= start:
        return []

    if len(intervals) == 1 or (end - start) == 1:
        return [(intervals[start].center, intervals[start], None, None)]

    middle = (start + end) // 2
    if (end - start) % 2:
        left_half = (start, middle)
        right_half = (middle + 1, end)
        intv = intervals[middle]
        center = intv.center
    else:
        lr = random.choice((True, False))

        if lr:
            left_half = (start, middle - 1)
            right_half = (middle, end)
            intv = intervals[middle - 1]
        else:
            left_half = (start, middle)
            right_half = (middle + 1, end)
            intv = intervals[middle]

        center = intv.center

    left_subtree_base = base + 1
    left_subtree = build_tree(
        intervals, left_half[0], left_half[1], left_subtree_base)

    right_subtree_base = base + 1 + len(left_subtree)
    right_subtree = build_tree(
        intervals, right_half[0], right_half[1], right_subtree_base)

    tree.append((center, intv,
                 None if not left_subtree else left_subtree_base,
                 None if not right_subtree else right_subtree_base))

    tree.extend(left_subtree)
    tree.extend(right_subtree)

    return tree


def verify_tree(tree, root=0):
    left_end = 0
    right_end = 0

    if tree[root][1]:
        left_end = tree[root][1].start
        right_end = tree[root][1].end

    if tree[root][2] is not None:
        left_end, left_max = verify_tree(tree, tree[root][2])
        if (left_max >= tree[root][0] or
                (tree[root][1] and left_max >= tree[root][1].start)):
            raise ValueError("left branch crosses center or interval (root={})"
                             .format(root))

    if tree[root][3] is not None:
        right_min, right_end = verify_tree(tree, tree[root][3])
        if (right_min <= tree[root][0] or
                (tree[root][1] and right_min <= tree[root][1].end)):
            raise ValueError(
                "right branch crosses center or interval (root={})"
                .format(root))

    return left_end, right_end


def render_tree(tree, var, indent):
    output = ""

    def render_interval(intv):
        if not intv:
            return "{ 1, 0 }"
        else:
            return "{{ 0x{0.start:x}, 0x{0.end:x} }}".format(intv)

    def render_link(link):
        if link is None:
            return "nullptr"
        else:
            return "&{}[{}]".format(var, link)

    for node in tree:
        output += (''.join(' ' for i in range(indent)) +
                   "{{ 0x{:x}, {}, {}, {} }},\n"
                   .format(node[0], render_interval(node[1]),
                           render_link(node[2]), render_link(node[3])))

    return output[:-1]  # Remove last newline


def parse_db_entry(entry):
    sep = entry.find(';')

    if sep == -1:
        raise ValueError("invalid database entry: {}".format(entry))

    codepoint = int(entry[:sep], 16)

    sep = entry.find(';', sep + 1)

    if sep == -1:
        raise ValueError("invalid database entry: \"{}\"".format(entry))

    cat_start = sep + 1
    cat_end = entry.find(';', cat_start)

    if cat_end == -1:
        raise ValueError("invalid database entry: \"{}\"".format(entry))

    category = entry[cat_start:cat_end]

    return codepoint, category


def parse_eaw_entry(entry):
    sep = entry.find(';')

    if sep == -1:
        raise ValueError("invalid eaw entry: {}".format(entry))

    rng_sep = entry.find('..')
    if rng_sep != -1 and rng_sep < sep:  # character range xxxx..yyyy
        codepoint = Interval(int(entry[:rng_sep], 16),
                             int(entry[rng_sep + 2:sep], 16))
    else:  # single code point
        codepoint = int(entry[:sep], 16)

    width_start = sep + 1
    width_end = width_start + 1

    if len(entry) == width_start:
        raise ValueError("invalid eaw entry: {}".format(entry))
    elif len(entry) > width_end:
        if entry[width_end] == 'a':  # Na (Narrow)
            width_end += 1

    width = entry[width_start:width_end]

    return codepoint, width


UCD_BASE = 'http://www.unicode.org/Public/UCD/latest/ucd/'
UCD_DATA = 'UnicodeData.txt'
UCD_EA_WIDTH = 'EastAsianWidth.txt'

UC_START = 0
UC_END = 0x10FFFF

# Control, Mark enclosing, Mark nonspacing, Line separators and Paragraph
# separators have zero width
ZERO_WIDTH_CATEGORIES = {'Cc', 'Cf', 'Cn', 'Cs', 'Me', 'Mn', 'Zl', 'Zp'}
# these act like combining characters
ZERO_WIDTH_INTERVAL = Interval(0x1160, 0x11FF)

# East asian Fullwidth and Wide code points are considered double width
DOUBLE_WIDTH_CODE_POINTS = {'F', 'W'}


if __name__ == '__main__':
    random.seed("cafnwab4gjc0runfyansmfn890mcta48wra4wcaw4yo98ujaucpaècaàp4c")

    zero_width = [Interval(UC_START, UC_END)]
    double_width = [Interval(UC_START, UC_END)]

    print("Fetching {}".format(UCD_DATA), file=sys.stderr)
    local_filename, _ = urllib.request.urlretrieve(UCD_BASE + UCD_DATA)
    with open(local_filename) as unicode_db:
        prev = 0
        print("Parsing {}".format(UCD_DATA), file=sys.stderr)
        for line in unicode_db:
            entry = line.lstrip()
            if not len(entry) or entry[0] == '#':
                continue

            cp, cat = parse_db_entry(entry)

            if (cp - prev) > 1:
                remove_interval(zero_width, Interval(prev + 1, cp - 1))

            prev = cp

            if (cat not in ZERO_WIDTH_CATEGORIES and
                    cat[0] not in ZERO_WIDTH_CATEGORIES and
                    cp not in ZERO_WIDTH_INTERVAL):
                remove_point(zero_width, cp)

        if prev < UC_END:
            remove_interval(zero_width, Interval(prev + 1, UC_END))

    print("Building zero-width search tree", file=sys.stderr)
    zero_width = build_tree(zero_width)

    print("Verifying tree... ", end='', file=sys.stderr)
    minmax = verify_tree(zero_width)
    print("OK ({0} nodes, min={1[0]:x}, max={1[1]:x})"
          .format(len(zero_width), minmax), file=sys.stderr)

    print("Generating C code for zero-width search tree", file=sys.stderr)
    zero_width = render_tree(zero_width, 'unicode_tables<T>::zero_width', 8)

    print("Fetching {}".format(UCD_EA_WIDTH), file=sys.stderr)
    local_filename, _ = urllib.request.urlretrieve(UCD_BASE + UCD_EA_WIDTH)
    with open(local_filename) as eaw_db:
        prev = 0
        print("Parsing {}".format(UCD_EA_WIDTH), file=sys.stderr)
        for line in eaw_db:
            entry = line.lstrip()
            if not len(entry) or entry[0] == '#':
                continue

            cp, width = parse_eaw_entry(entry)

            nxt = cp.start if isinstance(cp, Interval) else cp
            if (nxt - prev) > 1:
                remove_interval(double_width, Interval(prev + 1, nxt - 1))

            prev = cp.end if isinstance(cp, Interval) else cp

            if width not in DOUBLE_WIDTH_CODE_POINTS:
                if isinstance(cp, Interval):
                    remove_interval(double_width, cp)
                else:
                    remove_point(double_width, cp)

        if prev < UC_END:
            remove_interval(double_width, Interval(prev + 1, UC_END))

    print("Building double-width search tree", file=sys.stderr)
    double_width = build_tree(double_width)

    print("Verifying tree... ", end='', file=sys.stderr)
    minmax = verify_tree(double_width)
    print("OK ({0} nodes, min={1[0]:x}, max={1[1]:x})"
          .format(len(double_width), minmax), file=sys.stderr)

    print("Generating C code for double-width search tree", file=sys.stderr)
    double_width = render_tree(double_width, 'unicode_tables<T>::double_width', 8)

    print("Rendering header", file=sys.stderr)
    with open('unicode_data_template.hpp') as template:
        for line in template:
            print(
                line.format(zero_width=zero_width, double_width=double_width),
                end='')

    print("Done.", file=sys.stderr)
