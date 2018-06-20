
import numpy as np

# http://mccormickml.com/2016/04/19/word2vec-tutorial-the-skip-gram-model/

class SbicImpl(object):
    """
    Bayesian information criterion (SBIC)

    Segment audio by analyzing similarity of consecutive frames.

    This is a direct translation from C++ to Python of the essentia SBIC impl

    http://essentia.upf.edu/documentation/reference/streaming_SBic.html
    https://en.wikipedia.org/wiki/Bayesian_information_criterion
    """

    def __init__(self):
        super(SbicImpl, self).__init__()

        # tunable parameters
        self._cpw = 1.5  # complexity penalty weight
        self._minLength = 10  # min segment length
        self._inc1 = 60  # first pass window increment
        self._inc2 = 20  # second pass window increment
        self._size1 = 300  # first pass window size
        self._size2 = 200  # second pass window size

        self._cp = 0.0  # (feature) complexity penalty
        self.segmentation = []

    def logDet(self, matrix):
        """
        returns the logarithm of the determinant of (the covariance) matrix
        http://en.wikipedia.org/wiki/Cholesky_decomposition
        """

        dim1, dim2 = matrix.shape
        mp = np.zeros(dim1, dtype=float)
        vp = np.zeros(dim1, dtype=float)
        z = 1.0 / dim2
        zz = z * z

        for i in range(dim1):
            for j in range(dim2):
                a = matrix[i][j]
                mp[i] += a
                vp[i] += a * a

        logd = 0.0
        diag_cov = 0.0
        for i in range(dim1):
            diag_cov = vp[i] * z - mp[i] * mp[i] * zz
            logd += np.log(diag_cov) if diag_cov > 1e-5 else -5.0

        return logd

    def bicChangeSearch(self, matrix, inc, current):
        """ find a change point in the matrix
        """

        nFeatures, nFrames = matrix.shape

        seg = 0
        shift = inc - 1

        penalty = self._cpw * self._cp * np.log(nFrames)
        dmin = np.iinfo(np.int32).max

        s = self.logDet(matrix)

        while shift < (nFrames - inc):

            n1 = shift + 1
            s1 = self.logDet(matrix[:, 0:shift + 1])

            n2 = nFrames - n1
            s2 = self.logDet(matrix[:, shift + 1:nFrames])

            d = 0.5 * (n1 * s1 + n2 * s2 - nFrames * s + penalty)

            if d < dmin:
                seg = shift
                dmin = d
            shift += inc

        if dmin > 0:
            return 0

        return current + seg

    def delta_bic(self, matrix, segPoint):
        """ determine if two consecutive segments
        have the same probability distribution.
        Used to join similar segments

        """

        nFeatures, nFrames = matrix.shape

        s = self.logDet(matrix)
        s1 = self.logDet(matrix[:, 0:segPoint + 1])
        s2 = self.logDet(matrix[:, segPoint + 1:nFrames])

        t = self._cpw * self._cp * np.log(nFrames)
        t1 = segPoint * s1
        t2 = (nFrames - segPoint) * s2
        t3 = nFrames * s

        return 0.5 * (t1 + t2 - t3 + t)

    def delta_cost(self, window1, window2):
        """
        a function for testing the delta_bic implementation

        inputs that are similar should return positive
        """

        nFeatures, nFrames1 = window1.shape
        nFeatures, nFrames2 = window2.shape

        s = self.logDet(np.hstack([window1, window2]))
        s1 = self.logDet(window1)
        s2 = self.logDet(window2)

        t = self._cpw * self._cp * np.log(nFrames1 + nFrames2)
        t1 = nFrames1 * s1
        t2 = nFrames2 * s2
        t3 = (nFrames1 + nFrames2) * s

        return 0.5 * (t1 + t2 - t3 + t)

    def compute(self, matrix):
        """ segment audio given a matrix representation

        the matrix should have shape (nFeatures, nFrames). the return value
        is a list of integers, where any consecutive pair marks the beginning
        and end of a segment of audio which is considered similar. the first
        value will always be zero, and the final value will always be the
        index of the final frame.

        """

        # first dimension is the number of features
        # second dimension is the number of samples
        nFeatures, nFrames = matrix.shape

        if nFrames < 2:
            raise Exception(nFrames)

        if (nFrames <= self._minLength - 1):
            self.segmentation = [0, nFrames - 1]

        self._cp = 2 * nFeatures

        self.segmentation = []

        self.compute_coarse(matrix)
        self.compute_fine(matrix)
        self.compute_validate(matrix)

        return self.segmentation

    def compute_coarse(self, matrix):

        nFeatures, nFrames = matrix.shape

        currSeg = 0
        endSeg = -1
        while endSeg < (nFrames - 1):
            endSeg += self._size1

            if endSeg >= nFrames:
                endSeg = nFrames - 1

            window = matrix[:, currSeg:endSeg + 1]

            idx = self.bicChangeSearch(window, self._inc1, currSeg)
            if idx:
                self.segmentation.append(idx)
                currSeg = idx + self._inc1
                endSeg = currSeg - 1

    def compute_fine(self, matrix):

        nFeatures, nFrames = matrix.shape

        halfSize = self._size2 // 2


        currIdx = 0
        while currIdx < len(self.segmentation):
            currSeg = self.segmentation[currIdx] - halfSize
            if currSeg < 0:
                currSeg = 0

            endSeg = currSeg + self._size2 - 1
            if endSeg > nFrames:
                endSeg = nFrames - 1

            window = matrix[:, currSeg:endSeg + 1]

            idx = self.bicChangeSearch(window, self._inc2, currSeg)

            if idx:

                if currIdx == 0:
                    prevSeg = 0
                else:
                    prevSeg = self.segmentation[currIdx - 1]

                if (currIdx + 1) >= len(self.segmentation):
                    nextSeg = (nFrames - 1)
                else:
                    nextSeg = self.segmentation[currIdx + 1]

                if prevSeg <= idx <= nextSeg:
                    if idx != self.segmentation[currIdx]:
                        self.segmentation[currIdx] = idx
                    elif currIdx < len(self.segmentation):
                        self.segmentation.pop(currIdx)
                        continue
            currIdx += 1

    def compute_validate(self, matrix):

        nFeatures, nFrames = matrix.shape

        # any 2-pair of indices is a segment of frames
        self.segmentation.insert(0, 0)
        self.segmentation.append(nFrames - 1)

        # the whole matrix is one segment
        if len(self.segmentation) == 2:
            return

        # enforce minimum length on segments
        while (len(self.segmentation) > 1 and
              self.segmentation[1] < self._minLength):
            self.segmentation.pop(1)

        # join neighboring short segments
        idx = 2
        while idx < len(self.segmentation) - 1:
            a = self.segmentation[idx - 1]
            b = self.segmentation[idx - 2]
            c = self.segmentation[idx + 1]
            d = self.segmentation[idx]

            if d - a < self._minLength:
                int1 = a - b
                int2 = c - d

                if int1 <= int2:
                    self.segmentation.pop(idx - 1)
                else:
                    self.segmentation.pop(idx)
            else:
                idx += 1

        segCnt = len(self.segmentation)
        int1 = self.segmentation[segCnt - 1] - self.segmentation[segCnt - 2]
        if segCnt > 2 and int1 < self._minLength:
            self.segmentation.pop(segCnt - 2)

        # verify delta_bic is negative between consecutive  segments
        currSeg = self.segmentation[0]
        i = 1
        while i < len(self.segmentation) - 1:
            endSeg = self.segmentation[i + 1]
            window = matrix[:, currSeg:endSeg + 1]
            segPoint = self.segmentation[i] - self.segmentation[i - 1]

            if self.delta_bic(window, segPoint) > 0.0:
                self.segmentation.pop(i)
                continue

            currSeg = self.segmentation[i] + 1
            i += 1

        # in case the end of file segment was erased
        if self.segmentation[-1] != nFrames - 1:
            self.segmentation.append(nFrames - 1)

class SbicAudio(SbicImpl):
    """docstring for SbicAudio"""
    def __init__(self, frameRate):
        """
        frameRate: number of frames per second
        """
        super(SbicAudio, self).__init__()

        self._minLength = frameRate // 4
        self._inc1 = frameRate // 5
        self._inc2 = frameRate // 10
        self._size1 = 3 * frameRate
        self._size2 = frameRate

def main():

    impl = SbicImpl()

    np.random.seed(4)
    nFeatures, nFramesA, nFramesB = (4, 400, 600)
    matrix1 = .5 * np.random.rand(nFeatures, nFramesA) - .25
    matrix2 = .5 * np.random.rand(nFeatures, nFramesB) + 1

    matrix = np.hstack([matrix1, matrix2])

    print(matrix.shape)

    segments = impl.compute(matrix)

    expected = [0, nFramesA, nFramesA + nFramesB - 1]
    print(segments)
    print(expected)

    # validate that the segmentation returns the correct result
    if len(segments) != len(expected):
        raise Exception((segments, expected))

    for a, b in zip(segments, expected):
        if b - a > 10:
            raise Exception((a, b))

    # experiment: generalization of segmentation code.

    matrix3 = .5 * np.random.rand(nFeatures, nFramesA) - .25

    ones = np.ones([nFeatures, nFramesA], dtype=float)
    print(0, 0, impl.delta_cost(ones, ones))
    print(1, 1, impl.delta_cost(matrix1, matrix1))
    print(3, 3, impl.delta_cost(matrix3, matrix3))
    print(1, 2, impl.delta_cost(matrix1, matrix2))
    print(1, 3, impl.delta_cost(matrix1, matrix3))
    print(2, 3, impl.delta_cost(matrix2, matrix3))

if __name__ == '__main__':
    main()
