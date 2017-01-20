
import imp,os,sys,inspect

class PluginManager(object):
    """docstring for PluginManager"""

    _instance = None
    @staticmethod
    def instance():
        if not PluginManager._instance:
            PluginManager._instance = PluginManager()
        return PluginManager._instance

    def __init__(self):
        super(PluginManager, self).__init__()

        self.plugins = {}
        self.classtag = "Process"

    def load_dsp_plugin(self,filename):

        plugpath,name = os.path.split(filename)
        mod = None
        modname=os.path.splitext(name)[0]
        print("looking for plugin %s"%modname)
        try:

            f,path,desc = imp.find_module(modname,[plugpath,])
            mod = imp.load_module("plug_"+modname,f,path,desc)
        except Exception as e:
            print("plug load error for %s. "%modname,e)
            return
        finally:
            if not mod:
                return
            for name in mod.__dict__:
                cls = mod.__dict__[name]
                # take all Process classes as plugin modules
                # if they used "from x import *"" then i need to double
                # check that the class was defined in the module
                # i am currently loading. Otherwise i will duplicate
                # default processes
                if name.endswith(self.classtag) and \
                    inspect.isclass(cls) and \
                    os.path.samefile(filename , inspect.getfile(cls) ):
                    self.plugins[name] = cls
    def getPlugins(self):
        return self.plugins.values()

def loadPlugin(plug_path):
    pm = PluginManager.instance()
    pm.load_dsp_plugin( plug_path )

def getPluginProcesses():
    pm = PluginManager.instance()
    return pm.getPlugins();