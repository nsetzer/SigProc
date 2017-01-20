

#from PyDSP import *
from .process import *
from . import *

import numpy as np
import scipy.signal

class SpectralFluxProcess(IterativeProcess):

    def begin(self):

        #self.frame_zero = np.zeros( self.matrix.data[0,:].shape )

        #self.out = np.empty( self.matrix.data.shape )
        self.out = np.empty( len(self.matrix) )

        return len(self.matrix)

    def step(self,idx):
        lidx=idx-self.delta
        ridx=idx+self.delta
        lmat=len(self.matrix)

        # note: there are two ways to do bounds checking
        # some people set the frame to zero, others
        # copy the first and last frame. The code here
        # originally used a zero frame, but now copies the
        # first and last
        if lidx <     0: lidx=0
        if ridx >= lmat: ridx=lmat-1
        lframe = self.matrix.data[lidx]
        rframe = self.matrix.data[ridx]
        #lframe = self.frame_zero if lidx<0    else self.matrix.data[lidx]
        #rframe = self.frame_zero if ridx>lmat else self.matrix.data[ridx]
        tmp = rframe-lframe
        if self.savePositiveOnly:
            tmp *= tmp>0 # remove delta values less than zero
        self.out[idx] = sum(tmp)

        return True

    def end(self):
        self.out /= np.std(self.out)
        return DataSignal(self.out,self.matrix.sample_rate)

    @staticmethod
    def getOptions():
        opts = [
                MatrixInSpecifier(),
                DataSignalOutSpecifier(),
                ProcessOptionsSpecifier(store="delta",dtype=int, \
                    min=1,max=7,default=3, \
                    help="fixme"),
                ProcessOptionsSpecifier(store="savePositiveOnly",
                    name="Save Positive",dtype=bool, \
                    default=True, \
                    help="For some use cases, such as onset detection " \
                         "only positive flux is needed."),
               ]
        return opts


import scipy.ndimage as sim

class OnsetDetectorProcess(SimpleProcess):

    #Implements the peak-picking method described in:

    #"Evaluating the Online Capabilities of Onset Detection Methods"
    #Sebastian Böck, Florian Krebs and Markus Schedl
    #Proceedings of the 13th International Society for Music Information Retrieval Conference (ISMIR), 2012

    def getThreshold(self):
        return self.threshold


    def run(self):

        fps = self.signal.sample_rate
        # scale input, in milliseconds to # frames
        pre_avg  = int(self.pre_avg*fps/1000.0);
        pre_max  = int(self.pre_max*fps/1000.0);
        post_avg = int(self.post_avg*fps/1000.0);
        post_max = int(self.post_max*fps/1000.0);
        # convert to seconds
        delay    = self.delay/1000.0
        combine  = self.combine/1000.0

        activations = self.signal.data

        thresh = self.getThreshold()

        print("using threshhold", thresh)

        max_length = pre_max + post_max + 1
        max_origin = int(np.floor((pre_max - post_max) / 2))
        mov_max = sim.filters.maximum_filter1d(activations, max_length,
                  mode='constant', origin=max_origin)
        # moving average
        avg_length = pre_avg + post_avg + 1
        avg_origin = int(np.floor((pre_avg - post_avg) / 2))
        mov_avg = sim.filters.uniform_filter1d(activations, avg_length,
                  mode='constant', origin=avg_origin)

        detections = activations * (activations == mov_max)

        detections = detections * (detections >= mov_avg + thresh)
        # convert detected onsets to a list of timestamps

        x= np.nonzero(detections)[0]

        last_onset = 0
        onsets = []
        for i in x:
            onset = float(i) / float(fps) + delay
            # only report an onset if the last N miliseconds none was reported
            if onset > last_onset + combine:
                onsets.append( (onset,True) )
                last_onset = onset

        return Track(Track.HIGHLIGHT,DenseTimeInfo( onsets ))

    @staticmethod
    def getOptions():
        opts = [
                # note using data specifier to hide this option
                # in the riff viewer UI
                DataSignalInSpecifier(),
                TrackOutSpecifier(),
                ProcessOptionsSpecifier(store="threshold",
                    name="Threshold",dtype=float, \
                    default=1.25, \
                    help="threshold for peak-picking"),
                ProcessOptionsSpecifier(store="combine",
                    name="Combine Onsets",dtype=int, \
                    default=30, \
                    help="Only Report 1 Onset for ever N milliseconds"),
                ProcessOptionsSpecifier(store="pre_avg",
                    name="Pre-Average",dtype=int, \
                    default=100, \
                    help="use N miliseconds past information for moving average"),
                ProcessOptionsSpecifier(store="post_avg",
                    name="Post-Average",dtype=int, \
                    default=70, \
                    help="Use N miliseconds future information for moving average"),
                ProcessOptionsSpecifier(store="pre_max",
                    name="Pre-Maximum",dtype=int, \
                    default=30, \
                    help="use N miliseconds past information for moving maximum"),
                ProcessOptionsSpecifier(store="post_max",
                    name="Post-Maximum",dtype=int, \
                    default=30, \
                    help="Use N miliseconds future information for moving maximum"),
                ProcessOptionsSpecifier(store="delay",
                    name="Onset Delay",dtype=int, \
                    default=0, \
                    help="Report onset N milliseconds after detection."),
               ]
        return opts

class OnsetDetectorRelativeProcess(OnsetDetectorProcess):
    # theshold is some percentage of the std dev + a constant
    def getThreshold(self):
        return self.threshold + self.scale * np.mean(self.signal.data)

    @staticmethod
    def getOptions():
        opts = OnsetDetectorProcess.getOptions()
        opts.insert(3,ProcessOptionsSpecifier(store="scale",
                    name="Relative Threshold",dtype=float, \
                    min=0.0,max=1.0,
                    default=.5, \
                    help="threshold for peak-picking"),)
        return opts
