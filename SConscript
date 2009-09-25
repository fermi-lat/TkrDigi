# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/SConscript,v 1.3 2008/09/01 10:30:13 glastrm Exp $ 
# Authors: Monica Brigida <monica.brigida@ba.infn.it>, Leon Rochester <lsrea@slac.stanford.edu>, Michael Kuss <michael.kuss@pi.infn.it>
# Version: TkrDigi-02-07-03-gr1
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('TkrDigiLib', depsOnly = 1)
TkrDigi = libEnv.SharedLibrary('TkrDigi', listFiles(['src/*.cxx','src/General/*.cxx','src/Bari/*.cxx','src/Simple/*.cxx',
	'src/Dll/*.cxx','src/GaudiAlg/*.cxx'])) 

progEnv.Tool('TkrDigiLib')
test_TkrDigi = progEnv.Program('test_TkrDigi', listFiles(['src/test/*.cxx']))

progEnv.Tool('registerObjects', package = 'TkrDigi', libraries = [TkrDigi], testApps = [test_TkrDigi])
