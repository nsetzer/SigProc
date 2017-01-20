import numpy as np
import scipy.signal as signal2
import math
import wave
try:
    import pylab
except ImportError:
    pass
import operator

from .process import *
from . import *


class ChromagramProcess(SimpleProcess):
    """docstring for Chroma2Process"""

    def run(self):

        #signal = self.signal.data
        #fs = self.signal.sample_rate

        #Magnitude, freqs = specgram(signal,fs,self.step_size,self.logN,self.clip_exp)
        #Magnitude, freqs = specgram2(signal,fs,self.logN,self.step_size,clip=10**self.clip_exp)
        # transpose and skip zero bin
        Magnitude = self.matrix.data
        freqs     = self.matrix.freqs
        Magnitude = np.asarray(Magnitude.T)[1:,] # remove zero bin
        freqs = freqs[1:]
        CS = chromagram(Magnitude,freqs,self.nbin)

        x = np.asarray(-CS.T)

        mat = Matrix(x,self.matrix.sample_rate,Matrix.K_SPECTROGRAM)
        # error is in there for debugging the painting code
        chromatic_scale=[
            'A', 'A#/Bb', 'B',
            'C', 'C#/Db', 'D',
            'D#/Eb', 'E', 'F',
            'F#/Gb', 'G', 'G#/Ab']
        if self.nbin == 12:
            mat.ylabel_fptr = lambda idx : chromatic_scale[idx];
        if self.nbin == 120:
            mat.ylabel_fptr = lambda idx : chromatic_scale[idx//10] if idx<120 else chromatic_scale[-1];

        return mat

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SpectralMatrixInSpecifier(),
                MatrixOutSpecifier(),
                ProcessOptionsSpecifier(store="nbin",dtype=int, \
                    min=6,max=120,default=12, \
                    help="number of chroma bins"),
                ProcessOptionsSpecifier(store="A_tune",dtype=int, \
                    min=0,default=880, \
                    help="Tuning Frequency A5"),
                ]
        return opts


def specgram(signal,fs,step,logN,clip_exp):
    signal = signal/signal.max(); # normalize
    Nfft=2**logN #FFT Size
    overlap=Nfft-step
    window = np.hamming(Nfft)
    Pxx, freqs, _, _  = pylab.specgram(signal,
                                       Fs=fs, Fc=0, scale='linear',
                                       window=window, NFFT=Nfft,
                                       noverlap=overlap)
    clip=10**clip_exp
    np.clip(Pxx,clip,np.inf,out=Pxx)
    Magnitude= 20*np.log10(abs(Pxx[1:,])) # was np.log
    freqs = freqs[1:,] # skip zero bin

    return Magnitude, freqs


def specgram2(signal, Fs, logN, step, window=np.hamming, clip=10**-16):
    """
    this is a functional equivalent to mlab.specgram
    except matrix operations are converted to in-place row operations
    on memory constrained systems this prevents copying of massive matrices.

    default output is the magnitude periodogram.
    """
    Nfft = 2**logN
    noverlap = Nfft - step

    x = np.asarray(signal)
    n = len(x)
    tmp = int(max(0,np.ceil((n-Nfft)//step)))
    num_frames = tmp + 1
    xlen = tmp*step + Nfft

    if n < xlen:
        x = np.resize(x, (xlen,))
        x[n:] = 0

    wind = window(Nfft)
    windloss = (np.abs(wind)**2).sum()
    scale_factor = 2
    scale_by_freq = 1.0/(Fs*windloss)
    Nout = Nfft//2 + 1 # include zero bin
    result = np.empty( (n//step+1,Nout))

    for i in range(num_frames):
        idx = i*step
        y = x[idx:idx+Nfft] * wind
        y = np.fft.fft(y,n=Nfft,axis=0)[:Nout]
        # i think this is part of magnitude psd, should be optional
        np.absolute( y, out=y )
        y *= scale_by_freq
        y[1:-1] *= scale_factor # TODO check range
        result[i] = y.real

    # db scale
    np.clip(result,clip,np.inf,out=result)
    np.log10(result,out=result)
    result *= 20

    #np.clip(result,clip,np.inf,out=result)
    # was np.log
    freqs = np.fft.fftfreq(Nfft, 1/Fs)[:Nout]
    freqs[-1] *= -1 # sign correct last freq bin

    return result, freqs


def chromagram(Magnitude, freqs,nbin=12,A5=880):
    #Chroma centered on A5 = 880Hz
    #Number of chroma bins
    st=2**(1/float(nbin)) #Semitone

    tunechroma1=[np.log2(A5*st**i) for i in range(nbin)]
    tunechroma2=[int(v) for v in tunechroma1]
    #tunechroma2=[int(np.log2(A5*st**i)) for i in range(nbin)]
    chroma=np.asarray(tunechroma1)-np.asarray(tunechroma2);
    nchroma=len(chroma)

    freqschroma=np.asarray(np.log2(freqs)) - np.asarray([int(np.log2(f)) for f in freqs])
    nfreqschroma=len(freqschroma)

    CD=np.zeros((nfreqschroma, nchroma))
    for i in range(nchroma):
        CD[:,i] = np.abs(freqschroma - chroma[i])
    FlipMatrix=np.flipud(CD)
    min_index = []
    min_value = []
    for i in reversed(range(FlipMatrix.shape[0])):
        index, value = min(enumerate(FlipMatrix[i]), key=operator.itemgetter(1))
        min_index.append(index)
        min_value.append(value)
    #Numpy Array for Chroma Scale population
    CS = np.zeros((nchroma,Magnitude.shape[1]))

    for i in range(CS.shape[0]):
        #Find index value in min_index list
        a = [index for index,x in enumerate(min_index) if x == i]
        #Array for values in each index
        AIndex = np.zeros((len(a),Magnitude.shape[1]))
        for t,value in enumerate(a):
            AIndex[t,:] = Magnitude[value,:]
        MeanMag=[]
        for M in AIndex.T:
            MeanMag.append(np.mean(M))
        CS[i,:] = MeanMag

    #normalize the chromagram array
    CS= CS / CS.max()

    return CS