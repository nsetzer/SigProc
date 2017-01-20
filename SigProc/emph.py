

from . import *
from .process import *

import numpy as np;

class PreEmphProcess(IterativeProcess):
    # improves signal to noise ratio
    # by applying a simple low pass filter
    #
    # http://mirlab.org/jang/books/audiosignalprocessing/speechFeatureMfcc.asp?title=12-2%20MFCC
    # The goal of pre-emphasis is to compensate the high-frequency part
    # that was suppressed during the sound production mechanism of humans.
    # Moreover, it can also amplify the importance of high-frequency formants
    # TODO: research 'outer ear filter', Terhard's formula
    # claims have been made that pre-emph is a crude
    # approximation of this concept.

    # this is broken into an iterative process purely to demonstrate
    # the concept of an iterative process, and how that relates to
    # UI updates.
    def begin(self):
        self.num_steps = 200

        # todo: make normalization a default-true boolean option
        self.input_data = self.signal.data/self.signal.data.max()

        self.step_size = 1 + len(self.input_data)//self.num_steps
        self.last = 0;
        self.out = np.empty( len(self.input_data) )
        self.view = self.input_data.view()
        self.cm = [1,self.emph]
        return self.num_steps

    def step(self,idx):
        # http://www.dsprelated.com/freebooks/mdft/Matrix_Multiplication.html
        # multiplying two matrices :
        # n*m * m*p = n*p
        # 1*2 * 2*p = 1*p
        #[a b] [xi   xi+1 ...] = [(axi + bxi-1) (axi+1 + bxi)]
        #      [xi-1 xi   ...]
        i = idx*self.step_size
        j = min(i + self.step_size,len(self.input_data))

        # the following code replaces this for loop, which is very slow.
        #for  i in range(i,j):
        #    self.out[i] = v - self.emph*self.last
        #    self.last = v

        # benefit of a large num_steps is the input matrix is small,
        # but more research is needed to see where the time/space tradeoff is.
        x=np.vstack((self.view[i:j],
                     np.hstack(([self.last,],self.view[i:j-1]))
                     ))
        self.out[i:j] = np.dot(self.cm,x)
        self.last = self.view[j-1]

    def end(self):
        return Signal(self.out,self.signal.sample_rate)

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SignalInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="emph",dtype=float, \
                    min=0.0,max=1.0,default=.97, \
                    help="Pre Emphasis"),
                ]
        return opts