"""
This module is a python wrapper for calling main function of knng.cpp
We tried to abstract away the implementation of cpp code
that is the reason why we directly call main function of the cpp program 
instead of creating a new python main and call cpp APIs
"""

import sys
import numpy as np
import os
import ctypes

def _prepare_argv(py_argv, cur_dir):
    # Remove python script path from argv
    # we don't need it for the cpp program
    argv = np.array(py_argv)[1 :].tolist()

    #concat working dir path to the first element of argv
    argv[0] = cur_dir + '/' + argv[0]

    return argv

# load rknng executable
rknng = ctypes.CDLL('./rknng')

currentDirectory = os.getcwd()
prepared_argv = _prepare_argv(sys.argv, currentDirectory)

# create type for array of char => string
LP_c_char = ctypes.POINTER(ctypes.c_char)
# Create type for array of string
LP_LP_c_char = ctypes.POINTER(LP_c_char)

rknng.main.argtypes = (ctypes.c_int, LP_LP_c_char) 

argc = len(prepared_argv)

# add +1 here for null terminator
argv = (LP_c_char * (argc + 1))()

for i, arg in enumerate(prepared_argv):
    enc_arg = arg.encode('utf-8')
    argv[i] = ctypes.create_string_buffer(enc_arg)

rknng.main(argc, argv)