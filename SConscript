# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/SConscript,v 1.23 2011/05/27 03:38:35 heather Exp $ 
# Authors: Monica Brigida <monica.brigida@ba.infn.it>, Leon Rochester <lsrea@slac.stanford.edu>, Michael Kuss <michael.kuss@pi.infn.it>
# Version: TkrDigi-02-11-00
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='TkrDigi', toBuild='component')
TkrDigi = libEnv.ComponentLibrary('TkrDigi',
                                  listFiles(['src/*.cxx','src/Bari/*.cxx',
                                             'src/Simple/*.cxx', 
                                             'src/GaudiAlg/*.cxx',
                                             'src/General/*.cxx'])) 
progEnv.Tool('TkrDigiLib')

test_TkrDigi = progEnv.GaudiProgram('test_TkrDigi',
                                    listFiles(['src/test/*.cxx']),
                                    test=1, package='TkrDigi')

progEnv.Tool('registerTargets', package = 'TkrDigi',
             libraryCxts=[[TkrDigi,libEnv]],
             testAppCxts=[[test_TkrDigi,progEnv]],
             data = listFiles(['data/*.txt']),
             jo = ['src/test/jobOptions.txt'])




