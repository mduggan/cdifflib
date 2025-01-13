#!/usr/bin/env python
import random
import pytest

import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

from cdifflib import CSequenceMatcher
from difflib import SequenceMatcher


##
# A function to count lines of difference using a provided SequenceMatcher
# class.
#
def linecount(smclass, s1, s2):
    linecount = 0
    sm = smclass(None, s1, s2, autojunk=False)
    i = 0
    j = 0
    for ai, bj, n in sm.get_matching_blocks():
        linecount += max(ai - i, bj - j)
        i, j = ai + n, bj + n
    return linecount


def profile_sequence_matcher(smclass, a, b, n):
    for i in range(0, n):
        out = linecount(smclass, a, b)
    print("Diff from %s is %d" % (smclass.__name__, out))


def print_stats(prof):
    print("Top 10 by cumtime:")
    prof.sort_stats("cumulative")
    prof.print_stats(10)
    print("Top 10 by selftime:")
    prof.sort_stats("time")
    prof.print_stats(10)


def generate_similar_streams(nlines, ndiffs):
    lines = [None] * nlines
    chars = [chr(x) for x in range(32, 126)]
    for l in range(len(lines)):
        lines[l] = "".join([random.choice(chars) for x in range(60)])
    orig = list(lines)
    for r in range(ndiffs):
        row = random.randint(0, len(lines) - 1)
        lines[row] = "".join([random.choice(chars) for x in range(60)])
    return orig, lines


def assertNearlyEqual(x, y):
    """Simple test to make sure floats are close"""
    assert abs(x - y) < 1e-5


@pytest.fixture()
def test_streams():
    random.seed(1234)
    streama, streamb = generate_similar_streams(2000, 200)
    yield streama, streamb


def testCDifflibVsDifflibRandom(test_streams):
    """Test cdifflib gets same answer as difflib on semi-random sequence of lines"""
    cdiff = linecount(CSequenceMatcher, test_streams[0], test_streams[1])
    diff = linecount(SequenceMatcher, test_streams[0], test_streams[1])
    assert cdiff != 0
    assert cdiff == diff


def testCDifflibVsDifflibIdentical(test_streams):
    """Test cdifflib gets 0 difference on the same sequence of lines"""
    cdiff = linecount(CSequenceMatcher, test_streams[0], test_streams[0])
    assert cdiff == 0
    cdiff = linecount(CSequenceMatcher, test_streams[1], test_streams[1])
    assert cdiff == 0


def testCDifflibWithEmptyInput(test_streams):
    """Test cdifflib gets correct difference vs empty stream"""
    cdiff = linecount(CSequenceMatcher, [], [])
    assert cdiff == 0
    cdiff = linecount(CSequenceMatcher, test_streams[0], [])
    assert cdiff == len(test_streams[0])
    cdiff = linecount(CSequenceMatcher, [], test_streams[1])
    assert cdiff == len(test_streams[1])


def testCDifflibWithBadTypes(test_streams):
    """Check cdifflib raises the same type complaints as difflib"""
    with pytest.raises(TypeError):
        linecount(CSequenceMatcher, None, test_streams[1])
    with pytest.raises(TypeError):
        linecount(SequenceMatcher, None, test_streams[1])
    with pytest.raises(TypeError):
        linecount(CSequenceMatcher, test_streams[0], 1)
    with pytest.raises(TypeError):
        linecount(SequenceMatcher, test_streams[0], 1)
    with pytest.raises(TypeError):
        linecount(SequenceMatcher, test_streams[0], [{}, {}])
    with pytest.raises(TypeError):
        linecount(SequenceMatcher, [set([])], [1])


def testCDifflibWithNonLists(test_streams):
    """Check cdifflib handles non-list types the same as difflib"""
    cdiff = linecount(CSequenceMatcher, "not a list", "also not a list")
    diff = linecount(SequenceMatcher, "not a list", "also not a list")
    assert diff == cdiff
    assert cdiff == 5

    def gena():
        for x in test_streams[0]:
            yield x

    def genb():
        for x in test_streams[1]:
            yield x

    cdiff = linecount(CSequenceMatcher, gena(), genb())
    # actually difflib doesn't handle generators, just check cdiff result.
    assert cdiff > 0


def testCDifflibWithBug5Data():
    """Check cdifflib returns the same result for bug #5
    (autojunk handling issues)"""
    import testdata

    # note: convert both to lists for Python 3.3
    sm = SequenceMatcher(None, testdata.a5, testdata.b5)
    difflib_matches = list(sm.get_matching_blocks())

    sm = CSequenceMatcher(None, testdata.a5, testdata.b5)
    cdifflib_matches = list(sm.get_matching_blocks())

    assert difflib_matches == cdifflib_matches


def testSeq1ResetsCorrectly():
    s = CSequenceMatcher(None, "abcd", "bcde")
    assertNearlyEqual(s.ratio(), 0.75)
    s.set_seq1("bcde")
    assertNearlyEqual(s.ratio(), 1.0)


def main():
    from optparse import OptionParser
    import time

    parser = OptionParser(
        description="Test the C version of difflib. Either "
        "specify files, or leave empty for auto-generated "
        "random lines",
        usage="Usage: %prog [options] [file1 file2]",
    )
    parser.add_option(
        "-n",
        "--niter",
        dest="niter",
        type="int",
        help="num of iterations (default=%default)",
        default=1,
    )
    parser.add_option(
        "-l",
        "--lines",
        dest="lines",
        type="int",
        help="num of lines to generate if no files specified (default=%default)",
        default=20000,
    )
    parser.add_option(
        "-d",
        "--diffs",
        dest="diffs",
        type="int",
        help="num of random lines to change if no files specified (default=%default)",
        default=200,
    )
    parser.add_option(
        "-p",
        "--profile",
        dest="profile",
        default=False,
        action="store_true",
        help="run in the python profiler and print results",
    )
    parser.add_option(
        "-c",
        "--compare",
        dest="compare",
        default=False,
        action="store_true",
        help="also run the non-c difflib to compare outputs",
    )
    parser.add_option(
        "-y",
        "--yep",
        dest="yep",
        default=False,
        action="store_true",
        help="use yep to profile the c code",
    )

    (opts, args) = parser.parse_args()

    start = int(time.time())

    if opts.niter < 1:
        parser.error("Need to do at least 1 iteration..")

    if args:
        if len(args) != 2:
            parser.error("Need exactly 2 files to compare.")
        try:
            print("Reading input files...")
            s1 = open(args[0]).readlines()
            s2 = open(args[1]).readlines()
        except (IOError, OSError):
            parser.error("Couldn't load input files %s and %s" % (args[0], args[1]))
    else:
        print("Generating random similar streams...")
        s1, s2 = generate_similar_streams(opts.lines, opts.diffs)

    # shonky, but saves time..
    sys.path.append("build/lib.linux-x86_64-2.7/")
    sys.path.append("build/lib.linux-x86_64-2.7-pydebug/")
    sys.path.append("build/lib.macosx-10.6-intel-2.7")

    if opts.yep:
        import yep

        yep.start("cdifflib.prof")

    if opts.profile:
        import cProfile
        import pstats

        fn = "cdifflib_%d.prof" % start
        print("Profiling cdifflib.CSequenceMatcher...")
        cProfile.runctx(
            "p(sm,a,b,n)",
            dict(p=profile_sequence_matcher),
            dict(a=s1, b=s2, n=opts.niter, sm=CSequenceMatcher),
            fn,
        )
        print_stats(pstats.Stats(fn))

        if opts.compare:
            fn = "difflib_%d.prof" % start
            print("Profiling difflib.SequenceMatcher...")
            cProfile.runctx(
                "p(sm,a,b,n)",
                dict(p=profile_sequence_matcher),
                dict(a=s1, b=s2, n=opts.niter, sm=SequenceMatcher),
                fn,
            )
            print_stats(pstats.Stats(fn))

    else:
        print("Running cdifflib.CSequenceMatcher %d times..." % opts.niter)
        profile_sequence_matcher(CSequenceMatcher, s1, s2, opts.niter)
        if opts.compare:
            print("Running difflib.SequenceMatcher %d times..." % opts.niter)
            profile_sequence_matcher(SequenceMatcher, s1, s2, opts.niter)

    if opts.yep:
        yep.stop()


if __name__ == "__main__":
    main()
