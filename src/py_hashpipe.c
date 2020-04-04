#define PY_SSIZE_T_CLEAN
# include <Python.h>
#include <stdio.h>
#include <hashpipe.h>
#include "py_hashpipe.h"

#define KW_LEN_MAX 80

static char module_docstring[] =
    "module to communicate with hashpipe status buffer";
static char version_docstring[] =
    "Get the hashpipe version";

static PyObject *py_hashpipe_version(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] = {
    {"getversion", py_hashpipe_version, METH_NOARGS, version_docstring},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION < 3
#error Python 2 is not supproted
#endif

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"py_hashpipe",
	module_docstring,
	-1,
	module_methods,
};

//static PyObject *hashpipeError = NULL;

static PyMethodDef Status_methods[] = {
	{"getid", (PyCFunction) Status_getID, METH_NOARGS,
		"Get instance ID used to connect to status buffer. Function takes no arguments"
	},
	{"getkey", (PyCFunction) Status_getKey, METH_NOARGS,
		"Get KEY used to connect to status buffer. Function takes no arguments"
	},
	{"getstring", (PyCFunction) Status_getString, METH_VARARGS,
		"fetch string for given keyword from the status buffer. Function takes\n"
		"one string argument (keyword). Function returns string"
	},
	{"getdouble", (PyCFunction) Status_getDouble, METH_VARARGS,
		"fetch double for given keyword from the status buffer. Function takes\n"
		"one string argument (keyword). Function returns float value"
	},
	{"setstring", (PyCFunction) Status_setString, METH_VARARGS,
		"update status buffer with string value for given keyword. Function takes\n"
		"two arguments (keyword,value). First is a string, second is also a string\n"
		"Function returns true on success"
	},
	{"setdouble", (PyCFunction) Status_setDouble, METH_VARARGS,
		"update status buffer with string value for given keyword. Function takes\n"
		"two arguments (keyword,value). First is a string, second is a float\n"
		"Function returns true on success"
	},
	{NULL}  /* Sentinel */
};

static PyTypeObject HashpipeStatusType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyhashpipe.Status",
	.tp_doc = "Status object to communicate with hashpipe status buffer\n"
		"Call pyhashpipe.Status(int id) to connect do instance id, or\n"
		"pyhashpipe.Status(int id, char * key) to choose a different\n"
		"memory key",
	.tp_basicsize = sizeof(HashpipeStatusObj),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = Status_new,
	.tp_init = (initproc) Status_init,
	.tp_dealloc = (destructor) Status_dealloc,
	.tp_methods = Status_methods,
};


PyMODINIT_FUNC PyInit_pyhashpipe(void)
{
	PyObject *m;

	if (PyType_Ready(&HashpipeStatusType) < 0)
		return NULL;

	m = PyModule_Create(&moduledef);
	if (m == NULL)
		return NULL;

	Py_INCREF(&HashpipeStatusType);
	if (PyModule_AddObject(m, "Status", (PyObject *) &HashpipeStatusType) < 0)
	{
		Py_DECREF(&HashpipeStatusType);
		Py_DECREF(m);
		return NULL;
	}

	//hashpipeError = PyErr_NewException("py_hashpipe.Error", NULL, NULL);
	//PyModule_AddObject(m, "hashpipeError", hashpipeError);

	return m;
}

static PyObject *py_hashpipe_version(PyObject *self, PyObject *args) {
    /*
    C method.
    */
     
    return PyUnicode_FromString("1.7-dev");
}

///////////////////////////////////////////////////////////////////
//                 STATUS INIT/DEALLOC METHODS                   //
///////////////////////////////////////////////////////////////////

static PyObject * Status_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	HashpipeStatusObj * self;
	self = (HashpipeStatusObj *)type->tp_alloc(type,0);
	if(self != NULL)
	{
		self->status = NULL;
		self->key[0] = '\0';
		self->instance_id = 0;

	}
	return (PyObject *) self;
}

static int Status_init(HashpipeStatusObj *self, PyObject *args, PyObject *kwds)
{

	if(!PyArg_ParseTuple(args,"i|s",&self->instance_id,&self->key))
	{
		return -1;
	}
	if(strlen(self->key))
	{
		//key was provided as optional argument, that means we need to set env variable
		char keyfile[1000];
		snprintf(keyfile, sizeof(keyfile), "HASHPIPE_KEYFILE=%s",self->key);
		if(putenv(keyfile))
		{
			return -1;
		}
	}
	if(self->status){
		//if that is not null, that means something wrong happened or it's reused
		//maybe we should just ignore, need testing
		return -1;
	}
	if( (self->status = (hashpipe_status_t*) malloc(sizeof(hashpipe_status_t))) == NULL )
	{
		//cant malloc
		return -1;
	}
	int rv;
	self->instance_id &= 0x3f;
	rv = hashpipe_status_attach(self->instance_id, self->status);
	if(rv != HASHPIPE_OK)
	{
		PyErr_SetString(PyExc_RuntimeError,"cannot connect to hashpipe");
		return -1;
	}
	return 0;
}

static void Status_dealloc(HashpipeStatusObj * self)
{
	if(self->status)
	{
		hashpipe_status_detach(self->status);
		free(self->status);
		self->status = NULL;
	}

	Py_TYPE(self)->tp_free((PyObject *) self);
}

///////////////////////////////////////////////////////////////////
//                 STATUS METHODS                                //
///////////////////////////////////////////////////////////////////

static PyObject * Status_getID(HashpipeStatusObj *self, PyObject *Py_UNUSED(ignored))
{
	return PyLong_FromLong(self->instance_id);
}

static PyObject * Status_getKey(HashpipeStatusObj *self, PyObject *Py_UNUSED(ignored))
{
	return PyUnicode_FromString(self->key);
}

static PyObject * Status_getString(HashpipeStatusObj *self, PyObject *args)
{
	char * keyword;
	char answer[81];
	char localkeyword[81];
	if(!PyArg_ParseTuple(args,"s",&keyword))
	{
		PyErr_SetString(PyExc_TypeError,"input has to be string");
		Py_RETURN_FALSE;
	}
	memcpy(localkeyword,keyword,80);
	localkeyword[80] = '\0';
	memset(answer,0,80);
	if(hashpipe_status_lock(self->status))
	{
		PyErr_SetString(PyExc_RuntimeError,"unable to lock status buffer");
		Py_RETURN_FALSE;
	}
	hgets(self->status->buf, localkeyword, 80, answer);
	hashpipe_status_unlock(self->status);
	answer[80]='\0';
	return PyUnicode_FromString(answer);
}

static PyObject * Status_getDouble(HashpipeStatusObj *self, PyObject *args)
{
	char * keyword;
	char localkeyword[81];
	double tmpval;
	if(!PyArg_ParseTuple(args,"s",&keyword))
	{
		PyErr_SetString(PyExc_TypeError,"input has to be string");
		Py_RETURN_FALSE;
	}
	memcpy(localkeyword,keyword,80);
	localkeyword[80] = '\0';
	if(hashpipe_status_lock(self->status))
	{
		PyErr_SetString(PyExc_RuntimeError,"unable to lock status buffer");
		Py_RETURN_FALSE;
	}
	hgetr8(self->status->buf, localkeyword, &tmpval);
	hashpipe_status_unlock(self->status);
	return PyFloat_FromDouble(tmpval);
}

static PyObject * Status_setString(HashpipeStatusObj *self, PyObject *args)
{
	char * keyword;
	char * value;
	char localkeyword[81],localvalue[81];
	if(!PyArg_ParseTuple(args,"ss",&keyword,&value))
	{
		PyErr_SetString(PyExc_TypeError,"input has to be string");
		Py_RETURN_FALSE;
	}
	memcpy(localvalue,value,80);
	localvalue[80] = '\0';
	memcpy(localkeyword,keyword,80);
	localkeyword[80] = '\0';
	if(hashpipe_status_lock(self->status))
	{
		PyErr_SetString(PyExc_RuntimeError,"unable to lock status buffer");
		Py_RETURN_FALSE;
	}
	hputs(self->status->buf, localkeyword, localvalue);
	hashpipe_status_unlock(self->status);
	Py_RETURN_TRUE;
}

static PyObject * Status_setDouble(HashpipeStatusObj *self, PyObject *args)
{
	char * keyword;
	char localkeyword[81];
	double value;
	if(!PyArg_ParseTuple(args,"sd",&keyword,&value))
	{
		PyErr_SetString(PyExc_TypeError,"input has to be string");
		Py_RETURN_FALSE;
	}
	memcpy(localkeyword,keyword,80);
	localkeyword[80] = '\0';
	if(hashpipe_status_lock(self->status))
	{
		PyErr_SetString(PyExc_RuntimeError,"unable to lock status buffer");
		Py_RETURN_FALSE;
	}
	hputr8(self->status->buf, localkeyword, value);
	hashpipe_status_unlock(self->status);
	Py_RETURN_TRUE;
}

// end
