

from .logger import Logger
from .process import *
import numpy as np

from .timeinfo import DenseTimeInfo, SparseTimeInfo

class Signal(object):
    """N Dimensional Signal Data Representation"""
    def __init__(self, data,sample_rate=0):
        self.data = data
        self.sample_rate = int(sample_rate)
    def __repr__(self):
        return "<%s(%s,%d)>"%(self.__class__.__name__,self.data.shape,self.sample_rate)
    def toFile(self,filename,exif=None):
        MatrixWRX().write(filename,self.sample_rate,self.data,exif)
    def toWave(self,filename,exif=None):
        WaveWRX().write(filename,self.sample_rate,self.data,exif)

    def clone(self):
        sig = Signal(np.copy(self.data),self.sample_rate)
        return sig

    def raw(self):
        if self.data.dtype in (np.float32,np.float64):
            inf=np.iinfo(np.int16)
            return (self.data* -inf.min).astype(np.int16).tobytes()
        else:
            return self.data.astype(np.int16).tobytes()

    @staticmethod
    def open(fpath):
        if fpath.lower().endswith(".au"):
            Fs,data = AudioWRX().read(fpath,dtype=np.float32)
            return Signal(data,Fs)
        elif fpath.lower().endswith(".wav"):
            Fs,data = WaveWRX().read(fpath,dtype=np.float32)
            return Signal(data,Fs)
        raise Exception("unsupported file: %s"%fpath)

class AudioSignal(Signal):
    """ a Signal Type containing audio samples, that could be played through a speaker"""
    pass

class DataSignal(Signal):
    """ a Signal Type that contains Data, that would not make sense to play through a speaker"""
    pass

class Matrix(object):
    """Matrix is a 2D floating point representation of an image"""

    K_FEATURES    = 0
    K_SPECTROGRAM = 1

    def __init__(self, data,sample_rate=0,kind=0):
        self.data = np.asarray(data)
        self.sample_rate = int(sample_rate)
        self.source_rate = 0
        self.kind = kind
        self.ylabel_fptr = None

    def setKind(self,kind):
        self.kind = kind
    @property
    def shape(self):
        return self.data.shape
    @property
    def size(self):
        return self.data.size
    @property
    def frame_height(self):
        return self.data.shape[1]
    def __len__(self):
        return self.data.shape[0]
    @staticmethod
    def fromSignal(signal,data,rate):
        mat = Matrix(data,rate)
        mat.source_rate = signal.sample_rate
        return mat

    @staticmethod
    def fromMatrix(matrix,data,rate):
        mat = Matrix(data,rate)
        mat.source_rate = matrix.source_rate
        return mat

    def ylabel(self,index):
        """ given a pixel index on the y-axis, return a string representation

        for example a spectrogram may return the frequency component
        """
        if self.ylabel_fptr is None:
            return "%d"%index
        return self.ylabel_fptr(index)

    def __getitem__(self,key):
        return self.data.__getitem__(key)
    def __setitem__(self,key,value):
        return self.data.__setitem__(key,value)

    def toFile(self,filename,exif=None):
        MatrixWRX().write(filename,self.sample_rate,self.data,exif)

    #@deprecated
    @staticmethod
    def fromFile(filename):
        rate,data = MatrixWRX().read(filename)
        return Matrix(data,rate)
    # use this instead
    @staticmethod
    def open(filename):
        rate,data = MatrixWRX().read(filename)
        return Matrix(data,rate)

    def __repr__(self):
        return "<%s(%s,%d)>"%(self.__class__.__name__,self.data.shape,self.sample_rate)

    def clone(self):
        mat = Matrix(np.copy(self.data),self.sample_rate,self.kind)
        mat.source_rate = self.source_rate
        mat.ylabel_fptr = self.ylabel_fptr
        return mat



class SpectralMatrix(Matrix):

    def __init__(self,data,sample_rate,source_rate,freqs,Nfft=None,removeDC=False):
        super(SpectralMatrix,self).__init__(data,sample_rate)

        self.source_rate = source_rate
        self.kind = Matrix.K_SPECTROGRAM
        self.freqs = freqs
        self.Nfft = Nfft
        self.removeDC = removeDC  # true when the 0 bin was
                                  # removed from the spectrogram

    def ylabel(self,index):
        return "%d"%self.freqs[index]

    def clone(self):
        mat = super().clone()
        mat.kind = self.kind
        mat.freqs = self.freqs
        mat.Nfft = self.Nfft
        mat.removeDC = self.removeDC
        return mat

class NdArray(object):
    """NdArray is an N dimensional array"""

    def __init__(self, data,sample_rate=0):
        self.data = np.asarray(data)
        self.sample_rate = int(sample_rate)
        self.source_rate = 0

    @property
    def shape(self):
        return self.data.shape
    @property
    def size(self):
        return self.data.size
    def __len__(self):
        return self.data.shape[0]

    @staticmethod
    def fromSignal(signal,data,rate):
        mat = NdArray(data,rate)
        mat.source_rate = signal.sample_rate
        return mat

    @staticmethod
    def fromMatrix(matrix,data,rate):
        mat = Matrix(data,rate)
        mat.source_rate = matrix.source_rate
        return mat

    def __getitem__(self,key):
        return self.data.__getitem__(key)
    def __setitem__(self,key,value):
        return self.data.__setitem__(key,value)

    def toFile(self,filename,exif=None):
        pass

    @staticmethod
    def fromFile(filename):
        pass

    def __repr__(self):
        return "<%s(%s,%d)>"%(self.__class__.__name__,self.data.shape,self.sample_rate)

    def clone(self):
        mat = NdArray(np.copy(self.data),self.sample_rate,self.kind)
        mat.source_rate = self.source_rate
        return mat

class Track(object):
    """docstring for Track"""
    HIGHLIGHT=1 # mark an event at a given time
    TEXT=2      # apply a label at a given time and duration
    def __init__(self, kind, data=None):
        super(Track, self).__init__()
        self.kind = kind
        self.data = data

    def __repr__(self):
        return "<Track(%.2f-%.2f:%d)>"%(self.data.minimum(),
                                     self.data.maximum(),
                                     self.data.count())

    # TODO: deprecated
    def mean(self):
        return self.data.count() / (self.data.maximum() - self.data.minimum())

    def toFile(self,filename,exif=None):
        self.data.save(filename)

class InputSpecifier(ProcessOptionsSpecifier):
    pass

class OutputSpecifier(ProcessOptionsSpecifier):
    pass

class FileInSpecifier(InputSpecifier):
    """docstring for FileInSpecifier"""
    def __init__(self):
        super(FileInSpecifier,self).__init__(input=True,store="fileIn",dtype=str,
                help="file path input."
        )
class FileOutSpecifier(OutputSpecifier):
    """docstring for FileInSpecifier"""
    def __init__(self):
        super(FileOutSpecifier,self).__init__(output=True,store="fileOut",dtype=str,
                help="file path input."
        )
class SignalInSpecifier(InputSpecifier):
    """docstring for SignalInSpecifier"""
    def __init__(self,dtype=Signal,help="single-channel input."):
        super(SignalInSpecifier,self).__init__(input=True,name="Signal_In",store="signal",dtype=dtype,
                help=help
        )
class SignalOutSpecifier(OutputSpecifier):
    """docstring for SignalOutSpecifier"""
    def __init__(self,dtype=Signal,help="single-channel output."):
        super(SignalOutSpecifier,self).__init__(output=True,name="Signal_Out",store="",dtype=dtype,
                help=help
        )
class DataSignalInSpecifier(SignalInSpecifier):
    """docstring for DataSignalInSpecifier"""
    def __init__(self):
        super(DataSignalInSpecifier,self).__init__(dtype=DataSignal)
class DataSignalOutSpecifier(SignalOutSpecifier):
    """docstring for DataSignalOutSpecifier"""
    def __init__(self):
        super(DataSignalOutSpecifier,self).__init__(dtype=DataSignal)
class MatrixInSpecifier(InputSpecifier):
    """docstring for MatrixInSpecifier"""
    def __init__(self):
        super(MatrixInSpecifier,self).__init__(input=True,store="matrix",dtype=Matrix,
                help="2D input."
        )
class MatrixOutSpecifier(OutputSpecifier):
    """docstring for MatrixOutSpecifier"""
    def __init__(self):
        super(MatrixOutSpecifier,self).__init__(output=True,name="Matrix_Out",store="",dtype=Matrix,
                help="2D output."
        )
class SpectralMatrixInSpecifier(InputSpecifier):
    """docstring for SpectralMatrixInSpecifier"""
    def __init__(self):
        super(SpectralMatrixInSpecifier,self).__init__(input=True,store="matrix",
                dtype=SpectralMatrix,
                help="2D input."
        )
class SpectralMatrixOutSpecifier(OutputSpecifier):
    """docstring for SpectralMatrixOutSpecifier"""
    def __init__(self):
        super(SpectralMatrixOutSpecifier,self).__init__(output=True,name="SpectralMatrix_Out",
                store="",
                dtype=SpectralMatrix,
                help="2D output."
        )
class NdArrayInSpecifier(InputSpecifier):
    """docstring for NdArrayInSpecifier"""
    def __init__(self):
        super(NdArrayInSpecifier,self).__init__(input=True,store="array",dtype=NdArray,
                help="ND input."
        )
class NdArrayOutSpecifier(OutputSpecifier):
    """docstring for NdArrayOutSpecifier"""
    def __init__(self):
        super(NdArrayOutSpecifier,self).__init__(output=True,name="Matrix_Out",store="",dtype=NdArray,
                help="ND output."
        )
class TrackInSpecifier(InputSpecifier):
    """docstring for TrackInSpecifier"""
    def __init__(self):
        super(TrackInSpecifier,self).__init__(input=True,name="Track_In",store="track",dtype=Track,
                help="time label input."
        )
class TrackOutSpecifier(OutputSpecifier):
    """docstring for TrackOutSpecifier"""
    def __init__(self):
        super(TrackOutSpecifier,self).__init__(output=True,name="Track_Out",store="",dtype=Track,
                help="time label output."
        )
#from .vocoder import  StretchProcess, PitchShiftProcess
from .spectrogram import SpectrogramProcess
from .amplify import AmplifyProcess
from .chromagram import ChromagramProcess
from .ingest import BassIngestProcess, FFmpegIngestProcess, SoxIngestProcess, SoxHistogramer
from .highlight import DummyTrackProcess
from .spectralflux import SpectralFluxProcess, OnsetDetectorProcess, OnsetDetectorRelativeProcess
from .filterbank import FilterBank, FilterBankProcess, MidiFilterBankProcess
from .emph import PreEmphProcess
from .FeatureNormalize import FMVNProcess
from .FeatureStacker import FeatureStackProcess, SdcFeatureStackProcess
from .SphinxMFCC import MFCCProcess
from .zcr import ZeroCrossRatioProcess
#from .cor import CorrelateMatrixRowProcess,CorrelateProcess

import pkgutil,inspect,imp,os

PROCESS_CLASS_SUFFIX="Process"
__process_list__ = [
    SpectrogramProcess,
    AmplifyProcess,
    ChromagramProcess,
    BassIngestProcess, FFmpegIngestProcess, SoxIngestProcess, SoxHistogramer,
    DummyTrackProcess,
    PreEmphProcess,
    SpectralFluxProcess, OnsetDetectorProcess, OnsetDetectorRelativeProcess,
    FilterBankProcess, MidiFilterBankProcess,
    FMVNProcess,
    FeatureStackProcess, SdcFeatureStackProcess,
    MFCCProcess,
    ZeroCrossRatioProcess,
]

#def samefile(one,two):
#    #os.path.samefile(os.path.join(__path__[0],module_name+".py"),
#    return os.path.realpath(one) == os.path.realpath(two)
#__path__ = __path__ or []
# this magic loads all sub modules and imports only the *Process objects
#for loader, module_name, is_pkg in  pkgutil.walk_packages(__path__):
#    #desc = ('.py', 'U', 1)
#    #f = open(path,desc[1])
#    #mod = imp.load_module(module_name,f,path,desc)
#    rel_name = __name__+'.'+module_name # a guess that happened to work
#
#    # side effect of the dynamic loader is that i need to
#    # white list files that i don't want to reload, I also
#    # can't import these files before the dynamic loader code runs
#    if module_name in ["plugin_manager","riff"]:
#        continue
#
#    mod = loader.find_module(rel_name).load_module(rel_name)
#
#    for name in mod.__dict__:
#        cls = mod.__dict__[name]
#        if name.endswith(PROCESS_CLASS_SUFFIX) and \
#            inspect.isclass(cls) and \
#            samefile(os.path.join(__path__[0],module_name+".py"),
#              inspect.getfile(cls) ):
#                __process_list__.append(cls)
#        del cls
#    del mod
#    del name
#    del rel_name
#del loader
#del module_name
#del is_pkg

from .plugin_manager import loadPlugin, getPluginProcesses
del plugin_manager

def getSigProc(): # return a reference copy
    return list(__process_list__) + list(getPluginProcesses())

def getSigProcDict():

    return { getDisplayName(v.__name__) :v for v in getSigProc() }

import glob as __glob

def newRecipeManager(recipePath,ingest="ffmpeg",binPath=None):
    rm = RecipeManager()
    builtin_processes = getSigProcDict()
    # create a default ingest option
    if ingest == "ffmpeg" or ingest == "avconv":
        builtin_processes["ingest"] = FFmpegIngestProcess
        FFmpegIngestProcess.DEFAULT_BIN_PATH = binPath or FFmpegIngestProcess.DEFAULT_BIN_PATH
        print("[newRecipeManager] ffmpeg " + FFmpegIngestProcess.DEFAULT_BIN_PATH)
    elif ingest == "sox":
        builtin_processes["ingest"] = SoxIngestProcess
        SoxIngestProcess.DEFAULT_BIN_PATH = binPath or SoxIngestProcess.DEFAULT_BIN_PATH
        print("[newRecipeManager] sox " + FFmpegIngestProcess.DEFAULT_BIN_PATH)
    elif ingest == "bass":
        builtin_processes["ingest"] = BassIngestProcess
        print("[newRecipeManager] bass")

    print("procs: %s"%' '.join(list(sorted(builtin_processes.keys()))))
    rm.setProcessDict( builtin_processes )
    for ini in __glob.glob(os.path.join(recipePath,'*.ini')):
        rm.loadRecipe(ini)
    return rm

from .histogrammer import LinearGainLine,AutomaticLinearGainLine, LinearGain, \
    AutomaticGain, AutomaticScale, \
    ApplyGain, HistogramClip, LinearScale, Matrix2Image
from .recipe import RecipeManager

from .main import main as sigproc_main


del pkgutil
del inspect
del imp

from .riff import WaveWRX ,MatrixWRX, ArrayWRX
from .plh  import PLHWRX
from .audiofmt import AudioWRX
del riff
del plh


def openFile(fpath):
    # open au,wav,mat as Signal or Matrix

    if fpath.lower().endswith(".au"):
        Fs,data = AudioWRX().read(fpath)
        return Signal(data,Fs)
    elif fpath.lower().endswith(".wav"):
        Fs,data = WaveWRX().read(fpath)
        return Signal(data,Fs)

    raise Exception("unsupported file")

#for name in dir():
#    if name.endswith("Process"):
#        print(name)
#    else:
#        print(",",name)