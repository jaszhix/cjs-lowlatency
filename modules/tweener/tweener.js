/* -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil; -*- */
/* Copyright 2008  litl, LLC. */
/**
 * Tweener
 * Transition controller for movieclips, sounds, textfields and other objects
 *
 * @author              Zeh Fernando, Nate Chatellier, Arthur Debert
 * @version             1.31.71
 */

/*
 Licensed under the MIT License

 Copyright (c) 2006-2007 Zeh Fernando and Nate Chatellier

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://code.google.com/p/tweener/
 http://code.google.com/p/tweener/wiki/License
 */

const GLib = imports.gi.GLib;

const TweenList = imports.tweener.tweenList;
const Signals = imports.signals;

var _inited = false;
var _engineExists = false;
var _tweenList = null;

var _timeScale = 1;

var _specialPropertyList = [];
var _specialPropertyModifierList = [];
var _specialPropertySplitterList = [];

/*
 * Ticker should implement:
 *
 * property FRAME_RATE
 * start()
 * stop()
 * getTime() gets time in milliseconds from start()
 * signal prepare-frame
 *
 */
var _ticker = null;

var _prepareFrameId = 0;

class FrameTicker {
    constructor() {
        this.FRAME_RATE = 65;
        this._currentTime = 0;
    }

    start() {
        this._currentTime = 0;

        let me = this;

        this._timeoutID = GLib.timeout_add(
            GLib.PRIORITY_DEFAULT,
            Math.floor(1000 / me.FRAME_RATE),
            function() {
                me._currentTime += 1000 / me.FRAME_RATE;
                me.emit('prepare-frame');
                return true;
            });
    }

    stop() {
        if ('_timeoutID' in this) {
            GLib.source_remove(this._timeoutID);
            delete this._timeoutID;
        }

        this._currentTime = 0;
    }

    getTime() {
        return this._currentTime;
    }
};
Signals.addSignalMethods(FrameTicker.prototype);

_ticker = new FrameTicker();

/* TODOs:
 *
 * Special properties:
 *
 * Special properties are 'proxy' properties used in Tweener to tween
 * (animate) things that are not proper properties per se. One example
 * given is the 'frame' of an object in ActionScript, which is not an
 * object property. Using the special property '_frame' you could animate
 * it like this:
 *
 * Tweener.addTween(myMovieClip, {_frame:20, time:1});
 *
 * which would be equivalent to applying a fast-forward to it.
 *
 * This properties need a special support in the code, and I've removed it
 * for now until we see the need for it in our clutter based stuff.
 */

/* This is a bit pointless now, but let's keep it anyway... */
function _init() {
    if (_inited)
        return;

    _inited = true;
}

function setFrameTicker(ticker) {
    _ticker = ticker;
}

function _startEngine() {
    if (_engineExists)
        return;

    _engineExists = true;
    _tweenList = [];

    if (!_ticker) {
        throw new Error('Must call setFrameTicker()');
    }

    _prepareFrameId = _ticker.connect('prepare-frame',
        _onEnterFrame);
    _ticker.start();
}

function _stopEngine() {
    if (!_engineExists)
        return;

    _engineExists = false;
    _tweenList = false;

    _ticker.disconnect(_prepareFrameId);
    _prepareFrameId = 0;
    _ticker.stop();
}

function _removeTweenByIndex(i) {
    _tweenList[i] = null;

    let finalRemoval = arguments[1];

    if (finalRemoval != undefined && finalRemoval)
        _tweenList.splice(i, 1);

    return true;
}

function _resumeTweenByIndex(i) {
    let tweening = _tweenList[i];

    if (tweening == null || !tweening.isPaused)
        return false;

    let currentTime = _ticker.getTime();

    tweening.timeStart += currentTime - tweening.timePaused;
    tweening.timeComplete += currentTime - tweening.timePaused;
    tweening.timePaused = undefined;
    tweening.isPaused = false;

    return true;
}

/* FIXME: any way to get the function name from the fn itself? */
function _callOnFunction(fn, fnname, scope, fallbackScope, params)
{
    if (fn) {
        let eventScope = scope ? scope : fallbackScope;
        try {
            fn.apply(eventScope, params);
        } catch (e) {
            logError(e, 'Error calling ' + fnname);
        }
    }
}

function _updateTweenByIndex(i) {
    let tweening = _tweenList[i];

    if (tweening == null || !tweening.scope)
        return false;

    let currentTime = _ticker.getTime();

    if (currentTime < tweening.timeStart)
        return true; // Hasn't started, so return true

    let scope = tweening.scope;
    let t, b, c, d, nv;

    let isOver = false;

    if (tweening.isCaller) {
        do {
            t = ((tweening.timeComplete - tweening.timeStart)/tweening.count) *
                (tweening.timesCalled + 1);
            b = tweening.timeStart;
            c = tweening.timeComplete - tweening.timeStart;
            d = tweening.timeComplete - tweening.timeStart;
            nv = tweening.transition(t, b, c, d);

            if (currentTime >= nv) {
                _callOnFunction(tweening.onUpdate, 'onUpdate', tweening.onUpdateScope,
                    scope, tweening.onUpdateParams);

                tweening.timesCalled++;
                if (tweening.timesCalled >= tweening.count) {
                    isOver = true;
                    break;
                }

                if (tweening.waitFrames)
                    break;
            }
        } while (currentTime >= nv);
    } else {
        let mustUpdate, name;

        if (currentTime >= tweening.timeComplete) {
            isOver = true;
            mustUpdate = true;
        } else {
            mustUpdate = tweening.skipUpdates < 1 ||
                !tweening.skipUpdates ||
                tweening.updatesSkipped >= tweening.skipUpdates;
        }

        if (!tweening.hasStarted) {
            _callOnFunction(tweening.onStart, 'onStart', tweening.onStartScope,
                scope, tweening.onStartParams);

            for (name in tweening.properties) {
                let pv;

                if (tweening.properties[name].isSpecialProperty) {
                    // It's a special property, tunnel via the special property function
                    if (_specialPropertyList[name].preProcess != undefined) {
                        tweening.properties[name].valueComplete = _specialPropertyList[name].preProcess(scope, _specialPropertyList[name].parameters, tweening.properties[name].originalValueComplete, tweening.properties[name].extra);
                    }
                    pv = _specialPropertyList[name].getValue(scope, _specialPropertyList[name].parameters, tweening.properties[name].extra);
                } else {
                    // Directly read property
                    pv = scope[name];
                }
                tweening.properties[name].valueStart = isNaN(pv) ? tweening.properties[name].valueComplete : pv;
            }

            mustUpdate = true;
            tweening.hasStarted = true;
        }

        if (mustUpdate) {
            for (name in tweening.properties) {
                let property = tweening.properties[name];

                if (isOver) {
                    // Tweening time has finished, just set it to the final value
                    nv = property.valueComplete;
                } else {
                    if (property.hasModifier) {
                        // Modified
                        t = currentTime - tweening.timeStart;
                        d = tweening.timeComplete - tweening.timeStart;
                        nv = tweening.transition(t, 0, 1, d, tweening.transitionParams);
                        nv = property.modifierFunction(property.valueStart, property.valueComplete, nv, property.modifierParameters);
                    } else {
                        // Normal update
                        t = currentTime - tweening.timeStart;
                        b = property.valueStart;
                        c = property.valueComplete - property.valueStart;
                        d = tweening.timeComplete - tweening.timeStart;
                        nv = tweening.transition(t, b, c, d, tweening.transitionParams);
                    }
                }

                if (tweening.rounded)
                    nv = Math.round(nv);

                if (tweening.min !== undefined && nv < tweening.min)
                    nv = tweening.min;
                if (tweening.max !== undefined && nv > tweening.max)
                    nv = tweening.max;

                if (property.isSpecialProperty) {
                    // It's a special property, tunnel via the special property method
                    _specialPropertyList[name].setValue(scope, nv, _specialPropertyList[name].parameters, tweening.properties[name].extra);
                } else {
                    // Directly set property
                    scope[name] = nv;
                }
            }

            tweening.updatesSkipped = 0;

            _callOnFunction(tweening.onUpdate, 'onUpdate', tweening.onUpdateScope,
                scope, tweening.onUpdateParams);

        } else {
            tweening.updatesSkipped++;
        }
    }

    if (isOver) {
        _callOnFunction(tweening.onComplete, 'onComplete', tweening.onCompleteScope,
            scope, tweening.onCompleteParams);
    }

    return !isOver;
}

function _updateTweens() {
    if (_tweenList.length == 0)
        return false;

    for (let i = 0; i < _tweenList.length; i++) {
        if (_tweenList[i] == undefined || !_tweenList[i].isPaused) {
            if (!_updateTweenByIndex(i))
                _removeTweenByIndex(i);

            if (_tweenList[i] == null) {
                _removeTweenByIndex(i, true);
                i--;
            }
        }
    }

    return true;
}

/* Ran once every 'frame'. It's the main engine, updates all existing tweenings */
function _onEnterFrame() {
    if (!_updateTweens())
        _stopEngine();

    return true;
}

var restrictedWords = {
    time: true,
    delay: true,
    userFrames: true,
    skipUpdates: true,
    transition: true,
    transitionParams: true,
    onStart: true,
    onUpdate: true,
    onComplete: true,
    onOverwrite: true,
    onError: true,
    rounded: true,
    min: true,
    max: true,
    onStartParams: true,
    onUpdateParams: true,
    onCompleteParams: true,
    onOverwriteParams: true,
    onStartScope: true,
    onUpdateScope: true,
    onCompleteScope: true,
    onOverwriteScope: true,
    onErrorScope: true
};

function _constructPropertyList(obj) {
    let properties = {};
    let modifiedProperties = {};

    for (let istr in obj) {
        if (restrictedWords[istr])
            continue;

        if (_specialPropertySplitterList[istr] != undefined) {
            // Special property splitter
            let splitProperties = _specialPropertySplitterList[istr].splitValues(obj[istr], _specialPropertySplitterList[istr].parameters);
            for (let i = 0; i < splitProperties.length; i++) {
                if (_specialPropertySplitterList[splitProperties[i].name] != undefined) {
                    let splitProperties2 = _specialPropertySplitterList[splitProperties[i].name].splitValues(splitProperties[i].value, _specialPropertySplitterList[splitProperties[i].name].parameters);
                    for (let j = 0; j < splitProperties2.length; j++) {
                        properties[splitProperties2[j].name] = {
                            valueStart: undefined,
                            valueComplete: splitProperties2[j].value,
                            arrayIndex: splitProperties2[j].arrayIndex,
                            isSpecialProperty: false
                        };
                    }
                } else {
                    properties[splitProperties[i].name] = {
                        valueStart: undefined,
                        valueComplete: splitProperties[i].value,
                        arrayIndex: splitProperties[i].arrayIndex,
                        isSpecialProperty: false
                    };
                }
            }
        } else if (_specialPropertyModifierList[istr] != undefined) {
            // Special property modifier
            let tempModifiedProperties = _specialPropertyModifierList[istr].modifyValues(obj[istr]);
            for (let i = 0; i < tempModifiedProperties.length; i++) {
                modifiedProperties[tempModifiedProperties[i].name] = {
                    modifierParameters: tempModifiedProperties[i].parameters,
                    modifierFunction: _specialPropertyModifierList[istr].getValue
                };
            }
        } else {
            properties[istr] = {
                valueStart: undefined,
                valueComplete: obj[istr]
            };
        }
    }

    // Adds the modifiers to the list of properties
    for (let istr in modifiedProperties) {
        if (properties[istr]) {
            properties[istr].modifierParameters = modifiedProperties[istr].modifierParameters;
            properties[istr].modifierFunction = modifiedProperties[istr].modifierFunction;
        }
    }

    return properties;
}

function PropertyInfo(valueStart, valueComplete, originalValueComplete,
    arrayIndex, extra, isSpecialProperty,
    modifierFunction, modifierParameters) {
    this._init(valueStart, valueComplete, originalValueComplete,
        arrayIndex, extra, isSpecialProperty,
        modifierFunction, modifierParameters);
}

PropertyInfo.prototype = {
    _init: function(valueStart, valueComplete, originalValueComplete,
        arrayIndex, extra, isSpecialProperty,
        modifierFunction, modifierParameters) {
        this.valueStart             =       valueStart;
        this.valueComplete          =       valueComplete;
        this.originalValueComplete  =       originalValueComplete;
        this.arrayIndex             =       arrayIndex;
        this.extra                  =       extra;
        this.isSpecialProperty      =       isSpecialProperty;
        this.hasModifier            =       Boolean(modifierFunction);
        this.modifierFunction       =       modifierFunction;
        this.modifierParameters     =       modifierParameters;
    }
};

function _addTweenOrCaller(target, tweeningParameters = {}, isCaller = false) {
    if (!target)
        return false;

    let scopes; // List of objects to tween
    if (target instanceof Array) {
        // The first argument is an array
        scopes = target.slice();
    } else {
        // The first argument(s) is(are) object(s)
        scopes = Array(target);
    }

    let obj, istr, properties = {};

    obj = tweeningParameters;

    if (!isCaller) {
        properties = _constructPropertyList(obj);

        // Verifies whether the properties exist or not, for warning messages
        for (istr in properties) {
            if (_specialPropertyList[istr] != undefined) {
                properties[istr].isSpecialProperty = true;
            } else {
                for (let i = 0, len = scopes.length; i < len; i++) {
                    if (scopes[i][istr] == undefined)
                        log('The property ' + istr + ' doesn\'t seem to be a normal object property of ' + scopes[i] + ' or a registered special property');
                }
                properties[istr].isSpecialProperty = false;
            }
        }
    }

    // Creates the main engine if it isn't active
    if (!_inited) _init();
    if (!_engineExists) _startEngine();

    // Creates a "safer", more strict tweening object
    let time = obj.time || 0;
    let delay = obj.delay || 0;

    let transition;

    // FIXME: Tweener allows you to use functions with an all lower-case name
    if (typeof obj.transition == 'string') {
        transition = imports.tweener.equations[obj.transition];
    } else {
        transition = obj.transition;
    }

    if (!transition)
        transition = imports.tweener.equations['easeOutExpo'];

    let tween, copyProperties;

    for (let i = 0; i < scopes.length; i++) {
        if (!isCaller) {
            // Make a copy of the properties
            copyProperties = {};
            for (istr in properties) {
                copyProperties[istr] = new PropertyInfo(properties[istr].valueStart,
                    properties[istr].valueComplete,
                    properties[istr].valueComplete,
                    properties[istr].arrayIndex || 0,
                    {},
                    properties[istr].isSpecialProperty || false,
                    properties[istr].modifierFunction || null,
                    properties[istr].modifierParameters || null);
            }
        }

        tween = new TweenList.TweenList(scopes[i],
            _ticker.getTime() + ((delay * 1000) / _timeScale),
            _ticker.getTime() + (((delay * 1000) + (time * 1000)) / _timeScale),
            false,
            transition,
            obj.transitionParams || null);

        tween.properties               =       isCaller ? null : copyProperties;
        tween.onStart                  =       obj.onStart;
        tween.onUpdate                 =       obj.onUpdate;
        tween.onComplete               =       obj.onComplete;
        tween.onOverwrite              =       obj.onOverwrite;
        tween.onError                  =       obj.onError;
        tween.onStartParams            =       obj.onStartParams;
        tween.onUpdateParams           =       obj.onUpdateParams;
        tween.onCompleteParams         =       obj.onCompleteParams;
        tween.onOverwriteParams        =       obj.onOverwriteParams;
        tween.onStartScope             =       obj.onStartScope;
        tween.onUpdateScope            =       obj.onUpdateScope;
        tween.onCompleteScope          =       obj.onCompleteScope;
        tween.onOverwriteScope         =       obj.onOverwriteScope;
        tween.onErrorScope             =       obj.onErrorScope;
        tween.rounded                  =       obj.rounded;
        tween.min                      =       obj.min;
        tween.max                      =       obj.max;
        tween.skipUpdates              =       obj.skipUpdates;
        tween.isCaller                 =       isCaller;

        if (isCaller) {
            tween.count = obj.count;
            tween.waitFrames = obj.waitFrames;
        }

        if (!isCaller) {
            // Remove other tweenings that occur at the same time
            removeTweensByTime(tween.scope, tween.properties, tween.timeStart, tween.timeComplete);
        }

        // And finally adds it to the list
        _tweenList.push(tween);

        // Immediate update and removal if it's an immediate tween
        // If not deleted, it executes at the end of this frame execution
        if (time == 0 && delay == 0) {
            let myT = _tweenList.length-1;
            _updateTweenByIndex(myT);
            _removeTweenByIndex(myT);
        }
    }

    return true;
}

function addTween(target, tweeningParameters) {
    return _addTweenOrCaller(target, tweeningParameters, false);
}

function addCaller(target, tweeningParameters) {
    return _addTweenOrCaller(target, tweeningParameters, true);
}

function removeTweensByTime(scope, properties, timeStart, timeComplete) {
    let removed = false;
    let removedLocally;
    let name;

    for (let i = 0; i < _tweenList.length; i++) {
        removedLocally = false;

        if (_tweenList[i] &&
            scope == _tweenList[i].scope &&
            timeComplete > _tweenList[i].timeStart &&
            timeStart < _tweenList[i].timeComplete) {

            for (name in _tweenList[i].properties) {
                if (properties[name]) {
                    if (!removedLocally) {
                        _callOnFunction(_tweenList[i].onOverwrite, 'onOverwrite', _tweenList[i].onOverwriteScope,
                            _tweenList[i].scope, _tweenList[i].onOverwriteParams);
                    }

                    _tweenList[i].properties[name] = undefined;
                    delete _tweenList[i].properties[name];
                    removedLocally = true;
                    removed = true;
                }
            }

            if (removedLocally &&
                !Object.keys(_tweenList[i].properties).length) {
                _removeTweenByIndex(i);
            }
        }
    }

    return removed;
}

function _pauseTweenByIndex(i) {
    let tweening = _tweenList[i];

    if (tweening == null || tweening.isPaused)
        return false;

    tweening.timePaused = _ticker.getTime();
    tweening.isPaused = true;

    return true;
}

function _splitTweens(tween, properties) {
    let originalTween = _tweenList[tween];
    let newTween = originalTween.clone();
    let name;

    for (let i = 0; i < properties.length; i++) {
        name = properties[i];
        if (originalTween.properties[name]) {
            originalTween.properties[name] = undefined;
            delete originalTween.properties[name];
        }
    }

    let found = false;
    for (name in newTween.properties) {
        found = false;
        for (let i = 0; i < properties.length; i++) {
            if (properties[i] == name) {
                found = true;
                break;
            }
        }

        if (!found) {
            newTween.properties[name] = undefined;
            delete newTween.properties[name];
        }
    }

    _tweenList.push(newTween);
    return _tweenList.length - 1;
}

function _affectTweens(affectFunction, scope, properties) {
    let affected = false;

    if (!_tweenList)
        return false;

    for (let i = 0; i < _tweenList.length; i++) {
        if (!_tweenList[i] || _tweenList[i].scope != scope)
            continue;

        if (properties.length == 0) {
            // Can check everything
            affectFunction(i);
            affected = true;
        } else {
            // Must check whether this tween must have specific properties affected
            let affectedProperties = [];
            for (let j = 0; j < properties.length; j++) {
                if (_tweenList[i].properties[properties[j]]) {
                    affectedProperties.push(properties[j]);
                }
            }

            if (affectedProperties.length > 0) {
                let objectProperties = Object.keys(_tweenList[i].properties).length;
                if (objectProperties == affectedProperties.length) {
                    // The list of properties is the same as all properties, so affect it all
                    affectFunction(i);
                    affected = true;
                } else {
                    // The properties are mixed, so split the tween and affect only certian specific
                    // properties
                    affectFunction(_splitTweens(i, affectedProperties));
                    affected = true;
                }
            }
        }
    }

    return affected;
}

function _affectTweensWithFunction(func, args) {
    let properties = [];
    let scope = args[0];
    let affected = false;
    let scopes;

    if (scope instanceof Array) {
        scopes = scope.concat();
    } else {
        scopes = Array(scope);
    }

    for (let i = 1; args[i] != undefined; i++) {
        if (typeof(args[i]) == 'string' && properties.indexOf(args[i]) === -1) {
            if (_specialPropertySplitterList[args[i]]) {
                // special property, get splitter array first
                let sps = _specialPropertySplitterList[arguments[i]];
                let specialProps = sps.splitValues(scope, null);
                for (let j = 0; j < specialProps.length; j++)
                    properties.push(specialProps[j].name);
            } else
                properties.push(args[i]);
        }
    }

    // the return now value means: "affect at least one tween"
    for (let i = 0; i < scopes.length; i++) {
        affected = affected || _affectTweens(func, scopes[i], properties);
    }

    return affected;
}

function resumeTweens() {
    return _affectTweensWithFunction(_resumeTweenByIndex, arguments);
}

function pauseTweens() {
    return _affectTweensWithFunction(_pauseTweenByIndex, arguments);
}

function removeTweens() {
    return _affectTweensWithFunction(_removeTweenByIndex, arguments);
}

function _mapOverTweens(func) {
    let rv = false;

    if (_tweenList == null)
        return false;

    for (let i = 0, len = _tweenList.length; i < len; i++) {
        if (func(i))
            rv = true;
    }

    return rv;
}

function pauseAllTweens() {
    return _mapOverTweens(_pauseTweenByIndex);
}

function resumeAllTweens() {
    return _mapOverTweens(_resumeTweenByIndex);
}

function removeAllTweens() {
    return _mapOverTweens(_removeTweenByIndex);
}

function getTweenCount(scope) {
    if (!_tweenList)
        return 0;

    let c = 0;

    for (let i = 0, len = _tweenList.length; i < len; i++) {
        if (_tweenList[i] && _tweenList[i].scope == scope)
            c += Object.keys(_tweenList[i].properties).length;
    }

    return c;
}

function registerSpecialProperty(name, getFunction, setFunction,
    parameters, preProcessFunction) {
    _specialPropertyList[name] = {
        getValue: getFunction,
        setValue: setFunction,
        parameters: parameters,
        preProcess: preProcessFunction
    };
}

function registerSpecialPropertyModifier(name, modifyFunction, getFunction) {
    _specialPropertyModifierList[name] = {
        modifyValues: modifyFunction,
        getValue: getFunction
    };
}

function registerSpecialPropertySplitter(name, splitFunction, parameters) {
    _specialPropertySplitterList[name] = {
        splitValues: splitFunction,
        parameters: parameters
    };
}

function setTimeScale(scale) {
    _timeScale = scale;
}

function getTimeScale() {
    return _timeScale;
}
