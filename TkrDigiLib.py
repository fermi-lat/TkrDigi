# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/TkrDigiLib.py,v 1.1 2008/07/09 21:13:42 glastrm Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['TkrDigi'])

def exists(env):
    return 1;
