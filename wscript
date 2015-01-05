# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
import sys
VERSION='0.1'
APPNAME="ndnsDemo"

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['boost', 'default-compiler-flags'], tooldir=['.waf-tools'])

def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs',
               'boost', 'default-compiler-flags'])

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)


    USED_BOOST_LIBS = ['system', 'filesystem']

    conf.check_boost(lib=USED_BOOST_LIBS, mandatory=True)


def build (bld):
    bld(
        features='cxx',
        name='ndns_validator',
        source=bld.path.ant_glob(['src/**/*.cpp'],
                                 excl=['src/main.cpp',]),
        use='NDN_CXX BOOST',
        includes='src',
        export_includes='src',
    )

    for app in bld.path.ant_glob('app/**/*.cpp'):
        bld(features=['cxx', 'cxxprogram'],
            target = '../build/%s' % (str(app.change_ext('','.cpp'))),
            source = app,
            use = 'ndns_validator',
            )
