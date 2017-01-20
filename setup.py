#! python setup.py install

from setuptools import setup, Command

entry_points = [ "sigproc = SigProc:sigproc_main", ]

setup(name='SigProc',
      version='1.0',
      description="process manager",
      packages=['SigProc'],
      entry_points={"console_scripts":entry_points},
      )
