import os
from setuptools import setup
import re

# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

def extract_version():
    with open('version.h') as inf:
        lines = inf.readlines()

    ver_map = {'MAJOR': 0,
               'MINOR': 0,
               'BUGFIX': 0}
    for l in lines:
        match = re.match(r'#define VER_(MAJOR|MINOR|BUGFIX) (\d+)', l)
        if match:
            field, ver = match.groups()
            ver_map[field] = ver

    return '.'.join([ver_map['MAJOR'],
                     ver_map['MINOR'],
                     ver_map['BUGFIX']])

setup(
    name = "asl_f4_loader",
    version = extract_version(),
    author = "Jeff Ciesielski",
    author_email = "jeff@autosportlabs.com",
    description = ("A library and shell script to perform firmware updates on Cortex M4 Microcontrollers. (Currently: STM32F4)"),
    license = "GPL2",
    keywords = "firmware bootloader stm32f4",
    url = "https://github.com/autosportlabs/ASL_F4_bootloader/tree/master/host_tools",
    packages=['asl_f4_loader'],
    package_dir={'asl_f4_loader': 'host_tools',},
    classifiers=[
        "Development Status :: 4 - Beta",
        "Topic :: Utilities",
        "License :: OSI Approved :: GPLv2 License",
    ],
    entry_points = {
        'console_scripts': [
            'asl_f4_loader = asl_f4_loader.fw_update:main',
            'asl_f4_fw_postprocess = asl_f4_loader.fw_postprocess:main'
        ]
    },
    install_requires=[
        'ihextools >= 1.1.0',
        'pyserial >= 2.7',
        'crcmod >= 1.7',
        'XBVC >= 0.0.1'
    ]
)
