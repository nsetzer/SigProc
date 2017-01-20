

import numpy as np;
from scipy import signal
#import pylab
from .process import *
from . import *

class Cheby1FilterProcess(SimpleProcess):

    def run(self):
        #http://docs.scipy.org/doc/scipy-0.15.1/reference/generated/scipy.signal.cheby1.html

        Wn = self.cutoff/(self.signal.sample_rate/2)
        print("cheby1 using Wn = %f"%Wn)
        b,a = signal.cheby1(self.N, self.rp, Wn, self.btype,
                            analog=False, output="ba")

        # w, h = signal.freqs(b, a)
        # plot(w, 20*np.log10(abs(h)))

        # direct-form II
        out = signal.lfilter(b,a,self.signal.data)
        return Signal(out,self.signal.sample_rate)

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SignalInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="N",dtype=int, \
                    min=1,default=4, \
                    help="Order of the filter"),
                ProcessOptionsSpecifier(store="rp",name="Ripple", dtype=float, \
                    min=0,default=3,precision=4,
                    suffix="dB", \
                    help="The maximum ripple allowed below unity gain in the passband"),
                ProcessOptionsSpecifier(store="rp",name="Ripple", dtype=float, \
                    default=3,precision=4,
                    suffix="dB", \
                    help="The maximum ripple allowed below unity gain in the passband"),
                ProcessOptionsSpecifier(store="cutoff",dtype=float, \
                    help="cutoff frequency"),
                #ProcessOptionsSpecifier(store="Wn",dtype=float, \
                #    min=0,max=1.0,default=0.5,precision=4, \
                #    help="Point at which transition band drops below -ripple. " \
                #         "Where 1.0 is the Nyquist Frequency."),
                ProcessOptionsSpecifier(store="btype",dtype=str, \
                    # default is first item in list
                    options=["lowpass", "highpass",], \
                    default="lowpass", \
                    help="Type of the filter"),
                ]
        return opts

class Cheby1BandFilterProcess(SimpleProcess):

    def run(self):
        #http://docs.scipy.org/doc/scipy-0.15.1/reference/generated/scipy.signal.cheby1.html

        b,a = signal.cheby1(self.N, self.rp, (self.Wn1,self.wn2), self.btype,
                            analog=False, output="ba")

        # w, h = signal.freqs(b, a)
        # plot(w, 20*np.log10(abs(h)))

        # direct-form II
        out = signal.lfilter(b,a,self.signal.data)
        return Signal(out,self.signal.sample_rate)

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                FileInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="N",dtype=int, \
                    min=1,default=4, \
                    help="filter order"),
                ProcessOptionsSpecifier(store="rp",dtype=float, \
                    min=0,max=100.0,default=0.5,precision=4,
                    suffix="dB", \
                    help="The maximum ripple allowed below unity gain in the passband"),
                ProcessOptionsSpecifier(store="Wn1",dtype=float, \
                    min=0,max=1.0,default=0.5,precision=4, \
                    help="resample input to Sample Rate"),
                ProcessOptionsSpecifier(store="Wn2",dtype=float, \
                    min=0,max=1.0,default=0.5,precision=4, \
                    help="resample input to Sample Rate"),
                # note, bandpass, bandstop requires two options wn_low, wn_high
                # so i need two new typse to support this
                # str : from list, using options
                # and float_range, with range optionally being 1-2 depending
                # on a trigger value from the btype.
                ProcessOptionsSpecifier(store="btype",dtype=str, \
                    # default is first item in list
                    options=["bandpass", "bandstop"], \
                    help="filter type"),
                ]
        return opts