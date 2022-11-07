/*******************************************************************************
 *
 * This file is part of RKNNG software.
 * Copyright (C) 2015-2018 Sami Sieranoja
 * <samisi@uef.fi>, <sami.sieranoja@gmail.com>
 *
 * RKNNG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. You should have received a copy
 * of the GNU Lesser General Public License along with RKNNG.
 * If not, see <http://www.gnu.org/licenses/lgpl.html>.
 *******************************************************************************/

#include <Python.h>
#include <math.h>
#include <numpy/arrayobject.h>

#include <stdio.h>

#define m(x0) (*(npy_float64 *)((PyArray_DATA(py_m) + (x0)*PyArray_STRIDES(py_m)[0])))
#define m_shape(i) (py_m->dimensions[(i)])

#define r(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_r) + (x0)*PyArray_STRIDES(py_r)[0] +                          \
                     (x1)*PyArray_STRIDES(py_r)[1])))
#define r_shape(i) (py_r->dimensions[(i)])

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

#define F(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_F) + (x0)*PyArray_STRIDES(py_F)[0] +                          \
                     (x1)*PyArray_STRIDES(py_F)[1])))
#define F_shape(i) (py_F->dimensions[(i)])

//#include "contrib/argtable3.h"

#include "timer.h"
#include "util.h"
#include "globals.h"

#include "dataset.h"
#include "knngraph.h"

kNNGraph *g_ground_truth;
#include "recall.h"

#include "rp_div.h"
#include "brute_force.h"

extern "C" {
void test_rknn_lib() {
  printf("333333\n");
  DataSet *DS = loadStringData("data/birkbeckU.txt");
  debugStringDataset(DS);
}

void printDSVec(kNNGraph *knng, int id) {
  DataSet *DS = (DataSet *)knng->DS;
  printf("id:%d [%f, %f]\n", id, DS->data[id][0], DS->data[id][1]);
}

float knng_dist(kNNGraph *knng, int p1, int p2) {
  DataSet *DS = (DataSet *)knng->DS;
  return distance(DS, p1, p2);
}

PyObject *get_knng2(PyArrayObject *py_v) {
  int N = py_v->dimensions[1];
  int D = py_v->dimensions[0];

  //Convert input from python to C data structure
  DataSet *DS = init_DataSet(N, D);
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < D; j++) {
      set_val(DS, i, j, v(i, j));
    }
  }
  
  int k = 15;
  int W = 15 * 2;
  float nndes_start = 0.2;
  float endcond = 0.05;
  kNNGraph *knng = rpdiv_create_knng(DS, DS, k, W, endcond, nndes_start, 100);

  // Convert kNN graph to python format
  PyObject *python_val = PyList_New(N);
  for (int i = 0; i < N; ++i) {

    PyObject *col = PyList_New(2);
    PyObject *nnlist = PyList_New(k);
    PyObject *distlist = PyList_New(k);

    for (int nn_i = 0; nn_i < knng->k; nn_i++) {
      int idx = (int)get_kNN_item_id(knng, i, nn_i);
      float dist = get_kNN_item_dist(knng, i, nn_i);
      PyList_SetItem(nnlist, nn_i, Py_BuildValue("i", idx));
      PyList_SetItem(distlist, nn_i, Py_BuildValue("f", dist));
    }
    PyList_SetItem(col, 0, nnlist);
    PyList_SetItem(col, 1, distlist);
    PyList_SetItem(python_val, i, col);
  }

  return python_val;
}

kNNGraph *get_knng(const char *infn, int k, int data_type, int algo, float endcond,
                   float nndes_start, int W, int dfunc) {

  g_options.distance_type = dfunc;
  DataSet *DS;
  if (data_type == 1) {
    DS = read_ascii_dataset(infn);
  } else if (data_type == 2) {
    DS = loadStringData(infn);
  } else {
    printf("Incorrect data type:%d\n", data_type);
  }
  kNNGraph *knng;

  g_timer.tick();

  if (algo == 0) {
    knng = rpdiv_create_knng(DS, DS, k, W, endcond, nndes_start, 100);
  } else if (algo == 9) {
    knng = brute_force_search(DS, k);
  }

  // debugVecGraph(DS,knng,0);
  // debugStringGraph(DS,knng,0);
  // knng->list[10].items[0].id = 23;
  knng->DS = (void *)DS;

  printf("time=%fs\n", g_timer.get_time());

  return knng;
}

float get_elapsed_time() { return g_timer.get_time(); }
}
