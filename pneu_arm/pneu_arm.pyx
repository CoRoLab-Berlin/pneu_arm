# SPDX-FileCopyrightText: 2024 Manuel Weiss <manuel.weiss@bht-berlin.de>
#
# SPDX-License-Identifier: MIT

# distutils: language = c++
# cython: language_level=3
# cython: embedsignature=True
# distutils: sources = pneu_arm/cpp/comm.cpp
# distutils: undef_macros = NDEBUG

import numpy as np

from libcpp cimport bool
from libc.string cimport memcpy
cimport numpy as np
cimport cython


ctypedef double pneu_arm_t


cdef extern from 'cpp/comm.h':
    cdef cppclass C_Comm 'Comm':
        C_Comm() except +

        int InitComm()
        void CommGetDataFromTarget(double stateBuffer[])
        void CommSendDataToTarget(double commands[])


    
cdef class PneuArm:
    TG_IN_CHANNELS = 9
    TG_OUT_CHANNELS = 4
    cdef C_Comm* c_obj


    def __cinit__(self):
        self.c_obj = new C_Comm()

    def __init__(self):
        res = self.c_obj.InitComm()
        assert res >= 0, "InitComm failed"

    def __dealloc__(self):
        del self.c_obj

    def CommGetData(self):
        cdef np.ndarray[double, ndim=1, mode='c'] vin = np.empty(shape=(self.TG_IN_CHANNELS,), dtype=np.double)
        self.c_obj.CommGetDataFromTarget(<double*> np.PyArray_DATA(vin))
        return vin
    
    

    def CommSendData(self, vout):
        assert vout.shape == (self.TG_OUT_CHANNELS,)
        self.c_obj.CommSendDataToTarget(<double*> np.PyArray_DATA(vout))
