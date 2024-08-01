# SPDX-FileCopyrightText: 2022 Manuel Weiss <manuel.weiss@bht-berlin.de>
#
# SPDX-License-Identifier: MIT

# https://stackoverflow.com/a/60740179
# (note that even with pyproject.toml this is still useful to make `python setup.py sdist` work out-of-the-box)
from setuptools import dist

dist.Distribution().fetch_build_eggs(["Cython", "numpy"])

import site
import sys

import numpy as np
from Cython.Build import cythonize
from setuptools import Extension, find_packages, setup

# workaround for develop mode (pip install -e) with PEP517/pyproject.toml cf. https://github.com/pypa/pip/issues/7953
site.ENABLE_USER_SITE = "--user" in sys.argv[1:]


ext_modules = cythonize(
    Extension(
        "pneu_arm.pneu_arm",
        sources=[
            "pneu_arm/pneu_arm.pyx",
        ],
        extra_compile_args=["-O3", "-pipe", "-v"],
        language="c++",
    )
)


for m in ext_modules:
    m.include_dirs.insert(0, np.get_include())

setup(
    name="pneu_arm",
    version="0.0.1",
    description="Python package for the PneuArm",
    author="Manuel Weiss",
    author_email="manuel.weiss@bht-berlin.de",
    license="MIT",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
    ],
    packages=find_packages(exclude=["contrib", "docs", "tests"]),
    package_data={
        "pneu_arm": [
            "cpp/*.cpp",
            "cpp/*.hpp",
            "cpp/CMakeLists.txt",
        ]
    },
    zip_safe=False,
    install_requires=["numpy >= 1.23.2", "cython", "setuptools>=65.0.2"],
    python_requires=">=3.7",
    extras_require={
        # pip3 install --user -e ".[dev]"
        "dev": [],
    },
    ext_modules=ext_modules,
    include_dirs=[
        np.get_include(),
    ],
)
