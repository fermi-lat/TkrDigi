# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/SConscript,v 1.26 2012/01/16 21:26:26 heather Exp $ 
# Authors: Monica Brigida <monica.brigida@ba.infn.it>, Leon Rochester <lsrea@slac.stanford.edu>, Michael Kuss <michael.kuss@pi.infn.it>
# Version: TkrDigi-02-12-00
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




