# -*- python -*-
# $Header:$ 
# Authors: Monica Brigida <monica.brigida@ba.infn.it>, Leon Rochester <lsrea@slac.stanford.edu>, Michael Kuss <michael.kuss@pi.infn.it>
# Version: TkrDigi-02-07-02
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('TkrDigiLib', depsOnly = 1)
TkrDigi = libEnv.SharedLibrary('TkrDigi', listFiles(['src/*.cxx']) + listFiles(['src/General/*.cxx']) + \
listFiles(['src/Bari/*.cxx']) + listFiles(['src/Simple/*.cxx']) + listFiles(['src/Dll/*.cxx']) + listFiles(['src/GaudiAlg/*.cxx'])) 

progEnv.Tool('TkrDigiLib')
test_TkrDigi = progEnv.Program('test_TkrDigi', listFiles(['src/test/*.cxx']))

progEnv.Tool('registerObjects', package = 'TkrDigi', libraries = [TkrDigi], testApps = [test_TkrDigi])
