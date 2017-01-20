

import numpy as np;
#import pylab
from .process import *
from . import *

class AmplifyProcess(SimpleProcess):

    def run(self):

        out = np.asarray( self.signal.data ) *  ( np.power(10.0,(self.dbgain/20)) )

        return Signal(out,self.signal.sample_rate)

    def getProgress(self):
        return (50,"")

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SignalInSpecifier(),
                SignalOutSpecifier(),
                SummarySpecifier("change volume of track"),
                ProcessOptionsSpecifier(store="dbgain",name="Gain",dtype=float, \
                    min=-100,max=100,default=0,suffix="db", \
                    help="db gain"),
                DynamicLabelSpecifier( \
                    lambda x : "relative gain: %.3f%%"% \
                        ( 10**(x['dbgain']/20.0) * 100 ) )
                ]
        return opts
