

# todo this needs to be made a class
# which accepts:
# - a list of input files
# - a function to convert inputfiles to features
# - a function to open feature files.

def normalize_matrix_main(mat_fpaths,ndim,resolution):
    # NOTE: this could be modified for N simultaneous processes
    # using the subprocess module in python. e.g. 2 processes,
    # give each half of the values. then join results
    # and compute final mean.

    #NOTE: if not enough samples is small, and resolution is large
    # the percent error from the true mean/variance will increase.
    # best to use resolution=1000 (10 seconds) when num_samples
    # if much much greater, (1 hour).

    # https://en.wikipedia.org/wiki/Pooled_variance

    zm = np.zeros( ndim )
    zv = np.zeros( ndim )
    mdata = [ [] for i in range(ndim) ]
    vdata = [ [] for i in range(ndim) ]
    dslice = [ np.empty( resolution ) for i in range(ndim) ]
    c = 0
    total_samples = 0
    for f in mat_fpaths:
        #m = os.path.splitext(f)[0] + '.mat'
        mat = SigProc.Matrix.fromFile(f)
        #mat = open_superflux_feature_matrix(f,m,procs)
        # compute the mean over all files in each dimension

        nsamps = mat.shape[0]
        total_samples += nsamps
        for p in range(0,nsamps,resolution):
            a = min(resolution,nsamps - p) # available samples this pass
            for k,dim in enumerate(dslice):
                r = min(a,resolution - c) # samples to append use
                dim[c:c+r] = mat.data[p:p+r:,k]
                _c = c
                c = c+r # total count after update
                if c == resolution:
                    # compute mean/var given 'resolution' samples
                    mdata[k].append( np.mean(dim) )
                    vdata[k].append( np.std( dim) )
                    c = a - r # set to number of unused samples this pass
                    if c > 0:
                        dim[0:c] = mat.data[p+r:p+r+c,k]
                else:
                    dim[_c:_c+r] = mat.data[p:p+r:,k]

    mout = [ np.mean(x) for x in mdata ]
    vout = [ np.mean(x) for x in vdata ]
    print("unused",c,total_samples)
    return mout,vout

def normalize_matrix(files,ndim,resolution=1000):

    mdata,vdata = normalize_matrix_main(files,ndim,resolution)
    # compute final mean and variance
    # in each array, mdata and vdata each value corresponds
    # to the mean and variance over 'resolution' samples.
    # the approximate total mean and variance is simply the average value.
    mout = [ np.mean(x) for x in mdata ]
    vout = [ np.mean(x) for x in vdata ]

    return mout,vout