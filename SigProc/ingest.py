#! python35 $this

import numpy as np;
from .process import *
try:
    from yue.core.bass.ingest import FileIngest
except ImportError:
    FileIngest = None
from . import *
from .riff import WaveWRX

from subprocess import Popen, PIPE, STDOUT
import io,os

class BassIngestProcess(SimpleProcess):
    """
    note this implementation is broken
    it automatically applies preemph of .97
    """
    def run(self):

        f = FileIngest(self.fileIn,self.outRate)

        out = np.asarray(f.read())
        return Signal(out,self.outRate)

    def getProgress(self):
        return (50,"")

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                FileInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="outRate",dtype=int, \
                    min=6000,max=44100,default=8000, \
                    help="resample input to Sample Rate"),
                ]
        return opts

class ExternalIngestProcessImpl(SimpleProcess):

    def runProcess(self,command):

        cwd=os.getcwd();

        print(" ".join(command))
        p = Popen(command, shell=False, stdout=PIPE, stderr=PIPE, cwd=cwd)

        output, errors = p.communicate()
        if p.returncode!=0:
            print(output)
            print(errors)
            print(p.returncode)
            raise Exception(command)

        if errors:
            print(errors)

        # trim out any text ffmpeg writes before the main output
        index = output.index(b"RIFF")
        head = output[:index]
        if head:
            print(head)
        output = output[index:]


        #with open("temp.wav","wb") as wb:
        #    wb.write(output)

        # open the output as a wave file, in floating point mode
        rate,data = WaveWRX().read(io.BytesIO(output),dtype=np.float32)
        return Signal(data,rate)

    def run(self):

        if not os.path.exists(self.fileIn):
            raise Exception("Not Found: %s"%self.fileIn)

        return self.runProcess( self.getCommand() )

class FFmpegIngestProcess(ExternalIngestProcessImpl):
    DEFAULT_BIN_PATH = "ffmpeg"

    def getCommand(self):
        command = [ self.binPath,
                    "-loglevel","panic",
                    "-i",self.fileIn,
                    "-ac", "1",
                    "-ar", "%d"%self.outRate,
                    "-f","wav",
                    "pipe:1",]
        return command

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                FileInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="binPath",dtype=str, \
                    default=FFmpegIngestProcess.DEFAULT_BIN_PATH, \
                    help="path to FFmpeg"),
                ProcessOptionsSpecifier(store="logLevel",dtype=str, \
                    default="panic", \
                    help="ffmpeg logging level (panic, error, warning, info, debug)"),
                ProcessOptionsSpecifier(store="outRate",dtype=int, \
                    min=6000,max=44100,default=8000, \
                    help="resample input to Sample Rate"),
                ]
        return opts

class SoxIngestProcess(ExternalIngestProcessImpl):
    DEFAULT_BIN_PATH = "sox"

    def getCommand(self):
        command = [ self.binPath,
                    self.fileIn,
                    "-c", "1",
                    "-r", "%d"%self.outRate,
                    "-t","wav",
                    "-",]
        return command;

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                FileInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="binPath",dtype=str, \
                    default=SoxIngestProcess.DEFAULT_BIN_PATH, \
                    help="path to sox"),
                ProcessOptionsSpecifier(store="outRate",dtype=int, \
                    min=6000,max=44100,default=8000, \
                    help="resample input to Sample Rate"),
                ]
        return opts

class SoxHistogramer(SimpleProcess):

    def xgetCommand(self):
        # this doesnt work because of a buffering problem on windows
        # only the first 16k~ish bytes makes it into the resulting image
        command = [ self.binPath,
                    "-t", "raw",
                    "-b", "16",
                    "-e", "signed",
                    "-c", "1",
                    "-r", "%d"%self.signal.sample_rate,
                    "-",
                    "-n", "spectrogram",
                    "-Y", "200",
                    "-X","%d"%self.pixelsPerSecond,
                    "-m","-r","-o",
                    "-"]
        return command;

    def getCommand(self):
        command = [ self.binPath,
                    self.fileIn,
                    "-n", "spectrogram",
                    "-y", "%d"%self.frameHeight,
                    "-X","%d"%self.pixelsPerSecond,
                    "-m","-r",
                    "-o","-"]
        return command;

    def runProcess(self,command, inputData = None):

        cwd=os.getcwd();

        print(" ".join(command))
        p = Popen(command, shell=False, stdin=PIPE,stdout=PIPE, stderr=PIPE, cwd=cwd)

        output, errors = p.communicate(inputData)
        if p.returncode!=0:
            print(output.decode("utf-8"))
            print(errors.decode("utf-8"))
            print(p.returncode)
            raise Exception(command)

        if errors:
            print(errors)

        # trim out any text ffmpeg writes before the main output
        with open(self.fileOut,"wb") as wb:
            wb.write(output);

        return None

    def run(self):

        return self.runProcess( self.getCommand() ) #, self.signal.raw() )

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                FileInSpecifier(),
                FileOutSpecifier(),
                ProcessOptionsSpecifier(store="binPath",dtype=str, \
                    default="sox", \
                    help="path to sox"),
                ProcessOptionsSpecifier(store="frameHeight",dtype=int, \
                    default=129, \
                    help="Y-axis size in pixels; slow if not 1 + 2^n"),
                ProcessOptionsSpecifier(store="pixelsPerSecond",dtype=int, \
                    default=50, \
                    help="X-axis pixels/second; default derived or 100"),
                ]
        return opts

