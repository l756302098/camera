from catkin_pkg.python_setup import generate_distutils_setup
from setuptools import setup

setup_args = generate_distutils_setup(
    packages=['hk'],
    package_dir={'': 'src'},
    install_requires=['requests']
)

setup(**setup_args)