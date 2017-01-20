#! python $this

import struct
import numpy as np
def unpack(rf,f):
    if f=="4s":
        return rf.read(4)
    elif f==">i":
        b=rf.read(4)
        if b:
            print(b)
            return struct.unpack(f,b)[0]
        return 0
    b=rf.read(2)
    if b:
        return struct.unpack(f,b)[0]
    return 0


fpath = "./blues/blues.00000.au"

def mulaw(v):
    pass

other_encodings = {
    1  : "mulaw",
    4  : "int24",
    12 : "uint24",
}

numpy_encodings = {
    # 1
    2 : '>i1',
    3 : '>i2',
    # 4
    5 : '>i4',
    6 : '>f4',
    7 : '>f8',
    # 8,9 not implemented
    10 : '>u1',
    11 : '>u2',
    # 12
    13 : '>u4',
    # 18,19,20,21,23,24,25,26,27 not implemented
}

byte_sizes = {
    1 : 1,
    2 : 1,
    3 : 2,
    4 : 3,
    5 : 4,
    6 : 4,
    7 : 8,
    10 : 8,
    11 : 2,
    12 : 3,
    13 : 4
}

imax_sizes = {
    3 : 0x07FF,
    4 : 0x007FFFFF,
}

# these have never been tested for signed integers
def readBigEndian(rb, bytesize,signed = False):
    s = rb.read(bytesize)
    o = 2 ** (8*bytesize-1) if signed else 0 # TODO this might be wrong
    while len(s)==bytesize:
        i = 0;
        for byte in s:
            i = (i << 8) | byte
        yield(i-o)
        s = rb.read(bytesize)

def readLittleEndian(rb, bytesize,signed = False):
    s = rb.read(bytesize)
    o = 2 ** (8*bytesize-1) if signed else 0
    while len(s)==bytesize:
        i = 0;
        shift = 0
        for byte in s:
            i |= byte << shift
            shift += 8
        yield(i-o)
        s = rb.read(bytesize)

class AudioWRX(object):

    def read_np_enc(self,rb,encoding):
        dtype = numpy_encodings[encoding]
        data = np.asarray([],dtype=dtype)
        block_size = 10240 * byte_sizes[encoding]

        buf = rb.read(block_size)
        while buf:
            x = np.fromstring(buf,dtype=dtype)
            data = np.append(data,x)
            buf = rb.read(block_size)
        return data

    def _read(self,fpath):
        with open(fpath,"rb") as rb:

            magic     = unpack(rb,"4s")
            offset    = unpack(rb,">i")
            size      = unpack(rb,">i")
            encoding  = unpack(rb,">i")
            samplerate= unpack(rb,">i")
            channels  = unpack(rb,">i")

            print(magic     )
            print(offset    )
            print(size      )
            print(encoding  )
            print(samplerate)
            print(channels  )

            if encoding in other_encodings:

                if encoding == 4:
                    data = np.array(list(readLittleEndian(rb,3,True)))
                    return (samplerate,data)

                raise NotImplementedError("unsupported encoding: %d"%encoding)

            elif encoding in numpy_encodings:
                data = self.read_np_enc(rb,encoding)
                return (samplerate,data)

            else:
                raise Exception("unsupported encoding: %d"%encoding)
        return (samplerate,[])

    def read(self,fpath,dtype=None):

        Fs,data = self._read(fpath)

        if dtype in (np.float32,np.float64) and dtype != data.dtype:
            inf=np.iinfo(data.dtype)
            data = data.astype(dtype) / -inf.min
        return Fs,data




