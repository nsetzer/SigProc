
"""
A command line tool for running recipes.
"""
from . import newRecipeManager, Signal, Matrix, PipelineProcessRunner, ProcessBase, \
              TrackOutSpecifier, MatrixOutSpecifier, SignalOutSpecifier, SpectralMatrixOutSpecifier, \
              getSigProcDict, formatOptions, HistogramClip, Matrix2Image

import os, sys
import argparse

try:
    import numpy as np
    from PIL import Image
except ImportError as e:
    Image = None

def gendoc():

    for name,proc in sorted(getSigProcDict().items()):
        #SigProc.getDisplayName( proc.__name__ )
        sys.stdout.write("="*79+"\n")
        sys.stdout.write("Process: %s\n\n"%name)
        text=formatOptions( proc )
        if proc.__doc__:
            sys.stdout.write("%s\n"%proc.__doc__)
        sys.stdout.write("%s\n"%text)

def runRecipe(procs,jobs):

    pipe = PipelineProcessRunner(procs);

    for inFile,outFile in jobs:

        result = pipe.run(inFile)[-1]

        # result will be one of Signal, Matrix, Track
        # when possible, signals should be saved as waves
        if isinstance(result,Signal) and outFile.lower().endswith(".wav"):
            result.toWave(outFile)
        elif isinstance(result,Matrix) and outFile.lower().endswith(".png"):
            Matrix2Image(result.data,outFile)
        else:
            result.toFile(outFile)

def main():


    parser = argparse.ArgumentParser(description='run a Signal Processing Recipe.'+
        " By default uses ffmpeg to open audio files. this can be changed by using" +
        " the --ffmpeg or --sox flags to specify a binary path or by modifying" +
        " a recipe")
    parser.add_argument('--dir', dest='directory', default = "./recipes",
                        help='directory containing recipe *.ini files.')
    parser.add_argument('--ext', default = None,
                        help='output file extension.')
    parser.add_argument('--ffmpeg', default = None,
                        help='path to ffmpeg executeable (ffmpeg)')
    parser.add_argument('--sox', default = None,
                        help='path to sox executeable (default:not used)')
    parser.add_argument('--doc', action="store_true",
                        help='print documentation and exit.')
    parser.add_argument('recipe', type=str,
                        help='name of recipe to run')
    parser.add_argument('inFile', type=str, nargs="+",
                        help='input file')
    parser.add_argument('outFile', type=str,
                        help='output file or directory')

    if "--doc" in sys.argv:
        gendoc()
        sys.exit(0)


    args = parser.parse_args()


    if not os.path.exists(args.directory):
        sys.stderr.write("recipe directory not Found: %s\n"%args.directory)
        sys.stderr.write("specify --dir with a path to *.ini files.\n")
        sys.exit(1)

    jobs = []
    errors = 0

    ingestType = "ffmpeg"
    ingestPath = "ffmpeg"
    if args.sox:
        ingestType = "sox"
        ingestPath = args.sox
    elif args.ffmpeg:
        ingestPath = args.ffmpeg

    rm = newRecipeManager(args.directory,ingestType,ingestPath);

    if not rm.hasRecipe(args.recipe):
        sys.stderr.write("no recipe named `%s` found.\n"%args.recipe)
        sys.exit(1)

    # get the process pipeline to run
    # then determine a suggested file extension for output
    procs = rm.getRecipe(args.recipe)
    ext = args.ext
    if ext is None:
        mapping = { MatrixOutSpecifier:".mat",
                SpectralMatrixOutSpecifier: ".mat",
                SignalOutSpecifier:".wav",
                TrackOutSpecifier :".track",
        }
        spec = ProcessBase.getOutputSpecifier(procs[-1][0])
        ext = mapping.get(type(spec),".dat")

    # build the set of jobs to run, checking for input/output errors
    # the output can be a directory
    if os.path.isdir(args.outFile):
        for path in args.inFile:
            if not os.path.exists(path):
                sys.stderr.write("not Found: %s\n"%path)
                errors += 1;
            outFile = os.path.join(args.outFile,os.path.split(path)[1] + ext)
            jobs.append( (path,outFile) )
    else:
        if len(args.inFile)!=1:
            sys.stderr.write("invalid number of arguments or output not a directory.\n")
            errors += 1;
        if not os.path.exists(args.inFile[0]):
            sys.stderr.write("not Found: %s\n"%args.inFile[0])
            errors += 1;
        else:
            jobs.append( (args.inFile[0],args.outFile) )

    if errors>0:
        sys.exit(errors)

    runRecipe(procs, jobs)

if __name__ == '__main__':
    main()

