#! python $this
from .process import *
from . import *

import numpy as np

class _FeatNorm(IterativeProcess):
    # implementation detail for performaing normalization on
    # a feature dimension
    def begin(self):
        self.out = np.empty( self.matrix.data.shape )
        return len(self.matrix.data[0])
    def end(self):
        return Matrix(self.out,self.matrix.sample_rate)

class FMNProcess(_FeatNorm):
    def step(self,idx):
        self.out[:,idx] = self.matrix.data[:,idx] - np.mean(self.matrix.data[:,idx])
        return True
    @staticmethod
    def getOptions():
        opts = [
                SummarySpecifier("Perform Mean normalization on " \
                    "each feature vector dimension. Also known as CMN."),
                MatrixInSpecifier(), \
                MatrixOutSpecifier(),
               ]
        return opts

class FVNProcess(_FeatNorm):
    def step(self,idx):
        self.out[:,idx] = self.matrix.data[:,idx] / np.std( self.out[:,idx] )
        return True
    @staticmethod
    def getOptions():
        opts = [
                SummarySpecifier("Perform variance normalization on " \
                    "each feature vector dimension."),
                MatrixInSpecifier(), \
                MatrixOutSpecifier(),
               ]
        return opts

class old_FMVNProcess(_FeatNorm):
    def step(self,idx):
        self.out[:,idx] = self.matrix.data[:,idx] - np.mean(self.matrix.data[:,idx])
        self.out[:,idx] /= np.std( self.out[:,idx] )
        return True
    @staticmethod
    def getOptions():
        opts = [
                SummarySpecifier("Perform Mean and variance normalization on " \
                    "each feature vector dimension. Also known as CMVN."),
                MatrixInSpecifier(), \
                MatrixOutSpecifier(),
               ]
        return opts

class FMVNProcess(SimpleProcess):
    def run(self):
        mat = self.matrix.clone()
        m = np.mean(mat.data)
        s = np.std(mat.data)
        print("filt mean-variance norm: ",m,s)
        mat.data = (self.matrix.data - m) / s
        return mat

    @staticmethod
    def getOptions():
        opts = [
                SummarySpecifier("Perform Mean and variance normalization on " \
                    "each feature vector dimension. Also known as CMVN."),
                MatrixInSpecifier(), \
                MatrixOutSpecifier(),
               ]
        return opts

if __name__ == '__main__':
    x = np.asarray(range(12)).reshape(6,2)

    print(x)
    print("mean",np.mean(x.ravel()))
    print("var ",np.var(x[:,0]))
    print("std ",np.std(x[:,0]))
    inmat = Matrix(x,1)
    proc=FMVNProcess({"matrix":inmat})
    #proc.begin() # for reference, these 2 lines only
    #proc.step(0) # normalize the first dimension.
    for i in range(proc.begin()):
        proc.step(i)
    out = proc.end()
    print(out.data)
    print("mean",np.mean(out.data.ravel()))
    print("var ",np.var(out.data[:,0]))
    print("std ",np.std(out.data[:,0]))
    #print formatOptions(FMVNProcess)
