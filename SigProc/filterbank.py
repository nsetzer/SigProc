#! python $this

"""
    TODO:
    Matrix.fromSignal(signal,data,rate)
    Matrix.fromMatrix(matrix,data,rate)

    each returns a new matix using data and rate.
        and any other useful metadata from the source object.

        I basically require at this point "source sample rate"
        as the sample rate from the signal used to generate a matrix
        and don't want to have to set that every time.

    Annotation(...)
        IndexAnnotation(name,data)
            flat list of indices of interest
            name is what would be displayed for the kind
                "e.g. annotation of clipping regions"
        TextAnnotation(name,data)
            list of tuple paired indices and words, in sorted by index order.
            can be used to draw transcripts alongside the source.


"""


from . import *

import numpy as np
import scipy.fftpack as fft
import sys

def freq2mel( f ):
    return ((1127 * np.log(1.0 + f/700.0)))
def mel2freq( m ):
    return ((700.0*(np.exp((m/1127.0))-1)))

# FFT bin location of a given frequency
# these are only defined for the positive half
def freq2idx(SR,N,freq):
    return int(freq/float(SR)*N)
def idx2freq(SR,N, idx):
    return float(idx)/N*SR

# B : number of tones per octave (12)
# A : middle A frequency         (440)
def freq2midi(f, B, A):
    # https://en.wikipedia.org/wiki/MIDI_Tuning_Standard
    # 69 + 12 log2 ( f / 440 )
    # 5.75 = 69/12, so that we can specify the number of tones
    return float(B*(5.75 + np.log2(f/A)))
def midi2freq(m, B, A):
    return float(A) * np.power(2.0,m/float(B) - 5.75);

class FilterBank(object):
    """
        sample_rate is the sample rate of the audio channel
            that was used to build the spectrogram
        logN is the value used to generate the spectrogram, 2**logN is these
            number of FFT bins.
        numFilter specifies the number of output bins:
            in 'mel' mode, the specified number of filter banks will be applied.
            in 'midi' mode this many filterbanks PER octave will be applied
        minf is the cutoff lower frequency to consider in the FilterBank
        maxf is the upper cutoff frequency to consider in the FilterBank
            0 < minf < maxf < sample_rate/2
        equalize: when true scale the magnitude of each bin to 1.
        middleA : only used in midi mode, specifies the middle A note for tuning.
        kind : set to "mel" for MFCC features and "midi" for midi features
            "midi" features specify the number of output bins per octave,
                and use the midi frequency scale, but otherwise are the sample
                process as MFCC features.
    """
    def __init__(self,sample_rate,logN,numFilter,minf=300,maxf=5000,equalize=True,useDCT=True):

        self.sample_rate = sample_rate
        self.logN = logN
        self.N = 2**logN
        self.num_filter = numFilter
        self.minf = minf
        self.maxf = maxf
        self.equalize = equalize
        self.useDCT = useDCT

        print("Creating FilterBank with logn=%d"%self.logN)

        # centers marks the center of each  triangle, with two additional
        # values at index 0 and -1, which mark the ends of two triangles.
        self.centers      = np.zeros( (self.num_filter+2,), dtype=int )
        #self.idx_offset   = np.zeros( (self.num_filter,) , dtype=int )
        self.table_length = np.zeros( (self.num_filter,) , dtype=int )
        self.lut = [None,] * self.num_filter

        self.init_centers();

        self.init_lut()

        print("use dct: %s"%self.useDCT)

    def init_centers(self):
        # first and last bin mark the begining and end of the first and last
        # triangle filter, respectivley. each other index mark the middle
        # of a triangle filter, and also the end/begining of neighboring
        # overlapped filters
        cminf = freq2mel( self.minf );
        cmaxf = freq2mel( self.maxf );
        for i in range(self.num_filter+2):
            # divide filters equallu in mel-space
            m = cminf+((i)*((cmaxf-cminf)/(self.num_filter+1)));
            self.centers[i] = freq2idx(self.sample_rate,self.N,mel2freq(m));

    def init_lut(self):
        """
        the lut, look up table contains the precomputed values
        of the triangle filters.

        each filter i is located from centers[i] to centers[i]+table_length[i]
            in the frequency-bin space.
        """
        for i in range(self.num_filter):
            il = self.centers[i];
            ic = self.centers[i+1];
            ih = self.centers[i+2];
            fc = float(ic);

            # used to have an table_offset array
            # but it was exactly the same as centers
            self.table_length[i] = ih-il
            if self.table_length[i]==0:
                raise ValueError("fft size was too small")
            self.lut[i] = np.empty( (self.table_length[i],) )
            lut = self.lut[i]

            height = 1.0
            if self.equalize:
                height = 2.0 / self.table_length[i]

            k = 0

            if il==ic or ic==ih:
                lut[0] = height
            else:
                # sample a triangle stretched over il to ih.
                # triangles are equi-sized in mel/midi-space
                lut[k]=0.0
                k+=1
                for j in range(il+1,ic):
                    lut[k] = height*(((1.0-( il/fc / ( il/fc - 1.0) ))/fc)*j \
                        + ( il/fc / ( il/fc - 1.0) ))
                    k+=1
                lut[k]=height
                k+=1
                for j in range(ic+1,ih):
                    lut[k] = height*(((1.0-( ih/fc / ( ih/fc - 1.0) ))/fc)*j \
                        + ( ih/fc / ( ih/fc - 1.0) ))
                    k+=1
        return

    def apply(self,frame):
        """ apply filterbank to one frame"""
        out = np.empty( (self.num_filter,), dtype=np.float64 )

        for i in range(self.num_filter):
            s = frame[self.centers[i]:self.centers[i]+self.table_length[i]]
            if len(s) < len(self.lut[i]):
                k = len(self.lut[i]) - len(s)
                s = np.pad(s,( (0,k)), mode='constant')
            #out[i] = np.sum( s * self.lut[i]  )
            out[i] = np.dot(s,self.lut[i])


        if self.useDCT:
            #http://docs.scipy.org/doc/scipy-0.14.0/reference/generated/scipy.fftpack.dct.html

            out2 = fft.dct(np.log10(out),type=2)

            if np.count_nonzero(out) != out.size:
                print(out)
                print(out2)
                raise Exception("nonzero found");

            if np.any(np.isinf(out)):
                print(out)
                print(out2)
                raise Exception("inf found");
            if np.any(np.isnan(out)):
                print(out)
                print(out2)
                raise Exception("nan found");
            out = out2



        return out

class MidiFilterBank(FilterBank):

    def __init__(self,sample_rate,logN,numFilter,tones,middleA=440,skip_tones=0,equalize=True):
        self.sample_rate = sample_rate
        self.logN = logN
        self.N = 2**logN

        # OLD SETTINGS:
        # let there be 8 * numFilter, such that we get
        # numFilter bins per 8 octaves.
        #self.num_filter =  8 * numFilter # todo , used to be 8*numFilter
        #self.tones = numFilter

        # NEW SETTINGS
        self.num_filter = numFilter # must be 6 * N, where N is number of octaves
        self.tones = tones # 6 bins per octave.
        self.skip_tones = skip_tones

        self.middleA = middleA
        self.equalize = equalize
        self.useDCT = False

        print("filterbank params",numFilter,tones,middleA)

        self.centers      = np.zeros( (self.num_filter+2,) , dtype=int )
        self.idx_offset   = np.zeros( (self.num_filter,) , dtype=int )
        self.table_length = np.zeros( (self.num_filter,) , dtype=int )
        self.lut = [None,] * self.num_filter

        self.octave = np.zeros( (self.num_filter+2,) , dtype=int )
        # if numFilter==12, then each output bin%12 corresponds
        #self.notes = ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]

        self.init_centers();

        self.init_lut()

    def init_centers(self):

        # 0 octave starts at 16Hz, add 2*Tones so that
        # number begins at the start of the first octave
        start = self.skip_tones + 2*self.tones - 1
        end   = self.skip_tones + 2*self.tones + self.num_filter + 1

        i=0;

        for j in range(start,end):
            freq =  midi2freq(j,self.tones,self.middleA);

            if freq > self.sample_rate/2:
                freq =  midi2freq(j-1,self.tones,self.middleA);
                sys.stderr.write("warn: FilterBank last freq is %.1f due to nyquist cutoff.\n"%freq)
                break;
            idx = freq2idx(self.sample_rate,self.N,freq)
            if idx == self.centers[i-1]: # this is only a minor error
                print("resolution error",start,j,end,":",i,idx,\
                        midi2freq(j,self.tones,self.middleA), \
                        midi2freq(j-1,self.tones,self.middleA))

            self.centers[i] = idx
            self.octave[i] = j / self.tones

            i += 1
        print(self.centers)

        if i-2 != self.num_filter:
            sys.stderr.write("warn: fft resolution or sample rate " \
                "was too low for a full filterbank %d/%d.\n"% \
                 (i-2,self.num_filter) )
            self.num_filter = i - 2
        return

class FilterBankProcess(IterativeProcess):

    def begin(self):
        # Fs  sample rate
        # Fn fft size
        # numFilter : 26
        # minf : 300
        # maxf : 5000
        # note that logN is the same as the log of the
        # height of the input Matrix
                                    #nt(np.ceil(1+np.log2( self.matrix.frame_height ))),
        self.filterbank = FilterBank(self.matrix.source_rate,
                                    int(np.log2(self.matrix.Nfft)),
                                    self.num_filter,
                                    self.minf,
                                    self.maxf,
                                    self.equalize,self.useDCT);
        self.omatrix = Matrix( np.empty( \
                (self.matrix.shape[0],self.filterbank.num_filter), dtype=np.float64 ), \
                self.matrix.sample_rate );
        self.omatrix.source_rate = self.matrix.source_rate

        return len(self.matrix)

    def step(self,idx):
        frame = self.matrix.data[idx]
        self.omatrix.data[idx,:] = self.filterbank.apply(frame)

    def end(self):
        return self.omatrix

    @staticmethod
    def getOptions():
        opts = [
                SpectralMatrixInSpecifier(), \
                MatrixOutSpecifier(), \
                ProcessOptionsSpecifier(store="num_filter",
                    dtype=int,default=26), \
                ProcessOptionsSpecifier(store="minf",
                    name="Min Frequency",
                    suffix="Hz",
                    dtype=int,default=300), \
                ProcessOptionsSpecifier(store="maxf",
                    name="Max Frequency",
                    suffix="Hz",
                    dtype=int,default=5000), \
                ProcessOptionsSpecifier(store="equalize",
                    dtype=bool,
                    default=True), \
                ProcessOptionsSpecifier(store="useDCT",
                    dtype=bool,
                    default=False), \
                ]
        return opts

class MidiFilterBankProcess(FilterBankProcess):

    def begin(self):
                                    #nt(np.ceil(1+np.log2( self.matrix.frame_height ))),
        self.filterbank = MidiFilterBank(self.matrix.source_rate,
                                    int(np.log2(self.matrix.Nfft)),
                                    self.num_filter,self.tones,
                                    self.middleA,self.skip_tones,self.equalize);
        self.omatrix = Matrix( np.empty( \
                (self.matrix.shape[0],self.filterbank.num_filter) ), \
                self.matrix.sample_rate );
        self.omatrix.source_rate = self.matrix.source_rate

        return len(self.matrix)


    @staticmethod
    def getOptions():
        opts = [
                SpectralMatrixInSpecifier(), \
                MatrixOutSpecifier(), \
                ProcessOptionsSpecifier(store="num_filter",
                    dtype=int,default=84), \
                ProcessOptionsSpecifier(store="tones",dtype=int,default=12), \
                ProcessOptionsSpecifier(store="skip_tones",dtype=int,default=0), \
                ProcessOptionsSpecifier(store="middleA",dtype=int,default=440), \
                ProcessOptionsSpecifier(store="equalize",
                    dtype=bool,
                    default=True), \
                ]
        return opts
