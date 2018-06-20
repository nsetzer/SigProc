

### About

SigProc is a python package and command line tool for facilitating creation of features from audio files. SigProc can be used to generate  MFCC, SDC, and Chromagram features. It supports opening wav and HTK PLH file formats, with ffMpeg or Sox any audio file can be opened.

This tool is written for learning, not performance. However it is very quick to generate features, even on large data sets. It is also an experiment/exercise in self documenting code.

SigProc operates on "recipes" which are composed of several building block "processes". Processes are connected in a Pipeline chain. Inputs are automatically connected to outputs of a previous process by the Pipeline Process Runner.

The primary inter-process data types are 'Signal', 'Matrix', and 'Track'. a Signal is  1 dimensional representation of a (mono) audio signal. A Matrix is a 2 dimensional representation, for example a spectrogram. Finally, a 'Track' contains sparse information associated with time. All formats keep track of metadata that was used to generate the object. For example a representation of a spectrogram keeps track of the parameters used, so that the frequency bins can be computed.

### Requirements

Python 3.5 or greater ( may work for 2.7, 3.4 )

requires :   numpy scipy
optional :   PIL (pillow), matplotlib

### Installation

```bash
python setup.py install
```


### Examples

To generate documentation for all processes, execute:

```bash
sigproc --doc > sigproc.txt
```

To generate an image representation of the spectrogram at 100 frames per second, execute:

```bash
sigproc --dir=./recipes spectrogram16k100 in.mp3 out.png
```

![Spectrogram](https://github.com/nsetzer/SigProc/raw/master/img/spec.png "Spectrogram")

To generate an image representation of MFCC features, execute:

```bash
sigproc --dir=./recipes mfcc_16k in.mp3 out.png
```

![MFCC](https://github.com/nsetzer/SigProc/raw/master/img/mfcc.png "MFCC")

---

Example for using SigProc as a python package. Demonstrates loading recipes,
generating the pipeline, and running the pipeline on a single input file.

```python
import SigProc

inFile = "in.mp3"
recipe_directory = "./recipes"
recipe = "spectrogram16k100"

# load the specified recipe
rm = SigProc.newRecipeManager(recipe_directory);
procs = rm.getRecipe(recipe)

# create a process runner for the pipeline
pipe = SigProc.PipelineProcessRunner(procs);

# execute, returns instance of SigProc.SpectralMatrix()
result = pipe.run(inFile)[-1]
```

