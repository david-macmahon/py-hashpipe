from distutils.core import setup, Extension
import sysconfig
import os
import version

NAME = 'pyhashpipe'
DESCRIPTION = 'python hashpipe interface'
URL = 'https://github.com/david-macmahon/py-hashpipe'
EMAIL = 'kulpaj.dev@gmail.com'
AUTHOR = 'Janusz S. Kulpa'
VERSION = version.__version__


here = os.path.abspath(os.path.dirname(__file__))


try:
    with open(os.path.join(here, 'README.md')) as readme:
        # long_description = readme.read().split('\n')[2]
        long_description = '\n{}'.format(readme.read())
except Exception as exc:
    # Probably didn't find the file?
    long_description = DESCRIPTION


# extra_compile_args = sysconfig.get_config_var('CFLAGS').split()
extra_compile_args = ['-O2', '-Wall', '-Werror']
py_hashpipe_ext = Extension(
    'pyhashpipe',
    libraries = [':libhashpipestatus.a',':libhashpipe.a','m','rt'],
    library_dirs = ['src','/usr/local/lib'],
    include_dirs=['src', '/usr/local/include'],
    language='c',
    extra_compile_args=extra_compile_args,
    sources=['src/py_hashpipe.c'],
    #extra_link_args=['-lhashpipe'],
)


setuptools.setup(
    name=NAME,
    version=VERSION,
    description=DESCRIPTION,
    author=AUTHOR,
    author_email=EMAIL,
    url=URL,
    license='GNU GPLv2',
    long_description=long_description,
    long_description_content_type='text/markdown',
    # Specify version in-line here
    ext_modules=[py_hashpipe_ext],
    # Required for PyPI
    keywords='hashpipe pyhashpipe',
)

# end
