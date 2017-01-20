

import numpy as np;
import scipy

#import pylab
from .process import *
from . import *

#import matplotlib.pyplot as plt

class SpectrogramProcess(IterativeProcess):

    def resize_input(self,input,Nfft,step):
        # resize the input, and zero pad so that every step
        # can access window_size samples
        x = np.asarray(input)
        n = len(x)
        tmp = int(max(0,np.ceil((n-Nfft)//step)))
        num_frames = tmp + 1
        xlen = tmp*step + Nfft
        if n < xlen:
            x = np.resize(x, (xlen,))
            x[n:] = 0
        return num_frames,x

    def get_window(self,size):
        return np.hamming(size)

    def begin(self):

        Fs = self.signal.sample_rate
        self.Nfft = 2**self.logN

        self.num_frames,self.x = self.resize_input(self.signal.data,self.Nfft,self.step_size)
        self.windowVals = self.get_window(self.window_size)

        windloss = (np.abs(self.windowVals)**2).sum()
        self.scale_factor = 2
        self.scale_by_freq = 1.0/(Fs*windloss)
        s = 0 if self.removeDC  else 1
        self.Nout = self.Nfft//2 + s # include zero bin
        self.result = np.empty( (self.num_frames,self.Nout))

        return range(self.num_frames)

    def step(self,i):
        idx = i*self.step_size
        #y = self.x[idx:idx+self.Nfft] * self.windowVals
        y = np.zeros(self.Nfft) # zero pad
        y[:self.window_size] = self.x[idx:idx+self.window_size] * self.windowVals
        s2 = 1 if self.removeDC  else 0
        y = np.fft.fft(y,n=self.Nfft,axis=0)[s2:self.Nout+s2]
        # i think this is part of magnitude psd, should be optional
        np.absolute( y, out=y )
        y *= self.scale_by_freq
        y[1:-1] *= self.scale_factor # TODO check range
        self.result[i] = y.real

    def end(self):

        # db scale
        clip = 10**self.clip_exp
        np.clip(self.result,clip,np.inf,out=self.result)

        if self.logSpectrum:
            np.log10(self.result,out=self.result)
            self.result *= 20

        #np.clip(result,clip,np.inf,out=result)
        # was np.log
        freqs = np.fft.fftfreq(self.Nfft, 1/self.signal.sample_rate)
        freqs = freqs[:1+self.Nfft//2]
        if self.removeDC:
            freqs = freqs[1:]
        freqs[-1] *= -1 # sign correct last freq bin

        outrate = self.signal.sample_rate//self.step_size
        return SpectralMatrix(self.result,outrate,self.signal.sample_rate,freqs,self.Nfft,self.removeDC)

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SignalInSpecifier(),
                SpectralMatrixOutSpecifier(),
                SummarySpecifier("Only the positive frequency output is kept."),
                ProcessOptionsSpecifier(store="logN",dtype=int, \
                    min=6,max=15,default=9, \
                    help="compute FFT with frame size 2**logN"),
                DynamicOptionsSpecifier(store="step_size",dtype=int,
                    triggers = "logN", suffix = "samples",
                    # HTK defaults 100 FPS for 16kHz which would actually be 160
                    default = 160,
                    update_default = lambda x :  (x['signal'].sample_rate if 'signal' in x else 16000)/100,
                    min = 1,
                    help="amount of overlap"),
                DynamicOptionsSpecifier(store="window_size",dtype=int,
                    triggers = "logN", suffix = "samples",
                    # HTK defaults to 448
                    default = 512,
                    update_max = lambda x :  2**x['logN'], # order matters
                    update_default = lambda x :  2**x['logN'],
                    min = 1,
                    help="window size"),
                ProcessOptionsSpecifier(store="clip_exp",dtype=int, \
                    min=-32,max=-1, \
                    default=-16, \
                    prefix="10 ^", \
                    help="clip output of fft to range [10^x , inf)"),
                ProcessOptionsSpecifier(store="removeDC",dtype=bool, \
                    default=False, \
                    help="remove the 0 bin from spectral output"),
                # todo: make this tristate : normal : log : power
                ProcessOptionsSpecifier(store="logSpectrum",dtype=bool, \
                    default=True, \
                    help="compute Power Spectrum from FFT output. ( 20*log10(x) verse x )"),
                DynamicLabelSpecifier( \
                    lambda x : "Number of Features: %d"%((2**(x['logN'])//2)+(0 if x['removeDC'] else 1)) ) ,
                DynamicLabelSpecifier( \
                    lambda x : "Percent Overlap: %.3f"%(
                        100*(max(0,2**x['logN']-x['step_size']))/2**x['logN'] ) ) ,
                DynamicLabelSpecifier( \
                    lambda x : "Frames per second: %f"% \
                        ( x['signal'].sample_rate/x['step_size'] )),
                DynamicLabelSpecifier( \
                    lambda x : "Display is compressed to a height of 128" if x['logN']>8 else "")
                ]
        return opts

class PhaseSpectrogramProcess(SpectrogramProcess):

    def step(self,i):
        #print( i, i + self.Nfft, len(self.window) )
        #x = scipy.fftpack.fft( self.window * self.signal.data[i : i+self.Nfft] )
        x = self.fft(i)
        x = np.angle(x[:self.Nfft//2]) # TODO unwrap
        idx = i//self.step_size
        self.Pxx[idx,:] = x.astype(np.float32)
        return True;
