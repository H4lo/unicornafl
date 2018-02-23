/*
 * Core Definitions for QAPI Visitor Classes
 *
 * Copyright (C) 2012-2016 Red Hat, Inc.
 * Copyright IBM, Corp. 2011
 *
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qapi/qmp/qobject.h"
#include "qapi/qmp/qerror.h"
#include "qapi/visitor.h"
#include "qapi/visitor-impl.h"

void visit_start_struct(Visitor *v, const char *name, void **obj,
                        size_t size, Error **errp)
{
    Error *err = NULL;

    v->start_struct(v, name, obj, size, &err);
    if (obj && v->type == VISITOR_INPUT) {
        assert(!err != !*obj);
    }
    error_propagate(errp, err);
}

void visit_end_struct(Visitor *v, Error **errp)
{
    v->end_struct(v, errp);
}

void visit_start_implicit_struct(Visitor *v, void **obj, size_t size,
                                 Error **errp)
{
    if (v->start_implicit_struct) {
        v->start_implicit_struct(v, obj, size, errp);
    }
}

void visit_end_implicit_struct(Visitor *v)
{
    if (v->end_implicit_struct) {
        v->end_implicit_struct(v);
    }
}

void visit_start_list(Visitor *v, const char *name, Error **errp)
{
    v->start_list(v, name, errp);
}

GenericList *visit_next_list(Visitor *v, GenericList **list, size_t size)
{
    assert(list && size >= sizeof(GenericList));
    return v->next_list(v, list, size);
}

void visit_end_list(Visitor *v)
{
    v->end_list(v);
}

bool visit_optional(Visitor *v, const char *name, bool *present)
{
    if (v->optional) {
        v->optional(v, name, present);
    }
    return *present;
}

void visit_get_next_type(Visitor *v, const char *name, QType *type,
                         bool promote_int, Error **errp)
{
    if (v->get_next_type) {
        v->get_next_type(v, name, type, promote_int, errp);
    }
}

void visit_type_int(Visitor *v, const char *name, int64_t *obj, Error **errp)
{
    v->type_int64(v, name, obj, errp);
}

static void visit_type_uintN(Visitor *v, const char *name, uint64_t *obj,
                             uint64_t max, const char *type, Error **errp)
{
    Error *err = NULL;
    uint64_t value = *obj;

    v->type_uint64(v, name, &value, &err);
    if (err) {
        error_propagate(errp, err);
    } else if (value > max) {
        error_setg(errp, QERR_INVALID_PARAMETER_VALUE,
                   name ? name : "null", type);
    } else {
        *obj = value;
    }
}

void visit_type_uint8(Visitor *v, const char *name, uint8_t *obj, Error **errp)
{
    uint64_t value = *obj;
    visit_type_uintN(v, name, &value, UINT8_MAX, "uint8_t", errp);
    *obj = value;
}

void visit_type_uint16(Visitor *v, const char *name, uint16_t *obj, Error **errp)
{
    uint64_t value = *obj;
    visit_type_uintN(v, name, &value, UINT16_MAX, "uint16_t", errp);
    *obj = value;
}

void visit_type_uint32(Visitor *v, const char *name, uint32_t *obj, Error **errp)
{
    uint64_t value = *obj;
    visit_type_uintN(v, name, &value, UINT32_MAX, "uint32_t", errp);
    *obj = value;
}

void visit_type_uint64(Visitor *v, const char *name, uint64_t *obj,
                       Error **errp)
{
    v->type_uint64(v, name, obj, errp);
}

static void visit_type_intN(Visitor *v, const char *name, int64_t *obj,
                            int64_t min, int64_t max, const char *type,
                            Error **errp)
{
    Error *err = NULL;
    int64_t value = *obj;

    v->type_int64(v, name, &value, &err);
    if (err) {
        error_propagate(errp, err);
    } else if (value < min || value > max) {
        error_setg(errp, QERR_INVALID_PARAMETER_VALUE,
                   name ? name : "null", type);
    } else {
        *obj = value;
    }
}

void visit_type_int8(Visitor *v, const char *name, int8_t *obj, Error **errp)
{
    int64_t value = *obj;
    visit_type_intN(v, name, &value, INT8_MIN, INT8_MAX, "int8_t", errp);
    *obj = value;
}

void visit_type_int16(Visitor *v, const char *name, int16_t *obj,
                      Error **errp)
{
    int64_t value = *obj;
    visit_type_intN(v, name, &value, INT16_MIN, INT16_MAX, "int16_t", errp);
    *obj = value;
}

void visit_type_int32(Visitor *v, const char *name, int32_t *obj,
                      Error **errp)
{
    int64_t value = *obj;
    visit_type_intN(v, name, &value, INT32_MIN, INT32_MAX, "int32_t", errp);
    *obj = value;
}

void visit_type_int64(Visitor *v, const char *name, int64_t *obj,
                      Error **errp)
{
    v->type_int64(v, name, obj, errp);
}

void visit_type_size(Visitor *v, const char *name, uint64_t *obj,
                     Error **errp)
{
    if (v->type_size) {
        v->type_size(v, name, obj, errp);
    } else {
        v->type_uint64(v, name, obj, errp);
    }
}

void visit_type_bool(Visitor *v, const char *name, bool *obj, Error **errp)
{
    v->type_bool(v, name, obj, errp);
}

void visit_type_str(Visitor *v, const char *name, char **obj, Error **errp)
{
    Error *err = NULL;

    assert(obj);
    v->type_str(v, name, obj, &err);
    if (v->type == VISITOR_INPUT) {
        assert(!err != !*obj);
    }
    error_propagate(errp, err);
}

void visit_type_number(Visitor *v, const char *name, double *obj,
                       Error **errp)
{
    v->type_number(v, name, obj, errp);
}

void visit_type_any(Visitor *v, const char *name, QObject **obj, Error **errp)
{
    Error *err = NULL;

    assert(obj);
    v->type_any(v, name, obj, &err);
    if (v->type == VISITOR_INPUT) {
        assert(!err != !*obj);
    }
    error_propagate(errp, err);
}

static void output_type_enum(Visitor *v, const char *name, int *obj,
                             const char *const strings[], Error **errp)
{
    int i = 0;
    int value = *obj;
    char *enum_str;

    while (strings[i++] != NULL);
    if (value < 0 || value >= i - 1) {
        error_setg(errp, QERR_INVALID_PARAMETER, name ? name : "null");
        return;
    }

    enum_str = (char *)strings[value];
    visit_type_str(v, name, &enum_str, errp);
}

static void input_type_enum(Visitor *v, const char *name, int *obj,
                            const char *const strings[], Error **errp)
{
    Error *local_err = NULL;
    int64_t value = 0;
    char *enum_str;

    visit_type_str(v, name, &enum_str, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }

    while (strings[value] != NULL) {
        if (strcmp(strings[value], enum_str) == 0) {
            break;
        }
        value++;
    }

    if (strings[value] == NULL) {
        error_setg(errp, QERR_INVALID_PARAMETER, enum_str);
        g_free(enum_str);
        return;
    }

    g_free(enum_str);
    *obj = (int)value;
}

void visit_type_enum(Visitor *v, const char *name, int *obj,
                     const char *const strings[], Error **errp)
{
    assert(strings);
    if (v->type == VISITOR_INPUT) {
        input_type_enum(v, name, obj, strings, errp);
    } else if (v->type == VISITOR_OUTPUT) {
        output_type_enum(v, name, obj, strings, errp);
    }
}
