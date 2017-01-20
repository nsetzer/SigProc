#! python34 $this
import os,sys
import struct
import numpy as np
def unpack(rf,s,f):
    if 's' in f:
        return rf.read(s)
    b=rf.read(s)
    if b:
        return struct.unpack(f,b)[0]
    return 0

# http://www.ee.columbia.edu/ln/LabROSA/doc/HTKBook21/node58.html#SECTION03271000000000000000

"""
/fs/raidmaster/old-raid-util/grid/bin/wav2plhNN \
    --paramMode MFCC \
    --addDA \
    --noRndF \
    --nparam 13 \
    --frame 160 \
    --window 448 \
    --meanNorm NONE \
    --varNorm NONE \
    --in cnn.wav \
    --out cnn.plh

/fs/raidmaster/old-raid-util/grid/bin/wav2plhNN \
    --paramMode MFCC \
    --addDA \
    --noRndF \
    --nparam 13 \
    --frame 160 \
    --window 448 \
    --in cnn.wav \
    --out cnn.norm.plh
"""
K_WAVEFORM=0
K_MFCC=6
K_FBANK=7
K_MELSPEC=8
K_USER=9

Q_ENERGY     =0o100    # _E
Q_ESUPPRESSED=0o200    # _N
Q_DELTA      =0o0400   # _D
Q_ACCEL      =0o1000   # _A
Q_COMPRESSED =0o2000   # _C
Q_ZEROMEAN   =0o4000   # _Z
Q_CHKSUM     =0o10000  # _K
Q_ZEROCOEF   =0o20000  # _O

class PLHWRX(object):
    """docstring for PLHWRX"""
    def __init__(self):
        super(PLHWRX, self).__init__()

    def read_header(self,rf):
        nSamples   = unpack(rf,4,">i")
        sampPeriod = unpack(rf,4,">i")
        sampSize   = unpack(rf,2,">H")
        parmKind   = unpack(rf,2,"<h")

        # nSamples*sampSize # size of file, in bytes, excluding header
        #print(hex(parmKind))

        self.nSamples = nSamples
        self.sampSize = sampSize
        self.sampleRate = int(1/((sampPeriod+1)/1e7))
        self.parmKind = parmKind

    def read_data(self,rf):
        nDim = self.sampSize//4
        dat = np.fromfile(rf,dtype=">f",count=self.nSamples*nDim)
        return dat.reshape( (-1,nDim) )


    @staticmethod
    def read(path):

        inst=PLHWRX()
        with open(path,"rb") as rf:
            inst.read_header(rf)
            return inst.sampleRate,inst.read_data(rf), inst.parmKind

def main():
    rate,data =PLHWRX.read("cnn.plh")
    print(rate,data.shape)
    print(data[0])

if __name__ == '__main__':
    main()


