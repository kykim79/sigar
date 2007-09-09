/*
 * Copyright (C) [2004, 2005, 2006], Hyperic, Inc.
 * This file is part of SIGAR.
 * 
 * SIGAR is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include <Python.h>
#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"

#define PySigarString_FromNetAddr(a) pysigar_net_address_to_string(&a)

#define PySigarInt_FromChar(c) PyInt_FromLong((int)c)

#define PySigar_ParsePID \
    if (!PyArg_ParseTuple(args, "i", &pid)) return NULL

#define PySigar_ParseName \
    if (!PyArg_ParseTuple(args, "s", &name, &name_len)) return NULL

#define PySIGAR_OBJ ((PySigarObject *)self)

#define PySIGAR ((sigar_t *)PySIGAR_OBJ->ptr)

#define PySigar_new(t) PyType_GenericAlloc(&t, 0)

#define PySigar_TPFLAGS Py_TPFLAGS_DEFAULT

#define PySigar_AddType(name, t) \
    if (PyType_Ready(&t) == 0) { \
        Py_INCREF(&t); \
        PyModule_AddObject(module, name, (PyObject *)&t); \
    }

#define PySigar_Croak() PyErr_SetString(PyExc_ValueError, sigar_strerror(sigar, status))

typedef struct {
    PyObject_HEAD
    void *ptr;
} PySigarObject;

static PyTypeObject pysigar_PySigarType;

static void pysigar_free(PyObject *self)
{
    if (PySIGAR_OBJ->ptr) {
        if (self->ob_type == &pysigar_PySigarType) {
            sigar_close(PySIGAR);
        }
        else {
            free(PySIGAR_OBJ->ptr);
        }
        PySIGAR_OBJ->ptr = NULL;
    }

    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *pysigar_net_address_to_string(sigar_net_address_t *address)
{
    char addr_str[SIGAR_INET6_ADDRSTRLEN];
    sigar_net_address_to_string(NULL, address, addr_str);
    return PyString_FromString(addr_str);
}

#include "_sigar_generated.c"

static PyObject *pysigar_open(PyObject *pyself, PyObject *args)
{
    PyObject *self = PySigar_new(pysigar_PySigarType);
    sigar_open((sigar_t **)&PySIGAR_OBJ->ptr);
    return self;
}

static PyObject *pysigar_close(PyObject *self, PyObject *args)
{
    if (PySIGAR_OBJ->ptr) {
        sigar_close(PySIGAR);
        PySIGAR_OBJ->ptr = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static int pysigar_parse_uint64(PyObject *args, sigar_uint64_t *val)
{
    PyObject *obj;

    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return !SIGAR_OK;
    }

    *val = PyInt_AsUnsignedLongLongMask(obj);
    return SIGAR_OK;
}

static PyObject *pysigar_format_size(PyObject *self, PyObject *args)
{
    char buffer[56];
    sigar_uint64_t size;

    if (pysigar_parse_uint64(args, &size) == SIGAR_OK) {
        return PyString_FromString(sigar_format_size(size, buffer));
    }
    else {
        return NULL;
    }
}

static PyMethodDef pysigar_methods[] = {
    { "close", pysigar_close, METH_NOARGS, NULL },
    PY_SIGAR_METHODS
    {NULL}
};

static PyTypeObject pysigar_PySigarType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Sigar",                   /*tp_name*/
    sizeof(PySigarObject),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    pysigar_free,              /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    PySigar_TPFLAGS,           /*tp_flags*/
    0,                         /*tp_doc*/
    0,                         /*tp_traverse*/
    0,                         /*tp_clear*/
    0,                         /*tp_richcompare*/
    0,                         /*tp_weaklistoffset*/
    0,                         /*tp_iter*/
    0,                         /*tp_iternext*/
    pysigar_methods,           /*tp_methods*/
    0,                         /*tp_members*/
    0,                         /*tp_getset*/
    0,                         /*tp_base*/
    0,                         /*tp_dict*/
    0,                         /*tp_descr_get*/
    0,                         /*tp_descr_set*/
    0,                         /*tp_dictoffset*/
    0,                         /*tp_init*/
    0,                         /*tp_alloc*/
    0                          /*tp_new*/
};

static PyMethodDef pysigar_module_methods[] = {
    { "open", pysigar_open, METH_NOARGS, NULL },
    { "format_size", pysigar_format_size, METH_VARARGS, NULL },
    {NULL}
};

PyMODINIT_FUNC
init_sigar(void) 
{
    PyObject *module =
        Py_InitModule("_sigar", pysigar_module_methods);

    PySigar_AddType("Sigar", pysigar_PySigarType);

    PY_SIGAR_ADD_TYPES;
}