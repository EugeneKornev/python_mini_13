from setuptools import Extension, setup

setup(
    name="matrix",
    version="1.0.0",
    description="Python interface for expo of the matrix",
    ext_modules=[Extension(name="matrix", sources=["matrix_power.c"])]
)