#ifndef _PY_HASHPIPE_H_
#define _PY_HASHPIPE_H_

#define PY_SSIZE_T_CLEAN
# include <Python.h>
#include <stdio.h>
#include <hashpipe.h>
#include <limits.h>

typedef struct {
	PyObject_HEAD
	hashpipe_status_t * status;
	int instance_id;
	char key[NAME_MAX];
} HashpipeStatusObj;

//object methods
static PyObject * Status_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int Status_init(HashpipeStatusObj *self, PyObject *args, PyObject *kwds);
static void Status_dealloc(HashpipeStatusObj * self);


//methods
static PyObject * Status_getID(HashpipeStatusObj *self, PyObject *Py_UNUSED(ignored));
static PyObject * Status_getKey(HashpipeStatusObj *self, PyObject *Py_UNUSED(ignored));
static PyObject * Status_getString(HashpipeStatusObj *self, PyObject *args);
static PyObject * Status_getDouble(HashpipeStatusObj *self, PyObject *args);

static PyObject * Status_setString(HashpipeStatusObj *self, PyObject *args);
static PyObject * Status_setDouble(HashpipeStatusObj *self, PyObject *args);
static PyObject * Status_setInt(HashpipeStatusObj *self, PyObject *args);
static PyObject * Status_setFloat(HashpipeStatusObj *self, PyObject *args);


#endif
// end
