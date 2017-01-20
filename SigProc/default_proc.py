

from . import *
from .process import *

import numpy as np
import scipy.signal

class NewProcess(IterativeProcess):
    def begin(self):
        return 0
    def step(self,idx):
        return True
    def end(self):
        return None
    @staticmethod
    def getOptions():
        opts = [
                SignalInSpecifier(),
                SignalOutSpecifier(),
                ProcessOptionsSpecifier(store="value",dtype=int, \
                    min=0,max=100,default=0, \
                    help="fixme"),
               ]
        return opts
