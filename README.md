# Glassirum

![Превью](https://i.ibb.co/xtPh7GTK/photo-2026-11-02-22-58-00.jpg)

**Liquid Glass effect plug-in for [EffectCraft](https://github.com/effectcraft/effectcraft)** — a beveled glass slab with refraction, chromatic dispersion, frost, specular glints and tint. Written in plain C, compiled to a single WebAssembly module (~9 KB), no dependencies.

> Version **1.0.0** · plug-in API 2 manifest (Wasm ABI 1) · License **AGPL-3.0-or-later**

## Features

- **Rectangular glass slab.** The bevel follows the four edges of the layer's geometry; the interior is flat glass (pixels are copied unchanged).
- **Follows the layer.** Position, scale and size of the shape are tracked live through `sourceRectAtTime()` (see [Auto-bounds](#auto-bounds)).
- **Refraction** pulls the background inward at the bevel, like a convex lens.
- **5-sample chromatic dispersion** spreads R/G/B along the bevel direction.
- **Frost** (3-pass box blur ≈ Gaussian), computed only inside the slab.
- **Clamped specular + Fresnel rim** for a thin glossy edge instead of a burnt-out border.
- **Tint** with Multiply / Normal / Screen modes.
- **Mirror reflection** for reads at the slab edge: no streaks.
- **Preview-resolution aware**: bevel and refraction scale with the preview resolution.
- Optimised CPU path: only the slab window is processed, only the bevel strip is shaded, per-channel sampling.

## Install

1. Copy `wasm/glassirum.wasm` into the EffectCraft plug-ins folder (`%APPDATA%\EffectCraft\Plug-ins\` on Windows), or use **Effect ▸ Install Effect Plug-in…**.
2. Copy `scripts/glassirum_autobounds.jsx` into `%APPDATA%\EffectCraft\Scripts\Startup\` (it also works from **File ▸ Scripts**).
3. Restart EffectCraft. The effect appears in **Distort ▸ Glassirum**.

## Auto-bounds

Wasm plug-ins cannot call back into the host, so the slab rectangle is passed in through two point parameters, **Box Top-Left** and **Box Bottom-Right**. `glassirum_autobounds.jsx` binds them to these expressions:

```js
var r = sourceRectAtTime(time, false); toComp([r.left, r.top])
var r = sourceRectAtTime(time, false); toComp([r.left + r.width, r.top + r.height])
```

If the two parameters are `0, 0`, the plug-in falls back to the layer buffer (and, if it has transparent pixels, to its alpha bounding box).

Troubleshooting: if the glass follows the composition instead of the shape, delete the effect, add it again (an instance created by an older version lacks the two box parameters) and run the script.

## Parameters

| Parameter | Range | Default | Description |
|---|---|---|---|
| Light Angle | angle | -45° | Direction of the key light |
| Light Intensity | 0–200 | 100 | Specular glint strength |
| Refraction | -100–100 | 35 | Bevel refraction (negative inverts) |
| Depth | 0–100 | 40 | Bevel width |
| Dispersion | 0–50 | 20 | Chromatic dispersion |
| Frost | 0–100 | 0 | Blur of the glass |
| Splay | -100–100 | 0 | Rotates the refraction vector |
| Magnify, Magnify Falloff | – | 0, 1 | Reserved |
| Effect Scale | 0.1–10 | 1 | Global scale of the effect |
| Rim Scale | 0.1–20 | 1 | Bevel width multiplier |
| Tint Color / Amount / Mode | – | white / 0 / Multiply | Glass tint |
| Box Top-Left / Bottom-Right | points | 0,0 | Slab rectangle (bound by the script) |

## Build

Requires `clang` with the `wasm32` target:

```sh
clang --target=wasm32 -O3 -flto -nostdlib -Wl,--no-entry -Wl,--export-all -o wasm/glassirum.wasm main.c
```

## Notes

- Plug-ins in EffectCraft 0.22.0 run on the CPU only (single-threaded Wasm, no host imports), so there is no GPU path.
- `ec_api_version()` returns `1` (Wasm ABI), while the manifest declares `"api": 2`.

## License

Copyright (C) 2026 linuxdev ([github.com/linuxdev11](https://github.com/linuxdev11)).

Glassirum is free software licensed under the **GNU Affero General Public License v3.0 or later**. See [LICENSE](LICENSE).
<img width="1280" height="718" alt="photo_2026-11-02_22-58-00" src="https://github.com/user-attachments/assets/6c0b41ce-1a73-4483-aa86-3964327e7f26" />
<img width="1280" height="718" alt="photo_2026-11-02_22-58-00" src="https://github.com/user-attachments/assets/fa5fc00e-b876-4bc4-b57d-7d47be76ab0a" />
