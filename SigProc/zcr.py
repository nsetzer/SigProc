from .process import *
from . import *

import numpy as np

# http://marsyasweb.appspot.com/download/data_sets/
# https://en.wikipedia.org/wiki/Au_file_format

def zcrw(dat,Fs,step_size,window_size):

    """
    output: array containing zero-crossings-per-second
            output
    """
    dence = np.diff(np.sign(dat))/2.0
    out = []
    print(window_size/Fs)
    for idx in range(0,len(dence),step_size):
        dat = dence[idx:idx+window_size]
        out.append( len(np.where(dat)[0]) / (window_size/Fs) )
    Fs_out = Fs/step_size
    return np.array(out,dtype=np.float32), Fs_out;

class ZeroCrossRatioProcess(SimpleProcess):
    def run(self):

        sig,Fs_out = zcrw(self.signal.data,self.signal.sample_rate,
                          self.step_size,self.window_size)
        return Signal(sig,Fs_out)

    @staticmethod
    def getOptions():
        opts = [
                SummarySpecifier("Perform Mean and variance normalization on " \
                    "each feature vector dimension. Also known as CMVN."),
                SignalInSpecifier(), \
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="step_size",dtype=int,
                    triggers = "logN", suffix = "samples",
                    default = 160,
                    min = 1,
                    help="amount of overlap"),
                ProcessOptionsSpecifier(store="window_size",dtype=int,
                    triggers = "logN", suffix = "samples",
                    default = 4000,
                    min = 1,
                    help="number of samples used in calculation"),
                DynamicLabelSpecifier( \
                    lambda x : "Frames per second: %f"% \
                        ( x['signal'].sample_rate/x['step_size'] )),

               ]
        return opts