#! python $this
import math
import codecs

# todo when painting a label,
#   give the painter object, w and h (along with the configuration)
#   also provide a function which returns a value to a y position
#   on the grid. this is trivial for linear data, but hard for everything else.
#   for example, if i wanted to highlight a pixel on a chromagram

class TimeInfoImpl(object):
    """docstring for TimeSlice"""
    def __init__(self, data):
        super(TimeInfoImpl, self).__init__()
        # data is in the form [ (time,info), ... ]

        # resolution determines how many bins
        # each bin represents:
        #  idx*resolution <= t < (idx+1)*resolution
        self.resolution = 1.0
        self.offset = 0

        if len(data):
            data.sort(key=lambda x : x[0])
            self.min_time = data[0][0]
            self.max_time = data[-1][0]
            self._init_bins( self.min_time, self.max_time )
            self._build(data)
        else:
            self._init_bins( 0, 1)

        self.num_samples = len(data)

    def minimum(self):
        """the smallest possible time value within the data set, in seconds"""
        return self.offset * self.resolution

    def maximum(self):
        """the largest possible time value within the data set, in seconds"""
        return (len(self.bins) + self.offset+1)*self.resolution

    def count(self):
        """return the number of data points contained within"""
        return self.num_samples

    def slice(self,ts,te):
        """
        returns all entries in sorted order
        with a time value greater than
        """
        sidx = int(ts / self.resolution) - self.offset
        eidx = int(math.ceil(te / self.resolution)) - self.offset
        return self._slice(sidx,eidx,ts,te)

    def __repr__(self):
        #x = [ "%.3f"%t for t,_ in self.slice(self.minimum(),self.maximum()) ]
        return '%s<[...]>'

    def save(self,filepath):
        with codecs.open(filepath,"w","utf-8") as wf:
            print(self.minimum())
            print(self.maximum())
            print(self.bins)
            for t,v in self.slice(self.minimum(),self.maximum()):
                wf.write("%f\t%s\n"%(t,v))



class DenseTimeInfo(TimeInfoImpl):
    """DenseTimeInfo
        index data associated with time where there are
        a large number of samples in a given range.
    """
    def __init__(self, data):
        super(DenseTimeInfo, self).__init__( data )

    def _build(self,data):
        for t,v in data:
            idx = int(t/self.resolution) - self.offset
            self.bins[idx].append( (t,v) )

    def _init_bins(self,min_time,max_time):
        self.offset = int(min_time / self.resolution)
        num_bins = int((max_time + 1)/self.resolution) - self.offset
        self.bins = [ [] for i in range(num_bins) ]

    def _slice(self,sidx,eidx,ts,te):
        if sidx < 0:
            sidx = 0

        if eidx > len(self.bins):
            eidx = len(self.bins)

        for idx in range(sidx,eidx):
            for t,v in self.bins[idx]:
                if ts <= t < te:
                    yield (t,v)

class SparseTimeInfo(TimeInfoImpl):
    """SparseTimeInfo
        index data associated with time where there are few
        samples relative to the overall duration.
    """

    def _build(self, data):
        for t,v in data:
            idx = int(t/self.resolution) - self.offset
            if idx not in self.bins:
                self.bins[idx] = []
            self.bins[idx].append( (t,v) )


    def minimum(self):
        """the smallest possible time value within the data set, in seconds"""
        return min(self.bins.keys())

    def maximum(self):
        """the largest possible time value within the data set, in seconds"""
        return max(self.bins.keys())+1

    def _init_bins(self,min_time,max_time):
        self.bins = dict()

    def _slice(self,sidx,eidx,ts,te):
        for idx in range(sidx,eidx):
            for t,v in self.bins.get(idx,[]):
                if ts <= t < te:
                    yield (t,v)



def main():
    data = [
        (-5.00,True), (5.25,True), (5.5,True), (5.75,True),
        (6.00,True), (6.25,True), (6.5,True), (7.75,True),
    ]

    ti = SparseTimeInfo( data );
    print(list(ti.slice(-6,6)))

    #data = read_ctm( "cnn.ctm" )
    #ti = DenseTimeInfo( data );
    #print(list(ti.slice(3,5)))

if __name__ == '__main__':
    main()




