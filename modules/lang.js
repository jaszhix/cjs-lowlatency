/* -*- mode: js; indent-tabs-mode: nil; -*- */
/* exported Class, Interface, bind, copyProperties, copyPublicProperties,
countProperties */
// Copyright (c) 2008  litl, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// Utilities that are "meta-language" things like manipulating object props

var {Class, Interface, getMetaClass} = imports._legacy;

function countProperties(obj) {
    return Object.keys(obj).length;
}

function getPropertyDescriptor(obj, property) {
    if (obj.hasOwnProperty(property))
        return Object.getOwnPropertyDescriptor(obj, property);
    return getPropertyDescriptor(Object.getPrototypeOf(obj), property);
}

function _copyProperty(source, dest, property) {
    let descriptor = getPropertyDescriptor(source, property);
    Object.defineProperty(dest, property, descriptor);
}

function copyProperties(source, dest) {
    for (let property in source) {
        _copyProperty(source, dest, property);
    }
}

function copyPublicProperties(source, dest) {
    for (let property in source) {
        if (typeof(property) == 'string' &&
            property.substring(0, 1) == '_') {
            continue;
        } else {
            _copyProperty(source, dest, property);
        }
    }
}

/**
 * Binds obj to callback. Makes it possible to refer to "obj"
 * using this within the callback.
 * @param {object} obj the object to bind
 * @param {function} callback callback to bind obj in
 * @param arguments additional arguments to the callback
 * @returns: a new callback
 * @type: function
 */
function bind(obj, callback, ...bindArgs) {
    return function(...args) {
        return callback.apply(obj, args.concat(bindArgs));
    }
}
