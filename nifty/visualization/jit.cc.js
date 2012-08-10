(function () {
    window.$jit = function (a) {
        var a = a || window,
            c;
        for (c in $jit) $jit[c].$extend && (a[c] = $jit[c])
    };
    $jit.version = "2.0.1";
    var h = function (a) {
        return document.getElementById(a)
    };
    h.empty = function () {};
    h.extend = function (a, c) {
        for (var b in c || {}) a[b] = c[b];
        return a
    };
    h.lambda = function (a) {
        return "function" == typeof a ? a : function () {
            return a
        }
    };
    h.time = Date.now || function () {
        return +new Date
    };
    h.splat = function (a) {
        var c = h.type(a);
        return c ? "array" != c ? [a] : a : []
    };
    h.type = function (a) {
        var c = h.type.s.call(a).match(/^\[object\s(.*)\]$/)[1].toLowerCase();
        return "object" != c ? c : a && a.$$family ? a.$$family : a && a.nodeName && 1 == a.nodeType ? "element" : c
    };
    h.type.s = Object.prototype.toString;
    h.each = function (a, c) {
        if ("object" == h.type(a)) for (var b in a) c(a[b], b);
        else {
            b = 0;
            for (var d = a.length; b < d; b++) c(a[b], b)
        }
    };
    h.indexOf = function (a, c) {
        if (Array.indexOf) return a.indexOf(c);
        for (var b = 0, d = a.length; b < d; b++) if (a[b] === c) return b;
        return -1
    };
    h.map = function (a, c) {
        var b = [];
        h.each(a, function (a, e) {
            b.push(c(a, e))
        });
        return b
    };
    h.reduce = function (a, c, b) {
        var d = a.length;
        if (0 == d) return b;
        for (var e = 3 == arguments.length ? b : a[--d]; d--;) e = c(e, a[d]);
        return e
    };
    h.merge = function () {
        for (var a = {}, c = 0, b = arguments.length; c < b; c++) {
            var d = arguments[c];
            if ("object" == h.type(d)) for (var e in d) {
                var f = d[e],
                    g = a[e];
                a[e] = g && "object" == h.type(f) && "object" == h.type(g) ? h.merge(g, f) : h.unlink(f)
            }
        }
        return a
    };
    h.unlink = function (a) {
        var c;
        switch (h.type(a)) {
        case "object":
            c = {};
            for (var b in a) c[b] = h.unlink(a[b]);
            break;
        case "array":
            c = [];
            b = 0;
            for (var d = a.length; b < d; b++) c[b] = h.unlink(a[b]);
            break;
        default:
            return a
        }
        return c
    };
    h.zip = function () {
        if (0 === arguments.length) return [];
        for (var a = 0, c = [], b = arguments.length, d = arguments[0].length; a < d; a++) {
            for (var e = 0, f = []; e < b; e++) f.push(arguments[e][a]);
            c.push(f)
        }
        return c
    };
    h.rgbToHex = function (a, c) {
        if (3 > a.length) return null;
        if (4 == a.length && 0 == a[3] && !c) return "transparent";
        for (var b = [], d = 0; 3 > d; d++) {
            var e = (a[d] - 0).toString(16);
            b.push(1 == e.length ? "0" + e : e)
        }
        return c ? b : "#" + b.join("")
    };
    h.hexToRgb = function (a) {
        if (7 != a.length) {
            a = a.match(/^#?(\w{1,2})(\w{1,2})(\w{1,2})$/);
            a.shift();
            if (3 != a.length) return null;
            for (var c = [], b = 0; 3 > b; b++) {
                var d = a[b];
                1 == d.length && (d += d);
                c.push(parseInt(d, 16))
            }
            return c
        }
        a = parseInt(a.slice(1), 16);
        return [a >> 16, a >> 8 & 255, a & 255]
    };
    h.destroy = function (a) {
        h.clean(a);
        a.parentNode && a.parentNode.removeChild(a);
        a.clearAttributes && a.clearAttributes()
    };
    h.clean = function (a) {
        for (var a = a.childNodes, c = 0, b = a.length; c < b; c++) h.destroy(a[c])
    };
    h.addEvent = function (a, c, b) {
        a.addEventListener ? a.addEventListener(c, b, !1) : a.attachEvent("on" + c, b)
    };
    h.addEvents = function (a, c) {
        for (var b in c) h.addEvent(a, b, c[b])
    };
    h.hasClass = function (a, c) {
        return -1 < (" " + a.className + " ").indexOf(" " + c + " ")
    };
    h.addClass = function (a, c) {
        h.hasClass(a, c) || (a.className = a.className + " " + c)
    };
    h.removeClass = function (a, c) {
        a.className = a.className.replace(RegExp("(^|\\s)" + c + "(?:\\s|$)"), "$1")
    };
    h.getPos = function (a) {
        var c = function (a) {
            for (var d = {
                x: 0,
                y: 0
            }; a && !/^(?:html)$/i.test(a.tagName);) d.x += a.offsetLeft, d.y += a.offsetTop, a = a.offsetParent;
            return d
        }(a),
            a = function (a) {
                for (var d = {
                    x: 0,
                    y: 0
                }; a;) {
                    d.x += a.scrollLeft;
                    d.y += a.scrollTop;
                    if (/^(?:html)$/i.test(a.tagName)) break;
                    a = a.parentNode
                }
                return d
            }(a);
        return {
            x: c.x + a.x,
            y: c.y + a.y
        }
    };
    h.event = {
        get: function (a, c) {
            c = c || window;
            return a || c.event
        },
        getWheel: function (a) {
            return a.wheelDelta ? a.wheelDelta / 120 : -(a.detail || 0) / 3
        },
        isRightClick: function (a) {
            return 3 == a.which || 2 == a.button
        },
        getPos: function (a, c) {
            var c = c || window,
                a = a || c.event,
                b = c.document,
                b = b.documentElement || b.body;
            a.touches && a.touches.length && (a = a.touches[0]);
            return {
                x: a.pageX || a.clientX + b.scrollLeft,
                y: a.pageY || a.clientY + b.scrollTop
            }
        },
        stop: function (a) {
            a.stopPropagation && a.stopPropagation();
            a.cancelBubble = !0;
            a.preventDefault ? a.preventDefault() : a.returnValue = !1
        }
    };
    $jit.util = $jit.id = h;
    var m = function (a) {
        var a = a || {}, c = function () {
            for (var a in this) "function" != typeof this[a] && (this[a] = h.unlink(this[a]));
            this.constructor = c;
            if (m.prototyping) return this;
            a = this.initialize ? this.initialize.apply(this, arguments) : this;
            this.$$family = "class";
            return a
        }, b;
        for (b in m.Mutators) a[b] && (a = m.Mutators[b](a, a[b]), delete a[b]);
        h.extend(c, this);
        c.constructor = m;
        c.prototype = a;
        return c
    };
    m.Mutators = {
        Implements: function (a,
        c) {
            h.each(h.splat(c), function (c) {
                m.prototyping = c;
                var c = "function" == typeof c ? new c : c,
                    d;
                for (d in c) d in a || (a[d] = c[d]);
                delete m.prototyping
            });
            return a
        }
    };
    h.extend(m, {
        inherit: function (a, c) {
            for (var b in c) {
                var d = c[b],
                    e = a[b],
                    f = h.type(d);
                e && "function" == f ? d != e && m.override(a, b, d) : a[b] = "object" == f ? h.merge(e, d) : d
            }
            return a
        },
        override: function (a, c, b) {
            var d = m.prototyping;
            d && a[c] != d[c] && (d = null);
            a[c] = function () {
                var e = this.parent;
                this.parent = d ? d[c] : a[c];
                var f = b.apply(this, arguments);
                this.parent = e;
                return f
            }
        }
    });
    m.prototype.implement = function () {
        var a = this.prototype;
        h.each(Array.prototype.slice.call(arguments || []), function (c) {
            m.inherit(a, c)
        });
        return this
    };
    $jit.Class = m;
    $jit.json = {
        prune: function (a, c) {
            this.each(a, function (a, d) {
                d == c && a.children && (delete a.children, a.children = [])
            })
        },
        getParent: function (a, c) {
            if (a.id == c) return !1;
            var b = a.children;
            if (b && 0 < b.length) for (var d = 0; d < b.length; d++) {
                if (b[d].id == c) return a;
                var e = this.getParent(b[d], c);
                if (e) return e
            }
            return !1
        },
        getSubtree: function (a, c) {
            if (a.id == c) return a;
            for (var b = 0, d = a.children; d && b < d.length; b++) {
                var e = this.getSubtree(d[b], c);
                if (null != e) return e
            }
            return null
        },
        eachLevel: function (a, c, b, d) {
            if (c <= b && (d(a, c), a.children)) for (var e = 0, a = a.children; e < a.length; e++) this.eachLevel(a[e], c + 1, b, d)
        },
        each: function (a, c) {
            this.eachLevel(a, 0, Number.MAX_VALUE, c)
        }
    };
    $jit.Trans = {
        $extend: !0,
        linear: function (a) {
            return a
        }
    };
    var z = $jit.Trans;
    (function () {
        var a = function (a, b) {
            b = h.splat(b);
            return h.extend(a, {
                easeIn: function (d) {
                    return a(d, b)
                },
                easeOut: function (d) {
                    return 1 - a(1 - d, b)
                },
                easeInOut: function (d) {
                    return 0.5 >= d ? a(2 * d, b) / 2 : (2 - a(2 * (1 - d), b)) / 2
                }
            })
        };
        h.each({
            Pow: function (a, b) {
                return Math.pow(a, b[0] || 6)
            },
            Expo: function (a) {
                return Math.pow(2, 8 * (a - 1))
            },
            Circ: function (a) {
                return 1 - Math.sin(Math.acos(a))
            },
            Sine: function (a) {
                return 1 - Math.sin((1 - a) * Math.PI / 2)
            },
            Back: function (a, b) {
                b = b[0] || 1.618;
                return Math.pow(a, 2) * ((b + 1) * a - b)
            },
            Bounce: function (a) {
                for (var b, d = 0, e = 1;; d += e, e /= 2) if (a >= (7 - 4 * d) / 11) {
                    b = e * e - Math.pow((11 - 6 * d - 11 * a) / 4, 2);
                    break
                }
                return b
            },
            Elastic: function (a, b) {
                return Math.pow(2, 10 * --a) * Math.cos(20 * a * Math.PI * (b[0] || 1) / 3)
            }
        }, function (c, b) {
            z[b] = a(c)
        });
        h.each(["Quad", "Cubic", "Quart", "Quint"], function (c, b) {
            z[c] = a(function (a) {
                return Math.pow(a, [b + 2])
            })
        })
    })();
    var D = new m({
        initialize: function (a) {
            this.setOptions(a)
        },
        setOptions: function (a) {
            this.opt = h.merge({
                duration: 2500,
                fps: 40,
                transition: z.Quart.easeInOut,
                compute: h.empty,
                complete: h.empty,
                link: "ignore"
            }, a || {});
            return this
        },
        step: function () {
            var a = h.time(),
                c = this.opt;
            a < this.time + c.duration ? (a = c.transition((a - this.time) / c.duration), c.compute(a)) : (this.timer = clearInterval(this.timer),
            c.compute(1), c.complete())
        },
        start: function () {
            if (!this.check()) return this;
            this.time = 0;
            this.startTimer();
            return this
        },
        startTimer: function () {
            var a = this,
                c = this.opt.fps;
            if (this.timer) return !1;
            this.time = h.time() - this.time;
            this.timer = setInterval(function () {
                a.step()
            }, Math.round(1E3 / c));
            return !0
        },
        pause: function () {
            this.stopTimer();
            return this
        },
        resume: function () {
            this.startTimer();
            return this
        },
        stopTimer: function () {
            if (!this.timer) return !1;
            this.time = h.time() - this.time;
            this.timer = clearInterval(this.timer);
            return !0
        },
        check: function () {
            return !this.timer ? !0 : "cancel" == this.opt.link ? (this.stopTimer(), !0) : !1
        }
    }),
        q = function () {
            for (var a = arguments, c = 0, b = a.length, d = {}; c < b; c++) {
                var e = q[a[c]];
                e.$extend ? h.extend(d, e) : d[a[c]] = e
            }
            return d
        };
    q.AreaChart = {
        $extend: !0,
        animate: !0,
        labelOffset: 3,
        type: "stacked",
        Tips: {
            enable: !1,
            onShow: h.empty,
            onHide: h.empty
        },
        Events: {
            enable: !1,
            onClick: h.empty
        },
        selectOnHover: !0,
        showAggregates: !0,
        showLabels: !0,
        filterOnClick: !1,
        restoreOnRightClick: !1
    };
    q.Margin = {
        $extend: !1,
        top: 0,
        left: 0,
        right: 0,
        bottom: 0
    };
    q.Canvas = {
        $extend: !0,
        injectInto: "id",
        type: "2D",
        width: !1,
        height: !1,
        useCanvas: !1,
        withLabels: !0,
        background: !1,
        Scene: {
            Lighting: {
                enable: !1,
                ambient: [1, 1, 1],
                directional: {
                    direction: {
                        x: -100,
                        y: -100,
                        z: -100
                    },
                    color: [0.5, 0.3, 0.1]
                }
            }
        }
    };
    q.Tree = {
        $extend: !0,
        orientation: "left",
        subtreeOffset: 8,
        siblingOffset: 5,
        indent: 10,
        multitree: !1,
        align: "center"
    };
    q.Node = {
        $extend: !1,
        overridable: !1,
        type: "circle",
        color: "#ccb",
        alpha: 1,
        dim: 3,
        height: 20,
        width: 90,
        autoHeight: !1,
        autoWidth: !1,
        lineWidth: 1,
        transform: !0,
        align: "center",
        angularWidth: 1,
        span: 1,
        CanvasStyles: {}
    };
    q.Edge = {
        $extend: !1,
        overridable: !1,
        type: "line",
        color: "#ccb",
        lineWidth: 1,
        dim: 15,
        alpha: 1,
        epsilon: 7,
        CanvasStyles: {}
    };
    q.Fx = {
        $extend: !0,
        fps: 40,
        duration: 2500,
        transition: $jit.Trans.Quart.easeInOut,
        clearCanvas: !0
    };
    q.Label = {
        $extend: !1,
        overridable: !1,
        type: "HTML",
        style: " ",
        size: 10,
        family: "sans-serif",
        textAlign: "center",
        textBaseline: "alphabetic",
        color: "#fff"
    };
    q.Tips = {
        $extend: !1,
        enable: !1,
        type: "auto",
        offsetX: 20,
        offsetY: 20,
        force: !1,
        onShow: h.empty,
        onHide: h.empty
    };
    q.NodeStyles = {
        $extend: !1,
        enable: !1,
        type: "auto",
        stylesHover: !1,
        stylesClick: !1
    };
    q.Events = {
        $extend: !1,
        enable: !1,
        enableForEdges: !1,
        type: "auto",
        onClick: h.empty,
        onRightClick: h.empty,
        onMouseMove: h.empty,
        onMouseEnter: h.empty,
        onMouseLeave: h.empty,
        onDragStart: h.empty,
        onDragMove: h.empty,
        onDragCancel: h.empty,
        onDragEnd: h.empty,
        onTouchStart: h.empty,
        onTouchMove: h.empty,
        onTouchEnd: h.empty,
        onMouseWheel: h.empty
    };
    q.Navigation = {
        $extend: !1,
        enable: !1,
        type: "auto",
        panning: !1,
        zooming: !1
    };
    q.Controller = {
        $extend: !0,
        onBeforeCompute: h.empty,
        onAfterCompute: h.empty,
        onCreateLabel: h.empty,
        onPlaceLabel: h.empty,
        onComplete: h.empty,
        onBeforePlotLine: h.empty,
        onAfterPlotLine: h.empty,
        onBeforePlotNode: h.empty,
        onAfterPlotNode: h.empty,
        request: !1
    };
    var E = {
        initialize: function (a, c) {
            this.viz = c;
            this.canvas = c.canvas;
            this.config = c.config[a];
            this.nodeTypes = c.fx.nodeTypes;
            var b = this.config.type;
            this.labelContainer = (this.dom = "auto" == b ? "Native" != c.config.Label.type : "Native" != b) && c.labels.getLabelContainer();
            this.isEnabled() && this.initializePost()
        },
        initializePost: h.empty,
        setAsProperty: h.lambda(!1),
        isEnabled: function () {
            return this.config.enable
        },
        isLabel: function (a, c, b) {
            var a = h.event.get(a, c),
                c = this.labelContainer,
                d = a.target || a.srcElement,
                a = a.relatedTarget;
            return b ? a && a == this.viz.canvas.getCtx().canvas && !! d && this.isDescendantOf(d, c) : this.isDescendantOf(d, c)
        },
        isDescendantOf: function (a, c) {
            for (; a && a.parentNode;) {
                if (a.parentNode == c) return a;
                a = a.parentNode
            }
            return !1
        }
    }, F = {
        onMouseUp: h.empty,
        onMouseDown: h.empty,
        onMouseMove: h.empty,
        onMouseOver: h.empty,
        onMouseOut: h.empty,
        onMouseWheel: h.empty,
        onTouchStart: h.empty,
        onTouchMove: h.empty,
        onTouchEnd: h.empty,
        onTouchCancel: h.empty
    }, L = new m({
        initialize: function (a) {
            this.viz = a;
            this.canvas = a.canvas;
            this.edge = this.node = !1;
            this.registeredObjects = [];
            this.attachEvents()
        },
        attachEvents: function () {
            var a = this.canvas.getElement(),
                c = this;
            a.oncontextmenu = h.lambda(!1);
            h.addEvents(a, {
                mouseup: function (a, b) {
                    var f = h.event.get(a, b);
                    c.handleEvent("MouseUp", a, b, c.makeEventObject(a, b), h.event.isRightClick(f))
                },
                mousedown: function (a, b) {
                    var f = h.event.get(a, b);
                    c.handleEvent("MouseDown", a, b,
                    c.makeEventObject(a, b), h.event.isRightClick(f))
                },
                mousemove: function (a, b) {
                    c.handleEvent("MouseMove", a, b, c.makeEventObject(a, b))
                },
                mouseover: function (a, b) {
                    c.handleEvent("MouseOver", a, b, c.makeEventObject(a, b))
                },
                mouseout: function (a, b) {
                    c.handleEvent("MouseOut", a, b, c.makeEventObject(a, b))
                },
                touchstart: function (a, b) {
                    c.handleEvent("TouchStart", a, b, c.makeEventObject(a, b))
                },
                touchmove: function (a, b) {
                    c.handleEvent("TouchMove", a, b, c.makeEventObject(a, b))
                },
                touchend: function (a, b) {
                    c.handleEvent("TouchEnd", a, b, c.makeEventObject(a,
                    b))
                }
            });
            var b = function (a, b) {
                var f = h.event.get(a, b),
                    f = h.event.getWheel(f);
                c.handleEvent("MouseWheel", a, b, f)
            };
            !document.getBoxObjectFor && null == window.mozInnerScreenX ? h.addEvent(a, "mousewheel", b) : a.addEventListener("DOMMouseScroll", b, !1)
        },
        register: function (a) {
            this.registeredObjects.push(a)
        },
        handleEvent: function () {
            for (var a = Array.prototype.slice.call(arguments), c = a.shift(), b = 0, d = this.registeredObjects, e = d.length; b < e; b++) d[b]["on" + c].apply(d[b], a)
        },
        makeEventObject: function (a, c) {
            var b = this,
                d = this.viz.graph,
                e = this.viz.fx,
                f = e.nodeTypes,
                g = e.edgeTypes;
            return {
                pos: !1,
                node: !1,
                edge: !1,
                contains: !1,
                getNodeCalled: !1,
                getEdgeCalled: !1,
                getPos: function () {
                    var d = b.viz.canvas,
                        e = d.getSize(),
                        f = d.getPos(),
                        g = d.translateOffsetX,
                        o = d.translateOffsetY,
                        r = d.scaleOffsetX,
                        d = d.scaleOffsetY,
                        u = h.event.getPos(a, c);
                    return this.pos = {
                        x: 1 * (u.x - f.x - e.width / 2 - g) / r,
                        y: 1 * (u.y - f.y - e.height / 2 - o) / d
                    }
                },
                getNode: function () {
                    if (this.getNodeCalled) return this.node;
                    this.getNodeCalled = !0;
                    for (var a in d.nodes) {
                        var c = d.nodes[a],
                            g = c && f[c.getData("type")];
                        if (g = g && g.contains && g.contains.call(e, c, this.getPos())) return this.contains = g, b.node = this.node = c
                    }
                    return b.node = this.node = !1
                },
                getEdge: function () {
                    if (this.getEdgeCalled) return this.edge;
                    this.getEdgeCalled = !0;
                    var a = {}, c;
                    for (c in d.edges) {
                        var f = d.edges[c];
                        a[c] = !0;
                        for (var h in f) if (!(h in a)) {
                            var o = f[h],
                                r = o && g[o.getData("type")];
                            if (r = r && r.contains && r.contains.call(e, o, this.getPos())) return this.contains = r, b.edge = this.edge = o
                        }
                    }
                    return b.edge = this.edge = !1
                },
                getContains: function () {
                    if (this.getNodeCalled) return this.contains;
                    this.getNode();
                    return this.contains
                }
            }
        }
    }),
        y = {
            initializeExtras: function () {
                var a = new L(this),
                    c = this;
                h.each(["NodeStyles", "Tips", "Navigation", "Events"], function (b) {
                    var d = new y.Classes[b](b, c);
                    d.isEnabled() && a.register(d);
                    d.setAsProperty() && (c[b.toLowerCase()] = d)
                })
            },
            Classes: {}
        };
    y.Classes.Events = new m({
        Implements: [E, F],
        initializePost: function () {
            this.fx = this.viz.fx;
            this.ntypes = this.viz.fx.nodeTypes;
            this.etypes = this.viz.fx.edgeTypes;
            this.moved = this.touchMoved = this.touched = this.pressed = this.hovered = !1
        },
        setAsProperty: h.lambda(!0),
        onMouseUp: function (a, c, b, d) {
            a = h.event.get(a, c);
            if (!this.moved) if (d) this.config.onRightClick(this.hovered, b, a);
            else this.config.onClick(this.pressed, b, a);
            if (this.pressed) {
                if (this.moved) this.config.onDragEnd(this.pressed, b, a);
                else this.config.onDragCancel(this.pressed, b, a);
                this.pressed = this.moved = !1
            }
        },
        onMouseOut: function (a, c, b) {
            var d = h.event.get(a, c),
                e;
            if (this.dom && (e = this.isLabel(a, c, !0))) this.config.onMouseLeave(this.viz.graph.getNode(e.id), b, d), this.hovered = !1;
            else {
                a = d.relatedTarget;
                for (c = this.canvas.getElement(); a && a.parentNode;) {
                    if (c == a.parentNode) return;
                    a = a.parentNode
                }
                this.hovered && (this.config.onMouseLeave(this.hovered, b, d), this.hovered = !1)
            }
        },
        onMouseOver: function (a, c, b) {
            var d = h.event.get(a, c),
                e;
            if (this.dom && (e = this.isLabel(a, c, !0))) this.hovered = this.viz.graph.getNode(e.id), this.config.onMouseEnter(this.hovered, b, d)
        },
        onMouseMove: function (a, c, b) {
            a = h.event.get(a, c);
            if (this.pressed) this.moved = !0, this.config.onDragMove(this.pressed, b, a);
            else if (this.dom) this.config.onMouseMove(this.hovered, b, a);
            else {
                if (this.hovered) {
                    var c = this.hovered,
                        d = c.nodeFrom ? this.etypes[c.getData("type")] : this.ntypes[c.getData("type")];
                    if (d && d.contains && d.contains.call(this.fx, c, b.getPos())) {
                        this.config.onMouseMove(c, b, a);
                        return
                    }
                    this.config.onMouseLeave(c, b, a);
                    this.hovered = !1
                }
                if (this.hovered = b.getNode() || this.config.enableForEdges && b.getEdge()) this.config.onMouseEnter(this.hovered, b, a);
                else this.config.onMouseMove(!1, b, a)
            }
        },
        onMouseWheel: function (a, c, b) {
            this.config.onMouseWheel(b, h.event.get(a, c))
        },
        onMouseDown: function (a, c, b) {
            var d = h.event.get(a,
            c);
            if (this.dom) {
                if (a = this.isLabel(a, c)) this.pressed = this.viz.graph.getNode(a.id)
            }
            else this.pressed = b.getNode() || this.config.enableForEdges && b.getEdge();
            this.pressed && this.config.onDragStart(this.pressed, b, d)
        },
        onTouchStart: function (a, c, b) {
            var d = h.event.get(a, c),
                e;
            (this.touched = this.dom && (e = this.isLabel(a, c)) ? this.viz.graph.getNode(e.id) : b.getNode() || this.config.enableForEdges && b.getEdge()) && this.config.onTouchStart(this.touched, b, d)
        },
        onTouchMove: function (a, c, b) {
            a = h.event.get(a, c);
            this.touched && (this.touchMoved = !0, this.config.onTouchMove(this.touched, b, a))
        },
        onTouchEnd: function (a, c, b) {
            a = h.event.get(a, c);
            if (this.touched) {
                if (this.touchMoved) this.config.onTouchEnd(this.touched, b, a);
                else this.config.onTouchCancel(this.touched, b, a);
                this.touched = this.touchMoved = !1
            }
        }
    });
    y.Classes.Tips = new m({
        Implements: [E, F],
        initializePost: function () {
            if (document.body) {
                var a = h("_tooltip") || document.createElement("div");
                a.id = "_tooltip";
                a.className = "tip";
                h.extend(a.style, {
                    position: "absolute",
                    display: "none",
                    zIndex: 13E3
                });
                document.body.appendChild(a);
                this.tip = a;
                this.node = !1
            }
        },
        setAsProperty: h.lambda(!0),
        onMouseOut: function (a, c) {
            h.event.get(a, c);
            if (this.dom && this.isLabel(a, c, !0)) this.hide(!0);
            else {
                for (var b = a.relatedTarget, d = this.canvas.getElement(); b && b.parentNode;) {
                    if (d == b.parentNode) return;
                    b = b.parentNode
                }
                this.hide(!1)
            }
        },
        onMouseOver: function (a, c) {
            var b;
            if (this.dom && (b = this.isLabel(a, c, !1))) this.node = this.viz.graph.getNode(b.id), this.config.onShow(this.tip, this.node, b)
        },
        onMouseMove: function (a, c, b) {
            this.dom && this.isLabel(a, c) && this.setTooltipPosition(h.event.getPos(a,
            c));
            if (!this.dom) {
                var d = b.getNode();
                if (d) {
                    if (this.config.force || !this.node || this.node.id != d.id) this.node = d, this.config.onShow(this.tip, d, b.getContains());
                    this.setTooltipPosition(h.event.getPos(a, c))
                }
                else this.hide(!0)
            }
        },
        setTooltipPosition: function (a) {
            var c = this.tip,
                b = c.style,
                d = this.config;
            b.display = "";
            var e = document.body.clientWidth,
                f = c.offsetWidth,
                c = c.offsetHeight,
                g = d.offsetX,
                d = d.offsetY;
            b.top = (a.y + d + c > document.body.clientHeight ? a.y - c - d : a.y + d) + "px";
            b.left = (a.x + f + g > e ? a.x - f - g : a.x + g) + "px"
        },
        hide: function (a) {
            this.tip.style.display = "none";
            a && this.config.onHide()
        }
    });
    y.Classes.NodeStyles = new m({
        Implements: [E, F],
        initializePost: function () {
            this.fx = this.viz.fx;
            this.types = this.viz.fx.nodeTypes;
            this.nStyles = this.config;
            this.nodeStylesOnHover = this.nStyles.stylesHover;
            this.nodeStylesOnClick = this.nStyles.stylesClick;
            this.hoveredNode = !1;
            this.fx.nodeFxAnimation = new D;
            this.move = this.down = !1
        },
        onMouseOut: function (a, c) {
            this.down = this.move = !1;
            if (this.hoveredNode) {
                this.dom && this.isLabel(a, c, !0) && this.toggleStylesOnHover(this.hoveredNode, !1);
                for (var b = a.relatedTarget, d = this.canvas.getElement(); b && b.parentNode;) {
                    if (d == b.parentNode) return;
                    b = b.parentNode
                }
                this.toggleStylesOnHover(this.hoveredNode, !1);
                this.hoveredNode = !1
            }
        },
        onMouseOver: function (a, c) {
            var b;
            if (this.dom && (b = this.isLabel(a, c, !0))) b = this.viz.graph.getNode(b.id), b.selected || (this.hoveredNode = b, this.toggleStylesOnHover(this.hoveredNode, !0))
        },
        onMouseDown: function (a, c, b, d) {
            if (!d) {
                var e;
                this.dom && (e = this.isLabel(a, c)) ? this.down = this.viz.graph.getNode(e.id) : this.dom || (this.down = b.getNode());
                this.move = !1
            }
        },
        onMouseUp: function (a, c, b, d) {
            if (!d) {
                if (!this.move) this.onClick(b.getNode());
                this.down = this.move = !1
            }
        },
        getRestoredStyles: function (a, c) {
            var b = {}, d = this["nodeStylesOn" + c],
                e;
            for (e in d) b[e] = a.styles["$" + e];
            return b
        },
        toggleStylesOnHover: function (a, c) {
            this.nodeStylesOnHover && this.toggleStylesOn("Hover", a, c)
        },
        toggleStylesOnClick: function (a, c) {
            this.nodeStylesOnClick && this.toggleStylesOn("Click", a, c)
        },
        toggleStylesOn: function (a, c, b) {
            var d = this.viz;
            if (b) {
                c.styles || (c.styles = h.merge(c.data, {}));
                for (var e in this["nodeStylesOn" + a]) b = "$" + e, b in c.styles || (c.styles[b] = c.getData(e));
                d.fx.nodeFx(h.extend({
                    elements: {
                        id: c.id,
                        properties: this["nodeStylesOn" + a]
                    },
                    transition: z.Quart.easeOut,
                    duration: 300,
                    fps: 40
                }, this.config))
            }
            else a = this.getRestoredStyles(c, a), d.fx.nodeFx(h.extend({
                elements: {
                    id: c.id,
                    properties: a
                },
                transition: z.Quart.easeOut,
                duration: 300,
                fps: 40
            }, this.config))
        },
        onClick: function (a) {
            if (a) {
                var c = this.nodeStylesOnClick;
                c && (a.selected ? (this.toggleStylesOnClick(a, !1), delete a.selected) : (this.viz.graph.eachNode(function (a) {
                    if (a.selected) {
                        for (var d in c) a.setData(d,
                        a.styles["$" + d], "end");
                        delete a.selected
                    }
                }), this.toggleStylesOnClick(a, !0), a.selected = !0, delete a.hovered, this.hoveredNode = !1))
            }
        },
        onMouseMove: function (a, c, b) {
            this.down && (this.move = !0);
            if (!this.dom || !this.isLabel(a, c)) {
                var d = this.nodeStylesOnHover;
                if (d && !this.dom) {
                    if (this.hoveredNode && (a = this.types[this.hoveredNode.getData("type")]) && a.contains && a.contains.call(this.fx, this.hoveredNode, b.getPos())) return;
                    b = b.getNode();
                    if ((this.hoveredNode || b) && !b.hovered) b && !b.selected ? (this.fx.nodeFxAnimation.stopTimer(),
                    this.viz.graph.eachNode(function (a) {
                        if (a.hovered && !a.selected) {
                            for (var b in d) a.setData(b, a.styles["$" + b], "end");
                            delete a.hovered
                        }
                    }), b.hovered = !0, this.hoveredNode = b, this.toggleStylesOnHover(b, !0)) : this.hoveredNode && !this.hoveredNode.selected && (this.fx.nodeFxAnimation.stopTimer(), this.toggleStylesOnHover(this.hoveredNode, !1), delete this.hoveredNode.hovered, this.hoveredNode = !1)
                }
            }
        }
    });
    y.Classes.Navigation = new m({
        Implements: [E, F],
        initializePost: function () {
            this.pressed = this.pos = !1
        },
        onMouseWheel: function (a,
        c, b) {
            this.config.zooming && (h.event.stop(h.event.get(a, c)), a = 1 + b * (this.config.zooming / 1E3), this.canvas.scale(a, a))
        },
        onMouseDown: function (a, c, b) {
            if (this.config.panning && !("avoid nodes" == this.config.panning && (this.dom ? this.isLabel(a, c) : b.getNode()))) {
                this.pressed = !0;
                this.pos = b.getPos();
                var a = this.canvas,
                    c = a.translateOffsetX,
                    b = a.translateOffsetY,
                    d = a.scaleOffsetY;
                this.pos.x *= a.scaleOffsetX;
                this.pos.x += c;
                this.pos.y *= d;
                this.pos.y += b
            }
        },
        onMouseMove: function (a, c, b) {
            if (this.config.panning && this.pressed && !("avoid nodes" == this.config.panning && (this.dom ? this.isLabel(a, c) : b.getNode()))) {
                var a = this.pos,
                    b = b.getPos(),
                    d = this.canvas,
                    e = d.translateOffsetX,
                    f = d.translateOffsetY,
                    c = d.scaleOffsetX,
                    d = d.scaleOffsetY;
                b.x *= c;
                b.y *= d;
                b.x += e;
                b.y += f;
                e = b.x - a.x;
                a = b.y - a.y;
                this.pos = b;
                this.canvas.translate(1 * e / c, 1 * a / d)
            }
        },
        onMouseUp: function () {
            this.config.panning && (this.pressed = !1)
        }
    });
    var w;
    (function () {
        function a(a, c) {
            var f = document.createElement(a),
                g;
            for (g in c) "object" == typeof c[g] ? h.extend(f[g], c[g]) : f[g] = c[g];
            "canvas" == a && (!b && G_vmlCanvasManager) && (f = G_vmlCanvasManager.initElement(document.body.appendChild(f)));
            return f
        }
        var c = typeof HTMLCanvasElement,
            b = "object" == c || "function" == c;
        $jit.Canvas = w = new m({
            canvases: [],
            pos: !1,
            element: !1,
            labelContainer: !1,
            translateOffsetX: 0,
            translateOffsetY: 0,
            scaleOffsetX: 1,
            scaleOffsetY: 1,
            initialize: function (d, b) {
                this.viz = d;
                this.opt = this.config = b;
                var c = "string" == h.type(b.injectInto) ? b.injectInto : b.injectInto.id,
                    g = b.type,
                    j = c + "-label",
                    i = h(c),
                    k = b.width || i.offsetWidth,
                    l = b.height || i.offsetHeight;
                this.id = c;
                var o = {
                    injectInto: c,
                    width: k,
                    height: l
                };
                this.element = a("div", {
                    id: c + "-canvaswidget",
                    style: {
                        position: "relative",
                        width: k + "px",
                        height: l + "px"
                    }
                });
                this.labelContainer = this.createLabelContainer(b.Label.type, j, o);
                this.canvases.push(new w.Base[g]({
                    config: h.extend({
                        idSuffix: "-canvas"
                    }, o),
                    plot: function () {
                        d.fx.plot()
                    },
                    resize: function () {
                        d.refresh()
                    }
                }));
                if (c = b.background) o = new w.Background[c.type](d, h.extend(c, o)), this.canvases.push(new w.Base[g](o));
                for (g = this.canvases.length; g--;) this.element.appendChild(this.canvases[g].canvas), 0 < g && this.canvases[g].plot();
                this.element.appendChild(this.labelContainer);
                i.appendChild(this.element);
                var r = null,
                    u = this;
                h.addEvent(window, "scroll", function () {
                    clearTimeout(r);
                    r = setTimeout(function () {
                        u.getPos(!0)
                    }, 500)
                })
            },
            getCtx: function (a) {
                return this.canvases[a || 0].getCtx()
            },
            getConfig: function () {
                return this.opt
            },
            getElement: function () {
                return this.element
            },
            getSize: function (a) {
                return this.canvases[a || 0].getSize()
            },
            resize: function (a, b) {
                this.getPos(!0);
                this.translateOffsetX = this.translateOffsetY = 0;
                this.scaleOffsetX = this.scaleOffsetY = 1;
                for (var c = 0, g = this.canvases.length; c < g; c++) this.canvases[c].resize(a, b);
                c = this.element.style;
                c.width = a + "px";
                c.height = b + "px";
                this.labelContainer && (this.labelContainer.style.width = a + "px")
            },
            translate: function (a, b, c) {
                this.translateOffsetX += a * this.scaleOffsetX;
                this.translateOffsetY += b * this.scaleOffsetY;
                for (var g = 0, j = this.canvases.length; g < j; g++) this.canvases[g].translate(a, b, c)
            },
            scale: function (a, b) {
                var c = this.scaleOffsetX * a,
                    g = this.scaleOffsetY * b,
                    j = this.translateOffsetX * (a - 1) / c,
                    h = this.translateOffsetY * (b - 1) / g;
                this.scaleOffsetX = c;
                this.scaleOffsetY = g;
                c = 0;
                for (g = this.canvases.length; c < g; c++) this.canvases[c].scale(a, b, !0);
                this.translate(j, h, !1)
            },
            getPos: function (a) {
                return a || !this.pos ? this.pos = h.getPos(this.getElement()) : this.pos
            },
            clear: function (a) {
                this.canvases[a || 0].clear()
            },
            path: function (a, b) {
                var c = this.canvases[0].getCtx();
                c.beginPath();
                b(c);
                c[a]();
                c.closePath()
            },
            createLabelContainer: function (d, b, c) {
                if ("HTML" == d || "Native" == d) return a("div", {
                    id: b,
                    style: {
                        overflow: "visible",
                        position: "absolute",
                        top: 0,
                        left: 0,
                        width: c.width + "px",
                        height: 0
                    }
                });
                if ("SVG" == d) {
                    d = document.createElementNS("http://www.w3.org/2000/svg", "svg:svg");
                    d.setAttribute("width", c.width);
                    d.setAttribute("height", c.height);
                    var g = d.style;
                    g.position = "absolute";
                    g.left = g.top = "0px";
                    g = document.createElementNS("http://www.w3.org/2000/svg", "svg:g");
                    g.setAttribute("width", c.width);
                    g.setAttribute("height", c.height);
                    g.setAttribute("x", 0);
                    g.setAttribute("y", 0);
                    g.setAttribute("id", b);
                    d.appendChild(g);
                    return d
                }
            }
        });
        w.Base = {};
        w.Base["2D"] = new m({
            translateOffsetX: 0,
            translateOffsetY: 0,
            scaleOffsetX: 1,
            scaleOffsetY: 1,
            initialize: function (a) {
                this.viz = a;
                this.opt = a.config;
                this.size = !1;
                this.createCanvas();
                this.translateToCenter()
            },
            createCanvas: function () {
                var d = this.opt,
                    b = d.width,
                    c = d.height;
                this.canvas = a("canvas", {
                    id: d.injectInto + d.idSuffix,
                    width: b,
                    height: c,
                    style: {
                        position: "absolute",
                        top: 0,
                        left: 0,
                        width: b + "px",
                        height: c + "px"
                    }
                })
            },
            getCtx: function () {
                return !this.ctx ? this.ctx = this.canvas.getContext("2d") : this.ctx
            },
            getSize: function () {
                if (this.size) return this.size;
                var a = this.canvas;
                return this.size = {
                    width: a.width,
                    height: a.height
                }
            },
            translateToCenter: function (a) {
                var b = this.getSize(),
                    c = a ? b.width - a.width - 2 * this.translateOffsetX : b.width;
                height = a ? b.height - a.height - 2 * this.translateOffsetY : b.height;
                b = this.getCtx();
                a && b.scale(1 / this.scaleOffsetX, 1 / this.scaleOffsetY);
                b.translate(c / 2, height / 2)
            },
            resize: function (a, c) {
                var f = this.getSize(),
                    g = this.canvas,
                    j = g.style;
                this.size = !1;
                g.width = a;
                g.height = c;
                j.width = a + "px";
                j.height = c + "px";
                b ? this.translateToCenter() : this.translateToCenter(f);
                this.translateOffsetX = this.translateOffsetY = 0;
                this.scaleOffsetX = this.scaleOffsetY = 1;
                this.clear();
                this.viz.resize(a, c, this)
            },
            translate: function (a, b, c) {
                var g = this.scaleOffsetY;
                this.translateOffsetX += a * this.scaleOffsetX;
                this.translateOffsetY += b * g;
                this.getCtx().translate(a, b);
                !c && this.plot()
            },
            scale: function (a, b, c) {
                this.scaleOffsetX *= a;
                this.scaleOffsetY *= b;
                this.getCtx().scale(a, b);
                !c && this.plot()
            },
            clear: function () {
                var a = this.getSize(),
                    b = this.translateOffsetX,
                    c = this.translateOffsetY,
                    g = this.scaleOffsetX,
                    j = this.scaleOffsetY;
                this.getCtx().clearRect(1 * (-a.width / 2 - b) / g, 1 * (-a.height / 2 - c) / j, 1 * a.width / g, 1 * a.height / j)
            },
            plot: function () {
                this.clear();
                this.viz.plot(this)
            }
        });
        w.Background = {};
        w.Background.Circles = new m({
            initialize: function (a, b) {
                this.viz = a;
                this.config = h.merge({
                    idSuffix: "-bkcanvas",
                    levelDistance: 100,
                    numberOfCircles: 6,
                    CanvasStyles: {},
                    offset: 0
                }, b)
            },
            resize: function (a, b, c) {
                this.plot(c)
            },
            plot: function (a) {
                var a = a.getCtx(),
                    b = this.config,
                    c = b.CanvasStyles,
                    g;
                for (g in c) a[g] = c[g];
                g = b.numberOfCircles;
                b = b.levelDistance;
                for (c = 1; c <= g; c++) a.beginPath(), a.arc(0,
                0, b * c, 0, 2 * Math.PI, !1), a.stroke(), a.closePath()
            }
        })
    })();
    var x = function (a, c) {
        this.theta = a || 0;
        this.rho = c || 0
    };
    $jit.Polar = x;
    x.prototype = {
        getc: function (a) {
            return this.toComplex(a)
        },
        getp: function () {
            return this
        },
        set: function (a) {
            a = a.getp();
            this.theta = a.theta;
            this.rho = a.rho
        },
        setc: function (a, c) {
            this.rho = Math.sqrt(a * a + c * c);
            this.theta = Math.atan2(c, a);
            0 > this.theta && (this.theta += 2 * Math.PI)
        },
        setp: function (a, c) {
            this.theta = a;
            this.rho = c
        },
        clone: function () {
            return new x(this.theta, this.rho)
        },
        toComplex: function (a) {
            var c = Math.cos(this.theta) * this.rho,
                b = Math.sin(this.theta) * this.rho;
            return a ? {
                x: c,
                y: b
            } : new t(c, b)
        },
        add: function (a) {
            return new x(this.theta + a.theta, this.rho + a.rho)
        },
        scale: function (a) {
            return new x(this.theta, this.rho * a)
        },
        equals: function (a) {
            return this.theta == a.theta && this.rho == a.rho
        },
        $add: function (a) {
            this.theta += a.theta;
            this.rho += a.rho;
            return this
        },
        $madd: function (a) {
            this.theta = (this.theta + a.theta) % (2 * Math.PI);
            this.rho += a.rho;
            return this
        },
        $scale: function (a) {
            this.rho *= a;
            return this
        },
        isZero: function () {
            var a = Math.abs;
            return 1.0E-4 > a(this.theta) && 1.0E-4 > a(this.rho)
        },
        interpolate: function (a, c) {
            var b = Math.PI,
                d = 2 * b,
                e = function (a) {
                    return 0 > a ? a % d + d : a % d
                }, f = this.theta,
                g = a.theta,
                j = Math.abs(f - g);
            return {
                theta: j == b ? f > g ? e(g + (f - d - g) * c) : e(g - d + (f - g) * c) : j >= b ? f > g ? e(g + (f - d - g) * c) : e(g - d + (f - (g - d)) * c) : e(g + (f - g) * c),
                rho: (this.rho - a.rho) * c + a.rho
            }
        }
    };
    x.KER = new x(0, 0);
    var t = function (a, c) {
        this.x = a || 0;
        this.y = c || 0
    };
    $jit.Complex = t;
    t.prototype = {
        getc: function () {
            return this
        },
        getp: function (a) {
            return this.toPolar(a)
        },
        set: function (a) {
            a = a.getc(!0);
            this.x = a.x;
            this.y = a.y
        },
        setc: function (a, c) {
            this.x = a;
            this.y = c
        },
        setp: function (a, c) {
            this.x = Math.cos(a) * c;
            this.y = Math.sin(a) * c
        },
        clone: function () {
            return new t(this.x, this.y)
        },
        toPolar: function (a) {
            var c = this.norm(),
                b = Math.atan2(this.y, this.x);
            0 > b && (b += 2 * Math.PI);
            return a ? {
                theta: b,
                rho: c
            } : new x(b, c)
        },
        norm: function () {
            return Math.sqrt(this.squaredNorm())
        },
        squaredNorm: function () {
            return this.x * this.x + this.y * this.y
        },
        add: function (a) {
            return new t(this.x + a.x, this.y + a.y)
        },
        prod: function (a) {
            return new t(this.x * a.x - this.y * a.y, this.y * a.x + this.x * a.y)
        },
        conjugate: function () {
            return new t(this.x, - this.y)
        },
        scale: function (a) {
            return new t(this.x * a, this.y * a)
        },
        equals: function (a) {
            return this.x == a.x && this.y == a.y
        },
        $add: function (a) {
            this.x += a.x;
            this.y += a.y;
            return this
        },
        $prod: function (a) {
            var c = this.x,
                b = this.y;
            this.x = c * a.x - b * a.y;
            this.y = b * a.x + c * a.y;
            return this
        },
        $conjugate: function () {
            this.y = -this.y;
            return this
        },
        $scale: function (a) {
            this.x *= a;
            this.y *= a;
            return this
        },
        $div: function (a) {
            var c = this.x,
                b = this.y,
                d = a.squaredNorm();
            this.x = c * a.x + b * a.y;
            this.y = b * a.x - c * a.y;
            return this.$scale(1 / d)
        },
        isZero: function () {
            var a = Math.abs;
            return 1.0E-4 > a(this.x) && 1.0E-4 > a(this.y)
        }
    };
    t.KER = new t(0, 0);
    $jit.Graph = new m({
        initialize: function (a, c, b, d) {
            var e = {
                klass: t,
                Node: {}
            };
            this.Node = c;
            this.Edge = b;
            this.Label = d;
            this.opt = h.merge(e, a || {});
            this.nodes = {};
            this.edges = {};
            var f = this;
            this.nodeList = {};
            for (var g in G) f.nodeList[g] = function (a) {
                return function () {
                    var d = Array.prototype.slice.call(arguments);
                    f.eachNode(function (b) {
                        b[a].apply(b, d)
                    })
                }
            }(g)
        },
        getNode: function (a) {
            return this.hasNode(a) ? this.nodes[a] : !1
        },
        get: function (a) {
            return this.getNode(a)
        },
        getByName: function (a) {
            for (var c in this.nodes) {
                var b = this.nodes[c];
                if (b.name == a) return b
            }
            return !1
        },
        getAdjacence: function (a, c) {
            return a in this.edges ? this.edges[a][c] : !1
        },
        addNode: function (a) {
            if (!this.nodes[a.id]) {
                var c = this.edges[a.id] = {};
                this.nodes[a.id] = new n.Node(h.extend({
                    id: a.id,
                    name: a.name,
                    data: h.merge(a.data || {}, {}),
                    adjacencies: c
                }, this.opt.Node), this.opt.klass, this.Node, this.Edge, this.Label)
            }
            return this.nodes[a.id]
        },
        addAdjacence: function (a,
        c, b) {
            this.hasNode(a.id) || this.addNode(a);
            this.hasNode(c.id) || this.addNode(c);
            a = this.nodes[a.id];
            c = this.nodes[c.id];
            if (!a.adjacentTo(c)) {
                var d = this.edges[a.id] = this.edges[a.id] || {}, e = this.edges[c.id] = this.edges[c.id] || {};
                d[c.id] = e[a.id] = new n.Adjacence(a, c, b, this.Edge, this.Label);
                return d[c.id]
            }
            return this.edges[a.id][c.id]
        },
        removeNode: function (a) {
            if (this.hasNode(a)) {
                delete this.nodes[a];
                var c = this.edges[a],
                    b;
                for (b in c) delete this.edges[b][a];
                delete this.edges[a]
            }
        },
        removeAdjacence: function (a, c) {
            delete this.edges[a][c];
            delete this.edges[c][a]
        },
        hasNode: function (a) {
            return a in this.nodes
        },
        empty: function () {
            this.nodes = {};
            this.edges = {}
        }
    });
    var n = $jit.Graph,
        G;
    (function () {
        var a = function (a, b, c, g, j) {
            var h, c = c || "current";
            "current" == c ? h = this.data : "start" == c ? h = this.startData : "end" == c && (h = this.endData);
            a = "$" + (a ? a + "-" : "") + b;
            return g ? h[a] : !this.Config.overridable ? j[b] || 0 : a in h ? h[a] : a in this.data ? this.data[a] : j[b] || 0
        }, c = function (a, b, c, g) {
            var g = g || "current",
                j;
            "current" == g ? j = this.data : "start" == g ? j = this.startData : "end" == g && (j = this.endData);
            j["$" + (a ? a + "-" : "") + b] = c
        }, b = function (a, b) {
            var a = "$" + (a ? a + "-" : ""),
                c = this;
            h.each(b, function (b) {
                b = a + b;
                delete c.data[b];
                delete c.endData[b];
                delete c.startData[b]
            })
        };
        G = {
            getData: function (b, c, f) {
                return a.call(this, "", b, c, f, this.Config)
            },
            setData: function (a, b, f) {
                c.call(this, "", a, b, f)
            },
            setDataset: function (a, b) {
                var a = h.splat(a),
                    c;
                for (c in b) for (var g = 0, j = h.splat(b[c]), i = a.length; g < i; g++) this.setData(c, j[g], a[g])
            },
            removeData: function () {
                b.call(this, "", Array.prototype.slice.call(arguments))
            },
            getCanvasStyle: function (b,
            c, f) {
                return a.call(this, "canvas", b, c, f, this.Config.CanvasStyles)
            },
            setCanvasStyle: function (a, b, f) {
                c.call(this, "canvas", a, b, f)
            },
            setCanvasStyles: function (a, b) {
                var a = h.splat(a),
                    c;
                for (c in b) for (var g = 0, j = h.splat(b[c]), i = a.length; g < i; g++) this.setCanvasStyle(c, j[g], a[g])
            },
            removeCanvasStyle: function () {
                b.call(this, "canvas", Array.prototype.slice.call(arguments))
            },
            getLabelData: function (b, c, f) {
                return a.call(this, "label", b, c, f, this.Label)
            },
            setLabelData: function (a, b, f) {
                c.call(this, "label", a, b, f)
            },
            setLabelDataset: function (a,
            b) {
                var a = h.splat(a),
                    c;
                for (c in b) for (var g = 0, j = h.splat(b[c]), i = a.length; g < i; g++) this.setLabelData(c, j[g], a[g])
            },
            removeLabelData: function () {
                b.call(this, "label", Array.prototype.slice.call(arguments))
            }
        }
    })();
    n.Node = new m({
        initialize: function (a, c, b, d, e) {
            c = {
                id: "",
                name: "",
                data: {},
                startData: {},
                endData: {},
                adjacencies: {},
                selected: !1,
                drawn: !1,
                exist: !1,
                angleSpan: {
                    begin: 0,
                    end: 0
                },
                pos: new c,
                startPos: new c,
                endPos: new c
            };
            h.extend(this, h.extend(c, a));
            this.Config = this.Node = b;
            this.Edge = d;
            this.Label = e
        },
        adjacentTo: function (a) {
            return a.id in this.adjacencies
        },
        getAdjacency: function (a) {
            return this.adjacencies[a]
        },
        getPos: function (a) {
            a = a || "current";
            if ("current" == a) return this.pos;
            if ("end" == a) return this.endPos;
            if ("start" == a) return this.startPos
        },
        setPos: function (a, c) {
            var c = c || "current",
                b;
            "current" == c ? b = this.pos : "end" == c ? b = this.endPos : "start" == c && (b = this.startPos);
            b.set(a)
        }
    });
    n.Node.implement(G);
    n.Adjacence = new m({
        initialize: function (a, c, b, d, e) {
            this.nodeFrom = a;
            this.nodeTo = c;
            this.data = b || {};
            this.startData = {};
            this.endData = {};
            this.Config = this.Edge = d;
            this.Label = e
        }
    });
    n.Adjacence.implement(G);
    n.Util = {
        filter: function (a) {
            if (!a || "string" != h.type(a)) return function () {
                return !0
            };
            var c = a.split(" ");
            return function (a) {
                for (var d = 0; d < c.length; d++) if (a[c[d]]) return !1;
                return !0
            }
        },
        getNode: function (a, c) {
            return a.nodes[c]
        },
        eachNode: function (a, c, b) {
            var b = this.filter(b),
                d;
            for (d in a.nodes) b(a.nodes[d]) && c(a.nodes[d])
        },
        each: function (a, c, b) {
            this.eachNode(a, c, b)
        },
        eachAdjacency: function (a, c, b) {
            var d = a.adjacencies,
                b = this.filter(b),
                e;
            for (e in d) {
                var f = d[e];
                if (b(f)) {
                    if (f.nodeFrom != a) {
                        var g = f.nodeFrom;
                        f.nodeFrom = f.nodeTo;
                        f.nodeTo = g
                    }
                    c(f, e)
                }
            }
        },
        computeLevels: function (a, c, b, d) {
            var b = b || 0,
                e = this.filter(d);
            this.eachNode(a, function (a) {
                a._flag = !1;
                a._depth = -1
            }, d);
            a = a.getNode(c);
            a._depth = b;
            for (var f = [a]; 0 != f.length;) {
                var g = f.pop();
                g._flag = !0;
                this.eachAdjacency(g, function (a) {
                    a = a.nodeTo;
                    !1 == a._flag && e(a) && (0 > a._depth && (a._depth = g._depth + 1 + b), f.unshift(a))
                }, d)
            }
        },
        eachBFS: function (a, c, b, d) {
            var e = this.filter(d);
            this.clean(a);
            for (var f = [a.getNode(c)]; 0 != f.length;) a = f.pop(), a._flag = !0, b(a, a._depth),
            this.eachAdjacency(a, function (a) {
                a = a.nodeTo;
                !1 == a._flag && e(a) && (a._flag = !0, f.unshift(a))
            }, d)
        },
        eachLevel: function (a, c, b, d, e) {
            var f = a._depth,
                g = this.filter(e),
                j = this,
                b = !1 === b ? Number.MAX_VALUE - f : b;
            (function k(a, b, c) {
                var e = a._depth;
                e >= b && (e <= c && g(a)) && d(a, e);
                e < c && j.eachAdjacency(a, function (a) {
                    a = a.nodeTo;
                    a._depth > e && k(a, b, c)
                })
            })(a, c + f, b + f)
        },
        eachSubgraph: function (a, c, b) {
            this.eachLevel(a, 0, !1, c, b)
        },
        eachSubnode: function (a, c, b) {
            this.eachLevel(a, 1, 1, c, b)
        },
        anySubnode: function (a, c, b) {
            var d = !1,
                c = c || h.lambda(!0),
                e = "string" == h.type(c) ? function (a) {
                    return a[c]
                } : c;
            this.eachSubnode(a, function (a) {
                e(a) && (d = !0)
            }, b);
            return d
        },
        getSubnodes: function (a, c, b) {
            var d = [],
                c = c || 0,
                e;
            "array" == h.type(c) ? (e = c[0], c = c[1]) : (e = c, c = Number.MAX_VALUE - a._depth);
            this.eachLevel(a, e, c, function (a) {
                d.push(a)
            }, b);
            return d
        },
        getParents: function (a) {
            var c = [];
            this.eachAdjacency(a, function (b) {
                b = b.nodeTo;
                b._depth < a._depth && c.push(b)
            });
            return c
        },
        isDescendantOf: function (a, c) {
            if (a.id == c) return !0;
            for (var b = this.getParents(a), d = !1, e = 0; !d && e < b.length; e++) d = d || this.isDescendantOf(b[e], c);
            return d
        },
        clean: function (a) {
            this.eachNode(a, function (a) {
                a._flag = !1
            })
        },
        getClosestNodeToOrigin: function (a, c, b) {
            return this.getClosestNodeToPos(a, x.KER, c, b)
        },
        getClosestNodeToPos: function (a, c, b, d) {
            var e = null,
                b = b || "current",
                c = c && c.getc(!0) || t.KER,
                f = function (a, b) {
                    var c = a.x - b.x,
                        d = a.y - b.y;
                    return c * c + d * d
                };
            this.eachNode(a, function (a) {
                e = null == e || f(a.getPos(b).getc(!0), c) < f(e.getPos(b).getc(!0), c) ? a : e
            }, d);
            return e
        }
    };
    h.each("get getNode each eachNode computeLevels eachBFS clean getClosestNodeToPos getClosestNodeToOrigin".split(" "),

    function (a) {
        n.prototype[a] = function () {
            return n.Util[a].apply(n.Util, [this].concat(Array.prototype.slice.call(arguments)))
        }
    });
    h.each("eachAdjacency eachLevel eachSubgraph eachSubnode anySubnode getSubnodes getParents isDescendantOf".split(" "), function (a) {
        n.Node.prototype[a] = function () {
            return n.Util[a].apply(n.Util, [this].concat(Array.prototype.slice.call(arguments)))
        }
    });
    n.Op = {
        options: {
            type: "nothing",
            duration: 2E3,
            hideLabels: !0,
            fps: 30
        },
        initialize: function (a) {
            this.viz = a
        },
        removeNode: function (a, c) {
            var b = this.viz,
                d = h.merge(this.options, b.controller, c),
                e = h.splat(a),
                f, g, j;
            switch (d.type) {
            case "nothing":
                for (f = 0; f < e.length; f++) b.graph.removeNode(e[f]);
                break;
            case "replot":
                this.removeNode(e, {
                    type: "nothing"
                });
                b.labels.clearLabels();
                b.refresh(!0);
                break;
            case "fade:seq":
            case "fade":
                g = this;
                for (f = 0; f < e.length; f++) j = b.graph.getNode(e[f]), j.setData("alpha", 0, "end");
                b.fx.animate(h.merge(d, {
                    modes: ["node-property:alpha"],
                    onComplete: function () {
                        g.removeNode(e, {
                            type: "nothing"
                        });
                        b.labels.clearLabels();
                        b.reposition();
                        b.fx.animate(h.merge(d, {
                            modes: ["linear"]
                        }))
                    }
                }));
                break;
            case "fade:con":
                g = this;
                for (f = 0; f < e.length; f++) j = b.graph.getNode(e[f]), j.setData("alpha", 0, "end"), j.ignore = !0;
                b.reposition();
                b.fx.animate(h.merge(d, {
                    modes: ["node-property:alpha", "linear"],
                    onComplete: function () {
                        g.removeNode(e, {
                            type: "nothing"
                        });
                        d.onComplete && d.onComplete()
                    }
                }));
                break;
            case "iter":
                g = this;
                b.fx.sequence({
                    condition: function () {
                        return 0 != e.length
                    },
                    step: function () {
                        g.removeNode(e.shift(), {
                            type: "nothing"
                        });
                        b.labels.clearLabels()
                    },
                    onComplete: function () {
                        d.onComplete && d.onComplete()
                    },
                    duration: Math.ceil(d.duration / e.length)
                });
                break;
            default:
                this.doError()
            }
        },
        removeEdge: function (a, c) {
            var b = this.viz,
                d = h.merge(this.options, b.controller, c),
                e = "string" == h.type(a[0]) ? [a] : a,
                f, g, j;
            switch (d.type) {
            case "nothing":
                for (f = 0; f < e.length; f++) b.graph.removeAdjacence(e[f][0], e[f][1]);
                break;
            case "replot":
                this.removeEdge(e, {
                    type: "nothing"
                });
                b.refresh(!0);
                break;
            case "fade:seq":
            case "fade":
                g = this;
                for (f = 0; f < e.length; f++)(j = b.graph.getAdjacence(e[f][0], e[f][1])) && j.setData("alpha", 0, "end");
                b.fx.animate(h.merge(d, {
                    modes: ["edge-property:alpha"],
                    onComplete: function () {
                        g.removeEdge(e, {
                            type: "nothing"
                        });
                        b.reposition();
                        b.fx.animate(h.merge(d, {
                            modes: ["linear"]
                        }))
                    }
                }));
                break;
            case "fade:con":
                g = this;
                for (f = 0; f < e.length; f++) if (j = b.graph.getAdjacence(e[f][0], e[f][1])) j.setData("alpha", 0, "end"), j.ignore = !0;
                b.reposition();
                b.fx.animate(h.merge(d, {
                    modes: ["edge-property:alpha", "linear"],
                    onComplete: function () {
                        g.removeEdge(e, {
                            type: "nothing"
                        });
                        d.onComplete && d.onComplete()
                    }
                }));
                break;
            case "iter":
                g = this;
                b.fx.sequence({
                    condition: function () {
                        return 0 != e.length
                    },
                    step: function () {
                        g.removeEdge(e.shift(), {
                            type: "nothing"
                        });
                        b.labels.clearLabels()
                    },
                    onComplete: function () {
                        d.onComplete()
                    },
                    duration: Math.ceil(d.duration / e.length)
                });
                break;
            default:
                this.doError()
            }
        },
        sum: function (a, c) {
            var b = this.viz,
                d = h.merge(this.options, b.controller, c),
                e = b.root,
                f;
            b.root = c.id || b.root;
            switch (d.type) {
            case "nothing":
                f = b.construct(a);
                f.eachNode(function (a) {
                    a.eachAdjacency(function (a) {
                        b.graph.addAdjacence(a.nodeFrom, a.nodeTo, a.data)
                    })
                });
                break;
            case "replot":
                b.refresh(!0);
                this.sum(a, {
                    type: "nothing"
                });
                b.refresh(!0);
                break;
            case "fade:seq":
            case "fade":
            case "fade:con":
                that = this;
                f = b.construct(a);
                var g = !this.preprocessSum(f) ? ["node-property:alpha"] : ["node-property:alpha", "edge-property:alpha"];
                b.reposition();
                "fade:con" != d.type ? b.fx.animate(h.merge(d, {
                    modes: ["linear"],
                    onComplete: function () {
                        b.fx.animate(h.merge(d, {
                            modes: g,
                            onComplete: function () {
                                d.onComplete()
                            }
                        }))
                    }
                })) : (b.graph.eachNode(function (a) {
                    a.id != e && a.pos.isZero() && (a.pos.set(a.endPos), a.startPos.set(a.endPos))
                }), b.fx.animate(h.merge(d, {
                    modes: ["linear"].concat(g)
                })));
                break;
            default:
                this.doError()
            }
        },
        morph: function (a, c, b) {
            var b = b || {}, d = this.viz,
                e = h.merge(this.options, d.controller, c),
                f = d.root,
                g;
            d.root = c.id || d.root;
            switch (e.type) {
            case "nothing":
                g = d.construct(a);
                g.eachNode(function (a) {
                    var b = d.graph.hasNode(a.id);
                    a.eachAdjacency(function (a) {
                        var b = !! d.graph.getAdjacence(a.nodeFrom.id, a.nodeTo.id);
                        d.graph.addAdjacence(a.nodeFrom, a.nodeTo, a.data);
                        if (b) {
                            var b = d.graph.getAdjacence(a.nodeFrom.id, a.nodeTo.id),
                                c;
                            for (c in a.data || {}) b.data[c] = a.data[c]
                        }
                    });
                    if (b) {
                        var b = d.graph.getNode(a.id),
                            c;
                        for (c in a.data || {}) b.data[c] = a.data[c]
                    }
                });
                d.graph.eachNode(function (a) {
                    a.eachAdjacency(function (a) {
                        g.getAdjacence(a.nodeFrom.id, a.nodeTo.id) || d.graph.removeAdjacence(a.nodeFrom.id, a.nodeTo.id)
                    });
                    g.hasNode(a.id) || d.graph.removeNode(a.id)
                });
                break;
            case "replot":
                d.labels.clearLabels(!0);
                this.morph(a, {
                    type: "nothing"
                });
                d.refresh(!0);
                d.refresh(!0);
                break;
            case "fade:seq":
            case "fade":
            case "fade:con":
                that = this;
                g = d.construct(a);
                var j = "node-property" in b && h.map(h.splat(b["node-property"]),

                function (a) {
                    return "$" + a
                });
                d.graph.eachNode(function (a) {
                    var b = g.getNode(a.id);
                    if (b) {
                        var b = b.data,
                            c;
                        for (c in b) j && -1 < h.indexOf(j, c) ? a.endData[c] = b[c] : a.data[c] = b[c]
                    }
                    else a.setData("alpha", 1), a.setData("alpha", 1, "start"), a.setData("alpha", 0, "end"), a.ignore = !0
                });
                d.graph.eachNode(function (a) {
                    a.ignore || a.eachAdjacency(function (a) {
                        if (!a.nodeFrom.ignore && !a.nodeTo.ignore) {
                            var b = g.getNode(a.nodeFrom.id),
                                a = g.getNode(a.nodeTo.id);
                            b.adjacentTo(a) || (a = d.graph.getAdjacence(b.id, a.id), i = !0, a.setData("alpha", 1),
                            a.setData("alpha", 1, "start"), a.setData("alpha", 0, "end"))
                        }
                    })
                });
                var i = this.preprocessSum(g),
                    a = !i ? ["node-property:alpha"] : ["node-property:alpha", "edge-property:alpha"];
                a[0] += "node-property" in b ? ":" + h.splat(b["node-property"]).join(":") : "";
                a[1] = (a[1] || "edge-property:alpha") + ("edge-property" in b ? ":" + h.splat(b["edge-property"]).join(":") : "");
                "label-property" in b && a.push("label-property:" + h.splat(b["label-property"]).join(":"));
                d.reposition ? d.reposition() : d.compute("end");
                d.graph.eachNode(function (a) {
                    a.id != f && a.pos.getp().equals(x.KER) && (a.pos.set(a.endPos), a.startPos.set(a.endPos))
                });
                d.fx.animate(h.merge(e, {
                    modes: [b.position || "polar"].concat(a),
                    onComplete: function () {
                        d.graph.eachNode(function (a) {
                            a.ignore && d.graph.removeNode(a.id)
                        });
                        d.graph.eachNode(function (a) {
                            a.eachAdjacency(function (a) {
                                a.ignore && d.graph.removeAdjacence(a.nodeFrom.id, a.nodeTo.id)
                            })
                        });
                        e.onComplete()
                    }
                }))
            }
        },
        contract: function (a, c) {
            var b = this.viz;
            !a.collapsed && a.anySubnode(h.lambda(!0)) && (c = h.merge(this.options, b.config, c || {}, {
                modes: ["node-property:alpha:span", "linear"]
            }), a.collapsed = !0, function e(a) {
                a.eachSubnode(function (a) {
                    a.ignore = !0;
                    a.setData("alpha", 0, "animate" == c.type ? "end" : "current");
                    e(a)
                })
            }(a), "animate" == c.type ? (b.compute("end"), b.rotated && b.rotate(b.rotated, "none", {
                property: "end"
            }), function f(b) {
                b.eachSubnode(function (b) {
                    b.setPos(a.getPos("end"), "end");
                    f(b)
                })
            }(a), b.fx.animate(c)) : "replot" == c.type && b.refresh())
        },
        expand: function (a, c) {
            if ("collapsed" in a) {
                var b = this.viz,
                    c = h.merge(this.options, b.config, c || {}, {
                        modes: ["node-property:alpha:span", "linear"]
                    });
                delete a.collapsed;
                (function e(a) {
                    a.eachSubnode(function (a) {
                        delete a.ignore;
                        a.setData("alpha", 1, "animate" == c.type ? "end" : "current");
                        e(a)
                    })
                })(a);
                "animate" == c.type ? (b.compute("end"), b.rotated && b.rotate(b.rotated, "none", {
                    property: "end"
                }), b.fx.animate(c)) : "replot" == c.type && b.refresh()
            }
        },
        preprocessSum: function (a) {
            var c = this.viz;
            a.eachNode(function (a) {
                c.graph.hasNode(a.id) || (c.graph.addNode(a), a = c.graph.getNode(a.id), a.setData("alpha", 0), a.setData("alpha", 0, "start"), a.setData("alpha", 1, "end"))
            });
            var b = !1;
            a.eachNode(function (a) {
                a.eachAdjacency(function (a) {
                    var d = c.graph.getNode(a.nodeFrom.id),
                        g = c.graph.getNode(a.nodeTo.id);
                    d.adjacentTo(g) || (a = c.graph.addAdjacence(d, g, a.data), d.startAlpha == d.endAlpha && g.startAlpha == g.endAlpha && (b = !0, a.setData("alpha", 0), a.setData("alpha", 0, "start"), a.setData("alpha", 1, "end")))
                })
            });
            return b
        }
    };
    var I = {
        none: {
            render: h.empty,
            contains: h.lambda(!1)
        },
        circle: {
            render: function (a, c, b, d) {
                d = d.getCtx();
                d.beginPath();
                d.arc(c.x, c.y, b, 0, 2 * Math.PI, !0);
                d.closePath();
                d[a]()
            },
            contains: function (a,
            c, b) {
                var d = a.x - c.x,
                    a = a.y - c.y;
                return d * d + a * a <= b * b
            }
        },
        ellipse: {
            render: function (a, c, b, d, e) {
                var e = e.getCtx(),
                    f = 1,
                    g = 1,
                    j = 1,
                    h = 1,
                    k = 0;
                b > d ? (k = b / 2, g = d / b, h = b / d) : (k = d / 2, f = b / d, j = d / b);
                e.save();
                e.scale(f, g);
                e.beginPath();
                e.arc(c.x * j, c.y * h, k, 0, 2 * Math.PI, !0);
                e.closePath();
                e[a]();
                e.restore()
            },
            contains: function (a, c, b, d) {
                var e = 0,
                    f = 1,
                    g = 1,
                    j = 0,
                    h = 0;
                b > d ? (e = b / 2, g = d / b) : (e = d / 2, f = b / d);
                j = (a.x - c.x) * (1 / f);
                h = (a.y - c.y) * (1 / g);
                return j * j + h * h <= e * e
            }
        },
        square: {
            render: function (a, c, b, d) {
                d.getCtx()[a + "Rect"](c.x - b, c.y - b, 2 * b, 2 * b)
            },
            contains: function (a,
            c, b) {
                return Math.abs(c.x - a.x) <= b && Math.abs(c.y - a.y) <= b
            }
        },
        rectangle: {
            render: function (a, c, b, d, e) {
                e.getCtx()[a + "Rect"](c.x - b / 2, c.y - d / 2, b, d)
            },
            contains: function (a, c, b, d) {
                return Math.abs(c.x - a.x) <= b / 2 && Math.abs(c.y - a.y) <= d / 2
            }
        },
        triangle: {
            render: function (a, c, b, d) {
                var d = d.getCtx(),
                    e = c.x,
                    f = c.y - b,
                    g = e - b,
                    c = c.y + b,
                    b = e + b;
                d.beginPath();
                d.moveTo(e, f);
                d.lineTo(g, c);
                d.lineTo(b, c);
                d.closePath();
                d[a]()
            },
            contains: function (a, c, b) {
                return I.circle.contains(a, c, b)
            }
        },
        star: {
            render: function (a, c, b, d) {
                var d = d.getCtx(),
                    e = Math.PI / 5;
                d.save();
                d.translate(c.x, c.y);
                d.beginPath();
                d.moveTo(b, 0);
                for (c = 0; 9 > c; c++) d.rotate(e), 0 == c % 2 ? d.lineTo(0.200811 * (b / 0.525731), 0) : d.lineTo(b, 0);
                d.closePath();
                d[a]();
                d.restore()
            },
            contains: function (a, c, b) {
                return I.circle.contains(a, c, b)
            }
        }
    }, K = {
        line: {
            render: function (a, c, b) {
                b = b.getCtx();
                b.beginPath();
                b.moveTo(a.x, a.y);
                b.lineTo(c.x, c.y);
                b.stroke()
            },
            contains: function (a, c, b, d) {
                var e = Math.min,
                    f = Math.max,
                    g = e(a.x, c.x),
                    j = f(a.x, c.x),
                    e = e(a.y, c.y),
                    f = f(a.y, c.y);
                return b.x >= g && b.x <= j && b.y >= e && b.y <= f ? Math.abs(c.x - a.x) <= d ? !0 : Math.abs((c.y - a.y) / (c.x - a.x) * (b.x - a.x) + a.y - b.y) <= d : !1
            }
        },
        arrow: {
            render: function (a, c, b, d, e) {
                e = e.getCtx();
                d && (d = a, a = c, c = d);
                d = new t(c.x - a.x, c.y - a.y);
                d.$scale(b / d.norm());
                var b = new t(c.x - d.x, c.y - d.y),
                    f = new t(-d.y / 2, d.x / 2),
                    d = b.add(f),
                    b = b.$add(f.$scale(-1));
                e.beginPath();
                e.moveTo(a.x, a.y);
                e.lineTo(c.x, c.y);
                e.stroke();
                e.beginPath();
                e.moveTo(d.x, d.y);
                e.lineTo(b.x, b.y);
                e.lineTo(c.x, c.y);
                e.closePath();
                e.fill()
            },
            contains: function (a, c, b, d) {
                return K.line.contains(a, c, b, d)
            }
        },
        hyperline: {
            render: function (a, c,
            b, d) {
                function e(a, b) {
                    return a < b ? a + Math.PI > b ? !1 : !0 : b + Math.PI > a ? !0 : !1
                }
                var d = d.getCtx(),
                    f = function (a, b) {
                        var c = a.x * b.y - a.y * b.x,
                            d = a.squaredNorm(),
                            e = b.squaredNorm();
                        if (0 == c) return {
                            x: 0,
                            y: 0,
                            ratio: -1
                        };
                        var f = (a.y * e - b.y * d + a.y - b.y) / c,
                            c = (b.x * d - a.x * e + b.x - a.x) / c,
                            d = -f / 2,
                            e = -c / 2,
                            h = (f * f + c * c) / 4 - 1;
                        if (0 > h) return {
                            x: 0,
                            y: 0,
                            ratio: -1
                        };
                        h = Math.sqrt(h);
                        return {
                            x: d,
                            y: e,
                            ratio: 1E3 < h ? -1 : h,
                            a: f,
                            b: c
                        }
                    }(a, c);
                1E3 < f.a || 1E3 < f.b || 0 > f.ratio ? (d.beginPath(), d.moveTo(a.x * b, a.y * b), d.lineTo(c.x * b, c.y * b)) : (c = Math.atan2(c.y - f.y, c.x - f.x), a = Math.atan2(a.y - f.y, a.x - f.x), e = e(c, a), d.beginPath(), d.arc(f.x * b, f.y * b, f.ratio * b, c, a, e));
                d.stroke()
            },
            contains: h.lambda(!1)
        }
    };
    n.Plot = {
        initialize: function (a, c) {
            this.viz = a;
            this.config = a.config;
            this.node = a.config.Node;
            this.edge = a.config.Edge;
            this.animation = new D;
            this.nodeTypes = new c.Plot.NodeTypes;
            this.edgeTypes = new c.Plot.EdgeTypes;
            this.labels = a.labels
        },
        nodeHelper: I,
        edgeHelper: K,
        Interpolator: {
            map: {
                border: "color",
                color: "color",
                width: "number",
                height: "number",
                dim: "number",
                alpha: "number",
                lineWidth: "number",
                angularWidth: "number",
                span: "number",
                valueArray: "array-number",
                dimArray: "array-number",
                colorArray: "array-color"
            },
            canvas: {
                globalAlpha: "number",
                fillStyle: "color",
                strokeStyle: "color",
                lineWidth: "number",
                shadowBlur: "number",
                shadowColor: "color",
                shadowOffsetX: "number",
                shadowOffsetY: "number",
                miterLimit: "number"
            },
            label: {
                size: "number",
                color: "color"
            },
            compute: function (a, c, b) {
                return a + (c - a) * b
            },
            computeColor: function (a, c, b) {
                var d = this.compute;
                return h.rgbToHex([parseInt(d(a[0], c[0], b)), parseInt(d(a[1], c[1], b)), parseInt(d(a[2], c[2], b))])
            },
            moebius: function (a, c, b, d) {
                c = d.scale(-b);
                if (1 > c.norm()) {
                    var b = c.x,
                        d = c.y,
                        e = a.startPos.getc().moebiusTransformation(c);
                    a.pos.setc(e.x, e.y);
                    c.x = b;
                    c.y = d
                }
            },
            linear: function (a, c, b) {
                var c = a.startPos.getc(!0),
                    d = a.endPos.getc(!0);
                a.pos.setc(this.compute(c.x, d.x, b), this.compute(c.y, d.y, b))
            },
            polar: function (a, c, b) {
                c = a.startPos.getp(!0);
                b = a.endPos.getp().interpolate(c, b);
                a.pos.setp(b.theta, b.rho)
            },
            number: function (a, c, b, d, e) {
                var f = a[d](c, "start"),
                    d = a[d](c, "end");
                a[e](c, this.compute(f, d, b))
            },
            color: function (a, c, b,
            d, e) {
                var f = h.hexToRgb(a[d](c, "start")),
                    d = h.hexToRgb(a[d](c, "end"));
                a[e](c, this.computeColor(f, d, b))
            },
            "array-number": function (a, c, b, d, e) {
                for (var f = a[d](c, "start"), d = a[d](c, "end"), g = [], j = 0, h = f.length; j < h; j++) {
                    var k = f[j],
                        l = d[j];
                    if (k.length) {
                        for (var o = 0, r = k.length, u = []; o < r; o++) u.push(this.compute(k[o], l[o], b));
                        g.push(u)
                    }
                    else g.push(this.compute(k, l, b))
                }
                a[e](c, g)
            },
            "array-color": function (a, c, b, d, e) {
                for (var f = a[d](c, "start"), d = a[d](c, "end"), g = [], j = 0, i = f.length; j < i; j++) {
                    var k = h.hexToRgb(f[j]),
                        l = h.hexToRgb(d[j]);
                    g.push(this.computeColor(k, l, b))
                }
                a[e](c, g)
            },
            node: function (a, c, b, d, e, f) {
                d = this[d];
                if (c) for (var g = c.length, j = 0; j < g; j++) {
                    var h = c[j];
                    this[d[h]](a, h, b, e, f)
                }
                else for (h in d) this[d[h]](a, h, b, e, f)
            },
            edge: function (a, c, b, d, e, f) {
                var a = a.adjacencies,
                    g;
                for (g in a) this.node(a[g], c, b, d, e, f)
            },
            "node-property": function (a, c, b) {
                this.node(a, c, b, "map", "getData", "setData")
            },
            "edge-property": function (a, c, b) {
                this.edge(a, c, b, "map", "getData", "setData")
            },
            "label-property": function (a, c, b) {
                this.node(a, c, b, "label", "getLabelData", "setLabelData")
            },
            "node-style": function (a, c, b) {
                this.node(a, c, b, "canvas", "getCanvasStyle", "setCanvasStyle")
            },
            "edge-style": function (a, c, b) {
                this.edge(a, c, b, "canvas", "getCanvasStyle", "setCanvasStyle")
            }
        },
        sequence: function (a) {
            var c = this,
                a = h.merge({
                    condition: h.lambda(!1),
                    step: h.empty,
                    onComplete: h.empty,
                    duration: 200
                }, a || {}),
                b = setInterval(function () {
                    a.condition() ? a.step() : (clearInterval(b), a.onComplete());
                    c.viz.refresh(!0)
                }, a.duration)
        },
        prepare: function (a) {
            var c = this.viz.graph,
                b = {
                    "node-property": {
                        getter: "getData",
                        setter: "setData"
                    },
                    "edge-property": {
                        getter: "getData",
                        setter: "setData"
                    },
                    "node-style": {
                        getter: "getCanvasStyle",
                        setter: "setCanvasStyle"
                    },
                    "edge-style": {
                        getter: "getCanvasStyle",
                        setter: "setCanvasStyle"
                    }
                }, d = {};
            if ("array" == h.type(a)) for (var e = 0, f = a.length; e < f; e++) {
                var g = a[e].split(":");
                d[g.shift()] = g
            }
            else for (e in a) "position" == e ? d[a.position] = [] : d[e] = h.splat(a[e]);
            c.eachNode(function (a) {
                a.startPos.set(a.pos);
                h.each(["node-property", "node-style"], function (c) {
                    if (c in d) for (var e = d[c], f = 0, g = e.length; f < g; f++) a[b[c].setter](e[f],
                    a[b[c].getter](e[f]), "start")
                });
                h.each(["edge-property", "edge-style"], function (c) {
                    if (c in d) {
                        var e = d[c];
                        a.eachAdjacency(function (a) {
                            for (var d = 0, f = e.length; d < f; d++) a[b[c].setter](e[d], a[b[c].getter](e[d]), "start")
                        })
                    }
                })
            });
            return d
        },
        animate: function (a, c) {
            var a = h.merge(this.viz.config, a || {}),
                b = this,
                d = this.viz.graph,
                e = this.Interpolator,
                f = "nodefx" === a.type ? this.nodeFxAnimation : this.animation,
                g = this.prepare(a.modes);
            a.hideLabels && this.labels.hideLabels(!0);
            f.setOptions(h.extend(a, {
                $animating: !1,
                compute: function (f) {
                    d.eachNode(function (a) {
                        for (var b in g) e[b](a,
                        g[b], f, c)
                    });
                    b.plot(a, this.$animating, f);
                    this.$animating = !0
                },
                complete: function () {
                    a.hideLabels && b.labels.hideLabels(!1);
                    b.plot(a);
                    a.onComplete()
                }
            })).start()
        },
        nodeFx: function (a) {
            var c = this.viz,
                b = c.graph,
                d = this.nodeFxAnimation,
                e = h.merge(this.viz.config, {
                    elements: {
                        id: !1,
                        properties: {}
                    },
                    reposition: !1
                }),
                a = h.merge(e, a || {}, {
                    onBeforeCompute: h.empty,
                    onAfterCompute: h.empty
                });
            d.stopTimer();
            var f = a.elements.properties;
            a.elements.id ? (d = h.splat(a.elements.id), h.each(d, function (a) {
                if (a = b.getNode(a)) for (var c in f) a.setData(c,
                f[c], "end")
            })) : b.eachNode(function (a) {
                for (var b in f) a.setData(b, f[b], "end")
            });
            var d = [],
                g;
            for (g in f) d.push(g);
            g = ["node-property:" + d.join(":")];
            a.reposition && (g.push("linear"), c.compute("end"));
            this.animate(h.merge(a, {
                modes: g,
                type: "nodefx"
            }))
        },
        plot: function (a, c) {
            var b = this.viz,
                d = b.graph,
                e = b.canvas,
                b = b.root,
                f = this;
            e.getCtx();
            a = a || this.viz.controller;
            a.clearCanvas && e.clear();
            if (b = d.getNode(b)) {
                var g = !! b.visited;
                d.eachNode(function (b) {
                    var d = b.getData("alpha");
                    b.eachAdjacency(function (d) {
                        var h = d.nodeTo; !! h.visited === g && (b.drawn && h.drawn) && (!c && a.onBeforePlotLine(d), f.plotLine(d, e, c), !c && a.onAfterPlotLine(d))
                    });
                    b.drawn && (!c && a.onBeforePlotNode(b), f.plotNode(b, e, c), !c && a.onAfterPlotNode(b));
                    !f.labelsHidden && a.withLabels && (b.drawn && 0.95 <= d ? f.labels.plotLabel(e, b, a) : f.labels.hideLabel(b, !1));
                    b.visited = !g
                })
            }
        },
        plotTree: function (a, c, b) {
            var d = this,
                e = this.viz.canvas;
            e.getCtx();
            var f = a.getData("alpha");
            a.eachSubnode(function (f) {
                if (c.plotSubtree(a, f) && f.exist && f.drawn) {
                    var h = a.getAdjacency(f.id);
                    !b && c.onBeforePlotLine(h);
                    d.plotLine(h, e, b);
                    !b && c.onAfterPlotLine(h);
                    d.plotTree(f, c, b)
                }
            });
            a.drawn ? (!b && c.onBeforePlotNode(a), this.plotNode(a, e, b), !b && c.onAfterPlotNode(a), !c.hideLabels && c.withLabels && 0.95 <= f ? this.labels.plotLabel(e, a, c) : this.labels.hideLabel(a, !1)) : this.labels.hideLabel(a, !0)
        },
        plotNode: function (a, c, b) {
            var d = a.getData("type"),
                e = this.node.CanvasStyles;
            if ("none" != d) {
                var f = a.getData("lineWidth"),
                    g = a.getData("color"),
                    h = a.getData("alpha"),
                    i = c.getCtx();
                i.save();
                i.lineWidth = f;
                i.fillStyle = i.strokeStyle = g;
                i.globalAlpha = h;
                for (var k in e) i[k] = a.getCanvasStyle(k);
                this.nodeTypes[d].render.call(this, a, c, b);
                i.restore()
            }
        },
        plotLine: function (a, c, b) {
            var d = a.getData("type"),
                e = this.edge.CanvasStyles;
            if ("none" != d) {
                var f = a.getData("lineWidth"),
                    g = a.getData("color"),
                    h = c.getCtx(),
                    i = a.nodeFrom,
                    k = a.nodeTo;
                h.save();
                h.lineWidth = f;
                h.fillStyle = h.strokeStyle = g;
                h.globalAlpha = Math.min(i.getData("alpha"), k.getData("alpha"), a.getData("alpha"));
                for (var l in e) h[l] = a.getCanvasStyle(l);
                this.edgeTypes[d].render.call(this, a, c, b);
                h.restore()
            }
        }
    };
    n.Label = {};
    n.Label.Native = new m({
        initialize: function (a) {
            this.viz = a
        },
        plotLabel: function (a, c, b) {
            var d = a.getCtx();
            c.pos.getc(!0);
            d.font = c.getLabelData("style") + " " + c.getLabelData("size") + "px " + c.getLabelData("family");
            d.textAlign = c.getLabelData("textAlign");
            d.fillStyle = d.strokeStyle = c.getLabelData("color");
            d.textBaseline = c.getLabelData("textBaseline");
            this.renderLabel(a, c, b)
        },
        renderLabel: function (a, c) {
            var b = a.getCtx(),
                d = c.pos.getc(!0);
            b.fillText(c.name, d.x, d.y + c.getData("height") / 2)
        },
        hideLabel: h.empty,
        hideLabels: h.empty
    });
    n.Label.DOM = new m({
        labelsHidden: !1,
        labelContainer: !1,
        labels: {},
        getLabelContainer: function () {
            return this.labelContainer ? this.labelContainer : this.labelContainer = document.getElementById(this.viz.config.labelContainer)
        },
        getLabel: function (a) {
            return a in this.labels && null != this.labels[a] ? this.labels[a] : this.labels[a] = document.getElementById(a)
        },
        hideLabels: function (a) {
            this.getLabelContainer().style.display = a ? "none" : "";
            this.labelsHidden = a
        },
        clearLabels: function (a) {
            for (var c in this.labels) if (a || !this.viz.graph.hasNode(c)) this.disposeLabel(c), delete this.labels[c]
        },
        disposeLabel: function (a) {
            (a = this.getLabel(a)) && a.parentNode && a.parentNode.removeChild(a)
        },
        hideLabel: function (a, c) {
            var a = h.splat(a),
                b = c ? "" : "none",
                d = this;
            h.each(a, function (a) {
                if (a = d.getLabel(a.id)) a.style.display = b
            })
        },
        fitsInCanvas: function (a, c) {
            var b = c.getSize();
            return a.x >= b.width || 0 > a.x || a.y >= b.height || 0 > a.y ? !1 : !0
        }
    });
    n.Label.HTML = new m({
        Implements: n.Label.DOM,
        plotLabel: function (a, c, b) {
            var a = c.id,
                d = this.getLabel(a);
            if (!d && !(d = document.getElementById(a))) {
                var d = document.createElement("div"),
                    e = this.getLabelContainer();
                d.id = a;
                d.className = "node";
                d.style.position = "absolute";
                b.onCreateLabel(d, c);
                e.appendChild(d);
                this.labels[c.id] = d
            }
            this.placeLabel(d, c, b)
        }
    });
    n.Label.SVG = new m({
        Implements: n.Label.DOM,
        plotLabel: function (a, c, b) {
            var a = c.id,
                d = this.getLabel(a);
            if (!d && !(d = document.getElementById(a))) {
                var d = document.createElementNS("http://www.w3.org/2000/svg", "svg:text"),
                    e = document.createElementNS("http://www.w3.org/2000/svg", "svg:tspan");
                d.appendChild(e);
                e = this.getLabelContainer();
                d.setAttribute("id", a);
                d.setAttribute("class", "node");
                e.appendChild(d);
                b.onCreateLabel(d, c);
                this.labels[c.id] = d
            }
            this.placeLabel(d, c, b)
        }
    });
    n.Geom = new m({
        initialize: function (a) {
            this.viz = a;
            this.config = a.config;
            this.node = a.config.Node;
            this.edge = a.config.Edge
        },
        translate: function (a, c) {
            c = h.splat(c);
            this.viz.graph.eachNode(function (b) {
                h.each(c, function (c) {
                    b.getPos(c).$add(a)
                })
            })
        },
        setRightLevelToShow: function (a, c, b) {
            var d = this.getRightLevelToShow(a, c),
                e = this.viz.labels,
                f = h.merge({
                    execShow: !0,
                    execHide: !0,
                    onHide: h.empty,
                    onShow: h.empty
                }, b || {});
            a.eachLevel(0, this.config.levelsToShow, function (b) {
                b._depth - a._depth > d ? (f.onHide(b), f.execHide && (b.drawn = !1, b.exist = !1, e.hideLabel(b, !1))) : (f.onShow(b), f.execShow && (b.exist = !0))
            });
            a.drawn = !0
        },
        getRightLevelToShow: function (a, c) {
            var b = this.config,
                d = b.levelsToShow;
            if (!b.constrained) return d;
            for (; !this.treeFitsInCanvas(a, c, d) && 1 < d;) d--;
            return d
        }
    });
    var H = {
        construct: function (a) {
            var c = "array" == h.type(a),
                b = new n(this.graphOptions, this.config.Node, this.config.Edge, this.config.Label);
            c ? function (a, b) {
                for (var c = function (c) {
                    for (var f = 0, g = b.length; f < g; f++) if (b[f].id == c) return b[f];
                    return a.addNode({
                        id: c,
                        name: c
                    })
                }, g = 0, j = b.length; g < j; g++) {
                    a.addNode(b[g]);
                    var i = b[g].adjacencies;
                    if (i) for (var k = 0, l = i.length; k < l; k++) {
                        var o = i[k],
                            r = {};
                        "string" != typeof i[k] && (r = h.merge(o.data, {}), o = o.nodeTo);
                        a.addAdjacence(b[g], c(o), r)
                    }
                }
            }(b, a) : function (a, b) {
                a.addNode(b);
                if (b.children) for (var c = 0, g = b.children; c < g.length; c++) a.addAdjacence(b, g[c]), arguments.callee(a, g[c])
            }(b, a);
            return b
        },
        loadJSON: function (a,
        c) {
            this.json = a;
            this.labels && this.labels.clearLabels && this.labels.clearLabels(!0);
            this.graph = this.construct(a);
            this.root = "array" != h.type(a) ? a.id : a[c ? c : 0].id
        },
        toJSON: function (a) {
            if ("tree" == (a || "tree")) var c = {}, c = function e(a) {
                var b = {};
                b.id = a.id;
                b.name = a.name;
                b.data = a.data;
                var c = [];
                a.eachSubnode(function (a) {
                    c.push(e(a))
                });
                b.children = c;
                return b
            }(this.graph.getNode(this.root));
            else {
                var c = [],
                    b = !! this.graph.getNode(this.root).visited;
                this.graph.eachNode(function (a) {
                    var f = {};
                    f.id = a.id;
                    f.name = a.name;
                    f.data = a.data;
                    var g = [];
                    a.eachAdjacency(function (a) {
                        var c = a.nodeTo;
                        if ( !! c.visited === b) {
                            var e = {};
                            e.nodeTo = c.id;
                            e.data = a.data;
                            g.push(e)
                        }
                    });
                    f.adjacencies = g;
                    c.push(f);
                    a.visited = !b
                })
            }
            return c
        }
    }, v = $jit.Layouts = {}, M = {
        label: null,
        compute: function (a, c, b) {
            this.initializeLabel(b);
            var d = this.label,
                e = d.style;
            a.eachNode(function (a) {
                var b = a.getData("autoWidth"),
                    c = a.getData("autoHeight");
                if (b || c) {
                    delete a.data.$width;
                    delete a.data.$height;
                    delete a.data.$dim;
                    var i = a.getData("width"),
                        k = a.getData("height");
                    e.width = b ? "auto" : i + "px";
                    e.height = c ? "auto" : k + "px";
                    d.innerHTML = a.name;
                    b = d.offsetWidth;
                    c = d.offsetHeight;
                    i = a.getData("type"); - 1 === h.indexOf(["circle", "square", "triangle", "star"], i) ? (a.setData("width", b), a.setData("height", c)) : (b = b > c ? b : c, a.setData("width", b), a.setData("height", b), a.setData("dim", b))
                }
            })
        },
        initializeLabel: function (a) {
            this.label || (this.label = document.createElement("div"), document.body.appendChild(this.label));
            this.setLabelStyles(a)
        },
        setLabelStyles: function () {
            h.extend(this.label.style, {
                visibility: "hidden",
                position: "absolute",
                width: "auto",
                height: "auto"
            });
            this.label.className = "jit-autoadjust-label"
        }
    };
    v.Tree = function () {
        function a(a, b, c, d, e) {
            var f = b.Node,
                g = b.multitree;
            if (f.overridable) {
                var h = -1,
                    j = -1;
                a.eachNode(function (a) {
                    if (a._depth == c && (!g || "$orn" in a.data && a.data.$orn == d)) {
                        var b = a.getData("width", e),
                            a = a.getData("height", e);
                        h = h < b ? b : h;
                        j = j < a ? a : j
                    }
                });
                return {
                    width: 0 > h ? f.width : h,
                    height: 0 > j ? f.height : j
                }
            }
            return f
        }
        function c(a, b, c, d) {
            a.getPos(b)["left" == d || "right" == d ? "y" : "x"] += c
        }
        function b(a, b) {
            var c = [];
            h.each(a,

            function (a) {
                a = l.call(a);
                a[0] += b;
                a[1] += b;
                c.push(a)
            });
            return c
        }
        function d(a, b) {
            if (0 == a.length) return b;
            if (0 == b.length) return a;
            var c = a.shift(),
                e = b.shift();
            return [[c[0], e[1]]].concat(d(a, b))
        }
        function e(a, b) {
            b = b || [];
            if (0 == a.length) return b;
            var c = a.pop();
            return e(a, d(c, b))
        }
        function f(a, b, c, d, e) {
            if (a.length <= e || b.length <= e) return 0;
            var g = a[e][1],
                h = b[e][0];
            return Math.max(f(a, b, c, d, ++e) + c, g - h + d)
        }
        function g(a, c, e) {
            function g(a, h, j) {
                if (h.length <= j) return [];
                var k = h[j],
                    i = f(a, k, c, e, 0);
                return [i].concat(g(d(a,
                b(k, i)), h, ++j))
            }
            return g([], a, 0)
        }
        function j(a, c, e) {
            function g(a, h, j) {
                if (h.length <= j) return [];
                var k = h[j],
                    i = -f(k, a, c, e, 0);
                return [i].concat(g(d(b(k, i), a), h, ++j))
            }
            a = l.call(a);
            return g([], a.reverse(), 0).reverse()
        }
        function i(a, b, c, d) {
            var e = g(a, b, c),
                a = j(a, b, c);
            "left" == d ? a = e : "right" == d && (e = a);
            d = 0;
            for (b = []; d < e.length; d++) b[d] = (e[d] + a[d]) / 2;
            return b
        }
        function k(d, f, g, h, j) {
            function k(f, r, m) {
                var n = f.getData(P, g),
                    A = [],
                    p = [],
                    J = !1,
                    N = (r || f.getData(v, g)) + h.levelDistance;
                f.eachSubnode(function (b) {
                    if (b.exist && (!l || "$orn" in b.data && b.data.$orn == j)) J || (J = a(d, h, b._depth, j, g)), b = k(b, J[v], m + N), A.push(b.tree), p.push(b.extent)
                });
                for (var r = i(p, x, w, y), B = 0, z = []; B < A.length; B++) c(A[B], g, r[B], j), z.push(b(p[B], r[B]));
                n = [
                    [-n / 2, n / 2]
                ].concat(e(z));
                f.getPos(g)[t] = 0;
                "top" == j || "left" == j ? f.getPos(g)[q] = m : f.getPos(g)[q] = -m;
                return {
                    tree: f,
                    extent: n
                }
            }
            var l = h.multitree,
                m = ["x", "y"],
                n = ["width", "height"],
                p = +("left" == j || "right" == j),
                t = m[p],
                q = m[1 - p],
                P = n[p],
                v = n[1 - p],
                w = h.siblingOffset,
                x = h.subtreeOffset,
                y = h.align;
            k(f, !1, 0)
        }
        var l = Array.prototype.slice;
        return new m({
            compute: function (a, b) {
                var c = a || "start",
                    d = this.graph.getNode(this.root);
                h.extend(d, {
                    drawn: !0,
                    exist: !0,
                    selected: !0
                });
                M.compute(this.graph, c, this.config);
                (b || !("_depth" in d)) && this.graph.computeLevels(this.root, 0, "ignore");
                this.computePositions(d, c)
            },
            computePositions: function (a, b) {
                var c = this.config,
                    d = c.multitree,
                    e = c.align,
                    f = "center" !== e && c.indent,
                    c = c.orientation,
                    g = this;
                h.each(d ? ["top", "right", "bottom", "left"] : [c], function (c) {
                    k(g.graph, a, b, g.config, c, b);
                    var h = ["x", "y"][+("left" == c || "right" == c)];
                    (function O(a) {
                        a.eachSubnode(function (g) {
                            if (g.exist && (!d || "$orn" in g.data && g.data.$orn == c)) g.getPos(b)[h] += a.getPos(b)[h], f && (g.getPos(b)[h] += "left" == e ? f : -f), O(g)
                        })
                    })(a)
                })
            }
        })
    }();
    $jit.ST = function () {
        function a(a) {
            a = a || this.clickedNode;
            if (!this.config.constrained) return [];
            var c = this.geom,
                f = this.canvas,
                g = a._depth,
                h = [];
            this.graph.eachNode(function (b) {
                b.exist && !b.selected && (b.isDescendantOf(a.id) ? b._depth <= g && h.push(b) : h.push(b))
            });
            c = c.getRightLevelToShow(a, f);
            a.eachLevel(c, c, function (a) {
                a.exist && !a.selected && h.push(a)
            });
            for (c = 0; c < b.length; c++) f = this.graph.getNode(b[c]), f.isDescendantOf(a.id) || h.push(f);
            return h
        }
        function c() {
            var a = [],
                b = this.config;
            this.clickedNode.eachLevel(0, b.levelsToShow, function (c) {
                b.multitree && !("$orn" in c.data) && c.anySubnode(function (a) {
                    return a.exist && !a.drawn
                }) ? a.push(c) : c.drawn && !c.anySubnode("drawn") && a.push(c)
            });
            return a
        }
        var b = [];
        return new m({
            Implements: [H, y, v.Tree],
            initialize: function (a) {
                var b = $jit.ST,
                    a = this.controller = this.config = h.merge(q("Canvas", "Fx", "Tree", "Node", "Edge", "Controller", "Tips", "NodeStyles", "Events", "Navigation", "Label"), {
                        levelsToShow: 2,
                        levelDistance: 30,
                        constrained: !0,
                        Node: {
                            type: "rectangle"
                        },
                        duration: 700,
                        offsetX: 0,
                        offsetY: 0
                    }, a);
                a.useCanvas ? (this.canvas = a.useCanvas, this.config.labelContainer = this.canvas.id + "-label") : (a.background && (a.background = h.merge({
                    type: "Circles"
                }, a.background)), this.canvas = new w(this, a), this.config.labelContainer = ("string" == typeof a.injectInto ? a.injectInto : a.injectInto.id) + "-label");
                this.graphOptions = {
                    klass: t
                };
                this.graph = new n(this.graphOptions,
                this.config.Node, this.config.Edge);
                this.labels = new b.Label[a.Label.type](this);
                this.fx = new b.Plot(this, b);
                this.op = new b.Op(this);
                this.group = new b.Group(this);
                this.geom = new b.Geom(this);
                this.clickedNode = null;
                this.initializeExtras()
            },
            plot: function () {
                this.fx.plot(this.controller)
            },
            switchPosition: function (a, b, c) {
                var g = this.geom,
                    h = this.fx,
                    i = this;
                h.busy || (h.busy = !0, this.contract({
                    onComplete: function () {
                        g.switchOrientation(a);
                        i.compute("end", !1);
                        h.busy = !1;
                        if ("animate" == b) i.onClick(i.clickedNode.id, c);
                        else "replot" == b && i.select(i.clickedNode.id, c)
                    }
                }, a))
            },
            switchAlignment: function (a, b, c) {
                this.config.align = a;
                if ("animate" == b) this.select(this.clickedNode.id, c);
                else if ("replot" == b) this.onClick(this.clickedNode.id, c)
            },
            addNodeInPath: function (a) {
                b.push(a);
                this.select(this.clickedNode && this.clickedNode.id || this.root)
            },
            clearNodesInPath: function () {
                b.length = 0;
                this.select(this.clickedNode && this.clickedNode.id || this.root)
            },
            refresh: function () {
                this.reposition();
                this.select(this.clickedNode && this.clickedNode.id || this.root)
            },
            reposition: function () {
                this.graph.computeLevels(this.root,
                0, "ignore");
                this.geom.setRightLevelToShow(this.clickedNode, this.canvas);
                this.graph.eachNode(function (a) {
                    a.exist && (a.drawn = !0)
                });
                this.compute("end")
            },
            requestNodes: function (a, b) {
                var c = h.merge(this.controller, b),
                    g = this.config.levelsToShow;
                if (c.request) {
                    var j = [],
                        i = a._depth;
                    a.eachLevel(0, g, function (a) {
                        a.drawn && !a.anySubnode() && (j.push(a), a._level = g - (a._depth - i))
                    });
                    this.group.requestNodes(j, c)
                }
                else c.onComplete()
            },
            contract: function (b, c) {
                var f = this.config.orientation,
                    g = this.geom,
                    j = this.group;
                c && g.switchOrientation(c);
                var i = a.call(this);
                c && g.switchOrientation(f);
                j.contract(i, h.merge(this.controller, b))
            },
            move: function (a, b) {
                this.compute("end", !1);
                var c = b.Move,
                    g = {
                        x: c.offsetX,
                        y: c.offsetY
                    };
                c.enable && this.geom.translate(a.endPos.add(g).$scale(-1), "end");
                this.fx.animate(h.merge(this.controller, {
                    modes: ["linear"]
                }, b))
            },
            expand: function (a, b) {
                this.group.expand(c.call(this, a), h.merge(this.controller, b))
            },
            selectPath: function (a) {
                function c(a) {
                    null == a || a.selected || (a.selected = !0, h.each(f.group.getSiblings([a])[a.id], function (a) {
                        a.exist = !0;
                        a.drawn = !0
                    }), a = a.getParents(), a = 0 < a.length ? a[0] : null, c(a))
                }
                var f = this;
                this.graph.eachNode(function (a) {
                    a.selected = !1
                });
                for (var g = 0, a = [a.id].concat(b); g < a.length; g++) c(this.graph.getNode(a[g]))
            },
            setRoot: function (a, b, c) {
                function g() {
                    if (this.config.multitree && l.data.$orn) {
                        var b = {
                            left: "right",
                            right: "left",
                            top: "bottom",
                            bottom: "top"
                        }[l.data.$orn];
                        k.data.$orn = b;
                        (function u(c) {
                            c.eachSubnode(function (c) {
                                c.id != a && (c.data.$orn = b, u(c))
                            })
                        })(k);
                        delete l.data.$orn
                    }
                    this.root = a;
                    this.clickedNode = l;
                    this.graph.computeLevels(this.root,
                    0, "ignore");
                    this.geom.setRightLevelToShow(l, i, {
                        execHide: !1,
                        onShow: function (a) {
                            a.drawn || (a.drawn = !0, a.setData("alpha", 1, "end"), a.setData("alpha", 0), a.pos.setc(l.pos.x, l.pos.y))
                        }
                    });
                    this.compute("end");
                    this.busy = !0;
                    this.fx.animate({
                        modes: ["linear", "node-property:alpha"],
                        onComplete: function () {
                            h.busy = !1;
                            h.onClick(a, {
                                onComplete: function () {
                                    c && c.onComplete()
                                }
                            })
                        }
                    })
                }
                if (!this.busy) {
                    this.busy = !0;
                    var h = this,
                        i = this.canvas,
                        k = this.graph.getNode(this.root),
                        l = this.graph.getNode(a);
                    delete k.data.$orns;
                    "animate" == b ? (g.call(this), h.selectPath(l)) : "replot" == b && (g.call(this), this.select(this.root))
                }
            },
            addSubtree: function (a, b, c) {
                "replot" == b ? this.op.sum(a, h.extend({
                    type: "replot"
                }, c || {})) : "animate" == b && this.op.sum(a, h.extend({
                    type: "fade:seq"
                }, c || {}))
            },
            removeSubtree: function (a, b, c, g) {
                var j = [];
                this.graph.getNode(a).eachLevel(+!b, !1, function (a) {
                    j.push(a.id)
                });
                "replot" == c ? this.op.removeNode(j, h.extend({
                    type: "replot"
                }, g || {})) : "animate" == c && this.op.removeNode(j, h.extend({
                    type: "fade:seq"
                }, g || {}))
            },
            select: function (b, e) {
                var f = this.group,
                    g = this.geom,
                    j = this.graph.getNode(b),
                    i = this.canvas;
                this.graph.getNode(this.root);
                var k = h.merge(this.controller, e),
                    l = this;
                k.onBeforeCompute(j);
                this.selectPath(j);
                this.clickedNode = j;
                this.requestNodes(j, {
                    onComplete: function () {
                        f.hide(f.prepare(a.call(l)), k);
                        g.setRightLevelToShow(j, i);
                        l.compute("current");
                        l.graph.eachNode(function (a) {
                            var b = a.pos.getc(!0);
                            a.startPos.setc(b.x, b.y);
                            a.endPos.setc(b.x, b.y);
                            a.visited = !1
                        });
                        l.geom.translate(j.endPos.add({
                            x: k.offsetX,
                            y: k.offsetY
                        }).$scale(-1), ["start", "current", "end"]);
                        f.show(c.call(l));
                        l.plot();
                        k.onAfterCompute(l.clickedNode);
                        k.onComplete()
                    }
                })
            },
            onClick: function (a, b) {
                var c = this.canvas,
                    g = this,
                    j = this.geom,
                    i = this.config,
                    k = h.merge(this.controller, {
                        Move: {
                            enable: !0,
                            offsetX: i.offsetX || 0,
                            offsetY: i.offsetY || 0
                        },
                        setRightLevelToShowConfig: !1,
                        onBeforeRequest: h.empty,
                        onBeforeContract: h.empty,
                        onBeforeMove: h.empty,
                        onBeforeExpand: h.empty
                    }, b);
                if (!this.busy) {
                    this.busy = !0;
                    var l = this.graph.getNode(a);
                    this.selectPath(l, this.clickedNode);
                    this.clickedNode = l;
                    k.onBeforeCompute(l);
                    k.onBeforeRequest(l);
                    this.requestNodes(l, {
                        onComplete: function () {
                            k.onBeforeContract(l);
                            g.contract({
                                onComplete: function () {
                                    j.setRightLevelToShow(l, c, k.setRightLevelToShowConfig);
                                    k.onBeforeMove(l);
                                    g.move(l, {
                                        Move: k.Move,
                                        onComplete: function () {
                                            k.onBeforeExpand(l);
                                            g.expand(l, {
                                                onComplete: function () {
                                                    g.busy = !1;
                                                    k.onAfterCompute(a);
                                                    k.onComplete()
                                                }
                                            })
                                        }
                                    })
                                }
                            })
                        }
                    })
                }
            }
        })
    }();
    $jit.ST.$extend = !0;
    $jit.ST.Op = new m({
        Implements: n.Op
    });
    $jit.ST.Group = new m({
        initialize: function (a) {
            this.viz = a;
            this.canvas = a.canvas;
            this.config = a.config;
            this.animation = new D;
            this.nodes = null
        },
        requestNodes: function (a, c) {
            var b = 0,
                d = a.length,
                e = this.viz;
            if (0 == d) c.onComplete();
            for (var f = 0; f < d; f++) c.request(a[f].id, a[f]._level, {
                onComplete: function (a, f) {
                    f && f.children && (f.id = a, e.op.sum(f, {
                        type: "nothing"
                    }));
                    ++b == d && (e.graph.computeLevels(e.root, 0), c.onComplete())
                }
            })
        },
        contract: function (a, c) {
            var b = this,
                a = this.prepare(a);
            this.animation.setOptions(h.merge(c, {
                $animating: !1,
                compute: function (a) {
                    1 == a && (a = 0.99);
                    b.plotStep(1 - a, c, this.$animating);
                    this.$animating = "contract"
                },
                complete: function () {
                    b.hide(a, c)
                }
            })).start()
        },
        hide: function (a, c) {
            for (var b = 0; b < a.length; b++) a[b].eachLevel(1, !1, function (a) {
                a.exist && h.extend(a, {
                    drawn: false,
                    exist: false
                })
            });
            c.onComplete()
        },
        expand: function (a, c) {
            var b = this;
            this.show(a);
            this.animation.setOptions(h.merge(c, {
                $animating: !1,
                compute: function (a) {
                    b.plotStep(a, c, this.$animating);
                    this.$animating = "expand"
                },
                complete: function () {
                    b.plotStep(void 0, c, !1);
                    c.onComplete()
                }
            })).start()
        },
        show: function (a) {
            var c = this.config;
            this.prepare(a);
            h.each(a, function (a) {
                if (c.multitree && !("$orn" in a.data)) {
                    delete a.data.$orns;
                    var d = " ";
                    a.eachSubnode(function (a) {
                        "$orn" in a.data && (0 > d.indexOf(a.data.$orn) && a.exist && !a.drawn) && (d += a.data.$orn + " ")
                    });
                    a.data.$orns = d
                }
                a.eachLevel(0, c.levelsToShow, function (a) {
                    a.exist && (a.drawn = !0)
                })
            })
        },
        prepare: function (a) {
            return this.nodes = this.getNodesWithChildren(a)
        },
        getNodesWithChildren: function (a) {
            var c = [],
                b = this.config;
            a.sort(function (a, b) {
                return (a._depth <= b._depth) - (a._depth >= b._depth)
            });
            for (var d = 0; d < a.length; d++) if (a[d].anySubnode("exist")) {
                for (var e = d + 1, f = !1; !f && e < a.length; e++) if (!b.multitree || "$orn" in a[e].data) f = f || a[d].isDescendantOf(a[e].id);
                f || c.push(a[d])
            }
            return c
        },
        plotStep: function (a, c, b) {
            var d = this.viz,
                e = this.config,
                f = d.canvas.getCtx(),
                g = this.nodes,
                j, i, k = {};
            for (j = 0; j < g.length; j++) {
                i = g[j];
                k[i.id] = [];
                var l = e.multitree && !("$orn" in i.data),
                    o = l && i.data.$orns;
                i.eachSubgraph(function (a) {
                    if (l && o && 0 < o.indexOf(a.data.$orn) && a.drawn) a.drawn = !1, k[i.id].push(a);
                    else if ((!l || !o) && a.drawn) a.drawn = !1, k[i.id].push(a)
                });
                i.drawn = !0
            }
            0 < g.length && d.fx.plot();
            for (j in k) h.each(k[j], function (a) {
                a.drawn = !0
            });
            for (j = 0; j < g.length; j++) i = g[j], f.save(), d.fx.plotSubtree(i, c, a, b), f.restore()
        },
        getSiblings: function (a) {
            var c = {};
            h.each(a, function (a) {
                var d = a.getParents();
                if (0 == d.length) c[a.id] = [a];
                else {
                    var e = [];
                    d[0].eachSubnode(function (a) {
                        e.push(a)
                    });
                    c[a.id] = e
                }
            });
            return c
        }
    });
    $jit.ST.Geom = new m({
        Implements: n.Geom,
        switchOrientation: function (a) {
            this.config.orientation = a
        },
        dispatch: function () {
            var a = Array.prototype.slice.call(arguments),
                c = a.shift(),
                b = a.length,
                d = function (a) {
                    return "function" == typeof a ? a() : a
                };
            if (2 == b) return "top" == c || "bottom" == c ? d(a[0]) : d(a[1]);
            if (4 == b) switch (c) {
            case "top":
                return d(a[0]);
            case "right":
                return d(a[1]);
            case "bottom":
                return d(a[2]);
            case "left":
                return d(a[3])
            }
        },
        getSize: function (a, c) {
            var b = a.data,
                d = this.config,
                e = d.siblingOffset,
                b = d.multitree && "$orn" in b && b.$orn || d.orientation,
                d = a.getData("width") + e,
                e = a.getData("height") + e;
            return c ? this.dispatch(b, d, e) : this.dispatch(b, e, d)
        },
        getTreeBaseSize: function (a, c, b) {
            var d = this.getSize(a, !0),
                e = 0,
                f = this;
            if (b(c, a)) return d;
            if (0 === c) return 0;
            a.eachSubnode(function (a) {
                e += f.getTreeBaseSize(a, c - 1, b)
            });
            return (d > e ? d : e) + this.config.subtreeOffset
        },
        getEdge: function (a, c, b) {
            var d = function (b, c) {
                return function () {
                    return a.pos.add(new t(b, c))
                }
            }, e = this.node,
                f = a.getData("width"),
                g = a.getData("height");
            if ("begin" == c) {
                if ("center" == e.align) return this.dispatch(b, d(0, g / 2), d(-f / 2, 0), d(0, - g / 2), d(f / 2, 0));
                if ("left" == e.align) return this.dispatch(b, d(0, g), d(0, 0), d(0, 0), d(f, 0));
                if ("right" == e.align) return this.dispatch(b, d(0, 0), d(-f, 0), d(0, - g), d(0, 0));
                throw "align: not implemented";
            }
            if ("end" == c) {
                if ("center" == e.align) return this.dispatch(b, d(0, - g / 2), d(f / 2, 0), d(0, g / 2), d(-f / 2, 0));
                if ("left" == e.align) return this.dispatch(b, d(0, 0), d(f, 0), d(0, g), d(0, 0));
                if ("right" == e.align) return this.dispatch(b, d(0, - g), d(0, 0), d(0, 0), d(-f, 0));
                throw "align: not implemented";
            }
        },
        getScaledTreePosition: function (a, c) {
            var b = this.node,
                d = a.getData("width"),
                e = a.getData("height"),
                f = this.config.multitree && "$orn" in a.data && a.data.$orn || this.config.orientation,
                g = function (b, d) {
                    return function () {
                        return a.pos.add(new t(b,
                        d)).$scale(1 - c)
                    }
                };
            if ("left" == b.align) return this.dispatch(f, g(0, e), g(0, 0), g(0, 0), g(d, 0));
            if ("center" == b.align) return this.dispatch(f, g(0, e / 2), g(-d / 2, 0), g(0, - e / 2), g(d / 2, 0));
            if ("right" == b.align) return this.dispatch(f, g(0, 0), g(-d, 0), g(0, - e), g(0, 0));
            throw "align: not implemented";
        },
        treeFitsInCanvas: function (a, c, b) {
            c = c.getSize();
            c = this.dispatch(this.config.multitree && "$orn" in a.data && a.data.$orn || this.config.orientation, c.width, c.height);
            return this.getTreeBaseSize(a, b, function (a, b) {
                return 0 === a || !b.anySubnode()
            }) < c
        }
    });
    $jit.ST.Plot = new m({
        Implements: n.Plot,
        plotSubtree: function (a, c, b, d) {
            var e = this.viz,
                f = e.canvas,
                g = e.config,
                b = Math.min(Math.max(0.0010, b), 1);
            0 <= b && (a.drawn = !1, f = f.getCtx(), e = e.geom.getScaledTreePosition(a, b), f.translate(e.x, e.y), f.scale(b, b));
            this.plotTree(a, h.merge(c, {
                withLabels: !0,
                hideLabels: !! b,
                plotSubtree: function () {
                    var b = g.multitree && !("$orn" in a.data),
                        c = b && a.getData("orns");
                    return !b || c.indexOf(a.getData("orn")) > -1
                }
            }), d);
            0 <= b && (a.drawn = !0)
        },
        getAlignedPos: function (a, c, b) {
            var d = this.node;
            if ("center" == d.align) a = {
                x: a.x - c / 2,
                y: a.y - b / 2
            };
            else if ("left" == d.align) d = this.config.orientation, a = "bottom" == d || "top" == d ? {
                x: a.x - c / 2,
                y: a.y
            } : {
                x: a.x,
                y: a.y - b / 2
            };
            else if ("right" == d.align) d = this.config.orientation, a = "bottom" == d || "top" == d ? {
                x: a.x - c / 2,
                y: a.y - b
            } : {
                x: a.x - c,
                y: a.y - b / 2
            };
            else throw "align: not implemented";
            return a
        },
        getOrientation: function (a) {
            var c = this.config,
                b = c.orientation;
            c.multitree && (c = a.nodeFrom, a = a.nodeTo, b = "$orn" in c.data && c.data.$orn || "$orn" in a.data && a.data.$orn);
            return b
        }
    });
    $jit.ST.Label = {};
    $jit.ST.Label.Native = new m({
        Implements: n.Label.Native,
        renderLabel: function (a, c) {
            var b = a.getCtx(),
                d = c.pos.getc(!0),
                e = c.getData("width"),
                f = c.getData("height"),
                d = this.viz.fx.getAlignedPos(d, e, f);
            b.fillText(c.name, d.x + e / 2, d.y + f / 2)
        }
    });
    $jit.ST.Label.DOM = new m({
        Implements: n.Label.DOM,
        placeLabel: function (a, c, b) {
            var d = c.pos.getc(!0),
                e = this.viz.config,
                f = e.Node,
                g = this.viz.canvas,
                h = c.getData("width"),
                i = c.getData("height"),
                k = g.getSize(),
                l = d.x * g.scaleOffsetX + g.translateOffsetX,
                d = d.y * g.scaleOffsetY + g.translateOffsetY;
            if ("center" == f.align) h = {
                x: Math.round(l - h / 2 + k.width / 2),
                y: Math.round(d - i / 2 + k.height / 2)
            };
            else if ("left" == f.align) e = e.orientation, h = "bottom" == e || "top" == e ? {
                x: Math.round(l - h / 2 + k.width / 2),
                y: Math.round(d + k.height / 2)
            } : {
                x: Math.round(l + k.width / 2),
                y: Math.round(d - i / 2 + k.height / 2)
            };
            else if ("right" == f.align) e = e.orientation, h = "bottom" == e || "top" == e ? {
                x: Math.round(l - h / 2 + k.width / 2),
                y: Math.round(d - i + k.height / 2)
            } : {
                x: Math.round(l - h + k.width / 2),
                y: Math.round(d - i / 2 + k.height / 2)
            };
            else throw "align: not implemented";
            i = a.style;
            i.left = h.x + "px";
            i.top = h.y + "px";
            i.display = this.fitsInCanvas(h, g) ? "" : "none";
            b.onPlaceLabel(a, c)
        }
    });
    $jit.ST.Label.SVG = new m({
        Implements: [$jit.ST.Label.DOM, n.Label.SVG],
        initialize: function (a) {
            this.viz = a
        }
    });
    $jit.ST.Label.HTML = new m({
        Implements: [$jit.ST.Label.DOM, n.Label.HTML],
        initialize: function (a) {
            this.viz = a
        }
    });
    $jit.ST.Plot.NodeTypes = new m({
        none: {
            render: h.empty,
            contains: h.lambda(!1)
        },
        circle: {
            render: function (a, c) {
                var b = a.getData("dim"),
                    d = this.getAlignedPos(a.pos.getc(!0), b, b),
                    b = b / 2;
                this.nodeHelper.circle.render("fill", {
                    x: d.x + b,
                    y: d.y + b
                }, b, c)
            },
            contains: function (a, c) {
                var b = a.getData("dim"),
                    d = this.getAlignedPos(a.pos.getc(!0), b, b),
                    b = b / 2;
                this.nodeHelper.circle.contains({
                    x: d.x + b,
                    y: d.y + b
                }, c, b)
            }
        },
        square: {
            render: function (a, c) {
                var b = a.getData("dim"),
                    d = b / 2,
                    b = this.getAlignedPos(a.pos.getc(!0), b, b);
                this.nodeHelper.square.render("fill", {
                    x: b.x + d,
                    y: b.y + d
                }, d, c)
            },
            contains: function (a, c) {
                var b = a.getData("dim"),
                    d = this.getAlignedPos(a.pos.getc(!0), b, b),
                    b = b / 2;
                this.nodeHelper.square.contains({
                    x: d.x + b,
                    y: d.y + b
                }, c, b)
            }
        },
        ellipse: {
            render: function (a,
            c) {
                var b = a.getData("width"),
                    d = a.getData("height"),
                    e = this.getAlignedPos(a.pos.getc(!0), b, d);
                this.nodeHelper.ellipse.render("fill", {
                    x: e.x + b / 2,
                    y: e.y + d / 2
                }, b, d, c)
            },
            contains: function (a, c) {
                var b = a.getData("width"),
                    d = a.getData("height"),
                    e = this.getAlignedPos(a.pos.getc(!0), b, d);
                this.nodeHelper.ellipse.contains({
                    x: e.x + b / 2,
                    y: e.y + d / 2
                }, c, b, d)
            }
        },
        rectangle: {
            render: function (a, c) {
                var b = a.getData("width"),
                    d = a.getData("height"),
                    e = this.getAlignedPos(a.pos.getc(!0), b, d);
                this.nodeHelper.rectangle.render("fill", {
                    x: e.x + b / 2,
                    y: e.y + d / 2
                }, b, d, c)
            },
            contains: function (a, c) {
                var b = a.getData("width"),
                    d = a.getData("height"),
                    e = this.getAlignedPos(a.pos.getc(!0), b, d);
                this.nodeHelper.rectangle.contains({
                    x: e.x + b / 2,
                    y: e.y + d / 2
                }, c, b, d)
            }
        }
    });
    $jit.ST.Plot.EdgeTypes = new m({
        none: h.empty,
        line: {
            render: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    b = this.viz.geom.getEdge(f ? e : d, "end", b);
                this.edgeHelper.line.render(g, b, c)
            },
            contains: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    b = this.viz.geom.getEdge(f ? e : d, "end", b);
                return this.edgeHelper.line.contains(g, b, c, this.edge.epsilon)
            }
        },
        arrow: {
            render: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = a.getData("dim"),
                    g = this.viz.geom.getEdge(d, "begin", b),
                    b = this.viz.geom.getEdge(e, "end", b),
                    e = a.data.$direction;
                this.edgeHelper.arrow.render(g, b, f, e && 1 < e.length && e[0] != d.id, c)
            },
            contains: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    b = this.viz.geom.getEdge(f ? e : d, "end", b);
                return this.edgeHelper.arrow.contains(g, b, c, this.edge.epsilon)
            }
        },
        "quadratic:begin": {
            render: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    d = this.viz.geom.getEdge(f ? e : d, "end", b),
                    e = a.getData("dim"),
                    f = c.getCtx();
                f.beginPath();
                f.moveTo(g.x, g.y);
                switch (b) {
                case "left":
                    f.quadraticCurveTo(g.x + e, g.y, d.x, d.y);
                    break;
                case "right":
                    f.quadraticCurveTo(g.x - e, g.y, d.x, d.y);
                    break;
                case "top":
                    f.quadraticCurveTo(g.x, g.y + e, d.x, d.y);
                    break;
                case "bottom":
                    f.quadraticCurveTo(g.x, g.y - e, d.x, d.y)
                }
                f.stroke()
            }
        },
        "quadratic:end": {
            render: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    d = this.viz.geom.getEdge(f ? e : d, "end", b),
                    e = a.getData("dim"),
                    f = c.getCtx();
                f.beginPath();
                f.moveTo(g.x, g.y);
                switch (b) {
                case "left":
                    f.quadraticCurveTo(d.x - e, d.y, d.x, d.y);
                    break;
                case "right":
                    f.quadraticCurveTo(d.x + e, d.y,
                    d.x, d.y);
                    break;
                case "top":
                    f.quadraticCurveTo(d.x, d.y - e, d.x, d.y);
                    break;
                case "bottom":
                    f.quadraticCurveTo(d.x, d.y + e, d.x, d.y)
                }
                f.stroke()
            }
        },
        bezier: {
            render: function (a, c) {
                var b = this.getOrientation(a),
                    d = a.nodeFrom,
                    e = a.nodeTo,
                    f = d._depth < e._depth,
                    g = this.viz.geom.getEdge(f ? d : e, "begin", b),
                    d = this.viz.geom.getEdge(f ? e : d, "end", b),
                    e = a.getData("dim"),
                    f = c.getCtx();
                f.beginPath();
                f.moveTo(g.x, g.y);
                switch (b) {
                case "left":
                    f.bezierCurveTo(g.x + e, g.y, d.x - e, d.y, d.x, d.y);
                    break;
                case "right":
                    f.bezierCurveTo(g.x - e, g.y, d.x + e, d.y,
                    d.x, d.y);
                    break;
                case "top":
                    f.bezierCurveTo(g.x, g.y + e, d.x, d.y - e, d.x, d.y);
                    break;
                case "bottom":
                    f.bezierCurveTo(g.x, g.y - e, d.x, d.y + e, d.x, d.y)
                }
                f.stroke()
            }
        }
    });
    $jit.ST.Plot.NodeTypes.implement({
        "areachart-stacked": {
            render: function (a, c) {
                var b = a.pos.getc(!0),
                    d = a.getData("width"),
                    e = a.getData("height"),
                    e = this.getAlignedPos(b, d, e),
                    b = e.x,
                    e = e.y,
                    f = a.getData("stringArray"),
                    g = a.getData("dimArray");
                a.getData("valueArray");
                var j = a.getData("colorArray"),
                    i = j.length;
                a.getData("prev");
                var k = c.getCtx(),
                    l = a.getData("border");
                if (j && g && f) for (var o = 0, r = g.length, u = 0, s = 0; o < r; o++) {
                    k.fillStyle = k.strokeStyle = j[o % i];
                    k.beginPath();
                    k.moveTo(b - 1, e - u);
                    k.lineTo(b + d + 1, e - s);
                    k.lineTo(b + d + 1, e - s - g[o][1]);
                    k.lineTo(b - 1, e - u - g[o][0]);
                    k.lineTo(b - 1, e - u);
                    k.closePath();
                    k.fill();
                    if (l) {
                        var m = l.name == f[o],
                            n = m ? 0.7 : 0.8,
                            C = h.rgbToHex(h.map(h.hexToRgb(j[o % i].slice(1)), function (a) {
                                return a * n >> 0
                            }));
                        k.strokeStyle = C;
                        k.lineWidth = m ? 4 : 1;
                        k.save();
                        k.beginPath();
                        0 === l.index ? (k.moveTo(b, e - u), k.lineTo(b, e - u - g[o][0])) : (k.moveTo(b + d, e - s), k.lineTo(b + d, e - s - g[o][1]));
                        k.stroke();
                        k.restore()
                    }
                    u += g[o][0] || 0;
                    s += g[o][1] || 0
                }
            },
            contains: function (a, c) {
                var b = a.pos.getc(!0),
                    d = a.getData("width"),
                    e = a.getData("height"),
                    b = this.getAlignedPos(b, d, e),
                    f = b.x,
                    g = b.y,
                    b = a.getData("dimArray"),
                    h = c.x - f;
                if (c.x < f || c.x > f + d || c.y > g || c.y < g - e) return !1;
                for (var e = 0, f = b.length, i = g; e < f; e++) {
                    var k = b[e],
                        i = i - k[0],
                        g = g - k[1];
                    if (c.y >= i + (g - i) * h / d) return d = +(h > d / 2), {
                        name: a.getData("stringArray")[e],
                        color: a.getData("colorArray")[e],
                        value: a.getData("valueArray")[e][d],
                        index: d
                    }
                }
                return !1
            }
        },
        "areachart-opposite": {
            render: function (a,
            c) {
                var b = a.pos.getc(!0),
                    d = a.getData("width"),
                    e = a.getData("height"),
                    b = this.getAlignedPos(b, d, e).x,
                    e = a.getData("stringArray"),
                    f = a.getData("dimArray");
                a.getData("valueArray");
                var g = a.getData("colorArray"),
                    j = g.length;
                a.getData("prev");
                var i = c.getCtx(),
                    k = a.getData("border");
                if (g && f && e) for (var l = 0; 2 > l; ++l) if (i.fillStyle = i.strokeStyle = g[l], i.beginPath(), i.moveTo(b - 1, 0), i.lineTo(b + d, 0), i.lineTo(b + d, (2 * l - 1) * f[l][1]), i.lineTo(b - 1, (2 * l - 1) * f[l][0]), i.lineTo(b - 1, 0), i.closePath(), i.fill(), k) {
                    var o = k.name == e[l],
                        r = o ? 0.7 : 0.8,
                        u = h.rgbToHex(h.map(h.hexToRgb(g[l % j].slice(1)), function (a) {
                            return a * r >> 0
                        }));
                    i.strokeStyle = u;
                    i.lineWidth = o ? 4 : 1;
                    i.save();
                    i.beginPath();
                    0 === k.index ? (i.moveTo(b, 0), i.lineTo(b, (2 * l - 1) * f[l][0])) : (i.moveTo(b + d, 0), i.lineTo(b + d, (2 * l - 1) * f[l][1]));
                    i.stroke();
                    i.restore()
                }
            },
            contains: function (a, c) {
                var b = a.pos.getc(!0),
                    d = a.getData("width"),
                    e = a.getData("height"),
                    e = this.getAlignedPos(b, d, e).x;
                a.getData("dimArray");
                if (c.x < e || c.x > e + d) return !1;
                b = 0 > c.y ? 0 : 1;
                d = c.x - e > d / 2 ? 1 : 0;
                return {
                    name: a.getData("stringArray")[b],
                    color: a.getData("colorArray")[b],
                    value: a.getData("valueArray")[b][d],
                    index: d
                }
            }
        }
    });
    $jit.AreaChart = new m({
        st: null,
        colors: "#416D9C #70A35E #EBB056 #C74243 #83548B #909291 #557EAA".split(" "),
        selected: {},
        busy: !1,
        initialize: function (a) {
            this.controller = this.config = h.merge(q("Canvas", "Margin", "Label", "AreaChart"), {
                Label: {
                    type: "Native"
                }
            }, a);
            var a = this.config.showLabels,
                c = h.type(a),
                b = this.config.showAggregates,
                d = h.type(b);
            this.config.showLabels = "function" == c ? a : h.lambda(a);
            this.config.showAggregates = "function" == d ? b : h.lambda(b);
            this.initializeViz()
        },
        initializeViz: function () {
            var a = this.config,
                c = this;
            a.type.split(":");
            var b = {}, d = new $jit.ST({
                injectInto: a.injectInto,
                width: a.width,
                height: a.height,
                orientation: "bottom",
                levelDistance: 0,
                siblingOffset: 0,
                subtreeOffset: 0,
                withLabels: "Native" != a.Label.type,
                useCanvas: a.useCanvas,
                Label: {
                    type: a.Label.type
                },
                Node: {
                    overridable: !0,
                    type: "areachart-opposite",
                    align: "left",
                    width: 1,
                    height: 1
                },
                Edge: {
                    type: "none"
                },
                Tips: {
                    enable: a.Tips.enable,
                    type: "Native",
                    force: !0,
                    onShow: function (b, c, d) {
                        a.Tips.onShow(b, d, c);
                    }
                },
                Events: {
                    enable: !0,
                    type: "Native",
                    onClick: function (b, d, e) {
                        if (a.filterOnClick || a.Events.enable)(b = d.getContains()) && a.filterOnClick && c.filter(b.name), a.Events.enable && a.Events.onClick(b, d, e)
                    },
                    onRightClick: function () {
                        a.restoreOnRightClick && c.restore()
                    },
                    onMouseMove: function (b, d) {
                        if (a.selectOnHover) if (b) {
                            var e = d.getContains();
                            c.select(b.id, e.name, e.index)
                        }
                        else c.select(!1, !1, !1)
                    }
                },
                onCreateLabel: function (c, d) {
                    var e = a.Label,
                        f = d.getData("valueArray"),
                        l = h.reduce(f, function (a, b) {
                            return a + b[0]
                        }, 0),
                        f = h.reduce(f, function (a, b) {
                            return a + b[1]
                        }, 0);
                    if (d.getData("prev")) {
                        var o = {
                            wrapper: document.createElement("div"),
                            aggregate: document.createElement("div"),
                            label: document.createElement("div")
                        }, r = o.wrapper,
                            u = o.label,
                            s = o.aggregate,
                            m = r.style,
                            n = u.style,
                            C = s.style;
                        b[d.id] = o;
                        r.appendChild(u);
                        r.appendChild(s);
                        a.showLabels(d.name, l, f, d) || (u.style.display = "none");
                        a.showAggregates(d.name, l, f, d) || (s.style.display = "none");
                        m.position = "relative";
                        m.overflow = "visible";
                        m.fontSize = e.size + "px";
                        m.fontFamily = e.family;
                        m.color = e.color;
                        m.textAlign = "center";
                        C.position = n.position = "absolute";
                        c.style.width = d.getData("width") + "px";
                        c.style.height = d.getData("height") + "px";
                        u.innerHTML = d.name;
                        c.appendChild(r)
                    }
                },
                onPlaceLabel: function (c, d) {
                    if (d.getData("prev")) {
                        var e = b[d.id],
                            f = e.wrapper.style,
                            l = e.label.style,
                            o = e.aggregate.style,
                            r = d.getData("width");
                        d.getData("height");
                        var m = d.getData("dimArray"),
                            s = d.getData("valueArray"),
                            n = h.reduce(s, function (a, b) {
                                return a + b[0]
                            }, 0),
                            p = h.reduce(s, function (a, b) {
                                return a + b[1]
                            }, 0),
                            C = parseInt(f.fontSize,
                            10);
                        if (m && s) {
                            l.display = a.showLabels(d.name, n, p, d) ? "" : "none";
                            n = a.showAggregates(d.name, n, p, d);
                            o.display = !1 !== n ? "" : "none";
                            f.width = o.width = l.width = c.style.width = r + "px";
                            o.left = l.left = -r / 2 + "px";
                            for (var r = 0, p = s.length, A = 0, q = 0; r < p; r++) 0 < m[r][0] && (A += s[r][0], q += m[r][0]);
                            o.top = -C - a.labelOffset + "px";
                            l.top = a.labelOffset + q + "px";
                            c.style.top = parseInt(c.style.top, 10) - q + "px";
                            c.style.height = f.height = q + "px";
                            e.aggregate.innerHTML = !0 !== n ? n : A
                        }
                    }
                }
            }),
                e = d.canvas.getSize(),
                f = a.Margin;
            d.config.offsetY = -e.height / 2 + f.bottom + (a.showLabels && a.labelOffset + a.Label.size);
            d.config.offsetX = (f.right - f.left) / 2;
            this.delegate = d;
            this.canvas = this.delegate.canvas
        },
        loadJSON: function (a) {
            for (var c = h.time(), b = [], d = this.delegate, e = h.splat(a.label), f = h.splat(a.color || this.colors), g = this.config, j = !! g.type.split(":")[1], i = g.animate, k = 0, a = a.values, l = a.length; k < l - 1; k++) {
                var o = a[k],
                    r = a[k - 1],
                    m = a[k + 1],
                    s = h.splat(a[k].values),
                    n = h.splat(a[k + 1].values),
                    s = h.zip(s, n);
                b.push({
                    id: c + o.label,
                    name: o.label,
                    data: {
                        value: s,
                        $valueArray: s,
                        $colorArray: f,
                        $stringArray: e,
                        $next: m.label,
                        $prev: r ? r.label : !1,
                        $config: g,
                        $gradient: j
                    },
                    children: []
                })
            }
            d.loadJSON({
                id: c + "$root",
                name: "",
                data: {
                    $type: "none",
                    $width: 1,
                    $height: 1
                },
                children: b
            });
            this.normalizeDims();
            d.compute();
            d.select(d.root);
            i && d.fx.animate({
                modes: ["node-property:height:dimArray"],
                duration: 800
            })
        },
        updateJSON: function (a, c) {
            if (!this.busy) {
                this.busy = !0;
                for (var b = this.delegate, d = b.graph, e = a.label && h.splat(a.label), f = a.values, g = a.color, j = this.config.animate, i = this, k = {}, l = 0, o = f.length; l < o; l++) k[f[l].label] = f[l];
                d.eachNode(function (a) {
                    var b = k[a.name],
                        c = a.getData("stringArray"),
                        d = a.getData("valueArray"),
                        f = a.getData("next");
                    g && (j ? a.setData("colorArray", g.slice(), "end") : a.setData("colorArray", g.slice()));
                    b && (b.values = h.splat(b.values), h.each(d, function (a, d) {
                        a[0] = b.values[d];
                        e && (c[d] = e[d])
                    }), a.setData("valueArray", d));
                    f && (b = k[f]) && h.each(d, function (a, c) {
                        a[1] = b.values[c]
                    })
                });
                this.normalizeDims();
                b.compute();
                b.select(b.root);
                j && b.fx.animate({
                    modes: ["node-property:height:dimArray:colorArray"],
                    duration: 800,
                    onComplete: function () {
                        i.busy = !1;
                        c && c.onComplete()
                    }
                })
            }
        },
        filter: function (a, c) {
            if (!this.busy) {
                this.busy = !0;
                this.config.Tips.enable && this.delegate.tips.hide();
                this.select(!1, !1, !1);
                var b = h.splat(a),
                    d = this.delegate.graph.getNode(this.delegate.root),
                    e = this;
                this.normalizeDims();
                d.eachAdjacency(function (a) {
                    var a = a.nodeTo,
                        c = a.getData("dimArray", "end"),
                        d = a.getData("stringArray");
                    a.setData("dimArray", h.map(c, function (a, c) {
                        return -1 < h.indexOf(b, d[c]) ? a : [0, 0]
                    }), "end")
                });
                this.delegate.fx.animate({
                    modes: ["node-property:dimArray"],
                    duration: 1E3,
                    onComplete: function () {
                        e.busy = !1;
                        c && c.onComplete()
                    }
                })
            }
        },
        restore: function (a) {
            if (!this.busy) {
                this.busy = !0;
                this.filterArray = !1;
                this.config.Tips.enable && this.delegate.tips.hide();
                this.select(!1, !1, !1);
                this.normalizeDims();
                var c = this;
                this.delegate.fx.animate({
                    modes: ["node-property:height:dimArray"],
                    duration: 1E3,
                    onComplete: function () {
                        c.busy = !1;
                        a && a.onComplete()
                    }
                })
            }
        },
        select: function (a, c, b) {
            if (this.config.selectOnHover) {
                var d = this.selected;
                if (d.id != a || d.name != c || d.index != b) {
                    d.id = a;
                    d.name = c;
                    d.index = b;
                    this.delegate.graph.eachNode(function (a) {
                        a.setData("border", !1)
                    });
                    if (a && (a = this.delegate.graph.getNode(a), a.setData("border", d), d = a.getData(0 === b ? "prev" : "next")))(a = this.delegate.graph.getByName(d)) && a.setData("border", {
                        name: c,
                        index: 1 - b
                    });
                    this.delegate.plot()
                }
            }
        },
        getLegend: function () {
            var a = {}, c;
            this.delegate.graph.getNode(this.delegate.root).eachAdjacency(function (a) {
                c = a.nodeTo
            });
            var b = c.getData("colorArray"),
                d = b.length;
            h.each(c.getData("stringArray"), function (c, f) {
                a[c] = b[f % d]
            });
            return a
        },
        getMaxValue: function () {
            var a = 0;
            this.delegate.graph.eachNode(function (c) {
                var c = c.getData("valueArray"),
                    b = 0,
                    d = 0;
                h.each(c, function (c) {
                    b = +c[0];
                    d = +c[1];
                    c = d > b ? d : b;
                    a = a > c ? a : c
                })
            });
            return a
        },
        normalizeDims: function () {
            var a = 0;
            this.delegate.graph.getNode(this.delegate.root).eachAdjacency(function () {
                a++
            });
            var c = this.getMaxValue() || 1,
                b = this.delegate.canvas.getSize(),
                d = this.config,
                e = d.Margin,
                f = d.labelOffset + d.Label.size,
                g = (b.width - (e.left + e.right)) / a,
                j = d.animate,
                i = b.height - (e.top + e.bottom) - (d.showAggregates && f) - (d.showLabels && f);
            this.delegate.graph.eachNode(function (a) {
                var b = [];
                h.each(a.getData("valueArray"),

                function () {
                    b.push([0, 0])
                });
                a.setData("width", g);
                j ? (a.setData("height", i, "end"), a.setData("dimArray", h.map(a.getData("valueArray"), function (a) {
                    return [a[0] * (i / 2) / c, a[1] * (i / 2) / c]
                }), "end"), a.getData("dimArray") || a.setData("dimArray", b)) : (a.setData("height", i, "end"), a.setData("dimArray", h.map(a.getData("valueArray"), function (a) {
                    return [a[0] * (i / 2) / c, a[1] * (i / 2) / c]
                })))
            })
        }
    });
    $jit.StreamChart = new m({
        st: null,
        colors: "#416D9C #70A35E #EBB056 #C74243 #83548B #909291 #557EAA".split(" "),
        selected: {},
        busy: !1,
        initialize: function (a) {
            this.controller = this.config = h.merge(q("Canvas", "Margin", "Label", "AreaChart"), {
                Label: {
                    type: "Native"
                }
            }, a);
            var a = this.config.showLabels,
                c = h.type(a),
                b = this.config.showAggregates,
                d = h.type(b);
            this.config.showLabels = "function" == c ? a : h.lambda(a);
            this.config.showAggregates = "function" == d ? b : h.lambda(b);
            this.initializeViz()
        },
        initializeViz: function () {
            var a = this.config,
                c = this,
                b = a.type.split(":")[0];
            var d = new $jit.ST({
                    injectInto: a.injectInto,
                    width: a.width,
                    height: a.height,
                    orientation: "bottom",
                    levelDistance: 0,
                    siblingOffset: 0,
                    subtreeOffset: 0,
                    withLabels: "Native" != a.Label.type,
                    useCanvas: a.useCanvas,
                    Label: {
                        type: a.Label.type
                    },
                    Node: {
                        overridable: !0,
                        type: "streamchart-" + b,
                        align: "center",
                        width: 1,
                        height: 1
                    },
                    Edge: {
                        type: "none"
                    },
                    Tips: {
                        enable: a.Tips.enable,
                        type: "Native",
                        force: !0,
                        onShow: function (b, c, d) {
                            a.Tips.onShow(b, d, c)
                        }
                    },
                    Events: {
                        enable: !0,
                        type: "Native",
                        onClick: function (b, d, e) {
                            if (a.filterOnClick || a.Events.enable)(b = d.getContains()) && a.filterOnClick && c.filter(b.name), a.Events.enable && a.Events.onClick(b, d, e)
                        },
                        onRightClick: function () {
                            a.restoreOnRightClick && c.restore()
                        },
                        onMouseMove: function (b, d) {
                            if (a.selectOnHover) {
                                if (b) {
                                    var e = d.getContains();
                                    c.select(b.id, e.name, e.index, b)
                                }
                                else c.select(!1, !1, !1);
                                c.setupLabels()
                            }
                        },
                        onMouseEnter: function () {
                            d.canvas.getElement().style.cursor = "pointer"
                        },
                        onMouseLeave: function () {
                            d.canvas.getElement().style.cursor = "default"
                        }
                    }
                }),
                b = d.canvas.getSize(),
                e = a.Margin;
            d.config.offsetY = -b.height / 2 + e.bottom + (a.showLabels && a.labelOffset + a.Label.size);
            d.config.offsetX = (e.right - e.left) / 2;
            this.delegate = d;
            this.canvas = this.delegate.canvas
        },
        loadJSON: function (json) {
            var prefix = h.time(),
                ch = [],
                delegate = this.delegate, 
                name = h.splat(json.label),
                color = h.splat(json.color || this.colors),
                config = this.config,
                gradient = !!config.type.split(":")[1],
                animate = config.animate,
                quote = h.splat(json.quote)

            for (var k = 0, values = json.values, l = values.length; k < l - 1; k++) {
                var val = values[k], prev = values[k - 1], next = values[k + 1];
                var valLeft = h.splat(values[k].values), valRight = h.splat(values[k + 1].values);
                var valArray = h.zip(valLeft, valRight);
                var acumLeft = 0, acumRight = 0;
                ch.push({
                    id: prefix + val.label,
                    name: val.label,
                    data: {
                        value: valArray,
                        $valueArray: valArray,
                        $colorArray: color,
                        $quoteArray: quote,
                        $stringArray: name,
                        $next: next.label,
                        $prev: prev ? prev.label : false,
                        $config: config,
                        $gradient: gradient
                    },
                    children: []
                })
            }
            var root = {
                id: prefix + "$root",
                name: "",
                data: {
                    $type: "none",
                    $width: 1,
                    $height: 1
                },
                children: ch
            };
            delegate.loadJSON(root);

            this.normalizeDims();
            delegate.compute();
            delegate.select(delegate.root);
            if (animate) {
                d.fx.animate({
                    modes: ["node-property:height:dimArray"],
                    duration: 1500
                });
            }
        },
        setupLabels: function () {
            var filters = this.filterArray,
                g = this.delegate.graph,
                root = g.getNode(this.delegate.root),
                that = this, dims = {}, map = {}, quotes = {},
                ctx = this.canvas.getCtx(),
                size = this.canvas.getSize();

            root.eachAdjacency(function (a) {
                var n = a.nodeTo,
                    pos = n.pos.getc(true),
                    dimArray = n.getData("dimArray"),
                    stringArray = n.getData("stringArray"),
                    quoteArray = n.getData("quoteArray");

                if (!n.getData("prev") || !g.getByName(n.getData("next"))) { return; }

                for (var h = 0, j = stringArray.length; h < j; ++h) {
                    var key = stringArray[h].trim(),
                        dimLeft = dimArray[h][0],
                        dimRight = dimArray[h][1],
                        lpos = n.getData("left-pos"),
                        rpos = n.getData("right-pos");

                    quotes[key] = quoteArray[h];

                    if (!(key in dims) || dims[key] < dimLeft || dims[key] < dimRight) {
                        if (dimLeft > dimRight) {
                            dims[key] = dimLeft;
                            var pos = n.getData("left-pos"),
                                index = 2 * h,
                                mid = [(pos[index][0] + pos[index + 1][0]) / 2, (pos[index][1] + pos[index + 1][1]) / 2];
                            map[key] = mid;
                        } else {
                            var pos = n.getData("right-pos"),
                                index = 2 * h,
                                mid = [(pos[index][0] + pos[index + 1][0]) / 2, (pos[index][1] + pos[index + 1][1]) / 2];
                            dims[key] = dimRight;
                            map[key] = mid;
                        }
                    }
                }
            });
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.font = "13px Helvetica";
            for (var i = 0, l = filters.length; i < l; ++i) {
                var pos = map[filters[i]];
                //var i = h(a[b] + "_name").innerHTML.split(" ")[0];
                var name = filters[i]
                ctx.fillStyle = ctx.strokeStyle = "black";
                //alert(j);
                var key = filters[i].trim();
                ctx.fillText(quotes[key], pos[0], pos[1])
            }
        },
        select: function (a, c, b) {
            if (this.config.selectOnHover) {
                var d = this.selected;
                if (d.id != a || d.name != c || d.index != b) {
                    d.id = a;
                    d.name = c;
                    d.index = b;
                    this.delegate.graph.eachNode(function (a) {
                        a.setData("border", !1)
                    });
                    if (a && (a = this.delegate.graph.getNode(a), a.setData("border", d), d = a.getData(0 === b ? "prev" : "next")))(a = this.delegate.graph.getByName(d)) && a.setData("border", {
                        name: c,
                        index: 1 - b
                    });
                    this.delegate.plot()
                }
            }
        },
        filter: function (a,
        c) {
            if (!this.busy) {
                this.busy = !0;
                this.filterArray = a.slice();
                this.config.Tips.enable && this.delegate.tips.hide();
                this.select(!1, !1, !1);
                var b = h.splat(a),
                    d = this.delegate.graph.getNode(this.delegate.root),
                    e = this,
                    f = -1,
                    g = this.config.Margin,
                    j = this.canvas.getSize().height - g.top - g.bottom;
                this.normalizeDims();
                d.eachAdjacency(function (a) {
                    var a = a.nodeTo,
                        c = a.getData("dimArray", "end"),
                        d = a.getData("stringArray"),
                        e = 0,
                        g = 0;
                    a.setData("dimArray", h.map(c, function (a, c) {
                        return -1 < h.indexOf(b, d[c].trim()) ? (e += a[0], g += a[1], a) : [0, 0]
                    }), "end");
                    c = Math.max(e, g);
                    f = f < c ? c : f;
                    a.setData("height", c, "end")
                });
                d.eachAdjacency(function (a) {
                    var a = a.nodeTo,
                        b = a.getData("dimArray", "end"),
                        c = a.getData("height", "end");
                    a.setData("height", c * j / f, "end");
                    h.each(b, function (a, c) {
                        b[c][0] = j / f * a[0];
                        b[c][1] = j / f * a[1]
                    })
                });
                this.delegate.fx.animate({
                    modes: ["node-property:height:dimArray"],
                    duration: 300,
                    onComplete: function () {
                        e.busy = !1;
                        c && c.onComplete()
                    }
                })
            }
        },
        restore: function (a) {
            if (!this.busy) {
                this.busy = !0;
                this.config.Tips.enable && this.delegate.tips.hide();
                this.select(!1, !1, !1);
                this.normalizeDims();
                var c = this;
                this.delegate.fx.animate({
                    modes: ["node-property:height:dimArray"],
                    duration: 300,
                    onComplete: function () {
                        c.busy = !1;
                        a && a.onComplete()
                    }
                })
            }
        },
        getMaxValue: function () {
            var a = 0;
            this.delegate.graph.eachNode(function (c) {
                var c = c.getData("valueArray"),
                    b = 0,
                    d = 0;
                h.each(c, function (a) {
                    b += +a[0];
                    d += +a[1]
                });
                c = d > b ? d : b;
                a = a > c ? a : c
            });
            return a
        },
        normalizeDims: function () {
            var a = 0;
            this.delegate.graph.getNode(this.delegate.root).eachAdjacency(function () {
                a++
            });
            var c = this.getMaxValue() || 1,
                b = this.delegate.canvas.getSize(),
                d = this.config,
                e = d.Margin,
                f = d.labelOffset + d.Label.size,
                g = (b.width - (e.left + e.right)) / a,
                j = d.animate,
                i = b.height - (e.top + e.bottom) - (d.showAggregates && f) - (d.showLabels && f);
            this.delegate.config.levelDistance = i / 2;
            this.delegate.graph.eachNode(function (a) {
                var b = 0,
                    d = 0,
                    e = [];
                h.each(a.getData("valueArray"), function (a) {
                    b += +a[0];
                    d += +a[1];
                    e.push([0, 0])
                });
                var f = d > b ? d : b;
                a.setData("width", g);
                j ? (a.setData("height", f * i / c, "end"), a.setData("dimArray", h.map(a.getData("valueArray"), function (a) {
                    return [a[0] * i / c, a[1] * i / c]
                }), "end"),
                a.getData("dimArray") || a.setData("dimArray", e)) : (a.setData("height", f * i / c), a.setData("dimArray", h.map(a.getData("valueArray"), function (a) {
                    return [a[0] * i / c, a[1] * i / c]
                })))
            })
        }
    });
    (function () {
        function a(a) {
            for (var b = a.pos.getc(!0), d = a.getData("width"), e = a.getData("height"), f = this.getAlignedPos(b, d, e), b = f.x, e = f.y + e, f = a.getData("dimArray"), g = h.reduce(f, function (a, b) {
                return a + b[0]
            }, 0), j = h.reduce(f, function (a, b) {
                return a + b[1]
            }, 0), i = (g - j) / 2, k = [], l = [], o = 0, m = f.length, n = 0, s = 0; o < m; o++) g >= j ? (k.push([b, e - n]), l.push([b + d, e - s - i]), l.push([b + d, e - s - f[o][1] - i]), k.push([b, e - n - f[o][0]])) : (k.push([b, e - n + i]), l.push([b + d, e - s]), l.push([b + d, e - s - f[o][1]]), k.push([b, e - n - f[o][0] + i])), n += f[o][0] || 0, s += f[o][1] || 0;
            a.setData("left-pos", k);
            a.setData("right-pos", l)
        }
        $jit.ST.Plot.NodeTypes.implement({
            "streamchart-smooth": {
                render: function (c, b) {
                    var d = c.pos.getc(!0),
                        e = c.getData("width"),
                        f = c.getData("height");
                    this.getAlignedPos(d, e, f);
                    c.getData("stringArray");
                    var d = c.getData("dimArray"),
                        g = h.reduce(d, function (a, b) {
                            return a + b[0]
                        }, 0),
                        j = h.reduce(d,

                        function (a, b) {
                            return a + b[1]
                        }, 0),
                        d = c.getData("colorArray"),
                        f = d.length;
                    c.getData("config");
                    c.getData("gradient");
                    var i = this.viz.graph,
                        k = c.getData("prev"),
                        l = c.getData("next"),
                        k = k ? i.getByName(k) : !1,
                        i = l ? i.getByName(l) : !1;
                    k && k.pos.getc(!0);
                    i && i.pos.getc(!0);
                    offsetHeight = (g - j) / 2;
                    a.call(this, c);
                    g = b.getCtx();
                    c.getData("border");
                    var o = c.getData("left-pos"),
                        m = c.getData("right-pos");
                    if (k && i) {
                        a.call(this, k);
                        a.call(this, i);
                        var n = k.getData("left-pos"),
                            s = i.getData("right-pos")
                    }
                    else n = o.map(function (a) {
                        a = a.slice();
                        a[0] -= e;
                        return a
                    }), s = m.map(function (a) {
                        a = a.slice();
                        a[0] += e;
                        return a
                    });
                    j = n.map(function (a, b) {
                        var c = o[b][0],
                            d = o[b][1],
                            e = m[b][0],
                            f = m[b][1],
                            g = n[b][0],
                            h = n[b][1],
                            j = s[b][0],
                            i = s[b][1],
                            k = (g + c) / 2,
                            l = (h + d) / 2,
                            p = (c + e) / 2,
                            q = (d + f) / 2,
                            t = (e + j) / 2,
                            v = (f + i) / 2,
                            h = Math.sqrt((c - g) * (c - g) + (d - h) * (d - h)),
                            g = Math.sqrt((e - c) * (e - c) + (f - d) * (f - d)),
                            i = Math.sqrt((j - e) * (j - e) + (i - f) * (i - f)),
                            j = h / (h + g),
                            i = g / (g + i),
                            k = k + (p - k) * j,
                            l = l + (q - l) * j,
                            t = p + (t - p) * i,
                            v = q + (v - q) * i;
                        return [k + 0.8 * (p - k) + c - k, l + 0.8 * (q - l) + d - l, t + 0.8 * (p - t) + e - t, v + 0.8 * (q - v) + f - v]
                    });
                    k = 0;
                    for (i = o.length; k < i; k += 2) {
                        var p = o[k],
                            l = m[k],
                            q = j[k];
                        g.fillStyle = g.strokeStyle = d[(k >> 1) % f];
                        g.lineWidth = 0;
                        g.save();
                        g.beginPath();
                        g.moveTo(p[0], p[1]);
                        g.bezierCurveTo(q[0], q[1], q[2], q[3], l[0], l[1]);
                        p = o[k + 1];
                        l = m[k + 1];
                        q = j[k + 1];
                        g.lineTo(l[0], l[1]);
                        g.bezierCurveTo(q[2], q[3], q[0], q[1], p[0], p[1]);
                        g.closePath();
                        g.stroke();
                        g.fill();
                        g.restore()
                    }
                },
                contains: function (a, b) {
                    var d = this.viz.canvas,
                        e = d.getSize(),
                        f = d.getCtx(),
                        g = a.pos.getc(!0),
                        d = a.getData("width"),
                        j = a.getData("height"),
                        g = this.getAlignedPos(g, d, j).x;
                    a.getData("dimArray");
                    j = b.x - g;
                    if (b.x < g || b.x > g + d) return !1;
                    b.x += e.width / 2;
                    b.y += e.height / 2;
                    for (var g = a.getData("colorArray"), f = f.getImageData(0, 0, e.width, e.height).data, i = 4 * (b.x + b.y * e.width), e = 4 * (b.x + (b.y - 5) * e.width), i = h.rgbToHex([f[i], f[i + 1], f[i + 2]]).toUpperCase(), d = +(j > d / 2), j = 0, k = g.length; j < k; ++j) if (g[j] == i) return {
                        name: a.getData("stringArray")[j],
                        color: i,
                        value: a.getData("valueArray")[j][d],
                        index: d
                    };
                    i = h.rgbToHex([f[e], f[e + 1], f[e + 2]]).toUpperCase();
                    j = 0;
                    for (k = g.length; j < k; ++j) if (g[j] == i) return {
                        name: a.getData("stringArray")[j],
                        color: i,
                        value: a.getData("valueArray")[j][d],
                        index: d
                    };
                    return !1
                }
            },
            "streamchart-stacked": {
                render: function (c, b) {
                    var d = c.pos.getc(!0),
                        e = c.getData("width"),
                        f = c.getData("height");
                    this.getAlignedPos(d, e, f);
                    c.getData("stringArray");
                    var d = c.getData("dimArray"),
                        g = h.reduce(d, function (a, b) {
                            return a + b[0]
                        }, 0),
                        j = h.reduce(d, function (a, b) {
                            return a + b[1]
                        }, 0),
                        d = c.getData("colorArray"),
                        f = d.length;
                    c.getData("config");
                    c.getData("gradient");
                    var i = this.viz.graph,
                        k = c.getData("prev"),
                        l = c.getData("next"),
                        k = k ? i.getByName(k) : !1,
                        l = l ? i.getByName(l) : !1;
                    k && k.pos.getc(!0);
                    l && l.pos.getc(!0);
                    offsetHeight = (g - j) / 2;
                    a.call(this, c);
                    g = b.getCtx();
                    c.getData("border");
                    j = c.getData("left-pos");
                    i = c.getData("right-pos");
                    k && l ? (a.call(this, k), a.call(this, l), k.getData("left-pos"), l.getData("right-pos")) : (j.map(function (a) {
                        a = a.slice();
                        a[0] -= e;
                        return a
                    }), i.map(function (a) {
                        a = a.slice();
                        a[0] += e;
                        return a
                    }));
                    k = 0;
                    for (l = j.length; k < l; k += 2) {
                        var m = j[k],
                            n = i[k],
                            p;
                        g.fillStyle = g.strokeStyle = d[(k >> 1) % f];
                        g.save();
                        g.beginPath();
                        g.moveTo(m[0] - 0.8, m[1]);
                        g.lineTo(n[0],
                        n[1]);
                        p = j[k + 1];
                        n = i[k + 1];
                        g.lineTo(n[0], n[1]);
                        g.lineTo(p[0] - 0.8, p[1]);
                        g.lineTo(m[0] - 0.8, m[1]);
                        g.closePath();
                        g.fill();
                        g.restore()
                    }
                },
                contains: function (a, b) {
                    var d = this.viz.canvas;
                    d.getSize();
                    d.getCtx();
                    var e = a.pos.getc(!0),
                        d = a.getData("width"),
                        f = a.getData("height"),
                        e = this.getAlignedPos(e, d, f),
                        g = e.x,
                        h = e.y;
                    a.getData("dimArray");
                    e = b.x - g;
                    if (b.x < g || b.x > g + d || b.y < h || b.y > h + f) return !1;
                    for (var g = a.getData("left-pos"), h = a.getData("right-pos"), f = 0, i = g.length; f < i; f += 2) {
                        var k = g[f],
                            l = g[f + 1],
                            l = l[1] + (h[f + 1][1] - l[1]) * e / d;
                        if (b.y <= k[1] + (h[f][1] - k[1]) * e / d && b.y >= l) {
                            d = +(e > d / 2);
                            if (0 == f) break;
                            return {
                                name: a.getData("stringArray")[f >> 1],
                                color: a.getData("colorArray")[f >> 1],
                                value: a.getData("valueArray")[f >> 1][d],
                                index: d
                            }
                        }
                    }
                    return !1
                }
            }
        })
    })();
    v.TM = {};
    v.TM.SliceAndDice = new m({
        compute: function (a) {
            var c = this.graph.getNode(this.clickedNode && this.clickedNode.id || this.root);
            this.controller.onBeforeCompute(c);
            var b = this.canvas.getSize(),
                d = this.config,
                e = b.width,
                b = b.height;
            this.graph.computeLevels(this.root, 0, "ignore");
            c.getPos(a).setc(-e / 2, - b / 2);
            c.setData("width", e, a);
            c.setData("height", b + d.titleHeight, a);
            this.computePositions(c, c, this.layout.orientation, a);
            this.controller.onAfterCompute(c)
        },
        computePositions: function (a, c, b, d) {
            var e = 0;
            a.eachSubnode(function (a) {
                e += a.getData("area", d)
            });
            var f = this.config,
                g = a.getData("width", d),
                h = Math.max(a.getData("height", d) - f.titleHeight, 0),
                a = a == c ? 1 : c.getData("area", d) / e,
                i, k, l, m, n;
            "h" == b ? (b = "v", g *= a, i = "height", k = "y", l = "x", m = f.titleHeight, n = 0) : (b = "h", h *= a, i = "width", k = "x", l = "y", m = 0, n = f.titleHeight);
            var p = c.getPos(d);
            c.setData("width", g, d);
            c.setData("height", h, d);
            var q = 0,
                t = this;
            c.eachSubnode(function (a) {
                var e = a.getPos(d);
                e[k] = q + p[k] + m;
                e[l] = p[l] + n;
                t.computePositions(c, a, b, d);
                q = q + a.getData(i, d)
            })
        }
    });
    v.TM.Area = {
        compute: function (a) {
            var a = a || "current",
                c = this.graph.getNode(this.clickedNode && this.clickedNode.id || this.root);
            this.controller.onBeforeCompute(c);
            var b = this.config,
                d = this.canvas.getSize(),
                e = d.width,
                d = d.height,
                f = b.offset,
                g = e - f,
                f = d - f;
            this.graph.computeLevels(this.root, 0, "ignore");
            c.getPos(a).setc(-e / 2, - d / 2);
            c.setData("width", e, a);
            c.setData("height", d, a);
            this.computePositions(c, {
                top: -d / 2 + b.titleHeight,
                left: -e / 2,
                width: g,
                height: f - b.titleHeight
            }, a);
            this.controller.onAfterCompute(c)
        },
        computeDim: function (a, c, b, d, e, f) {
            if (1 == a.length + c.length) this.layoutLast(1 == a.length ? a : c, b, d, f);
            else if (2 <= a.length && 0 == c.length && (c = [a.shift()]), 0 == a.length) 0 < c.length && this.layoutRow(c, b, d, f);
            else {
                var g = a[0];
                e(c, b) >= e([g].concat(c), b) ? this.computeDim(a.slice(1), c.concat([g]), b, d, e, f) : (c = this.layoutRow(c, b, d, f), this.computeDim(a, [], c.dim, c, e, f))
            }
        },
        worstAspectRatio: function (a, c) {
            if (!a || 0 == a.length) return Number.MAX_VALUE;
            for (var b = 0, d = 0, e = Number.MAX_VALUE, f = 0, g = a.length; f < g; f++) var h = a[f]._area,
                b = b + h,
                e = e < h ? e : h,
                d = d > h ? d : h;
            f = c * c;
            b *= b;
            return Math.max(f * d / b, b / (f * e))
        },
        avgAspectRatio: function (a, c) {
            if (!a || 0 == a.length) return Number.MAX_VALUE;
            for (var b = 0, d = 0, e = a.length; d < e; d++) var f = a[d]._area / c,
                b = b + (c > f ? c / f : f / c);
            return b / e
        },
        layoutLast: function (a, c, b, d) {
            a = a[0];
            a.getPos(d).setc(b.left, b.top);
            a.setData("width", b.width, d);
            a.setData("height",
            b.height, d)
        }
    };
    v.TM.Squarified = new m({
        Implements: v.TM.Area,
        computePositions: function (a, c, b) {
            var d = this.config,
                e = Math.max;
            this.layout.orientation = c.width >= c.height ? "h" : "v";
            var f = a.getSubnodes([1, 1], "ignore");
            if (0 < f.length) {
                this.processChildrenLayout(a, f, c, b);
                for (var a = 0, g = f.length; a < g; a++) {
                    var h = f[a],
                        i = d.offset,
                        c = e(h.getData("height", b) - i - d.titleHeight, 0),
                        i = e(h.getData("width", b) - i, 0),
                        k = h.getPos(b),
                        c = {
                            width: i,
                            height: c,
                            top: k.y + d.titleHeight,
                            left: k.x
                        };
                    this.computePositions(h, c, b)
                }
            }
        },
        processChildrenLayout: function (a,
        c, b, d) {
            var a = b.width * b.height,
                e, f = c.length,
                g = 0,
                h = [];
            for (e = 0; e < f; e++) h[e] = parseFloat(c[e].getData("area", d)), g += h[e];
            for (e = 0; e < f; e++) c[e]._area = a * h[e] / g;
            a = this.layout.horizontal() ? b.height : b.width;
            c.sort(function (a, b) {
                var c = b._area - a._area;
                return c ? c : b.id == a.id ? 0 : b.id < a.id ? 1 : -1
            });
            e = [c[0]];
            this.squarify(c.slice(1), e, a, b, d)
        },
        squarify: function (a, c, b, d, e) {
            this.computeDim(a, c, b, d, this.worstAspectRatio, e)
        },
        layoutRow: function (a, c, b, d) {
            return this.layout.horizontal() ? this.layoutV(a, c, b, d) : this.layoutH(a, c,
            b, d)
        },
        layoutV: function (a, c, b, d) {
            var e = 0;
            h.each(a, function (a) {
                e += a._area
            });
            for (var c = e / c, f = 0, g = 0, j = a.length; g < j; g++) {
                var i = a[g]._area / c,
                    k = a[g];
                k.getPos(d).setc(b.left, b.top + f);
                k.setData("width", c, d);
                k.setData("height", i, d);
                f += i
            }
            a = {
                height: b.height,
                width: b.width - c,
                top: b.top,
                left: b.left + c
            };
            a.dim = Math.min(a.width, a.height);
            a.dim != a.height && this.layout.change();
            return a
        },
        layoutH: function (a, c, b, d) {
            var e = 0;
            h.each(a, function (a) {
                e += a._area
            });
            for (var f = e / c, g = b.top, j = 0, i = 0, k = a.length; i < k; i++) {
                var l = a[i],
                    c = l._area / f;
                l.getPos(d).setc(b.left + j, g);
                l.setData("width", c, d);
                l.setData("height", f, d);
                j += c
            }
            a = {
                height: b.height - f,
                width: b.width,
                top: b.top + f,
                left: b.left
            };
            a.dim = Math.min(a.width, a.height);
            a.dim != a.width && this.layout.change();
            return a
        }
    });
    v.TM.Strip = new m({
        Implements: v.TM.Area,
        computePositions: function (a, c, b) {
            var d = a.getSubnodes([1, 1], "ignore"),
                e = this.config,
                f = Math.max;
            if (0 < d.length) {
                this.processChildrenLayout(a, d, c, b);
                for (var a = 0, g = d.length; a < g; a++) {
                    var h = d[a],
                        i = e.offset,
                        c = f(h.getData("height", b) - i - e.titleHeight,
                        0),
                        i = f(h.getData("width", b) - i, 0),
                        k = h.getPos(b),
                        c = {
                            width: i,
                            height: c,
                            top: k.y + e.titleHeight,
                            left: k.x
                        };
                    this.computePositions(h, c, b)
                }
            }
        },
        processChildrenLayout: function (a, c, b, d) {
            var a = b.width * b.height,
                e, f = c.length,
                g = 0,
                h = [];
            for (e = 0; e < f; e++) h[e] = +c[e].getData("area", d), g += h[e];
            for (e = 0; e < f; e++) c[e]._area = a * h[e] / g;
            a = this.layout.horizontal() ? b.width : b.height;
            e = [c[0]];
            this.stripify(c.slice(1), e, a, b, d)
        },
        stripify: function (a, c, b, d, e) {
            this.computeDim(a, c, b, d, this.avgAspectRatio, e)
        },
        layoutRow: function (a, c, b, d) {
            return this.layout.horizontal() ? this.layoutH(a, c, b, d) : this.layoutV(a, c, b, d)
        },
        layoutV: function (a, c, b, d) {
            var e = 0;
            h.each(a, function (a) {
                e += a._area
            });
            for (var f = e / c, g = 0, j = 0, i = a.length; j < i; j++) {
                var k = a[j],
                    l = k._area / f;
                k.getPos(d).setc(b.left, b.top + (c - l - g));
                k.setData("width", f, d);
                k.setData("height", l, d);
                g += l
            }
            return {
                height: b.height,
                width: b.width - f,
                top: b.top,
                left: b.left + f,
                dim: c
            }
        },
        layoutH: function (a, c, b, d) {
            var e = 0;
            h.each(a, function (a) {
                e += a._area
            });
            for (var f = e / c, g = b.height - f, j = 0, i = 0, k = a.length; i < k; i++) {
                var l = a[i],
                    m = l._area / f;
                l.getPos(d).setc(b.left + j, b.top + g);
                l.setData("width", m, d);
                l.setData("height", f, d);
                j += m
            }
            return {
                height: b.height - f,
                width: b.width,
                top: b.top,
                left: b.left,
                dim: c
            }
        }
    });
    $jit.TM = {};
    var p = $jit.TM;
    $jit.TM.$extend = !0;
    p.Base = {
        layout: {
            orientation: "h",
            vertical: function () {
                return "v" == this.orientation
            },
            horizontal: function () {
                return "h" == this.orientation
            },
            change: function () {
                this.orientation = this.vertical() ? "h" : "v"
            }
        },
        initialize: function (a) {
            this.controller = this.config = h.merge(q("Canvas", "Node", "Edge", "Fx", "Controller", "Tips", "NodeStyles", "Events", "Navigation", "Label"), {
                orientation: "h",
                titleHeight: 13,
                offset: 2,
                levelsToShow: 0,
                constrained: !1,
                animate: !1,
                Node: {
                    type: "rectangle",
                    overridable: !0,
                    width: 3,
                    height: 3,
                    color: "#444"
                },
                Label: {
                    textAlign: "center",
                    textBaseline: "top"
                },
                Edge: {
                    type: "none"
                },
                duration: 700,
                fps: 45
            }, a);
            this.layout.orientation = this.config.orientation;
            a = this.config;
            a.useCanvas ? (this.canvas = a.useCanvas, this.config.labelContainer = this.canvas.id + "-label") : (a.background && (a.background = h.merge({
                type: "Circles"
            }, a.background)), this.canvas = new w(this,
            a), this.config.labelContainer = ("string" == typeof a.injectInto ? a.injectInto : a.injectInto.id) + "-label");
            this.graphOptions = {
                klass: t,
                Node: {
                    selected: !1,
                    exist: !0,
                    drawn: !0
                }
            };
            this.graph = new n(this.graphOptions, this.config.Node, this.config.Edge);
            this.labels = new p.Label[a.Label.type](this);
            this.fx = new p.Plot(this);
            this.op = new p.Op(this);
            this.group = new p.Group(this);
            this.geom = new p.Geom(this);
            this.clickedNode = null;
            this.busy = !1;
            this.initializeExtras()
        },
        refresh: function () {
            if (!this.busy) {
                this.busy = !0;
                var a = this;
                this.config.animate ? (this.compute("end"), 0 < this.config.levelsToShow && this.geom.setRightLevelToShow(this.graph.getNode(this.clickedNode && this.clickedNode.id || this.root)), this.fx.animate(h.merge(this.config, {
                    modes: ["linear", "node-property:width:height"],
                    onComplete: function () {
                        a.busy = !1
                    }
                }))) : ("Native" != this.config.Label.type && (a = this, this.graph.eachNode(function (c) {
                    a.labels.hideLabel(c, !1)
                })), this.busy = !1, this.compute(), 0 < this.config.levelsToShow && this.geom.setRightLevelToShow(this.graph.getNode(this.clickedNode && this.clickedNode.id || this.root)), this.plot())
            }
        },
        plot: function () {
            this.fx.plot()
        },
        leaf: function (a) {
            return 0 == a.getSubnodes([1, 1], "ignore").length
        },
        enter: function (a) {
            if (!this.busy) {
                this.busy = !0;
                var c = this,
                    b = this.config,
                    d = this.graph,
                    e = this.clickedNode,
                    f = {
                        onComplete: function () {
                            0 < b.levelsToShow && c.geom.setRightLevelToShow(a);
                            (0 < b.levelsToShow || b.request) && c.compute();
                            b.animate ? (d.nodeList.setData("alpha", 0, "end"), a.eachSubgraph(function (a) {
                                a.setData("alpha", 1, "end")
                            }, "ignore"), c.fx.animate({
                                duration: 500,
                                modes: ["node-property:alpha"],
                                onComplete: function () {
                                    c.clickedNode = a;
                                    c.compute("end");
                                    c.clickedNode = e;
                                    c.fx.animate({
                                        modes: ["linear", "node-property:width:height"],
                                        duration: 1E3,
                                        onComplete: function () {
                                            c.busy = !1;
                                            c.clickedNode = a
                                        }
                                    })
                                }
                            })) : (c.busy = !1, c.clickedNode = a, c.refresh())
                        }
                    };
                if (b.request) this.requestNodes(a, f);
                else f.onComplete()
            }
        },
        out: function () {
            if (!this.busy) {
                this.busy = !0;
                this.events.hoveredNode = !1;
                var a = this,
                    c = this.config,
                    b = this.graph,
                    d = b.getNode(this.clickedNode && this.clickedNode.id || this.root).getParents()[0],
                    e = this.clickedNode;
                if (d) if (callback = {
                    onComplete: function () {
                        a.clickedNode = d;
                        c.request ? a.requestNodes(d, {
                            onComplete: function () {
                                a.compute();
                                a.plot();
                                a.busy = !1
                            }
                        }) : (a.compute(), a.plot(), a.busy = !1)
                    }
                }, 0 < c.levelsToShow && this.geom.setRightLevelToShow(d), c.animate) this.clickedNode = d, this.compute("end"), this.clickedNode = e, this.fx.animate({
                    modes: ["linear", "node-property:width:height"],
                    duration: 1E3,
                    onComplete: function () {
                        a.clickedNode = d;
                        b.eachNode(function (a) {
                            a.setDataset(["current", "end"], {
                                alpha: [0, 1]
                            })
                        }, "ignore");
                        e.eachSubgraph(function (a) {
                            a.setData("alpha", 1)
                        }, "ignore");
                        a.fx.animate({
                            duration: 500,
                            modes: ["node-property:alpha"],
                            onComplete: function () {
                                callback.onComplete()
                            }
                        })
                    }
                });
                else callback.onComplete();
                else this.busy = !1
            }
        },
        requestNodes: function (a, c) {
            var b = h.merge(this.controller, c),
                d = this.config.levelsToShow;
            if (b.request) {
                var e = [],
                    f = a._depth;
                a.eachLevel(0, d, function (a) {
                    var b = d - (a._depth - f);
                    a.drawn && (!a.anySubnode() && 0 < b) && (e.push(a), a._level = b)
                });
                this.group.requestNodes(e, b)
            }
            else b.onComplete()
        },
        reposition: function () {
            this.compute("end")
        }
    };
    p.Op = new m({
        Implements: n.Op,
        initialize: function (a) {
            this.viz = a
        }
    });
    p.Geom = new m({
        Implements: n.Geom,
        getRightLevelToShow: function () {
            return this.viz.config.levelsToShow
        },
        setRightLevelToShow: function (a) {
            var c = this.getRightLevelToShow(),
                b = this.viz.labels;
            a.eachLevel(0, c + 1, function (d) {
                d._depth - a._depth > c ? (d.drawn = !1, d.exist = !1, d.ignore = !0, b.hideLabel(d, !1)) : (d.drawn = !0, d.exist = !0, delete d.ignore)
            });
            a.drawn = !0;
            delete a.ignore
        }
    });
    p.Group = new m({
        initialize: function (a) {
            this.viz = a;
            this.canvas = a.canvas;
            this.config = a.config
        },
        requestNodes: function (a, c) {
            var b = 0,
                d = a.length,
                e = this.viz;
            if (0 == d) c.onComplete();
            for (var f = 0; f < d; f++) c.request(a[f].id, a[f]._level, {
                onComplete: function (a, f) {
                    f && f.children && (f.id = a, e.op.sum(f, {
                        type: "nothing"
                    }));
                    ++b == d && (e.graph.computeLevels(e.root, 0), c.onComplete())
                }
            })
        }
    });
    p.Plot = new m({
        Implements: n.Plot,
        initialize: function (a) {
            this.viz = a;
            this.config = a.config;
            this.node = this.config.Node;
            this.edge = this.config.Edge;
            this.animation = new D;
            this.nodeTypes = new p.Plot.NodeTypes;
            this.edgeTypes = new p.Plot.EdgeTypes;
            this.labels = a.labels
        },
        plot: function (a, c) {
            var b = this.viz,
                d = b.graph;
            b.canvas.clear();
            this.plotTree(d.getNode(b.clickedNode && b.clickedNode.id || b.root), h.merge(b.config, a || {}, {
                withLabels: !0,
                hideLabels: !1,
                plotSubtree: function (a) {
                    return a.anySubnode("exist")
                }
            }), c)
        }
    });
    p.Label = {};
    p.Label.Native = new m({
        Implements: n.Label.Native,
        initialize: function (a) {
            this.config = a.config;
            this.leaf = a.leaf
        },
        renderLabel: function (a, c) {
            if (this.leaf(c) || this.config.titleHeight) {
                var b = c.pos.getc(!0),
                    d = a.getCtx(),
                    e = c.getData("width");
                c.getData("height");
                d.fillText(c.name, b.x + e / 2, b.y, e)
            }
        }
    });
    p.Label.SVG = new m({
        Implements: n.Label.SVG,
        initialize: function (a) {
            this.viz = a;
            this.leaf = a.leaf;
            this.config = a.config
        },
        placeLabel: function (a, c, b) {
            var d = c.pos.getc(!0),
                e = this.viz.canvas,
                f = e.translateOffsetX,
                g = e.translateOffsetY,
                h = e.scaleOffsetX,
                i = e.scaleOffsetY,
                e = e.getSize(),
                f = Math.round(d.x * h + f + e.width / 2),
                d = Math.round(d.y * i + g + e.height / 2);
            a.setAttribute("x", f);
            a.setAttribute("y", d);
            !this.leaf(c) && !this.config.titleHeight && (a.style.display = "none");
            b.onPlaceLabel(a, c)
        }
    });
    p.Label.HTML = new m({
        Implements: n.Label.HTML,
        initialize: function (a) {
            this.viz = a;
            this.leaf = a.leaf;
            this.config = a.config
        },
        placeLabel: function (a, c, b) {
            var d = c.pos.getc(!0),
                e = this.viz.canvas,
                f = e.translateOffsetX,
                g = e.translateOffsetY,
                h = e.scaleOffsetX,
                i = e.scaleOffsetY,
                e = e.getSize(),
                f = Math.round(d.x * h + f + e.width / 2),
                d = Math.round(d.y * i + g + e.height / 2),
                g = a.style;
            g.left = f + "px";
            g.top = d + "px";
            g.width = c.getData("width") * h + "px";
            g.height = c.getData("height") * i + "px";
            g.zIndex = 100 * c._depth;
            g.display = "";
            !this.leaf(c) && !this.config.titleHeight && (a.style.display = "none");
            b.onPlaceLabel(a, c)
        }
    });
    p.Plot.NodeTypes = new m({
        none: {
            render: h.empty
        },
        rectangle: {
            render: function (a, c) {
                var b = this.viz.leaf(a),
                    d = this.config,
                    e = d.offset,
                    f = d.titleHeight,
                    g = a.pos.getc(!0),
                    j = a.getData("width"),
                    i = a.getData("height"),
                    k = a.getData("border"),
                    l = c.getCtx(),
                    m = g.x + e / 2,
                    n = g.y + e / 2;
                if (!(j <= e || i <= e)) if (b) {
                    if (d.cushion && (b = l.createRadialGradient(m + (j - e) / 2, n + (i - e) / 2, 1, m + (j - e) / 2, n + (i - e) / 2, j < i ? i : j), d = a.getData("color"), f = h.rgbToHex(h.map(h.hexToRgb(d),

                    function (a) {
                        return 0.2 * a >> 0
                    })), b.addColorStop(0, d), b.addColorStop(1, f), l.fillStyle = b), l.fillRect(m, n, j - e, i - e), k) l.save(), l.strokeStyle = k, l.strokeRect(m, n, j - e, i - e), l.restore()
                }
                else 0 < f && (l.fillRect(g.x + e / 2, g.y + e / 2, j - e, f - e), k && (l.save(), l.strokeStyle = k, l.strokeRect(g.x + e / 2, g.y + e / 2, j - e, i - e), l.restore()))
            },
            contains: function (a, c) {
                if (this.viz.clickedNode && !a.isDescendantOf(this.viz.clickedNode.id) || a.ignore) return !1;
                var b = a.pos.getc(!0),
                    d = a.getData("width"),
                    e = this.viz.leaf(a) ? a.getData("height") : this.config.titleHeight;
                return this.nodeHelper.rectangle.contains({
                    x: b.x + d / 2,
                    y: b.y + e / 2
                }, c, d, e)
            }
        }
    });
    p.Plot.EdgeTypes = new m({
        none: h.empty
    });
    p.SliceAndDice = new m({
        Implements: [H, y, p.Base, v.TM.SliceAndDice]
    });
    p.Squarified = new m({
        Implements: [H, y, p.Base, v.TM.Squarified]
    });
    p.Strip = new m({
        Implements: [H, y, p.Base, v.TM.Strip]
    })
})();
