/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* Copyright 2010 litl, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>

#include <gjs/gjs-module.h>
#include <gjs/compat.h>
#include <gi/foreign.h>

#include <cairo.h>
#include <cairo-gobject.h>
#include "cairo-private.h"

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(mname) \
static JSBool                                       \
mname##_func(JSContext *context,                    \
              unsigned   argc,                      \
              JS::Value *vp)                        \
{                                                   \
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);             \
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());  \
    cairo_t *cr;

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC_END                               \
    return gjs_cairo_check_status(context, cairo_status(cr), "context"); \
}

#define _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(m)                        \
    if (argc > 0) {                                                \
        gjs_throw(context, "Context." #m "() takes no arguments"); \
        return false;                                              \
    }

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0(method, cfunc)                     \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr);                                                             \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(method, cfunc)                    \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    int ret;                                                               \
   _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(method)                                \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    ret = (int)cfunc(cr);                                                  \
    argv.rval().setInt32(ret);                                             \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0B(method, cfunc)                    \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    cairo_bool_t ret;                                                      \
   _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(method)                                \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    ret = cfunc(cr);                                                       \
    argv.rval().setBoolean(ret);                                           \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC2FFAFF(method, cfunc, n1, n2)        \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    double arg1, arg2;                                                     \
    if (!gjs_parse_call_args(context, #method, "ff", argv,                 \
                        #n1, &arg1, #n2, &arg2))                           \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, &arg1, &arg2);                                               \
    if (cairo_status(cr) == CAIRO_STATUS_SUCCESS) {                        \
      JSObject *array = JS_NewArrayObject(context, 0, NULL);               \
      if (!array)                                                          \
        return false;                                                      \
      JS::Value r;                                                         \
      r = JS::NumberValue(arg1);                                           \
      if (!JS_SetElement(context, array, 0, &r)) return false;             \
      r = JS::NumberValue(arg2);                                           \
      if (!JS_SetElement(context, array, 1, &r)) return false;             \
      argv.rval().setObject(*array);                                       \
    }                                                                      \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFF(method, cfunc)                  \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    double arg1, arg2;                                                     \
   _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(method)                                \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, &arg1, &arg2);                                               \
    if (cairo_status(cr) == CAIRO_STATUS_SUCCESS) {                        \
      JSObject *array = JS_NewArrayObject(context, 0, NULL);               \
      if (!array)                                                          \
        return false;                                                      \
      JS::Value r;                                                         \
      r = JS::NumberValue(arg1);                                           \
      if (!JS_SetElement(context, array, 0, &r)) return false;             \
      r = JS::NumberValue(arg2);                                           \
      if (!JS_SetElement(context, array, 1, &r)) return false;             \
      argv.rval().setObject(*array);                                       \
    }                                                                      \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFFFF(method, cfunc)                \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    double arg1, arg2, arg3, arg4;                                         \
   _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(method)                                \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, &arg1, &arg2, &arg3, &arg4);                                 \
    {                                                                      \
      JSObject *array = JS_NewArrayObject(context, 0, NULL);               \
      if (!array)                                                          \
        return false;                                                      \
      JS::Value r;                                                         \
      r = JS::NumberValue(arg1);                                           \
      if (!JS_SetElement(context, array, 0, &r)) return false;             \
      r = JS::NumberValue(arg2);                                           \
      if (!JS_SetElement(context, array, 1, &r)) return false;             \
      r = JS::NumberValue(arg3);                                           \
      if (!JS_SetElement(context, array, 2, &r)) return false;             \
      r = JS::NumberValue(arg4);                                           \
      if (!JS_SetElement(context, array, 3, &r)) return false;             \
      argv.rval().setObject(*array);                                       \
    }                                                                      \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC0F(method, cfunc)                    \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    double ret;                                                            \
   _GJS_CAIRO_CONTEXT_CHECK_NO_ARGS(method)                                \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    ret = cfunc(cr);                                                       \
    argv.rval().setNumber(ret);                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC1(method, cfunc, fmt, t1, n1)        \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1))                                       \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1);                                                       \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC2(method, cfunc, fmt, t1, n1, t2, n2) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1, #n2, &arg2))                           \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1, arg2);                                                 \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC2B(method, cfunc, fmt, t1, n1, t2, n2) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    cairo_bool_t ret;                                                      \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                 \
                        #n1, &arg1, #n2, &arg2))                           \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    ret = cfunc(cr, arg1, arg2);                                           \
    argv.rval().setBoolean(ret);                                           \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC3(method, cfunc, fmt, t1, n1, t2, n2, t3, n3) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    t3 arg3;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1, #n2, &arg2, #n3, &arg3))               \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1, arg2, arg3);                                           \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC4(method, cfunc, fmt, t1, n1, t2, n2, t3, n3, t4, n4) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    t3 arg3;                                                               \
    t4 arg4;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1, #n2, &arg2, #n3, &arg3, #n4, &arg4))   \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1, arg2, arg3, arg4);                                     \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC5(method, cfunc, fmt, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    t3 arg3;                                                               \
    t4 arg4;                                                               \
    t5 arg5;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1, #n2, &arg2, #n3, &arg3,                \
                        #n4, &arg4, #n5, &arg5))                           \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1, arg2, arg3, arg4, arg5);                               \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

#define _GJS_CAIRO_CONTEXT_DEFINE_FUNC6(method, cfunc, fmt, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_BEGIN(method)                               \
    t1 arg1;                                                               \
    t2 arg2;                                                               \
    t3 arg3;                                                               \
    t4 arg4;                                                               \
    t5 arg5;                                                               \
    t6 arg6;                                                               \
    if (!gjs_parse_call_args(context, #method, fmt, argv,                  \
                        #n1, &arg1, #n2, &arg2, #n3, &arg3,                \
                        #n4, &arg4, #n5, &arg5, #n6, &arg6))               \
        return false;                                                      \
    cr = gjs_cairo_context_get_context(context, obj);                      \
    cfunc(cr, arg1, arg2, arg3, arg4, arg5, arg6);                         \
    argv.rval().setUndefined();                                            \
_GJS_CAIRO_CONTEXT_DEFINE_FUNC_END

typedef struct {
    void *dummy;
    JSContext  *context;
    JSObject   *object;
    cairo_t * cr;
} GjsCairoContext;

GJS_DEFINE_PROTO_WITH_GTYPE("CairoContext", cairo_context, CAIRO_GOBJECT_TYPE_CONTEXT, JSCLASS_BACKGROUND_FINALIZE)
GJS_DEFINE_PRIV_FROM_JS(GjsCairoContext, gjs_cairo_context_class);

static void
_gjs_cairo_context_construct_internal(JSContext       *context,
                                      JS::HandleObject obj,
                                      cairo_t         *cr)
{
    GjsCairoContext *priv;

    priv = g_slice_new0(GjsCairoContext);

    g_assert(priv_from_js(context, obj) == NULL);
    JS_SetPrivate(obj, priv);

    priv->context = context;
    priv->object = obj;
    priv->cr = cairo_reference(cr);
}

GJS_NATIVE_CONSTRUCTOR_DECLARE(cairo_context)
{
    GJS_NATIVE_CONSTRUCTOR_VARIABLES(cairo_context)
    JSObject *surface_wrapper;
    cairo_surface_t *surface;
    cairo_t *cr;

    GJS_NATIVE_CONSTRUCTOR_PRELUDE(cairo_context);

    if (!gjs_parse_call_args(context, "Context", "o", argv,
                        "surface", &surface_wrapper))
        return false;

    surface = gjs_cairo_surface_get_surface(context, surface_wrapper);
    if (!surface) {
        gjs_throw(context, "first argument to Context() should be a surface");
        return false;
    }

    cr = cairo_create(surface);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    _gjs_cairo_context_construct_internal(context, object, cr);
    cairo_destroy(cr);

    GJS_NATIVE_CONSTRUCTOR_FINISH(cairo_context);

    return true;
}

static void
gjs_cairo_context_finalize(JSFreeOp *fop,
                           JSObject *obj)
{
    GjsCairoContext *priv;
    priv = (GjsCairoContext*) JS_GetPrivate(obj);
    if (priv == NULL)
        return;

    if (priv->cr != NULL)
        cairo_destroy(priv->cr);

    g_slice_free(GjsCairoContext, priv);
}

/* Properties */
JSPropertySpec gjs_cairo_context_proto_props[] = {
    JS_PS_END
};

/* Methods */

_GJS_CAIRO_CONTEXT_DEFINE_FUNC5(arc, cairo_arc, "fffff",
                                double, xc, double, yc, double, radius,
                                double, angle1, double, angle2)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC5(arcNegative, cairo_arc_negative, "fffff",
                                double, xc, double, yc, double, radius,
                                double, angle1, double, angle2)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC6(curveTo, cairo_curve_to, "ffffff",
                                double, x1, double, y1, double, x2, double, y2,
                                double, x3, double, y3)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(clip, cairo_clip)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(clipPreserve, cairo_clip_preserve)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFFFF(clipExtents, cairo_clip_extents)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(closePath, cairo_close_path)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(copyPage, cairo_copy_page)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2FFAFF(deviceToUser, cairo_device_to_user, "x", "y")
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2FFAFF(deviceToUserDistance, cairo_device_to_user_distance, "x", "y")
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(fill, cairo_fill)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(fillPreserve, cairo_fill_preserve)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFFFF(fillExtents, cairo_fill_extents)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getAntialias, cairo_get_antialias)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFF(getCurrentPoint, cairo_get_current_point)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getDashCount, cairo_get_dash_count)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getFillRule, cairo_get_fill_rule)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getLineCap, cairo_get_line_cap)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getLineJoin, cairo_get_line_join)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0F(getLineWidth, cairo_get_line_width)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0F(getMiterLimit, cairo_get_miter_limit)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0I(getOperator, cairo_get_operator)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0F(getTolerance, cairo_get_tolerance)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0B(hasCurrentPoint, cairo_has_current_point)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(identityMatrix, cairo_identity_matrix)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2B(inFill, cairo_in_fill, "ff", double, x, double, y)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2B(inStroke, cairo_in_stroke, "ff", double, x, double, y)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(lineTo, cairo_line_to, "ff", double, x, double, y)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(moveTo, cairo_move_to, "ff", double, x, double, y)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(newPath, cairo_new_path)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(newSubPath, cairo_new_sub_path)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(paint, cairo_paint)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(paintWithAlpha, cairo_paint_with_alpha, "f", double, alpha)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFFFF(pathExtents, cairo_path_extents)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(pushGroup, cairo_push_group)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(pushGroupWithContent, cairo_push_group_with_content, "i",
                                cairo_content_t, content)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(popGroupToSource, cairo_pop_group_to_source)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC4(rectangle, cairo_rectangle, "ffff",
                                double, x, double, y, double, width, double, height)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC6(relCurveTo, cairo_rel_curve_to, "ffffff",
                                double, dx1, double, dy1, double, dx2, double, dy2,
                                double, dx3, double, dy3)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(relLineTo, cairo_rel_line_to, "ff", double, dx, double, dy)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(relMoveTo, cairo_rel_move_to, "ff", double, dx, double, dy)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(resetClip, cairo_reset_clip)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(restore, cairo_restore)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(rotate, cairo_rotate, "f", double, angle)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(save, cairo_save)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(scale, cairo_scale, "ff", double, sx, double, sy)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setAntialias, cairo_set_antialias, "i", cairo_antialias_t, antialias)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setFillRule, cairo_set_fill_rule, "i", cairo_fill_rule_t, fill_rule)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setFontSize, cairo_set_font_size, "f", double, size)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setLineCap, cairo_set_line_cap, "i", cairo_line_cap_t, line_cap)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setLineJoin, cairo_set_line_join, "i", cairo_line_join_t, line_join)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setLineWidth, cairo_set_line_width, "f", double, width)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setMiterLimit, cairo_set_miter_limit, "f", double, limit)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setOperator, cairo_set_operator, "i", cairo_operator_t, op)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC1(setTolerance, cairo_set_tolerance, "f", double, tolerance)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC3(setSourceRGB, cairo_set_source_rgb, "fff",
                                double, red, double, green, double, blue)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC4(setSourceRGBA, cairo_set_source_rgba, "ffff",
                                double, red, double, green, double, blue, double, alpha)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(showPage, cairo_show_page)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(stroke, cairo_stroke)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0(strokePreserve, cairo_stroke_preserve)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC0AFFFF(strokeExtents, cairo_stroke_extents)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2(translate, cairo_translate, "ff", double, tx, double, ty)
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2FFAFF(userToDevice, cairo_user_to_device, "x", "y")
_GJS_CAIRO_CONTEXT_DEFINE_FUNC2FFAFF(userToDeviceDistance, cairo_user_to_device_distance, "x", "y")


static JSBool
dispose_func(JSContext *context,
             unsigned   argc,
             JS::Value *vp)
{
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    JS::RootedObject obj(context, rec.thisv().toObjectOrNull());

    GjsCairoContext *priv;

    priv = priv_from_js(context, obj);
    if (priv->cr != NULL) {
        cairo_destroy(priv->cr);
        priv->cr = NULL;
    }
    rec.rval().setUndefined();
    return true;
}

static JSBool
appendPath_func(JSContext *context,
                unsigned   argc,
                JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    JSObject *path_wrapper;
    cairo_path_t *path;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "path", "o", argv,
                        "path", &path_wrapper))
        return false;

    path = gjs_cairo_path_get_path(context, path_wrapper);
    if (!path) {
        gjs_throw(context, "first argument to appendPath() should be a path");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    cairo_append_path(cr, path);
    argv.rval().setUndefined();
    return true;
}

static JSBool
copyPath_func(JSContext *context,
              unsigned   argc,
              JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    cairo_path_t *path;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "", "", argv))
        return false;

    cr = gjs_cairo_context_get_context(context, obj);
    path = cairo_copy_path(cr);
    argv.rval().setObjectOrNull(gjs_cairo_path_from_path(context, path));
    return true;
}

static JSBool
copyPathFlat_func(JSContext *context,
                  unsigned   argc,
                  JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    cairo_path_t *path;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "", "", argv))
        return false;

    cr = gjs_cairo_context_get_context(context, obj);
    path = cairo_copy_path_flat(cr);
    argv.rval().setObjectOrNull(gjs_cairo_path_from_path(context, path));
    return true;
}

static JSBool
mask_func(JSContext *context,
          unsigned   argc,
          JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    JSObject *pattern_wrapper;
    cairo_pattern_t *pattern;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "mask", "o", argv,
                        "pattern", &pattern_wrapper))
        return false;

    pattern = gjs_cairo_pattern_get_pattern(context, pattern_wrapper);
    if (!pattern) {
        gjs_throw(context, "first argument to mask() should be a pattern");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    cairo_mask(cr, pattern);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    argv.rval().setUndefined();
    return true;
}

static JSBool
maskSurface_func(JSContext *context,
                 unsigned   argc,
                 JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    JSObject *surface_wrapper;
    double x, y;
    cairo_surface_t *surface;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "maskSurface", "off", argv,
                        "surface", &surface_wrapper,
                        "x", &x,
                        "y", &y))
        return false;

    surface = gjs_cairo_surface_get_surface(context, surface_wrapper);
    if (!surface) {
        gjs_throw(context, "first argument to maskSurface() should be a surface");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);

    cairo_mask_surface(cr, surface, x, y);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    argv.rval().setUndefined();
    return true;
}

static JSBool
setDash_func(JSContext *context,
             unsigned   argc,
             JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    guint i;
    cairo_t *cr;
    JS::RootedObject dashes(context);
    double offset;
    bool retval = false;
    guint len;
    GArray *dashes_c = NULL;

    if (!gjs_parse_call_args(context, "setDash", "of", argv,
                        "dashes", dashes.address(), "offset", &offset))
        return false;


    if (!JS_IsArrayObject(context, dashes)) {
        gjs_throw(context, "dashes must be an array");
        goto out;
    }

    if (!JS_GetArrayLength(context, dashes, &len)) {
        gjs_throw(context, "Can't get length of dashes");
        goto out;
    }

    dashes_c = g_array_sized_new (false, false, sizeof(double), len);
    for (i = 0; i < len; ++i) {
        JS::Value elem;
        double b;

        elem = JS::UndefinedValue();
        if (!JS_GetElement(context, dashes, i, &elem)) {
            goto out;
        }
        if (elem.isUndefined())
            continue;

        if (!JS::ToNumber(context, elem, &b))
            goto out;
        if (b <= 0) {
            gjs_throw(context, "Dash value must be positive");
            goto out;
        }

        g_array_append_val(dashes_c, b);
    }

    cr = gjs_cairo_context_get_context(context, obj);
    cairo_set_dash(cr, (double*)dashes_c->data, dashes_c->len, offset);
    argv.rval().setUndefined();
    retval = true;
 out:
    if (dashes_c != NULL)
        g_array_free (dashes_c, true);
    return retval;
}

static JSBool
setSource_func(JSContext *context,
               unsigned   argc,
               JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    JSObject *pattern_wrapper;
    cairo_pattern_t *pattern;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "setSource", "o", argv,
                        "pattern", &pattern_wrapper))
        return false;

    pattern = gjs_cairo_pattern_get_pattern(context, pattern_wrapper);
    if (!pattern) {
        gjs_throw(context, "first argument to setSource() should be a pattern");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);

    cairo_set_source(cr, pattern);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    argv.rval().setUndefined();

    return true;
}

static JSBool
setSourceSurface_func(JSContext *context,
                      unsigned   argc,
                      JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    JSObject *surface_wrapper;
    double x, y;
    cairo_surface_t *surface;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "setSourceSurface", "off", argv,
                        "surface", &surface_wrapper,
                        "x", &x,
                        "y", &y))
        return false;

    surface = gjs_cairo_surface_get_surface(context, surface_wrapper);
    if (!surface) {
        gjs_throw(context, "first argument to setSourceSurface() should be a surface");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);

    cairo_set_source_surface(cr, surface, x, y);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    argv.rval().setUndefined();

    return true;
}

static JSBool
showText_func(JSContext *context,
              unsigned   argc,
              JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    char *utf8;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "showText", "s", argv,
                        "utf8", &utf8))
        return false;

    cr = gjs_cairo_context_get_context(context, obj);

    cairo_show_text(cr, utf8);
    g_free(utf8);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    argv.rval().setUndefined();

    return true;
}

static JSBool
selectFontFace_func(JSContext *context,
                    unsigned   argc,
                    JS::Value *vp)
{
    JS::CallArgs argv = JS::CallArgsFromVp (argc, vp);
    JS::RootedObject obj(context, argv.thisv().toObjectOrNull());

    char *family;
    cairo_font_slant_t slant;
    cairo_font_weight_t weight;
    cairo_t *cr;

    if (!gjs_parse_call_args(context, "selectFontFace", "sii", argv,
                        "family", &family,
                        "slang", &slant,
                        "weight", &weight))
        return false;

    cr = gjs_cairo_context_get_context(context, obj);

    cairo_select_font_face(cr, family, slant, weight);
    g_free(family);

    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;
    argv.rval().setUndefined();

    return true;
}

static JSBool
popGroup_func(JSContext *context,
              unsigned   argc,
              JS::Value *vp)
{
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    JS::RootedObject obj(context, rec.thisv().toObjectOrNull());

    cairo_t *cr;
    cairo_pattern_t *pattern;
    JSObject *pattern_wrapper;

    if (argc > 0) {
        gjs_throw(context, "Context.popGroup() takes no arguments");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    pattern = cairo_pop_group(cr);
    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    /* pattern belongs to the context, so keep the reference */
    pattern_wrapper = gjs_cairo_pattern_from_pattern(context, pattern);
    if (!pattern_wrapper) {
        gjs_throw(context, "failed to create pattern");
        return false;
    }

    rec.rval().setObject(*pattern_wrapper);

    return true;
}
static JSBool
getSource_func(JSContext *context,
               unsigned   argc,
               JS::Value *vp)
{
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    JS::RootedObject obj(context, rec.thisv().toObjectOrNull());

    cairo_t *cr;
    cairo_pattern_t *pattern;
    JSObject *pattern_wrapper;

    if (argc > 0) {
        gjs_throw(context, "Context.getSource() takes no arguments");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    pattern = cairo_get_source(cr);
    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    /* pattern belongs to the context, so keep the reference */
    pattern_wrapper = gjs_cairo_pattern_from_pattern(context, pattern);
    if (!pattern_wrapper) {
        gjs_throw(context, "failed to create pattern");
        return false;
    }

    rec.rval().setObject(*pattern_wrapper);

    return true;
}

static JSBool
getTarget_func(JSContext *context,
               unsigned   argc,
               JS::Value *vp)
{
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    JS::RootedObject obj(context, rec.thisv().toObjectOrNull());

    cairo_t *cr;
    cairo_surface_t *surface;
    JSObject *surface_wrapper;

    if (argc > 0) {
        gjs_throw(context, "Context.getTarget() takes no arguments");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    surface = cairo_get_target(cr);
    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    /* surface belongs to the context, so keep the reference */
    surface_wrapper = gjs_cairo_surface_from_surface(context, surface);
    if (!surface_wrapper) {
        /* exception already set */
        return false;
    }

    rec.rval().setObject(*surface_wrapper);

    return true;
}

static JSBool
getGroupTarget_func(JSContext *context,
                    unsigned   argc,
                    JS::Value *vp)
{
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    JS::RootedObject obj(context, rec.thisv().toObjectOrNull());

    cairo_t *cr;
    cairo_surface_t *surface;
    JSObject *surface_wrapper;

    if (argc > 0) {
        gjs_throw(context, "Context.getGroupTarget() takes no arguments");
        return false;
    }

    cr = gjs_cairo_context_get_context(context, obj);
    surface = cairo_get_group_target(cr);
    if (!gjs_cairo_check_status(context, cairo_status(cr), "context"))
        return false;

    /* surface belongs to the context, so keep the reference */
    surface_wrapper = gjs_cairo_surface_from_surface(context, surface);
    if (!surface_wrapper) {
        /* exception already set */
        return false;
    }

    rec.rval().setObject(*surface_wrapper);

    return true;
}

JSFunctionSpec gjs_cairo_context_proto_funcs[] = {
    JS_FS("$dispose", dispose_func, 0, 0),
    JS_FS("appendPath", appendPath_func, 0, 0),
    JS_FS("arc", arc_func, 0, 0),
    JS_FS("arcNegative", arcNegative_func, 0, 0),
    JS_FS("clip", clip_func, 0, 0),
    JS_FS("clipExtents", clipExtents_func, 0, 0),
    JS_FS("clipPreserve", clipPreserve_func, 0, 0),
    JS_FS("closePath", closePath_func, 0, 0),
    JS_FS("copyPage", copyPage_func, 0, 0),
    JS_FS("copyPath", copyPath_func, 0, 0),
    JS_FS("copyPathFlat", copyPathFlat_func, 0, 0),
    JS_FS("curveTo", curveTo_func, 0, 0),
    JS_FS("deviceToUser", deviceToUser_func, 0, 0),
    JS_FS("deviceToUserDistance", deviceToUserDistance_func, 0, 0),
    JS_FS("fill", fill_func, 0, 0),
    JS_FS("fillPreserve", fillPreserve_func, 0, 0),
    JS_FS("fillExtents", fillExtents_func, 0, 0),
    // fontExtents
    JS_FS("getAntialias", getAntialias_func, 0, 0),
    JS_FS("getCurrentPoint", getCurrentPoint_func, 0, 0),
    // getDash
    JS_FS("getDashCount", getDashCount_func, 0, 0),
    JS_FS("getFillRule", getFillRule_func, 0, 0),
    // getFontFace
    // getFontMatrix
    // getFontOptions
    JS_FS("getGroupTarget", getGroupTarget_func, 0, 0),
    JS_FS("getLineCap", getLineCap_func, 0, 0),
    JS_FS("getLineJoin", getLineJoin_func, 0, 0),
    JS_FS("getLineWidth", getLineWidth_func, 0, 0),
    // getMatrix
    JS_FS("getMiterLimit", getMiterLimit_func, 0, 0),
    JS_FS("getOperator", getOperator_func, 0, 0),
    // getScaledFont
    JS_FS("getSource", getSource_func, 0, 0),
    JS_FS("getTarget", getTarget_func, 0, 0),
    JS_FS("getTolerance", getTolerance_func, 0, 0),
    // glyphPath
    // glyphExtents
    JS_FS("hasCurrentPoint", hasCurrentPoint_func, 0, 0),
    JS_FS("identityMatrix", identityMatrix_func, 0, 0),
    JS_FS("inFill", inFill_func, 0, 0),
    JS_FS("inStroke", inStroke_func, 0, 0),
    JS_FS("lineTo", lineTo_func, 0, 0),
    JS_FS("mask", mask_func, 0, 0),
    JS_FS("maskSurface", maskSurface_func, 0, 0),
    JS_FS("moveTo", moveTo_func, 0, 0),
    JS_FS("newPath", newPath_func, 0, 0),
    JS_FS("newSubPath", newSubPath_func, 0, 0),
    JS_FS("paint", paint_func, 0, 0),
    JS_FS("paintWithAlpha", paintWithAlpha_func, 0, 0),
    JS_FS("pathExtents", pathExtents_func, 0, 0),
    JS_FS("popGroup", popGroup_func, 0, 0),
    JS_FS("popGroupToSource", popGroupToSource_func, 0, 0),
    JS_FS("pushGroup", pushGroup_func, 0, 0),
    JS_FS("pushGroupWithContent", pushGroupWithContent_func, 0, 0),
    JS_FS("rectangle", rectangle_func, 0, 0),
    JS_FS("relCurveTo", relCurveTo_func, 0, 0),
    JS_FS("relLineTo", relLineTo_func, 0, 0),
    JS_FS("relMoveTo", relMoveTo_func, 0, 0),
    JS_FS("resetClip", resetClip_func, 0, 0),
    JS_FS("restore", restore_func, 0, 0),
    JS_FS("rotate", rotate_func, 0, 0),
    JS_FS("save", save_func, 0, 0),
    JS_FS("scale", scale_func, 0, 0),
    JS_FS("selectFontFace", selectFontFace_func, 0, 0),
    JS_FS("setAntialias", setAntialias_func, 0, 0),
    JS_FS("setDash", setDash_func, 0, 0),
    // setFontFace
    // setFontMatrix
    // setFontOptions
    JS_FS("setFontSize", setFontSize_func, 0, 0),
    JS_FS("setFillRule", setFillRule_func, 0, 0),
    JS_FS("setLineCap", setLineCap_func, 0, 0),
    JS_FS("setLineJoin", setLineJoin_func, 0, 0),
    JS_FS("setLineWidth", setLineWidth_func, 0, 0),
    // setMatrix
    JS_FS("setMiterLimit", setMiterLimit_func, 0, 0),
    JS_FS("setOperator", setOperator_func, 0, 0),
    // setScaledFont
    JS_FS("setSource", setSource_func, 0, 0),
    JS_FS("setSourceRGB", setSourceRGB_func, 0, 0),
    JS_FS("setSourceRGBA", setSourceRGBA_func, 0, 0),
    JS_FS("setSourceSurface", setSourceSurface_func, 0, 0),
    JS_FS("setTolerance", setTolerance_func, 0, 0),
    // showGlyphs
    JS_FS("showPage", showPage_func, 0, 0),
    JS_FS("showText", showText_func, 0, 0),
    // showTextGlyphs
    JS_FS("stroke", stroke_func, 0, 0),
    JS_FS("strokeExtents", strokeExtents_func, 0, 0),
    JS_FS("strokePreserve", strokePreserve_func, 0, 0),
    // textPath
    // textExtends
    // transform
    JS_FS("translate", translate_func, 0, 0),
    JS_FS("userToDevice", userToDevice_func, 0, 0),
    JS_FS("userToDeviceDistance", userToDeviceDistance_func, 0, 0),
    JS_FS_END
};

JSObject *
gjs_cairo_context_from_context(JSContext *context,
                               cairo_t *cr)
{
    JS::RootedObject object(context,
                            JS_NewObject(context, &gjs_cairo_context_class, NULL, NULL));
    if (!object)
        return NULL;

    _gjs_cairo_context_construct_internal(context, object, cr);

    return object;
}

cairo_t *
gjs_cairo_context_get_context(JSContext       *context,
                              JS::HandleObject object)
{
    GjsCairoContext *priv;
    priv = priv_from_js(context, object);
    if (priv == NULL)
        return NULL;

    return priv->cr;
}

static bool
context_to_g_argument(JSContext      *context,
                      JS::Value       value,
                      const char     *arg_name,
                      GjsArgumentType argument_type,
                      GITransfer      transfer,
                      bool            may_be_null,
                      GArgument      *arg)
{
    JS::RootedObject obj(context, value.toObjectOrNull());
    cairo_t *cr;

    cr = gjs_cairo_context_get_context(context, obj);
    if (!cr)
        return false;
    if (transfer == GI_TRANSFER_EVERYTHING)
        cairo_reference(cr);

    arg->v_pointer = cr;
    return true;
}

static bool
context_from_g_argument(JSContext  *context,
                        JS::Value  *value_p,
                        GArgument  *arg)
{
    JSObject *obj;

    obj = gjs_cairo_context_from_context(context, (cairo_t*)arg->v_pointer);
    if (!obj)
        return false;

    *value_p = JS::ObjectValue(*obj);
    return true;
}

static bool
context_release_argument(JSContext  *context,
                         GITransfer  transfer,
                         GArgument  *arg)
{
    cairo_destroy((cairo_t*)arg->v_pointer);
    return true;
}

static GjsForeignInfo foreign_info = {
    context_to_g_argument,
    context_from_g_argument,
    context_release_argument
};

void
gjs_cairo_context_init(JSContext *context)
{
    gjs_struct_foreign_register("cairo", "Context", &foreign_info);
}
