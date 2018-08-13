#! python
import os
import sys
import subprocess
import logging
import argparse
import shlex

from distutils.spawn import find_executable as which

msvc_path_default = "C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\Tools\\vsvars32.bat"

def sh(cmd):
    logging.info(" ".join(cmd))
    return subprocess.Popen(cmd).communicate()

def maybe_quote(arg):
    if ' ' in arg:
        # these do not produce valid bat strings on windows
        #return shlex.quote(arg)
        #return str(arg).__repr__()
        return "\"%s\"" % arg
    return arg

def generate_win32(args):

    cmd = [
        args.cmake,
        '-G', args.target,
        '-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE',
        '-DBUILD_SHARED_LIBS=TRUE',
        args.script_dir
    ]

    if not os.path.exists(args.vcvars):
        raise FileNotFoundError(args.vcvars)

    # write a batch script which sets the environment
    batch = ".\\init.bat"
    with open(batch, "w") as wf:
        # cd to the script directory (allows user to double click script)
        wf.write("cd %~dp0\n")
        # run vcvars to configure environment
        wf.write("call \"%s\"\n" % args.vcvars)
        # invoke cmake
        wf.write(' '.join([maybe_quote(arg) for arg in cmd]))
        wf.write("\n")

    with open(".\\build.bat", "w") as wf:
        # cd to the script directory (allows user to double click script)
        wf.write("cd %~dp0/..\n")
        # run vcvars to configure environment
        wf.write("call \"%s\"\n" % args.vcvars)
        # invoke cmake
        wf.write('%s --build msvc --config Release\n' % maybe_quote(args.cmake))

    with open(".\\build_debug.bat", "w") as wf:
        # cd to the script directory (allows user to double click script)
        wf.write("cd %~dp0/..\n")
        # run vcvars to configure environment
        wf.write("call \"%s\"\n" % args.vcvars)
        # invoke cmake
        wf.write('%s --build msvc --config Debug\n' % maybe_quote(args.cmake))

    sh([batch])

def generate(args):

    if args.profile == 'cover':
        profile = "Debug"
    else:
        profile = args.profile.title()

    cmd = [
        args.cmake,
        '-G', args.target,
        '-DCMAKE_BUILD_TYPE=%s' % profile,
        '-DCMAKE_MACOSX_RPATH=NEW',
        '-DCMAKE_C_COMPILER=%s' % args.CC,
        '-DCMAKE_CXX_COMPILER=%s' % args.CXX,
        args.script_dir
    ]
    sh(cmd)

def parseArgs(argv):

    CC = which("gcc")
    CXX = which("g++")
    CMAKE = which("cmake") or which("cmake3")

    default_target = "Unix Makefiles"
    if sys.platform == "win32":
        default_target = "Visual Studio 14 2015 Win64"
    elif which("ninja"):
        default_target = "Ninja"

    parser = argparse.ArgumentParser(description="""
        Configure SIGPROC build
    """)

    parser.add_argument("--target",
        default=default_target,
        help="build target (%s)" % default_target)

    parser.add_argument("--cmake",
        default=CMAKE,
        help="cmake binary path (%s)" % CMAKE)

    if sys.platform != "win32":
        parser.add_argument("profile",
            choices=['release', 'debug', 'cover'],
            default="release", nargs="?",
            help="generate build for type (release)")

        parser.add_argument("--CC",
            default=CC,
            help="c compiler path (%s)" % CC)

        parser.add_argument("--CXX",
            default=CXX,
            help="c++ compiler path (%s)" % CXX)

        parser.add_argument("--build_dir",
            default=None,
            help="build directory (./build/${profile})")
    else:
        parser.add_argument("--build_dir",
            default="./build/msvc",
            help="build directory (./build/msvc)")

        parser.add_argument("--vcvars",
            default=msvc_path_default,
            help="path to MSVC vcvars environment configuration script (%s)" % msvc_path_default)

    args = parser.parse_args()

    if args.build_dir is None:
        args.build_dir = "./build/" + args.profile

    args.script_dir = os.path.split(os.path.realpath(__file__))[0]

    for kv in args.__dict__.items():
        print("%-15s : %s" % kv)

    return args

def main():

    logging.basicConfig(level=logging.INFO)
    args = parseArgs(sys.argv)

    if not os.path.exists(args.build_dir):
        os.makedirs(args.build_dir)

    os.chdir(args.build_dir)

    if sys.platform == "win32":
        generate_win32(args)
    else:
        generate(args)

if __name__ == '__main__':
    main()

