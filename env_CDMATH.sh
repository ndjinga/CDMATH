#!/bin/bash

export CDMATH_INSTALL=@CMAKE_INSTALL_PREFIX@
export PETSC_DIR=@PETSC_DIR@
export PETSC_ARCH=@PETSC_ARCH@
export PETSC_INCLUDES=@PETSC_INCLUDES_PATH@
export PETSC_LIBRARIES=@PETSC_LIBRARIES@
export MEDCOUPLING_INSTALL_PYTHON=@MEDCOUPLING_INSTALL_PYTHON@
export PV_LIB_DIR=@PV_LIB_DIR@
export PV_PYTHON_DIR=@PV_PYTHON_DIR@

export LD_LIBRARY_PATH=$CDMATH_INSTALL/lib:$CDMATH_INSTALL/lib/medcoupling:$CDMATH_INSTALL/lib/med:$PETSC_DIR/$PETSC_ARCH/lib:$PETSC_DIR/lib:$PETSC_LIBRARIES:$PV_LIB_DIR:${LD_LIBRARY_PATH}
export PYTHONPATH=$CDMATH_INSTALL/lib/cdmath:$CDMATH_INSTALL/bin/cdmath:$CDMATH_INSTALL/bin/cdmath/postprocessing:$CDMATH_INSTALL/$MEDCOUPLING_INSTALL_PYTHON:$PV_PYTHON_DIR:${PYTHONPATH}

