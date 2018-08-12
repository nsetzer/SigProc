
import os
import sys
import subprocess
import logging
import argparse

from distutils.spawn import find_executable as which

def sh(cmd):

    logging.info(" ".join(cmd))
    return subprocess.Popen(cmd)

def parseArgs(argv):

    args = lambda: None

    args.profile = "debug"
    args.build_dir = "./build/" + args.profile
    args.target = "Unix Makefiles"
    args.script_dir = os.path.split(os.path.realpath(__file__))[0]

    return args

def main():

    logging.basicConfig(level=logging.INFO)
    args = parseArgs(sys.argv)

    CC = which("gcc")
    CXX = which("g++")
    CMAKE = which("cmake") or which("cmake3")

    # if which("ninja"):
    #    args.target = "Ninja"

    if not os.path.exists(args.build_dir):
        os.makedirs(args.build_dir)

    os.chdir(args.build_dir)

    cmd = [
        CMAKE,
        '-G', args.target,
        '-DCMAKE_BUILD_TYPE=Debug',
        '-DCMAKE_MACOSX_RPATH=NEW',
        '-DCMAKE_C_COMPILER=%s' % CC,
        '-DCMAKE_CXX_COMPILER=%s' % CXX,
        args.script_dir
    ]
    sh(cmd)

if __name__ == '__main__':
    main()

