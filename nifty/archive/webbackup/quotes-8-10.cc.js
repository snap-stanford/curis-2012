var Quotes = {};
var Colors = {};
var Ranking = [];
var TeamData = {};

// Colors are from kuler.adobe.com; "Hey Mr. Grey Rainbow" by stephenmweathers
var SetColors = ["#CC5C54", "#F69162", "#FFFFCD", "#85A562", "#7AB5DB"];

function init() {
    function v(a) {
        0 === i && (i = 1 + a, n = this, r(a))
    }
    function j(a) {
        j.show = a;
        u.style.display = a ? "" : "none";
        c.style.display = a ? "" : "none";
        w[a ? "addClass" : "removeClass"]("filtered");
        a && (c.style.top = (window.getHeight() - c.offsetHeight) / 2 + "px", c.style.left = (window.getWidth() - c.offsetWidth) / 2 + "px")
    }
    function x(a, c, h) {
        var b = $("goals"),
            g = 0,
            f = 0,
            d;
        b || (b = window.G_vmlCanvasManager ? G_vmlCanvasManager.initElement(document.body.appendChild(document.createElement("canvas"))) : document.createElement("canvas"), b.id = "goals",
        setTimeout(function () {
            $("goals-canvas-wrapper").appendChild(b)
        }, 10));
        b.width = 900;
        b.height = 300;
        d = b.getContext("2d");
        d.clearRect(0, 0, 900, 300);
        d.font = "bold 12px 'Helvetica Neue'";
        d.fillStyle = d.strokeStyle = "#555";
        h.each(function (e) {
            var b = (e.minute < 48 ? e.minute + (e.offset || 0) : e.minute + (e.offset || 0) + 17) / 120 * 900;
            d.beginPath();
            if (e.country == a) {
                d.moveTo(b, 150);
                d.lineTo(b, 0);
                g++
            } else {
                d.moveTo(b, 150);
                d.lineTo(b, 300);
                f++
            }
            d.stroke();
            d.closePath();
            e.country == a ? d.fillText("'" + e.minute + " " + e.player, b + 5, 12 * g) : d.fillText("'" + e.minute + " " + e.player, b + 5, 300 - 12 * (f - 1))
        })
    }
    function r(a) {
        var l = n.id,
            h = TeamData[l].matches[a],
            b = h.result,
            g = h.against,
            f = $(l + "_name"),
            d = $(g + "_name"),
            e = c.getElement(".team1.name"),
            k = c.getElement(".team2.name"),
            a = n.getElements(".match")[a],
            o = [f.innerHTML, d.innerHTML];
        e.innerHTML = b[0] + " - " + o[0];
        k.innerHTML = b[1] + " - " + o[1];
        (new Request.JSON({
            url: "data/matches/" + a.id + ".json",
            method: "get",
            onSuccess: function (a) {
                var b = {
                    label: [o[0], o[1]],
                    color: [TeamData[l].color, TeamData[g].color],
                    values: a
                }, e = 0,
                    d = c.getElement(".name-wrapper.team1"),
                    f = c.getElement(".name-wrapper.team2");
                d.addEvent("click", function () {
                    c.chart.filter(b.label[0])
                });
                f.addEvent("click", function () {
                    c.chart.filter(b.label[1])
                });
                $$("#main-panel .name-wrapper .color").each(function (a) {
                    a.style.background = b.color[e++]
                });
                if (c.chart) e = 0, c.chart.delegate.graph.eachNode(function (b) {
                    0 < e && (b.name = a[e - 1].label, b.getData("next") && (a[e] ? b.setData("next", a[e].label) : b.setData("next", !1)));
                    e++
                }), c.chart.updateJSON(b);
                else {
                    var k = new $jit.AreaChart({
                        injectInto: "infovis",
                        width: 900,
                        height: 300,
                        animate: !0,
                        Margin: {
                            top: 0,
                            left: 0,
                            right: 0,
                            bottom: 0
                        },
                        Events: {
                            enable: !0,
                            onClick: function (a) {
                                a || k.restore()
                            }
                        },
                        showAggregates: !1,
                        showLabels: !1,
                        type: "stacked",
                        Label: {
                            type: "Native",
                            size: 8,
                            family: "Arial",
                            color: "black"
                        },
                        Tips: {
                            enable: !0,
                            onShow: function (a, b) {
                                var e = +b.value;
                                1E6 <= e ? e = Math.round(e / 1E5) / 10 + "M" : 1E3 <= e && (e = Math.round(e / 100) / 10 + "K");
                                a.innerHTML = "<b>" + e + "</b> tweets for <b>" + b.name + "</b>"
                            }
                        },
                        filterOnClick: !1,
                        restoreOnRightClick: !1
                    });
                    k.loadJSON(b);
                    c.canvas = k.canvas.canvases[0].canvas;
                    c.chart = k
                }
                d = {};
                d[l] = b.color[0];
                d[h.against] = b.color[1];
                x(l, h.against, h.goals, d);
                j(!0);
                c.chart.canvas.getPos(!0)
            }
        })).send();
        s()
    }
    function s() {
        var a = n.getElements(".match").length,
            c = i;
        c == a ? p.addClass("disabled") : p.removeClass("disabled");
        1 == c ? q.addClass("disabled") : q.removeClass("disabled");
        1 <= c ? m.removeClass("hidden") : m.addClass("hidden")
    }


    var u = $("background"),
        c = $("main-panel");
    $$("div.row");
    $$("div.country-name");
    var m = $$(".navigation")[0],
        w = $("container"),
        q = m.getElement(".previous"),
        p = m.getElement(".next"),
        t = m.getElement(".back"),
        i = 0,
        n;
    u.addEvent("click", function (a) {
        t.fireEvent("click", a)
    });
    t.addEvent("click", function () {
        this.hasClass("disabled") || (j(!1), i = 0)
    }, !1);
    q.addEvent("click", function () {
        !this.hasClass("disabled") && !c.chart.busy && (i--, r(i - 1), s())
    }, !1);
    p.addEvent("click", function () {
        !this.hasClass("disabled") && !c.chart.busy && (i++, r(i - 1), s())
    }, !1);
    window.addEvent("resize", function () {
        j.show && j(!0)
    });
    window.addEvent("keyup", function (a) {
        if (j.show) switch (a.key) {
        case "right":
            p.fireEvent("click", a);
            break;
        case "left":
            q.fireEvent("click",
            a);
            break;
        case "esc":
            t.fireEvent("click", a)
        }
    });
    $$(".row-wrapper > .color").each(function (a) {
        a.style.background = TeamData[a.getParent().getElement(".country-name").id.split("_")[0]].color
    });
    /*(function () {
        for (var a in TeamData) $(a).innerHTML = TeamData[a].matches.map(function (a, c) {
            var b = c.date + "_" + a + "_" + c.against,
                g = c.result,
                g = g[0] - g[1],
                g = 0 == g ? "Tie" : 0 < g ? "Win" : "Loss";
            return "<div id='" + b + "' class='match'><div class='image-match' style='background-image:url(https://euro2012.twitter.com/data/matches/" + b + "/match.png)'></div><div class='against-name'><span class='" + g.toLowerCase() + "'>" + g + "</span> vs. " + $(c.against + "_name").innerHTML + "</div></div>"
        }.bind(this, a)).join("")
    })();*/
    $$(".match").addEvent("click", function () {
        v.call(this.getParent(), this.getParent().getElements(".match").indexOf(this))
    });
    (function () {
        var a, c = $$(".stream-nav a");
        $("all-top-quotes").addEvent("click", function (b) {
            b.stop();
            a.busy || (c.removeClass("selected"), this.addClass("selected"), a.filter(Ranking, {
                onComplete: function () {
                    a.setupLabels()
                }
            }))
        });
        $("top-20-quotes").addEvent("click",

        function (b) {
            a.busy || (b.stop(), c.removeClass("selected"), this.addClass("selected"), filteredQuotes = Ranking.filter(function (a) {
                return Ranking.indexOf(a) > -1 && Ranking.indexOf(a) < 20
            }), a.filter(filteredQuotes, {
                onComplete: function () {
                    a.setupLabels()
                }
            }))
        });
        $("top-10-quotes").addEvent("click", function (b) {
            a.busy || (b.stop(), c.removeClass("selected"), this.addClass("selected"), filteredQuotes = Ranking.filter(function (a) {
                return Ranking.indexOf(a) > -1 && Ranking.indexOf(a) < 10
            }), a.filter(filteredQuotes, {
                onComplete: function () {
                    a.setupLabels()
                }
            }))
        });
        var h = new Fx.Scroll(window, {
            onComplete: function () {
                $$(".row-wrapper.selected").removeClass("selected")
            }
        });
        $$(".row-wrapper > a").addEvent("click", function (a) {
            a.stop();
            h.toElement($("container"))
        });
        (new Request.JSON({
            //url: "data/match-summary.json",
            url: "data/clusterinfo-2012-07-07.json",
            method: "get",
            onSuccess: function (b) {
                var quotes = b.quote;
                var labels = b.label;
                for (var i = 0; i < labels.length; ++i) {
                  Quotes[labels[i]] = quotes[i];
                  Ranking.push(labels[i]);
                }
                b.color = b.label.map(function (a) {
                    //var color = Math.floor(Math.random() * 16777216).toString(16).toUpperCase();
                    //Colors[a] = '#000000'.slice(0, -color.length) + color;
                    return SetColors[Ranking.indexOf(a) % 5]
                });
                a = new $jit.StreamChart({
                    injectInto: "stream-viz",
                    type: window.G_vmlCanvasManager ? "stacked" : "smooth",
                    animate: !1,
                    Margin: {
                        top: 5,
                        left: 0,
                        right: 0,
                        bottom: 5
                    },
                    labelOffset: 0,
                    showAggregates: !1,
                    showLabels: !1,
                    Tips: {
                        enable: !0,
                        onShow: function (a, b, c) {
                            //var c = c.name.slice(0, 10).split("-"),
                            var c = c.name,
                                d = +b.value;
                            1E6 <= d ? d = Math.round(d / 1E5) / 10 + "M" : 1E3 <= d && (d = Math.round(d / 100) / 10 + "K");
                            //a.innerHTML = "<b>" + d + "</b> mentions of <b>\"" + Quotes[b.name.trim()] + "\"</b> on " + [c[1], c[2], c[0]].join("/")
                            a.innerHTML = "<b>" + d + "</b> mentions of <b>\"" + Quotes[b.name] + "\"</b> on " + c;
                        }
                    },
                    Events: {
                        enable: !0,
                        onClick: function (a) {
                            a && (a = $(a.name.trim()).getParent(), a.addClass("selected"), h.toElement(a));
                        }
                    },
                    filterOnClick: !1,
                    restoreOnRightClick: !1,
                    selectOnHover: !1
                });
                a.loadJSON(b);
                a.canvas.getPos = function () {
                    for (var a = this.getElement(), b = 0, c = 0; a && !/^(?:html)$/i.test(a.tagName);) b += a.offsetLeft, c += a.offsetTop, a = a.offsetParent;
                    return this.pos = {
                        x: b,
                        y: c
                    }
                };
                a.config.animate = !0;
                a.filter(Ranking, {
                    onComplete: function () {
                        a.setupLabels()
                    }
                });
                var c = $("background-stream-viz");
                if (!c) {
                    if (window.G_vmlCanvasManager) return;
                    c = document.createElement("canvas");
                    c.id = "background-stream-viz";
                    $(c).inject("stream-viz", "top")
                }
                c.width = 900;
                c.height = 480;
                var f = c.getContext("2d"),
                    b = b.values,
                    d = 900 / (b.length - 1);
                f.lineWidth = 0.2;
                f.strokeStyle = "#ddd";
                f.fillStyle = "#bbb";
                b.each(function (a, b) {
                    f.beginPath();
                    f.moveTo(d * b, 0);
                    f.lineTo(d * b, 460);
                    f.closePath();
                    f.stroke();
                    var c = a.label.split(" ")[0].split("-"),
                        c = c[1] + "/" + c[2];
                    f.fillText(c, d * b + 5, 460)
                })
            }
        })).send()
    })()
}
window.addEvent("domready", init);
