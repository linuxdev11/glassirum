// Glassirum auto-bounds script for EffectCraft
// Copyright (C) 2026 linuxdev (https://github.com/linuxdev11)
// SPDX-License-Identifier: AGPL-3.0-or-later
// Licensed under the GNU Affero General Public License v3.0 or later, see LICENSE.
// Auto-binds Glassirum's Box Top-Left & Box Bottom-Right parameters
// to the layer's exact geometry via sourceRectAtTime(time, false) + toComp().
(function () {
    var EXPR_TL = "var r = sourceRectAtTime(time, false); toComp([r.left, r.top])";
    var EXPR_BR = "var r = sourceRectAtTime(time, false); toComp([r.left + r.width, r.top + r.height])";

    globalThis.__glassirumSyncBounds = function () {
        try {
            var comp = app.project ? app.project.activeItem : null;
            if (!comp || !(comp instanceof CompItem)) return;
            for (var i = 1; i <= comp.numLayers; i++) {
                var l = comp.layer(i);
                var fxGroup = l.property("ADBE Effect Parade");
                if (!fxGroup) continue;
                for (var j = 1; j <= fxGroup.numProperties; j++) {
                    var eff = fxGroup.property(j);
                    if (!eff || eff.matchName !== "org.effectcraft.plugin.glassirum") continue;

                    // Upgrade legacy 14-param instance to 16-param instance automatically
                    if (eff.numProperties < 16) {
                        var saved = [];
                        for (var k = 1; k <= eff.numProperties; k++) {
                            saved.push(eff.property(k).value);
                        }
                        var wasEnabled = eff.enabled;
                        var newEff = fxGroup.addProperty("org.effectcraft.plugin.glassirum");
                        if (newEff && newEff.numProperties >= 16) {
                            for (var k = 0; k < saved.length; k++) {
                                try { newEff.property(k + 1).setValue(saved[k]); } catch (e) {}
                            }
                            newEff.enabled = wasEnabled;
                            eff.remove();
                            eff = newEff;
                        }
                    }

                    var pTL = eff.property("Box Top-Left") || eff.property(15);
                    var pBR = eff.property("Box Bottom-Right") || eff.property(16);
                    if (pTL && pTL.expression !== EXPR_TL) {
                        pTL.expression = EXPR_TL;
                    }
                    if (pBR && pBR.expression !== EXPR_BR) {
                        pBR.expression = EXPR_BR;
                    }
                }
            }
        } catch (e) {}
    };

    __glassirumSyncBounds();
    app.scheduleTask("__glassirumSyncBounds()", 250, true);
})();
