/*
 * data-types.h
 * Copyright (C) 2016 Kovid Goyal <kovid at kovidgoyal.net>
 *
 * Distributed under terms of the GPL3 license.
 */

#pragma once


#include <stdint.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define UNUSED __attribute__ ((unused))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) > (y)) ? (y) : (x))

typedef Py_UCS4 char_type;
typedef uint64_t color_type;
typedef uint32_t decoration_type;
typedef uint32_t combining_type;
typedef unsigned int index_type;
#define CELL_SIZE (sizeof(char_type) + sizeof(color_type) + sizeof(decoration_type) + sizeof(combining_type))

#define CHAR_MASK 0xFFFFFF
#define ATTRS_SHIFT 24
#define WIDTH_MASK  0xFF
#define DECORATION_SHIFT  2
#define BOLD_SHIFT 4
#define ITALIC_SHIFT 5
#define REVERSE_SHIFT 6
#define STRIKE_SHIFT 7
#define COL_MASK 0xFFFFFFFF
#define COL_SHIFT  32
#define HAS_BG_MASK (0xFF << COL_SHIFT)
#define CC_MASK 0xFFFF
#define CC_SHIFT 16

#define CURSOR_TO_ATTRS(c, w) \
    ((w) | (((c->decoration & 3) << DECORATION_SHIFT) | ((c->bold & 1) << BOLD_SHIFT) | \
            ((c->italic & 1) << ITALIC_SHIFT) | ((c->reverse & 1) << REVERSE_SHIFT) | ((c->strikethrough & 1) << STRIKE_SHIFT))) << ATTRS_SHIFT

#define ATTRS_TO_CURSOR(a, c) \
    c->decoration = (a >> DECORATION_SHIFT) & 3; c->bold = (a >> BOLD_SHIFT) & 1; c->italic = (a >> ITALIC_SHIFT) & 1; \
    c->reverse = (a >> REVERSE_SHIFT) & 1; c->strikethrough = (a >> STRIKE_SHIFT) & 1;

#define SET_ATTRIBUTE(chars, shift, val) \
    mask = shift == DECORATION_SHIFT ? 3 : 1; \
    val = (val & mask) << (ATTRS_SHIFT + shift); \
    mask = ~(mask << (ATTRS_SHIFT + shift)); \
    for (index_type i = 0; i < self->xnum; i++)  (chars)[i] = ((chars)[i] & mask) | val;

#define COPY_CELL(src, s, dest, d) \
        (dest)->chars[d] = (self)->chars[s]; \
        (dest)->colors[d] = (self)->colors[s]; \
        (dest)->decoration_fg[d] = (self)->decoration_fg[s]; \
        (dest)->combining_chars[d] = (self)->combining_chars[s];

#define COPY_LINE(src, dest) \
    memcpy((dest)->chars, (src)->chars, sizeof(char_type) * MIN((src)->xnum, (dest)->xnum)); \
    memcpy((dest)->colors, (src)->colors, sizeof(color_type) * MIN((src)->xnum, (dest)->xnum)); \
    memcpy((dest)->decoration_fg, (src)->decoration_fg, sizeof(decoration_type) * MIN((src)->xnum, (dest)->xnum)); \
    memcpy((dest)->combining_chars, (src)->combining_chars, sizeof(combining_type) * MIN((src)->xnum, (dest)->xnum)); 

#define COLORS_TO_CURSOR(col, c) \
    c->fg = col & COL_MASK; c->bg = (col >> COL_SHIFT)

#define METHOD(name, arg_type) {#name, (PyCFunction)name, arg_type, name##_doc},

#define INIT_TYPE(type) \
    int init_##type(PyObject *module) {\
        if (PyType_Ready(&type##_Type) < 0) return 0; \
        if (PyModule_AddObject(module, #type, (PyObject *)&type##_Type) != 0) return 0; \
        Py_INCREF(&type##_Type); \
        return 1; \
    }

#define RICHCMP(type) \
    static PyObject * richcmp(PyObject *obj1, PyObject *obj2, int op) { \
        PyObject *result = NULL; \
        int eq; \
        if (op != Py_EQ && op != Py_NE) { Py_RETURN_NOTIMPLEMENTED; } \
        if (!PyObject_TypeCheck(obj1, &type##_Type)) { Py_RETURN_FALSE; } \
        if (!PyObject_TypeCheck(obj2, &type##_Type)) { Py_RETURN_FALSE; } \
        eq = __eq__((type*)obj1, (type*)obj2); \
        if (op == Py_NE) result = eq ? Py_False : Py_True; \
        else result = eq ? Py_True : Py_False; \
        Py_INCREF(result); \
        return result; \
    }



typedef struct {
    PyObject_HEAD

    char_type *chars;
    color_type *colors;
    decoration_type *decoration_fg;
    combining_type *combining_chars;
    index_type xnum, ynum;
} Line;


typedef struct {
    PyObject_HEAD

    uint8_t *buf;
    index_type xnum, ynum, *line_map;
    index_type block_size;
    uint8_t *continued_map;
    Line *line;

    // Pointers into buf
    char_type *chars;
    color_type *colors;
    decoration_type *decoration_fg;
    combining_type *combining_chars;
} LineBuf;


typedef struct {
    PyObject_HEAD

    PyObject *x, *y, *shape, *blink, *hidden, *color;
    uint8_t bold, italic, reverse, strikethrough, decoration;
    uint32_t fg, bg, decoration_fg;

} Cursor;

Line* alloc_line();
Cursor* alloc_cursor();
