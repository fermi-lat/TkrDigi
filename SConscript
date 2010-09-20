# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/SConscript,v 1.15 2009/11/13 01:40:18 jrb Exp $ 
# Authors: Monica Brigida <monica.brigida@ba.infn.it>, Leon Rochester <lsrea@slac.stanford.edu>, Michael Kuss <michael.kuss@pi.infn.it>
# Version: TkrDigi-02-10-05-gr01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('TkrDigiLib', depsOnly = 1)
TkrDigi = libEnv.SharedLibrary('TkrDigi',
                               listFiles(['src/*.cxx','src/Bari/*.cxx',
                                          'src/Simple/*.cxx', 'src/Dll/*.cxx',
                                          'src/GaudiAlg/*.cxx',
                                          'src/General/*.cxx'])) 
progEnv.Tool('TkrDigiLib')
test_TkrDigi = progEnv.GaudiProgram('test_TkrDigi',
                                    listFiles(['src/test/*.cxx']), test=1)

progEnv.Tool('registerTargets', package = 'TkrDigi',
             libraryCxts=[[TkrDigi,libEnv]],
             testAppCxts=[[test_TkrDigi,progEnv]])




