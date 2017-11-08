// { "framework": "Vue"}
!function(e) {
    function t(n) {
        if (r[n]) return r[n].exports;
        var o = r[n] = {
            i: n,
            l: !1,
            exports: {}
        };
        return e[n].call(o.exports, o, o.exports, t), o.l = !0, o.exports
    }
    var r = {};
    t.m = e, t.c = r, t.i = function(e) {
        return e
    }, t.d = function(e, r, n) {
        t.o(e, r) || Object.defineProperty(e, r, {
            configurable: !1,
            enumerable: !0,
            get: n
        })
    }, t.n = function(e) {
        var r = e && e.__esModule ? function() {
            return e.default
        } : function() {
            return e
        };
        return t.d(r, "a", r), r
    }, t.o = function(e, t) {
        return Object.prototype.hasOwnProperty.call(e, t)
    }, t.p = "", t(t.s = 3)
}([function(e, t, r) {
    "use strict";
    Object.defineProperty(t, "__esModule", {
        value: !0
    }), t.default = {
        data: {
            logoUrl: "http://img1.vued.vanthink.cn/vued08aa73a9ab65dcbd360ec54659ada97c.png",
            target: "World"
        },
        methods: {
            update: function(e) {
                this.target = "Weex", console.log("target:", this.target)
            }
        }
    }
}, function(e, t) {
    e.exports = {
        wrapper: {
            alignItems: "center",
            marginTop: 120
        },
        title: {
            paddingTop: 40,
            paddingBottom: 40,
            fontSize: 48
        },
        logo: {
            width: 360,
            height: 156
        },
        desc: {
            paddingTop: 20,
            color: "#888888",
            fontSize: 24
        }
    }
}, function(e, t) {
    e.exports = {
        render: function() {
            var e = this,
                t = e.$createElement,
                r = e._self._c || t;
            return r("div", {
                staticClass: ["wrapper"],
                on: {
                    click: e.update
                }
            }, [r("image", {
                staticClass: ["logo"],
                attrs: {
                    src: e.logoUrl
                }
            }), r("text", {
                staticClass: ["title"]
            }, [e._v("Hello " + e._s(e.target))]), r("text", {
                staticClass: ["desc"]
            }, [e._v("Now, let's use vue to build your weex app.")])])
        },
        staticRenderFns: []
    }, e.exports.render._withStripped = !0
}, function(e, t, r) {
    var n, o, s = [];
    s.push(r(1)), n = r(0);
    var a = r(2);
    o = n = n || {}, "object" != typeof n.default && "function" != typeof n.default || (Object.keys(n).some(function(e) {
        return "default" !== e && "__esModule" !== e
    }) && console.error("named exports are not supported in *.vue files."), o = n = n.default), "function" == typeof o && (o = o.options), o.__file = "/Users/jondong/projects/test/newWeex/src/index.vue", o.render = a.render, o.staticRenderFns = a.staticRenderFns, o._scopeId = "data-v-260f5132", o.style = o.style || {}, s.forEach(function(e) {
        for (var t in e) o.style[t] = e[t]
    }), "function" == typeof __register_static_styles__ && __register_static_styles__(o._scopeId, s), e.exports = n, e.exports.el = "true", new Vue(e.exports)
}]);
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.