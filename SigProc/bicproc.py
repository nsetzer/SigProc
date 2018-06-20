

from .process import *
from . import *

from .bic import SbicAudio

class BicProcess(SimpleProcess):

    def run(self):

        fps = self.matrix.sample_rate

        impl = SbicAudio(fps)

        matrix = np.transpose(self.matrix.data)

        print(matrix.shape)

        segments = impl.compute(matrix)
        info = SparseTimeInfo([(s, True) for s in segments])
        return Track(Track.HIGHLIGHT, info)

    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        opts = [ \
                SpectralMatrixInSpecifier(),
                TrackOutSpecifier(),
                ]
        return opts
