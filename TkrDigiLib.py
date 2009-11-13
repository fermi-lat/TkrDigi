# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/TkrDigiLib.py,v 1.2 2008/08/28 20:21:21 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['TkrDigi'])
    env.Tool('facilitiesLib')
    env.Tool('EventLib')
    env.Tool('GlastSvcLib')
    env.Tool('TkrUtilLib')
    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('addLibrary', library = env['clhepLibs'])
def exists(env):
    return 1;
