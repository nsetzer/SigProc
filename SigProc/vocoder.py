#! python3 $this
"""
Phase Vocoder in Python
"""
import sys
import numpy as np

from .process import IterativeProcess,ProcessOptionsSpecifier,SummarySpecifier
from . import *

# http://zulko.github.io/blog/2014/03/29/soundstretching-and-pitch-shifting-in-python/
# http://audioprograming.wordpress.com/2012/03/02/a-phase-vocoder-in-python/

def speedx(sound_array, factor):
    """ Multiplies the sound's speed by some `factor` """
    #if factor > 1:
    #    from scipy.signal import cheby1,butter,filtfilt,lfilter
    #    N = 2 # filter order
    #    rp = 3  # db Ripple
    #    Wn = 1/factor # critical frequency (0=0, 1=Nyquist)
    #    print("cutoff",Wn)
    #    bType="lowpass"
    #    b, a = butter(N, .9*Wn, 'low')
    #    print(b)
    #    print(a)
    #    sound_array = lfilter(b,a,sound_array)
    #    print(max(sound_array))

    indices = np.round( np.arange(0, len(sound_array), factor) )
    indices = indices[indices < len(sound_array)].astype(int)
    #TODO LPF
    # SEE: F:\Player\chubby.py for a better resampler
    return sound_array[ indices ]

class StretchProcess(IterativeProcess):
    """Phase Vocoder performs scaling
        in the time and frequency domain.
    """

    def begin(self):

        self.siglen = len(self.signal.data)
        self.factor =  2**(self.semitone_inc / 12.0)
        self.ifactor = 1.0/self.factor

        self.phase  = np.zeros(self.window_size)
        self.hanning_window = np.hanning(self.window_size)

        self.result = np.zeros( self.siglen / self.ifactor + self.window_size)

        e = self.siglen - (self.window_size+self.hop_size)
        s = self.hop_size*self.ifactor
        return np.arange(0, e, s)

    def step(self,idx):
        """
        process a single chunk
        """

        # two potentially overlapping subarrays
        a1 = self.signal.data[idx: idx + self.window_size]
        a2 = self.signal.data[idx + self.hop_size: idx + self.window_size + self.hop_size]

        # resynchronize the second array on the first
        s1 =  np.fft.fft(self.hanning_window * a1)
        s2 =  np.fft.fft(self.hanning_window * a2)
        self.phase = (self.phase + np.angle(s2/s1)) % 2*np.pi
        a2_rephased = np.fft.ifft(np.abs(s2)*np.exp(1j*self.phase))

        # add to result
        i2 = int(idx/self.ifactor)
        a2_windowed = np.real(self.hanning_window*a2_rephased)
        self.result[i2 : i2 + self.window_size] += a2_windowed


        return True

    def end(self):
        """

        """
        amp = np.abs(self.signal.data).max()
        result = amp * (self.result/np.abs(self.result).max())
        #result = result.astype('int16')
        print("vox",amp,min(result),max(result))
        return Signal(result,self.signal.sample_rate)

    @staticmethod
    def _getOptions():
        """
        returns a ist of ProcessOptionsSpecifier
        """
        opts = [ \

                SignalInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="semitone_inc",dtype=float, \
                    min=-25,max=+25, \
                    help="change the pitch by +/- N semitones."), \
                ProcessOptionsSpecifier(store="window_size",default=2**13, \
                    help="fft window size"), \
                ProcessOptionsSpecifier(store="hop_size",default=2**11, \
                    help="hop_size determines ammount of overlap"), \
               ]
        return opts
    @staticmethod
    def getOptions():
        opts = StretchProcess._getOptions()
        opts.insert(0,SummarySpecifier(
                "Stretch the duration of the input sequence using" \
                " overlapping windows"))
        return opts

class PitchShiftProcess(StretchProcess):
    def end(self):
        result = super(PitchShiftProcess,self).end()
        # resample the stream so that the total number
        # of samples is the same as the number of input samples.
        result.data = speedx(result.data[self.window_size:], self.factor)
        #print(self.sample_rate*self.factor)
        return result
    @staticmethod
    def getOptions():
        opts = StretchProcess._getOptions()
        opts.insert(0,SummarySpecifier("Shift the input signal up or down in the frequency domain."))
        return opts
