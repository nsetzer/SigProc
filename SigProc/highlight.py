

import numpy as np;
#import pylab
from .process import *
from . import *

class DummyTrackProcess(SimpleProcess):

    def run(self):
        t = len(self.signal.data)//self.signal.sample_rate
        ti = DenseTimeInfo([ (i,True) for i in range(1,t) ])
        tr = Track(Track.HIGHLIGHT,ti)
        return tr

    def getProgress(self):
        return (50,"")

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SignalInSpecifier(),
                TrackOutSpecifier(),
                ]
        return opts