
#from PyDSP import *
from . import *

import numpy as np
import scipy.signal

class FeatureStackProcess(IterativeProcess):

    def begin(self):

        #self.frame_zero = np.zeros( self.matrix.data[0,:].shape )

        #self.out = np.empty( self.matrix.data.shape )
        num_frames = len(self.matrix)
        num_dims = self.num_params
        if self.compute_deltadelta:
            num_dims += self.num_params
            self.compute_delta = True
        if self.compute_delta:
            num_dims += self.num_params

        self.out = np.zeros( (num_frames,num_dims) )

        self.select_lo = self.skip_bin
        self.select_hi = self.select_lo + self.num_params

        return len(self.matrix) + self.delta*2

    def compute_delta_frames(self,idx,a,b,c):
        # in a frame, a:b selects the features
        # and b:c is the location to write the features
        lidx=idx-self.delta
        ridx=idx+self.delta
        if lidx <     0:
            lidx=0
        if ridx >= len(self.matrix):
            ridx=len(self.matrix)-1
        if self.compute_delta:
            lframe = self.out[lidx,a:b]
            rframe = self.out[ridx,a:b]
            self.out[idx,b:c] = rframe-lframe

    def step(self,idx):
        # select base frame
        if idx < len(self.matrix):
            self.out[idx,0:self.num_params] = \
                self.matrix.data[idx,self.select_lo:self.select_hi]

        a=  self.num_params
        b=2*self.num_params
        c=3*self.num_params

        # deltas/deltadeltas are computed on a delay.

        # compute delta
        cidx = idx - self.delta
        if 0 <= cidx < len(self.matrix):
            self.compute_delta_frames(cidx,0,a,b)

        # compute delta delta
        cidx = cidx - self.delta
        if 0 <= cidx < len(self.matrix):
            self.compute_delta_frames(cidx,a,b,c)

    def end(self):
        return Matrix(self.out,self.matrix.sample_rate)

    @staticmethod
    def getOptions():
        opts = [
                MatrixInSpecifier(),
                MatrixOutSpecifier(),
                ProcessOptionsSpecifier(store="num_params",dtype=int, \
                    min=1, \
                    help="number of bins to keep"),
                ProcessOptionsSpecifier(store="skip_bin",dtype=int, \
                    default=0,
                    help="set to 1 to skip the zero bin."),
                ProcessOptionsSpecifier(store="compute_delta",dtype=bool, \
                    default=True, \
                    help="fixme"),
                ProcessOptionsSpecifier(store="compute_deltadelta",dtype=bool, \
                    default=True, \
                    help="implies compute_delta"),
                ProcessOptionsSpecifier(store="delta",dtype=int, \
                    min=1,max=7,default=3, \
                    help="fixme"),
               ]
        return opts

class SdcFeatureStackProcess(IterativeProcess):
    """
    Standard SDC Feature vectors are (N-d-P-k) 7-1-3-7
        N: number of cepstral coefficients
        d: delta
        k: time shift between consecutive delta computations
        P: number of blocks whose delta coeffs are concatenated

    References:
    1) "Effective Arabic Dialect Classification Using Diverse Phonotactic Models"
    Murat Akbacak, Dimitra Vergyri, Andreas Stolcke,
    Nicolas Scheffer, Arindam Mandal

    2) "Approaches to Language Identification using Gaussian Mixture Models and
      Shifted Delta Cepstral Features"
    Pedro A. Torres-Carrasquillo, Elliot Singer
    Mary A. Kohler, Richard J. Greene, Douglas A. Reynolds, and J.R. Deller, Jr.

    """
    def compute_delta_frames(self,idx):
        # in a frame, a:b selects the features
        # and b:c is the location to write the features
        lidx=idx-self.n_delta
        ridx=idx+self.n_delta

        # todo: what kind of padding to use?
        # this should be configurable
        if lidx<0:
            lidx=0
        if ridx>=len(self.matrix):
            ridx=len(self.matrix)-1

        lframe = self.matrix.data[lidx,self.select_lo:self.select_hi]
        rframe = self.matrix.data[ridx,self.select_lo:self.select_hi]

        self.deltas[idx,0:self.n_params] = rframe-lframe

    def begin(self):

        num_frames = len(self.matrix)
        num_dims = self.n_params * self.n_concat

        # final output is the same size as the input
        self.out = np.zeros( (num_frames,num_dims) )
        # cache delta computation into this array
        self.deltas = np.empty( self.matrix.shape )

        self.select_lo = self.skip_bin
        self.select_hi = self.select_lo + self.n_params

        return  len(self.matrix) + self.n_shift*self.n_concat

    def step(self,idx):

        if idx < len(self.matrix):
            self.compute_delta_frames(idx)

        oidx = idx - self.n_shift * self.n_concat
        if oidx < 0: # deltas have not been computed for this sample yet
            return;

        for i in range(self.n_concat):
            didx = oidx + i*self.n_shift;
            if didx < len(self.matrix):
                a = i * self.n_params;
                b = (i+1) * self.n_params;
                #print(idx,oidx,didx,self.deltas[didx,0:self.n_params])
                self.out[oidx,a:b] = self.deltas[didx,0:self.n_params]

    def end(self):
        if np.any(np.isinf(self.out)):
            raise Exception("inf")
        if np.any(np.isnan(self.out)):
            raise Exception("nan")
        if not np.any(self.out):
            raise Exception("zero")
        return Matrix(self.out,self.matrix.sample_rate)

    @staticmethod
    def getOptions():
        opts = [
                MatrixInSpecifier(),
                MatrixOutSpecifier(),
                SummarySpecifier("Stack Filterbank Features using the Shifted Delta Cepstrum (SDC) N-d-k-P method"),
                ProcessOptionsSpecifier(store="skip_bin",dtype=int, \
                    default=0,
                    help="set to 1 to skip the zero bin."),
                ProcessOptionsSpecifier(store="n_params",dtype=int, \
                    min=1,default=7, \
                    help="SDC N: number of bins to keep"),
                ProcessOptionsSpecifier(store="n_delta",dtype=int, \
                    min=1,default=1, \
                    help="SDC d: +/- delta computation"),
                ProcessOptionsSpecifier(store="n_shift",dtype=int, \
                    min=1,default=3, \
                    help="SDC k: shift between consecutive blocks"),
                ProcessOptionsSpecifier(store="n_concat",dtype=int, \
                    min=1,default=7, \
                    help="SDC P: number of blocks to concat"),
               ]
        return opts