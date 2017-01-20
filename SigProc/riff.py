#! python35 $this
import os,sys
import struct
import numpy as np

def unpack(rf,f):
    if f=="4s":
        return rf.read(4)
    elif f=="<i":
        b=rf.read(4)
        if b:
            return struct.unpack(f,b)[0]
        return 0
    b=rf.read(2)
    if b:
        return struct.unpack(f,b)[0]
    return 0

class Riff(object):
    """
        read and write modified riff files
        * supports any number arbitrary sections
        * can only have 1 data section
        * can read standard wave files
        * can read matrix files.
        * supports reading / writing a utf-8 dictionary
          of key value string pairs. under the DICT chunk
    """
    FMT_B=0 # 1 byte int
    FMT_H=1 # 2 byte int # also same as wave pcm mode.
    FMT_I=2 # 4 byte int
    FMT_U=3 # 4 byte uint
    FMT_F=4 # 4 byte float
    FMT_D=5 # 8 byte double

    # if we need a new data type it must be tacked on to the end of this list
    # as it is, uni32 is already tacked on...
    data_types = [np.int8,np.int16,np.int32,np.int64,np.float32,np.float64]
    bit_rates  = [      8,      16,      32,      64,        32,        64]

    def __init__(self):

        self.bitrate=0 # bits per sample
        self.channels=0
        self.samplerate=0
        self.mode = b'IMAT'
        self.filesize=0 # not used in writing
        self.datasize=0
        self.fmt = Riff.FMT_H
        # block size can be tuned to read/write faster/slower
        self.block_size=1024*1024 # in number of samples
        self.exif = None

    def writeHeader(self,uf):

        uf.write(b'RIFF')
        uf.write(struct.pack("<i",self.filesize))
        uf.write(self.mode)
        uf.write(b'fmt ')
        uf.write(struct.pack("<i",16)) # sub chunk size

        uf.write(struct.pack("<h",self.fmt))
        uf.write(struct.pack("<h",self.channels))
        uf.write(struct.pack("<i",self.samplerate))
        t = (self.channels*self.samplerate*self.bitrate)>>3
        uf.write(struct.pack("<i",t))
        uf.write(struct.pack("<h",2)) # block align
        bitrate = self.channels * Riff.bit_rates[self.fmt]
        uf.write( struct.pack("<h",bitrate) )

    def readHeader(self,rf):

        unpack(rf,"4s") # RIFF
        self.filesize=unpack(rf,"<i")
        mode=unpack(rf,"4s")
        if mode!=self.mode:
            raise ValueError("expected RIFF kind to be %s but found %s."%\
                (self.mode,mode))

        header = unpack(rf,"4s")
        chunksize = 0
        while header != b'fmt ':
            chunksize=unpack(rf,"<i")
            sys.stderr.write("RIFF: skipping section %s:%d\n"%(header,chunksize))
            rf.seek(chunksize,os.SEEK_CUR)
            header = unpack(rf,"4s")

        chunksize = unpack(rf,"<i")             # 4 .. 16 or 18
        #self.bitrate    = unpack(rf,"<i")       # 4
        #print("bitrate is",self.bitrate)
        #print("fmt is",self.fmt)
        self.fmt        = unpack(rf,"<h")       # 2
        self.bitrate = Riff.bit_rates[self.fmt]
        self.channels   = unpack(rf,"<h")       # 2
        self.samplerate = unpack(rf,"<i")       # 4

        byte_rate   = unpack(rf,"<i")           # 4
        block_align   = unpack(rf,"<h")         # 2
        bitspersample = unpack(rf,"<h")         # 2
        ######################################### total: 16 + 4 for chunksize

        #print("mode       : %s"%mode)
        #print("chunksize  : %d"%chunksize,chunksize-16)
        #print("fmt        : %d"%self.fmt)
        #print("channels   : %d"%self.channels)
        #print("samplerate : %d"%self.samplerate)
        #print("byte_rate    : %d"%byte_rate)
        #print("block_align  : %d"%block_align)
        #print("bitspersample: %d"%bitspersample)

        # some wave implementations write :
        #       2-byte ExtraParamSize
        #       X-Byte ExtraParams
        # technically for fmt=PCM this should not be here.
        # for fmt=1=PCM, it can optionally be \x00\x00 per this quote

        # http://blog.bangsplatpresents.com/?p=1017
        # """
        # If AudioFormat is something other than “1″,
        # the two bytes after the end of the traditional
        # WAVE header is a length of the extra compression
        # information, and the actual information comes
        # after that. These two bytes, called ExtraParamSize,
        # are not expected to be present if AudioFormat is 1,
        # but strictly speaking, it doesn’t hurt if they
        # are there, but set to zero, which means there
        # are not bytes past those. Any program that can
        # generate compressed WAVE files might choose to
        # add this field regardless of the AudioFormat.
        # """
        # if non-zero I need to unread those bytes
        if (chunksize-16 > 0):
            opt_size = unpack(rf,"<h")
            if opt_size > 0:
                rf.seek(opt_size,os.SEEK_CUR)

        # method 2: using the chunksize-16 i can absolutely determine
        # the size of ExtraParamSize and ExtraParams
        # todo, subtract the other values from chunksize
        #chunksize = chunksize-16
        #if chunksize > 0:
        #    ExtraParamSize = unpack(rf,"<h")
        #    sys.stderr.write("RIFF: ExtraParamSize:%d\n"%ExtraParamSize)
        #    chunksize -= 2
        #    if chunksize != ExtraParamSize:
        #        raise ValueError("malformed header")
        #    if chunksize > 0:
        #        rf.seek(chunksize,os.SEEK_CUR)

    def writeBlock(self,wf,samples):
        t = samples.tostring('C')
        wf.write( t )
        return len(t)

    def writeData(self,wf,data):

        wf.write(b'data')
        wf.write(struct.pack("<i",self.datasize))
        idx=0
        size = len(data)
        bytes = 0
        while idx < size:
            bytes += self.writeBlock(wf,data[idx:idx+self.block_size])
            idx += self.block_size
        return bytes # should be len(data)*self.bitrate//8

    def writeExif(self,wf,exif):
        # exif should be a kv-map of byte-strings
        # if it is not an attempt will be made to
        # str-encode it as utf-8
        # otherwise an exception will fly up the stack
        exif_info = b''
        for k,v in exif.items():
            if not isinstance(k,bytes):
                k = str(k).encode('utf-8')
            if not isinstance(v,bytes):
                v = str(v).encode('utf-8')
            exif_info += struct.pack("<i",len(k))
            exif_info += k
            exif_info += struct.pack("<i",len(v))
            exif_info += v

        wf.write(b'DICT')
        wf.write(struct.pack("<i",len(exif_info)))
        wf.write(exif_info)
        #print(exif_info)

        self.filesize += 8 + len(exif_info)

    def readBlock(self,rf,nsamples,dtype):

        block_size = nsamples*self.bitrate//8
        data = rf.read(block_size)
        #print(len(data),block_size,dtype,self.bitrate//8)
        return np.fromstring(data,dtype=dtype)

    def readData(self,rf):
        # read 4 byte section name and 4 byte section size
        # if section name is data return the data based on self.fmt
        # if section does not match read the entire section but return None.
        name=unpack(rf,"4s")
        if len(name)==0:
            return False
        size=unpack(rf,"<i")

        if name == b'DICT':
            self.exif = self.readExif(rf,name,size)
            return True
        elif name != b'data':
            sys.stderr.write("RIFF: skipping section %s:%d at position: %d\n"%(name,size,rf.tell()))
            rf.seek(size,os.SEEK_CUR)
            return self.data is None

        # two different behaviors when writing to a named pipe (stdout)
        # avconv writes 0 to the size of the 'data' blcok
        # ffmpeg writes -1 to the size of the 'data' blcok
        if size <= 0:
            size = 0x7FFFFFFF;
        else:
            size //= self.bitrate//8
        dtype = Riff.data_types[self.fmt]
        # create an empty array of type dtype
        data = np.asarray([],dtype=dtype)

        while size > 0:
            temp = self.readBlock(rf,min(size,self.block_size),dtype)
            if len(temp):
                size -= len(temp)
                data = np.append(data,temp)
            else:
                break
        self.data = data
        return True

    def write(self,filepath,data,samplerate,channels=1):
        self.datasize  = len(data)*channels*self.bitrate//8
        self.filesize += self.datasize + 28
        self.samplerate=samplerate
        self.channels=channels

        with open(filepath,"wb") as wf:
            self.writeHeader(wf)
            self.writeData(wf,np.ravel(data))
            if self.exif:
                self.writeExif(wf,self.exif)
                # update the header, or have it pre-calc the exif
                # and update filesize before calling write

    def readExif(self,rf,name,size):
        #name=unpack(rf,"4s")
        #size=unpack(rf,"<i")

        if name != b'DICT':
            sys.stderr.write("RIFF (exif): skipping section %s:%d\n"%(name,size))
            rf.seek(size,os.SEEK_CUR)
            return None

        exif = {}
        while size > 0:
            klen = unpack(rf,"<i")
            k = rf.read(klen)
            vlen = unpack(rf,"<i")
            v = rf.read(vlen)
            size -= 4 + klen + vlen
            exif[k]=v

        return exif

    def read(self,filepath):
        """
        return an a 1 or 2 dimensional vector
        data[ t ] returns a sample at time = t
        for 1D, WAVE file this will be a floating point value
        for 1D, MATRIX it will be of data type dtype.
        for 2D, MATRIX data[ t ] returns the frame at time t
        """
        data = None
        if isinstance(filepath,str):
            rf = open(filepath,"rb");
        else:
            rf= filepath

        self.data = None # set by readData on success
        with rf:
            self.readHeader(rf)
            cont = True
            while cont:
                cont = self.readData(rf)
        data = self.data
        if self.channels>1:
            return (self.samplerate, np.asarray(data).reshape(-1,self.channels) )
        return (self.samplerate, np.asarray(data))
    # todo open, write_samples, close functions for journaling mode

class WaveWRX(Riff):
    """WaveWRX read and write wave files"""
    def __init__(self):
        super(WaveWRX,self).__init__()
        self.bitrate=16
        self.mode = b'WAVE'
        self.fmt = 1 # PCM

    def read(self,filepath,dtype=np.int16):
        """
            reads a 16 bit PCM WAVE file with 1 or more channels.
            if mono, data[ t ] returns the sample at time t
            if stereo, data [ t ] returns a frame with [ left, right ]

            returns a numpy array of type int16,
                unless dtype is float32 or float64.
            if dtype is float, data will be scaled fom -1 to 1.

            compress stereo to mono: (data[1,:] + x[data,:]) / 2.0

            minor feature/bug: writing column vectors of 1D
                are read back as row vectors.
                write: [ [0], [1], [2], ] ==>  read: [0,1,2]
                this is because data is raveled using C index order
                before being written to the file
        """
        rate,data = super(WaveWRX,self).read(filepath)
        # if specified to convert to float, scale by the maximum
        # of the data type
        if dtype in (np.float32,np.float64) and dtype != data.dtype:
            inf=np.iinfo(data.dtype)
            data = data.astype( dtype ) / -inf.min
        return (rate,data)

    def write(self,filepath,samplerate,data,exif=None):

        index=Riff.FMT_H # PCM/16bit int
        data = np.asarray(data)
        try:
            index=Riff.data_types.index( data.dtype )
        except ValueError as e:
            raise ValueError( "dtype %s is not supported"%data.dtype )
        if index!=Riff.FMT_H:
            if data.dtype in (np.float32,np.float64):
                inf=np.iinfo(np.int16)
                data = (data* -inf.min).astype(np.int16)
            else:
                data = data.astype(np.int16)

        if data.ndim==1:
            channels = 1
        else:
            _,channels = data.shape

        self.exif = exif

        super(WaveWRX,self).write(filepath,data,samplerate,channels)

class MatrixWRX(Riff):
    """MatrixWRX read and write 2-D matrix files"""
    def __init__(self):
        super(MatrixWRX,self).__init__()
        self.mode = b'IMAT'

    def write(self,filepath,samplerate,data,exif=None):

        self.fmt=Riff.FMT_H # PCM/16bit int
        # TODO : data = np.asarray(data)
        try:
            self.fmt=Riff.data_types.index( np.asarray(data).dtype )
        except ValueError as e:
            raise ValueError( "dtype %s is not supported"%data.dtype )

        self.bitrate = Riff.bit_rates[self.fmt]

        if data.ndim==1:
            channels = 1
        else:
            _,channels = data.shape

        self.exif = exif

        super(MatrixWRX,self).write(filepath,data,samplerate,channels)

class ArrayWRX(MatrixWRX):
    """ArrayWRX read and write N-D matrix files"""
    def __init__(self):
        super(ArrayWRX,self).__init__()
        self.mode = b'DARY'

    def write(self,filepath,samplerate,data,exif=None):

        if exif is None:
            exif = dict()

        exif["_shape"] = ','.join(str(d) for d  in data.shape)
        data = data.reshape(-1)

        super(ArrayWRX,self).write(filepath,samplerate,data,exif)

    def read(self,filepath):

        samplerate,data = super(ArrayWRX,self).read(filepath)

        shape = self.exif[b"_shape"].decode("utf-8")
        shape = [int(d) for d in shape.split(",")]
        data = data.reshape(*shape)

        return samplerate,data



def main():

    #mode = "r+b" if os.path.exists(r.path) else "w+b"
    #with open(r.path,mode) as uf:
    #    writeHeader(r,uf)
    wr = ArrayWRX()
    wr.write("./temp.ary",0,np.zeros(32).reshape(8,4,1))
    sr,data = wr.read("./temp.ary")
    print(data.shape)

    #rate,signal = wr.read("top.wav")
    #print(rate,signal.shape)

    #wr.write("top2.wav",signal,rate)
    #wm = MatrixWRX()
    #chan=5
    #x = np.asarray( range(65) ,dtype=np.int8).reshape(-1,chan)
    #wm.write( "test.mat",x,20,chan )
    #print(x)
    #r,s = wm.read("test.mat")
    #print(r)
    #print(s)
    #with open(r.path,"rb") as rf:
    #    r.readHeader(rf)

if __name__ == '__main__':
    main()