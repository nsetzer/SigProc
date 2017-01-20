

import numpy as np
try:
    from PIL import Image
except ImportError as e:
    Image = None


def LinearGainLine(x1,y1,x2,y2):
    """
    returns tuple (m,b)
    that satisfies the equation y=mx+b

    uses two points in histogram space to determine slope
    x : 0..nbins
    y : 0..255
    """
    if (x2 > x1):
        m = -(y1-y2) / (x2-x1)
    else:
        m = 0
    b  = x1
    return m,b

def AutomaticLinearGainLine(data,ratio=.025,nbins=4096):
    """ returns tuple (m,b)
    that satisfies the equation y=mx+b

    uses the data to determine the line of best fit
    """
    n = data.shape[0] * data.shape[1] * ratio
    hist,edges = np.histogram(data,nbins)
    i,j = _histminmaxidx(hist,edges,n)
    print(i,j,edges[0],edges[i],edges[j])
    return LinearGainLine(i,0,j,255)

def LinearGain(m,b,nbins):
    gain_line = [0,]*nbins
    print ("linear scale: y=(%.3f)*(x%+.3f)"%(m,-b))
    for i in range(nbins):
        v = m*(i-b)
        gain_line[i] = max(0,min(255,v));
    return gain_line


def LinearScale(data,amin,amax):
    _min = data.min()
    data = data - _min + amin;
    _max = data.max()
    scale = amax
    if _max != 0.0:
        scale /= _max;
    print("scale: y = %fx%+f"%(scale,-_min))
    data = (scale*data)
    return data

def _histminmaxidx(hist,edges,ncount):
    s=0
    for i,v in enumerate(hist):
        s += v
        if s > ncount:
            break;

    s = 0
    j = len(hist)-1
    for _ in range(len(hist)):
        v = hist[j]
        s += v
        if s > ncount:
            break;
        j -= 1
    return i,j

def _histminmax(hist,edges,ncount):
    """
    ncount: the number of pixels to count in from the edge
    find the min/max after counting in a percentage from
    the edges of a histogram to ignore outlier
    """

    i,j = _histminmaxidx(hist,edges,ncount)
    vmin = edges[i]
    vmax = edges[j]
    print(i,j)
    nmin = sum(hist[:i]) # count of values not with the range
    nmax = sum(hist[j:])

    return vmin,vmax,nmin,nmax

def HistogramClip(data,vmin=None,vmax=None,ratio=.025,nbins=4096):
    """
    compute a histogram and finds the min/max given a ratio
    of outliers to ignore. then clips values outside of the
    min/max range discovered.

    ratio: is the percentage of pixels to count in from the edges
           of the histogram. use this to ignore outliers in the data.


    """
    if vmin is None or vmax is None:
        n = data.shape[0] * data.shape[1] * ratio
        hist,edges = np.histogram(data,nbins)
        vmin,vmax,_,_ = _histminmax(hist,edges,n)

    data = np.clip(data,vmin,vmax)
    return data,vmin,vmax

def AutomaticGain(data,ratio,nbins):

    # first compute a histogram to determine the range of values
    # for the data.
    n = data.shape[0] * data.shape[1] * ratio
    hist,edges = np.histogram(data,nbins)
    #vmin,vmax,nmin,nmax = _histminmax(hist,edges,n)
    i,j= _histminmaxidx(hist,edges,n)

    # now, compute a histogram with equal bin widths across the
    # area of interest. pixels outsize the ROI are set to either 0 or 255

    hist[:i] = 0
    hist[j:] = 0
    #hist,edges = np.histogram(data,nbins,range=(vmin,vmax))
    gain = hist.cumsum()
    # ignore pixels less than the minimum, the AGC gain starts
    # at zero for the first pixel we want to consider.
    #gain -= nmin
    gain = (255.0) * gain / (gain[-1])
    # clamp the gain to an 8-bit value. this accounts for the
    # other end of the range which is now greater than 255.0
    # thanks to the previous subtraction
    np.clip(gain,0.0,255.0,gain)

    gain = np.array(gain,dtype=np.uint8)

    return hist,edges,gain;

def ApplyGain(data,edges,gain):
    # scale the data to the range 0..histsize
    # then assign each pixel the value given by the look up table
    # returns a new matrix of the same size as the input
    m = edges[0]
    M = edges[-1]
    S = M-m
    data = ((data-m)/S)*(len(gain)-1)
    data = np.array(data,dtype=np.int)
    np.clip(data,0,(len(gain)-1),data)
    return gain[data];

def AutomaticScale(data,ratio=.025,nbins=4096):

    hist,edges,gain = AutomaticGain(data,ratio,nbins)

    return ApplyGain(data,edges,gain)

def Matrix2Image(matrix,outFile):
    if Image is None:
        sys.stdout.write("PIL not supported\n")
        sys.exit(1)

    data = matrix.data

    #data,a,b = HistogramClip(data)
    #print("output min/max: %f/%f"%(a,b))
    #data = LinearScale(data,0.0,255.0)

    data = AutomaticScale(data)

    data = np.rot90(data,1)
    _max = data.max()
    _min = data.min()
    print("scaled min/max: %f/%f"%(_min,_max))
    tmp = Image.fromarray(data.astype(np.uint8))
    tmp.save(outFile)