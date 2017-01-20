#! python $this
#! FOR /F "usebackq" %%i IN (`hostname`) DO SET MYVAR=%%i && echo %MYVAR%
from threading import Thread
from time import sleep

def getDisplayName(name,suffix="Process"):
    """ comvert a camel-case name to an all lower case underscore
        separated name.
    """
    if name.endswith(suffix) and suffix:
        name = name[:-len(suffix)]
    out = name[0]
    for i in range(1,len(name)):
        if name[i].isupper() and not name[i-1].isupper():
            out += '_'
        out += name[i]
    return out.lower()

#from .recipe import RecipeManager

class AsyncReturn(Thread):
    def __init__(self,callable,*args,**kwargs):
        super(AsyncReturn,self).__init__()
        self.return_value = None
        self.callable=callable
        self.args = args
        self.kwargs=kwargs

    def run(self):
        if self.callable is not None:
            self.return_value = self.callable(*self.args,**self.kwargs)

   # def join(self,timeout=None):
   #    # timeout as floating point number specifing seconds
   #    # after calling this function test isAlive()
   #    super(AsyncReturn,self).join( timeout )
   #    return self.return_value
    def getResult(self):
        return self.return_value

class ProcessRunner(object):
    def __init__(self,proc,number_updates=100):
        self.proc = proc
        # number_updates is meaning less for SimpleProcess
        self.number_updates = number_updates
        self.iter = None

    def run(self):
        if isinstance(self.proc,SimpleProcess):
            return self.runSimpleProcess(self.proc)
        else:
            return self.runIterativeProcess(self.proc)

    def runSimpleProcess(self,proc):
        # simple processes are ironically more difficult to monitor.
        # if proc has attr getProgress
        # fork the process on another thread and query
        # the progress every N seconds default: .25s
        # if the attr does not exist run the process
        # in the current thread

        if proc.supportsProgress():
            timeout = .5
            proc_thread = AsyncReturn(proc.run)
            proc_thread.start()
            while proc_thread.isAlive():
                proc_thread.join( timeout )
                try:
                    #TODO catch AttributeError only
                    m = proc.getProgress()
                    self._displayProgressProxy(*m)
                except Exception as e:
                    print(e)

            rv = proc_thread.getResult()
            del proc_thread
            return rv
        else:
            return proc.run();

    def runIterativeProcess(self,proc):
        expected_iter_count = proc.begin()
        num_updates = self.number_updates
        if isinstance(expected_iter_count,int):
            self.iter = ( _ for _ in range(expected_iter_count) )
            if expected_iter_count < num_updates:
                num_updates = expected_iter_count
        else:
            self.iter = ( _ for _ in expected_iter_count )
            expected_iter_count = len(expected_iter_count)

        if expected_iter_count==0:
            return None

        update = max(1,expected_iter_count//num_updates)
        self.doRunMain(proc,update,num_updates)
        #print("calling iter end func",proc)
        return proc.end()

    def doRunMain(self,proc,update,num_updates):
        progress = 0
        while True:

            for _ in range(update):
                idx=None
                try:
                    idx = next(self.iter)
                except StopIteration:
                    return

                if proc.step( idx ) is False:
                    return
            progress += 1
            percent = min(1.0,progress/float(num_updates))
            self._displayProgressProxy( percent, proc.status() )
        # if we still have not yet returned, then process the remaining data.
        return

    def _displayProgressProxy(self,percent,message=None):
        """ proxy function for altering the percent or message
            before sending the values on to display progress
        """
        self.displayProgress(percent,message)

    def displayProgress(self,percent,message=None):
        """
        reimplement to update your gui, or write
        to stdout, or somehow update the user
        """
        #ostr = "progress: %d"%int(percent)
        #if message:
        #    ostr += "\t"+ message
        #print ostr
        pass

class PipelineProcessRunner(ProcessRunner):
    """ PipelineProcessRunner can run a chain of ProcessBase objects
        by constructing them at runtime, and passing the output of
        previous steps as input into the next step.
    """
    def __init__(self,proc_list,input=None,number_updates=100):
        super(PipelineProcessRunner, self).__init__(None,number_updates)
        self.proc_list = proc_list
        self.input = input
        self.step=0

    def run(self, input = None):

        # add the input to the first setting dictionary
        #proc0,dict0=self.proc_list[0]
        #dict0 = dict(dict0)
        #opt=ProcessBase.getInputSpecifier(proc0)
        #dict0[opt.store]=self.input
        #self.proc_list[0] = proc0,dict0
        #print(dict0)
        outlst = []
        # take input either from the argument to this
        # class, or as an argument from the class constructor
        output = input or self.input
        self.step = 0
        for proc_t,opt_dict in self.proc_list:
            opts = dict(opt_dict) # copy it
            if output :
                opt = ProcessBase.getInputSpecifier(proc_t)
                if opt:
                    opts[opt.store] = output
                else:
                    print("proc has no input specifier")
            sopts = ''.join(["\t%s:%s\n"%(k,v) for k,v in sorted(opts.items())])
            print("Running %s:\n%s"%(proc_t.__name__,sopts))
            # create the proc object from the new input and
            # the pre-defined options.
            proc = proc_t(opts)
            #print
            if isinstance(proc,SimpleProcess):
                output = self.runSimpleProcess(proc)
            else:
                output = self.runIterativeProcess(proc)

            #if idx in self.output_index:
            outlst.append(output)
            self.step += 1

        #outlst.append(output)
        return outlst

    #def _displayProgressProxy(self,percent,message=None):
    #    percent = (self.step + percent)/len(self.proc_list)
    #    message = "%s: %s"%(self.proc_list[self.step][0].__name__,message)
    #    self.displayProgress(percent,message)

class ProcessOptionsSpecifier(object):
    """ specifies a command line option"""
    def __init__(self,**kwargs):
        self.data={}

        self.data["store"] = kwargs["store"]            # internal storage name
        self.data["name"]  = kwargs.get("name",self.data["store"]) #display name

         # dtype is the data type, int, float, bool, str, etc
        if "dtype" not in kwargs and "default" in kwargs:
            self.data["dtype"] = type( kwargs.get("default") )
        else:
            self.data["dtype"] = kwargs.get("dtype",int)

        self.data["group"] = kwargs.get("group", None)  # control is part of
                                                        # a logical group
                                                        # for display purposes

        self.data["min"] = kwargs.get("min",None)   # constrain value to
        self.data["max"] = kwargs.get("max",None)   #   min/max,
        self.data["step"] = kwargs.get("step",None) # 'step' is a suggested
                                                    # increment value for
                                                    # spin boxes.

        # precision is used for floating point spin boxes.
        self.data["precision"] = kwargs.get("precision",3)

        self.data["prefix"] = kwargs.get("prefix","") # 0x o etc
        self.data["suffix"] = kwargs.get("suffix","") # Hz, lbs, etc
        self.data["default"] = kwargs.get("default",None)
        # _default should be a callable taking a single dicitonary of values
        self.data["_default"] = kwargs.get("_default",None)


        self.data["options"] = kwargs.get("options",[]) # provide a constraint,
                                                # that is, value must be one
                                                # of the options listed here.
                                                # handy for drop down menus
        self.data["input"] = kwargs.get("input",False)
        self.data["output"] = kwargs.get("output",False)
        self.data["required"] = self.output==False and \
                                self.default==None and \
                                kwargs.get("required",True) != False
        self.data["help"] = kwargs.get("help",None)

    def __str__(self):
        return "%s<%s>"%(self.__class__.__name__,str(self.data))
    def __repr__(self):
        return "%s<%s>"%(self.__class__.__name__,str(self.data))
    def __getattr__(self,key):
        if key in self.data:
            return self.data[key]
        raise AttributeError(key)
    def format(self,pad=15):
        """
            generate a string representation of the options specifier
            pad controls how many characters to use when displaying the
            variable name. set to 0 to display only the help message.
        """
        help=""
        if pad > 4:
            _pad = " %%-%ds:"%(pad-1)
            if self.input or self.output:
                help = _pad%(self.name)
            elif self.required :
                help = _pad%(self.store)
            else:
                help = _pad%("["+self.store+"]")
        if hasattr(self.dtype,"__name__"):
            help += " as type %s"%self.dtype.__name__
        if self.default is not None:
            p = "" if not self.prefix else "%s "%self.prefix
            s = "" if not self.suffix else " %s"%self.suffix
            help += " (%s%s%s)"%(p,self.default,s)

        if self.min != None or self.max!=None:
            help += " "
            help += "[%s.."%self.min if self.min != None else "[.."
            help += "%s]"%self.max if self.max != None else "]"
        #if self.group: # grp is internal so i dont need to display it
        #    help += " grp:%s"%self.group
        # reformat the given help msg so that it fits
        # onto an 80 character terminal nicely.
        if self.help:
            help += str_fmt_width(self.help,pad,65)
        return help

    def parse(self,sValue):
        if self.dtype==bool:
            return sValue.lower()=="true"
        else:
            return self.dtype( sValue )

class DynamicOptionsSpecifier(ProcessOptionsSpecifier):
    def __init__(self,**kwargs):
        super(DynamicOptionsSpecifier,self).__init__(**kwargs)
        # when any field in triggers is updated build a namespace object
        # of the current form. for each update rule update the value
        # of the variable based on the result from the update function
        # so for example we can adjust the min/max range dynamically
        # DynamicOptionsSpecifier can have defaults but they are likely to
        # be ignored. the Qt Menu generator calls all of the update functions
        # after creating the menu to initialize the defaults.
        self.triggers = kwargs.get("triggers",[])
        if isinstance(self.triggers,str):
            self.triggers = [self.triggers,]
        self.update_rules = []
        for key,_callable in kwargs.items():
            if key.startswith("update_"):
                update_value = key[len("update_"):]
                self.update_rules.append( (update_value,_callable) )
    def update(self,nspace):
        for var,_callable in self.update_rules:
            self.data[var] = _callable(nspace)

    def getUpdateRule(self):
        for name,rule in self.update_rules:
            if name == "default":
                return rule
        return None;

class MetaSpecifier(ProcessOptionsSpecifier):
    """SummarySpecifier should be a one or two line SummarySpecifier
        of what the process is and does.
    """
    def __init__(self, msg):
        self.name=""
        self.msg = msg
    def __getattr__(self,key):
        return None
    def format(self,pad=15):
        return ""

class SummarySpecifier(MetaSpecifier):
    """SummarySpecifier should be a one or two line SummarySpecifier
        of what the process is and does.
    """
    def __init__(self, msg):
        self.name=""
        self.msg = msg
    def __getattr__(self,key):
        return None
    def format(self,pad=15,width=65):
        return str_fmt_width(self.msg,pad,width-pad) + "\n"

class HiddenSpecifier(MetaSpecifier):
    """
        HiddenSpecifier is the same as ProcessOptionsSpecifier
        but the information should not normally be given to the user
    """
    pass

class LabelSpecifier(MetaSpecifier):
    """
    """
    def __init__(self, msg):
        self.name=""
        self.msg = msg
    def __getattr__(self,key):
        return None
    def format(self,pad=None):
        return self.msg

class DynamicLabelSpecifier(MetaSpecifier):
    """
    """
    def __init__(self, _callable):
        self.name=""
        self._callable = _callable
        self.msg = ""
    def __getattr__(self,key):
        return None
    def format(self,pad=None):
        return self.msg

def str_fmt_width(string,pad,width):
    """
        util function for breaking a string on spaces
        while trying to keep lines less than width.
        pad, when non zero inserts that number of spaces
        on the left side of the formated string.
    """

    base = "\n"
    if pad > 1:
        base = "\n" +" "*pad +":"
    words = string.split()
    out = ""
    temp = base
    for word in words:
        test = temp + " " + word
        if len(test)>width-pad:
            out += test
            temp = base
        else:
            temp = test
    if temp != base:
        out += temp
    return out

def formatOptions(options):
    # options can be an option list
    # a Process type or instance of a Process Type.
    if isinstance(options,type):
        options = options.getOptions()
    elif hasattr(options,"getOptions"):
        options = options.getOptions()

    pad = 0
    for opt in options:
        pad = max(pad,len(opt.name))
    out = ""
    p_inp = False
    p_out = False
    p_opt = False
    for opt in options:
        if not p_inp and opt.input:
            out += "Input: \n"
            p_inp = True
        elif not p_out and opt.output:
            out += "Output: \n"
            p_out = True
        elif not p_opt and p_out:
            out += "Options: \n"
            p_opt = True
        out += opt.format(pad+4) + "\n"
        if opt.input or opt.output:
            out += "\n"
    return out

class ProcessBase(object):
    def __init__(self,options=None):
        #self.data = data

        if options is None:
            self.options = {}
        else:
            self.options = options

        # check for missing options, filling in defaults
        # when possible. raise ValueError if an option cannot be found
        for opt in self.getOptions():
            if opt.store not in self.options:
                if opt.default!=None:
                    self.options[opt.store] = opt.default
                elif opt.required:
                    raise ValueError(\
                    "Key '%s' not found in input options for %s"%(
                        opt.store,self.__class__.__name__))
    def __getattr__(self,attr):
        if attr in self.options:
            return self.options[attr]
        return None
    @staticmethod
    def getInputSpecifier(proc):
        for opt in proc.getOptions():
            if opt.input:
                return opt
        return None
    @staticmethod
    def getOutputSpecifier(proc):
        for opt in proc.getOptions():
            if opt.output:
                return opt
        return None
    @staticmethod
    def getSummarySpecifier(proc):
        for opt in proc.getOptions():
            if isinstance(opt,SummarySpecifier):
                return opt
        return None
    @staticmethod
    def getOptions():
        """returns a list of ProcessOptionsSpecifier"""
        return []
    @staticmethod
    def getDefaultDict(proc):
        opts = proc.getOptions()
        dct = {}
        for opt in opts:
            if isinstance(opt,MetaSpecifier) or opt.input or opt.output:
                continue
            elif opt.default != None:
                dct [opt.store] = opt.default
            else:
                dct[opt.store] = opt.dtype()
        return dct

class SimpleProcess(ProcessBase):
    """
        An unmanaged process
        displayProgress must be called to display any progress updates.
        member variable number_updates is a hint for how many times to call
        displayProgress while running.
        once started this process cannot be stopped.
    """
    def getProgress(self):
        """
            read only "const" function only queries the state or
            returns -1
        """
        return (-1,"")

    def supportsProgress(self):
        """ return true to enable progress reporting"""
        return False

    def run(self):
        pass

class IterativeProcess(ProcessBase):
    """ template class for a process that
        does work in small batches
        because step() is meant to do a small amount of work
            any process implemented using this class can be stopped early.
    """

    def begin(self):
        """
            begin should perform the initialization
            It should return an estimate for the number of times
            step() will need to be called.
            return 0, or negative will be interpreted as "no estimate"
        """
        return 0
    def step(self,idx):
        """
            step should perform a small amount of work. until all of the
            input data has been consumed. When finished return False
        """
        return False
    def end(self):
        """
            end is called after step() returns false.
            Anything returned from here will be returned to the caller
            of the process.
        """
        return None
    def status(self):
        """
            status can return a short string describing what is going on.
        """
        return ""

class SimpleCountingProcess(SimpleProcess):
    """docstring for InstrumentCountingProcess"""
    def run(self):
        self.count=self.start
        update = max(1,self.target // 10)
        while self.count < self.target:
            if self.count%update==0:
                sleep(.5) # sleep for demo purposes
            self.count += self.increment

        return self.count

    def getProgress(self):
        #TODO catch AttributeError only
        return (float(self.count)/self.target,"%s/%s"%(self.count,self.target))

    @staticmethod
    def getOptions():
        """
        returns a list of ProcessOptionsSpecifier
        """
        opts = [ \
                ProcessOptionsSpecifier(input=True,store="start",default=0,dtype=int), \
                ProcessOptionsSpecifier(output=True,store="",dtype=int), \
                ProcessOptionsSpecifier(store="target",default=100), \
                ProcessOptionsSpecifier(store="increment",default=1), \
               ]
        return opts

class IterativeCountingProcess(IterativeProcess):
    """
        Example for an IterativeProcess
        It counts to a specified number, in an iterative way so that
        a controlling process could display the progress to the user.
    """
    def begin(self):
        self.count = self.start
        # returns approximately how many times step() must
        # be called for the process to finish.
        # this value need only be an estimate.
        return int(self.target-self.start / self.increment)

    def step(self,idx):

        if self.count < self.target:
            self.count += self.increment
            return True
        return False

    def end(self):
        return self.status()

    def status(self):
        return self.count

    @staticmethod
    def getOptions():
        """
        returns a list of ProcessOptionsSpecifier
        """
        opts = [ \
                ProcessOptionsSpecifier(input=True,store="start",default=0,dtype=int), \
                ProcessOptionsSpecifier(output=True,store="",dtype=int), \
                ProcessOptionsSpecifier(store="target",default=100,
                    help="the value to count to."), \
                ProcessOptionsSpecifier(store="increment",default=1,
                    help="how much to increment the count by at each step") \
               ]
        return opts

if __name__ == "__main__":
    import sys
    #import cProfile
    ##pr = ProcessRunner( IterativeCountingProcess( \
    ##    {"target":10,"increment":1}) , number_updates = 5 )
    ##pr.displayProgress = lambda p,m : sys.stdout.write("%.2f\n"%p)
    ##print(pr.run())
    ###cProfile.run("pr.run()")
##
    ### if a default is set we can just create and run the process
    ##pr = ProcessRunner( SimpleCountingProcess() )
    ##pr.displayProgress = lambda p,m : sys.stdout.write("%.2f %s\n"%(p,m))
    ##print(pr.run())
    #cProfile.run("pr.run()")

    #print ProcessOptionsSpecifier(store="value").opts()
    #print IterativeCountingProcess.getOptions()

    #print pr.run()

    #pr = ProcessRunner( SimpleCountingProcess({},\
    #    {"target":1000} ), number_updates = 15 )
    print(formatOptions(SimpleCountingProcess))
    print(formatOptions(IterativeCountingProcess()))
