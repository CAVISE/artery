"""
Find out current path to site packages.
"""

from distutils.sysconfig import get_python_lib

print(get_python_lib(standard_lib=False))
