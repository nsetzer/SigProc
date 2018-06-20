#! python3 $this
import os,sys,codecs
import argparse

if sys.version_info[0]<3:
    from ConfigParser import ConfigParser,NoSectionError
else:
    from configparser import ConfigParser,NoSectionError

from .process import getDisplayName, DynamicOptionsSpecifier

class RecipeError(Exception):
    pass

class Recipe(object):
    """docstring for Recipe"""
    def __init__(self, filepath=None):
        super(Recipe, self).__init__()
        self.config = ConfigParser()
        self.filepath=filepath

        if filepath and os.path.exists(filepath):
            self.config.read(filepath)

    def name(self):
        # todo use the procman name formatter.
        if self.filepath:
            filename = os.path.splitext(os.path.split(self.filepath)[1])[0]
            return getDisplayName(filename)
        return ""

    def getProcess(self):
        kv = self.config.items("process")
        kv.sort(key=lambda x:int(x[0]))
        return [ v for _,v in kv ]

    def readSection(self,section):
        try:
            kv = self.config.items(section)
            return { k:v for k,v in kv }
        except NoSectionError as e:
            return {}

    def save(self,filepath=None):
        if filepath:
            self.filepath=filepath
        if self.filepath:
            with open(self.filepath, 'wb') as wf:
                config.write(wf)
        else:
            raise ValueError()

    def getSectionProcessName(self,section):
        """
            by convention the name of a process is the name of the section
            but to be able to have multiple processes in the same file with
            different parameters we check the field dsp_proc for the true
            name if that field exists

        """
        kv = self.readSection(section)
        if "dsp_proc" in kv:
            return kv["dsp_proc"]
        return section

    def getDSPSettings(self,section, dsp_proc):
        """
            for a DSP process, return a dictionary containing
            the settings the recipe section specifies.
            the value for each field will be the default as specified
            by the process, but the value in the config section will override
            the default value.
        """
        info = self.readSection(section)
        opts = dsp_proc.getOptions()
        out = {}
        dynopts = []

        for opt in opts:
            if opt.input or opt.output or not opt.store:
                continue
            out[opt.store] = opt.default

            svalue = info.get(opt.store.lower(),None)

            if svalue is None and opt.required:
                raise RecipeError(\
                    "required setting '%s' not found " \
                    "in recipe %s for section %s"% \
                    (opt.store.lower(),self.filepath,section) )

            if svalue is not None:
                out[opt.store] = opt.parse(svalue)
            else:
                # option was not given by the recipe
                if isinstance(opt,DynamicOptionsSpecifier):
                    print("recipe found dynamic option sepecifier")
                    dynopts.append(opt)

        # all this to get the default window size set correctly
        for opt in dynopts:
            # if we have a dynamic specifier that has
            # an update rule for default values
            rule = opt.getUpdateRule()
            print("dynupdate",opt.name,opt.default)
            if rule is not None:
                out[opt.store] = rule(out)
        return out

class RecipeManager(object):
    def __init__(self):
        # map of opened recipe objects
        self.recipes = {}
        # map of registered processes
        self.process_dict = {}

    def loadRecipe(self,filepath):
        recipe = Recipe(filepath)
        name = recipe.name()
        print("loaded %s."%name)
        self.recipes[name] = recipe
        return name

    def saveRecipe(self,filepath,procs):
        with codecs.open(filepath,"w","utf-8") as wf:
            self._recipe_to_stream(wf,procs)

    def _recipe_to_stream( self, stream, procs ):

        for proc,opts in procs:
            name=getDisplayName(proc.__name__)
            stream.write( "[%s]\n"%name )
            for k,v in opts.items():
                stream.write( "%s=%s\n"%(k,v) )
            stream.write("\n")

    def setProcessDict(self,d):
        self.process_dict = d

    def getAllRecipes(self):
        return sorted(self.recipes.keys())

    def hasRecipe(self,recipe_name):
        return recipe_name in self.recipes;

    def getRecipe(self,recipe_name,open_sub_recipes=True,removeIngest=False):
        """
            return a list of DSP processes and the options
            needed to run them.
        """

        if recipe_name not in self.recipes:
            raise KeyError("No Such Recipe: %s"%recipe_name)

        recipe = self.recipes[recipe_name]
        recipe_opts = []
        print("load recipe: %s"%recipe_name)
        for idx,section in enumerate(recipe.getProcess()):
            print("%d : %s"%(idx,section))

            if removeIngest and section == "ingest":
                continue

            process_name = recipe.getSectionProcessName(section)

            if process_name in self.process_dict:
                proc = self.process_dict[process_name]
                recipe_opts.append(
                        ( proc,
                        recipe.getDSPSettings( section, proc ) )
                    )
            elif process_name in self.recipes:
                if open_sub_recipes:
                    recipe_opts += self.getRecipe(process_name)
                else:
                    recipe_opts.append( (None,{"recipe":process_name}) )
            else:
                raise RecipeError("no process or recipe found for section name '%s'"%section)
        return recipe_opts
