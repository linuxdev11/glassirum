/*
 * Glassirum - Liquid Glass effect plug-in for EffectCraft (WebAssembly, plug-in API 2 manifest)
 *
 * Copyright (C) 2026 linuxdev (https://github.com/linuxdev11)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Geometry model: a rectangular glass slab. The bevel runs along the four edges of the
 * rectangle given by Box Top-Left / Box Bottom-Right (bound to the layer's
 * sourceRectAtTime() by scripts/glassirum_autobounds.jsx); without it the whole buffer is used.
 * The interior is flat glass (pixels copied unchanged). Reads outside the slab are mirrored.
 *
 * Build:
 *   clang --target=wasm32 -O3 -flto -nostdlib -Wl,--no-entry -Wl,--export-all -o glassirum.wasm main.c
 */

#define NULL ((void*)0)

typedef __SIZE_TYPE__ size_t;

void* memset(void* dest, int c, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    unsigned char val = (unsigned char)c;
    for (size_t i = 0; i < n; ++i) d[i] = val;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; ++i) d[i] = s[i];
    return dest;
}

static const char MANIFEST[] =
    "{\"api\":2,"
    "\"id\":\"org.effectcraft.plugin.glassirum\","
    "\"name\":\"Glassirum\","
    "\"category\":\"Distort\","
    "\"version\":\"1.0.0\","
    "\"author\":\"linuxdev (github.com/linuxdev11)\","
    "\"description\":\"True Liquid Glass plate visual effect plugin (4-edge bevel along shape/layer bounds, flat parallel center, mirror reflection).\","
    "\"params\":["
        "{\"id\":\"light_angle\",\"name\":\"Light Angle\",\"type\":\"angle\",\"default\":-45.0},"
        "{\"id\":\"light_intensity\",\"name\":\"Light Intensity\",\"type\":\"slider\",\"default\":100.0,\"min\":0.0,\"max\":200.0,\"sliderMin\":0.0,\"sliderMax\":200.0,\"decimals\":1},"
        "{\"id\":\"refraction\",\"name\":\"Refraction\",\"type\":\"slider\",\"default\":35.0,\"min\":-100.0,\"max\":100.0,\"sliderMin\":-50.0,\"sliderMax\":50.0,\"decimals\":1},"
        "{\"id\":\"depth\",\"name\":\"Depth\",\"type\":\"slider\",\"default\":40.0,\"min\":0.0,\"max\":100.0,\"sliderMin\":1.0,\"sliderMax\":100.0,\"decimals\":1},"
        "{\"id\":\"dispersion\",\"name\":\"Dispersion\",\"type\":\"slider\",\"default\":20.0,\"min\":0.0,\"max\":50.0,\"sliderMin\":0.0,\"sliderMax\":30.0,\"decimals\":1},"
        "{\"id\":\"frost\",\"name\":\"Frost\",\"type\":\"slider\",\"default\":0.0,\"min\":0.0,\"max\":100.0,\"sliderMin\":0.0,\"sliderMax\":50.0,\"decimals\":1},"
        "{\"id\":\"splay\",\"name\":\"Splay\",\"type\":\"slider\",\"default\":0.0,\"min\":-100.0,\"max\":100.0,\"sliderMin\":-45.0,\"sliderMax\":45.0,\"decimals\":1},"
        "{\"id\":\"magnify\",\"name\":\"Magnify\",\"type\":\"slider\",\"default\":0.0,\"min\":-100.0,\"max\":100.0,\"sliderMin\":-10.0,\"sliderMax\":10.0,\"decimals\":2},"
        "{\"id\":\"magnify_falloff\",\"name\":\"Magnify Falloff\",\"type\":\"slider\",\"default\":1.0,\"min\":0.0,\"max\":10.0,\"sliderMin\":0.1,\"sliderMax\":5.0,\"decimals\":2},"
        "{\"id\":\"effect_scale\",\"name\":\"Effect Scale\",\"type\":\"slider\",\"default\":1.0,\"min\":0.1,\"max\":10.0,\"sliderMin\":0.5,\"sliderMax\":3.0,\"decimals\":2},"
        "{\"id\":\"rim_scale\",\"name\":\"Rim Scale\",\"type\":\"slider\",\"default\":1.0,\"min\":0.1,\"max\":20.0,\"sliderMin\":0.5,\"sliderMax\":10.0,\"decimals\":1},"
        "{\"id\":\"tint_color\",\"name\":\"Tint Color\",\"type\":\"color\",\"default\":[1.0,1.0,1.0,1.0]},"
        "{\"id\":\"tint_amount\",\"name\":\"Tint Amount\",\"type\":\"slider\",\"default\":0.0,\"min\":0.0,\"max\":100.0,\"sliderMin\":0.0,\"sliderMax\":100.0,\"decimals\":1},"
        "{\"id\":\"tint_mode\",\"name\":\"Tint Mode\",\"type\":\"popup\",\"default\":0,\"options\":[\"Multiply\",\"Normal\",\"Screen\"]},"
        "{\"id\":\"box_tl\",\"name\":\"Box Top-Left\",\"type\":\"point\",\"default\":[0.0,0.0]},"
        "{\"id\":\"box_br\",\"name\":\"Box Bottom-Right\",\"type\":\"point\",\"default\":[0.0,0.0]}"
    "]}";

static const int MANIFEST_LEN = sizeof(MANIFEST) - 1;

__attribute__((export_name("ec_api_version")))
int ec_api_version(void) { return 1; }

__attribute__((export_name("ec_manifest_ptr")))
int ec_manifest_ptr(void) { return (int)&MANIFEST[0]; }

__attribute__((export_name("ec_manifest_len")))
int ec_manifest_len(void) { return MANIFEST_LEN; }

__attribute__((export_name("ec_pad")))
int ec_pad(int params, int nparams, double time, double scale) {
    (void)params; (void)nparams; (void)time; (void)scale;
    return 0;
}

static const int BASE_OFFSET = 65536;

__attribute__((export_name("ec_alloc")))
int ec_alloc(int bytes) {
    if (bytes <= 0) bytes = 1024;
    unsigned long long b = (unsigned long long)(unsigned int)bytes;
    unsigned long long needed = (unsigned long long)BASE_OFFSET + b * 4ULL + 65536ULL;
    if (needed > 0xFFFFFFFFULL) {
        needed = 0xFFFFFFFFULL;
    }
    size_t current_pages = (size_t)__builtin_wasm_memory_size(0);
    size_t current_bytes = current_pages * 65536ULL;
    if (needed > current_bytes) {
        size_t pages_to_grow = (needed - current_bytes + 65535ULL) / 65536ULL;
        if ((int)__builtin_wasm_memory_grow(0, (int)pages_to_grow) == -1) {
            return 0;
        }
    }
    return BASE_OFFSET;
}

static inline float clampf(float v, float min_v, float max_v) {
    return v < min_v ? min_v : (v > max_v ? max_v : v);
}

static inline float sqrtf(float v) { return __builtin_sqrtf(v); }

static inline float fast_sinf(float x) {
    const float TWO_PI = 6.283185307f;
    const float INV_TWO_PI = 0.159154943f;
    const float PI = 3.1415926535f;
    if (__builtin_isnan(x) || __builtin_isinf(x)) return 0.0f;
    float k = (float)((int)(x * INV_TWO_PI + (x >= 0.0f ? 0.5f : -0.5f)));
    x -= k * TWO_PI;
    while (x > PI) x -= TWO_PI;
    while (x < -PI) x += TWO_PI;
    float x2 = x * x;
    return x * (1.0f - x2 * (0.16666667f - x2 * 0.00833333f));
}

static inline float fast_cosf(float x) {
    return fast_sinf(x + 1.57079632f);
}

// Single mirror reflection of c into [lo, hi] (displacement is always < box span)
static inline float reflect1(float c, float lo, float hi) {
    if (c < lo) c = lo + lo - c;
    if (c > hi) c = hi + hi - c;
    return c < lo ? lo : (c > hi ? hi : c);
}

// Bilinear sampler; MR/MG/MB select which channels are fetched (compile-time constants after inlining)
static inline __attribute__((always_inline)) void samp(
    const float* src, int width, int height,
    float lo_x, float hi_x, float lo_y, float hi_y,
    float x, float y, int mr, int mg, int mb, float* o
) {
    x = reflect1(x, lo_x, hi_x);
    y = reflect1(y, lo_y, hi_y);
    int x0 = (int)x, y0 = (int)y;
    int x1 = x0 + 1, y1 = y0 + 1;
    float fx = x - (float)x0, fy = y - (float)y0;
    if (x1 >= width)  x1 = width - 1;
    if (y1 >= height) y1 = height - 1;
    const float* p00 = src + (y0 * width + x0) * 4;
    const float* p10 = src + (y0 * width + x1) * 4;
    const float* p01 = src + (y1 * width + x0) * 4;
    const float* p11 = src + (y1 * width + x1) * 4;
    float w00 = (1.0f - fx) * (1.0f - fy), w10 = fx * (1.0f - fy);
    float w01 = (1.0f - fx) * fy,          w11 = fx * fy;
    if (mr) o[0] = p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11;
    if (mg) o[1] = p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11;
    if (mb) o[2] = p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11;
}

// Separable moving-average blur restricted to the rectangle [x0..x1] x [y0..y1]
static void box_blur_region(
    const float* src, float* dst, float* temp,
    int stride_w, int x0, int x1, int y0, int y1, int r
) {
    float inv_w = 1.0f / (float)(2 * r + 1);
    for (int y = y0; y <= y1; ++y) {
        int row = y * stride_w;
        const float* s0 = src + (row + x0) * 4;
        float ar = s0[0] * (float)(r + 1), ag = s0[1] * (float)(r + 1), ab = s0[2] * (float)(r + 1);
        for (int i = 1; i <= r; ++i) {
            int tx = x0 + i; if (tx > x1) tx = x1;
            const float* s = src + (row + tx) * 4;
            ar += s[0]; ag += s[1]; ab += s[2];
        }
        for (int x = x0; x <= x1; ++x) {
            float* o = temp + (row + x) * 4;
            o[0] = ar * inv_w; o[1] = ag * inv_w; o[2] = ab * inv_w;
            int xi = x + r + 1; if (xi > x1) xi = x1;
            int xo = x - r;     if (xo < x0) xo = x0;
            const float* si = src + (row + xi) * 4;
            const float* so = src + (row + xo) * 4;
            ar += si[0] - so[0]; ag += si[1] - so[1]; ab += si[2] - so[2];
        }
    }
    for (int x = x0; x <= x1; ++x) {
        const float* t0 = temp + (y0 * stride_w + x) * 4;
        float ar = t0[0] * (float)(r + 1), ag = t0[1] * (float)(r + 1), ab = t0[2] * (float)(r + 1);
        for (int i = 1; i <= r; ++i) {
            int ty = y0 + i; if (ty > y1) ty = y1;
            const float* t = temp + (ty * stride_w + x) * 4;
            ar += t[0]; ag += t[1]; ab += t[2];
        }
        for (int y = y0; y <= y1; ++y) {
            float* o = dst + (y * stride_w + x) * 4;
            o[0] = ar * inv_w; o[1] = ag * inv_w; o[2] = ab * inv_w;
            int yi = y + r + 1; if (yi > y1) yi = y1;
            int yo = y - r;     if (yo < y0) yo = y0;
            const float* ti = temp + (yi * stride_w + x) * 4;
            const float* to = temp + (yo * stride_w + x) * 4;
            ar += ti[0] - to[0]; ag += ti[1] - to[1]; ab += ti[2] - to[2];
        }
    }
}

typedef struct {
    const float* sample_src;
    int width, height;
    float lo_x, hi_x, lo_y, hi_y;
    float inv_bw;
    float refr_mult, cos_sp, sin_sp;
    int has_splay;
    float disp_factor;
    float hx, hy, hz, spec_power;
    float tint_r, tint_g, tint_b, tint_norm;
    int tint_mode;
} Ctx;

static const float SPEC_T[5] = { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
static const float W_R[3] = { 0.50f, 0.35f, 0.15f };           // k = 0..2
static const float W_G[5] = { 0.05f, 0.25f, 0.40f, 0.25f, 0.05f };
static const float W_B[3] = { 0.15f, 0.35f, 0.50f };           // k = 2..4

static inline __attribute__((always_inline)) void apply_tint(const Ctx* c, float* r, float* g, float* b) {
    float n = c->tint_norm;
    if (c->tint_mode == 0) {
        *r *= (1.0f - n) + c->tint_r * n;
        *g *= (1.0f - n) + c->tint_g * n;
        *b *= (1.0f - n) + c->tint_b * n;
    } else if (c->tint_mode == 1) {
        *r = *r * (1.0f - n) + c->tint_r * n;
        *g = *g * (1.0f - n) + c->tint_g * n;
        *b = *b * (1.0f - n) + c->tint_b * n;
    } else {
        *r = 1.0f - (1.0f - *r) * (1.0f - c->tint_r * n);
        *g = 1.0f - (1.0f - *g) * (1.0f - c->tint_g * n);
        *b = 1.0f - (1.0f - *b) * (1.0f - c->tint_b * n);
    }
}

static inline __attribute__((always_inline)) void bevel_px(
    const Ctx* c, float* out, float fx, float fy, float dx, float dy, float dir_x, float dir_y, float d
) {
    float u = d * c->inv_bw;
    float t = 1.0f - u;
    float S = t * t * (3.0f - 2.0f * t);

    float vx, vy;
    float delta = dy - dx;
    if (delta > 1.0f)       { vx = dir_x; vy = 0.0f; }
    else if (delta < -1.0f) { vx = 0.0f;  vy = dir_y; }
    else {
        float blend = (delta + 1.0f) * 0.5f;
        float inv_l = 1.0f / sqrtf(blend * blend + (1.0f - blend) * (1.0f - blend) + 1e-6f);
        vx = dir_x * (blend * inv_l);
        vy = dir_y * ((1.0f - blend) * inv_l);
    }

    float refr_mag = c->refr_mult * S;
    float rx = vx * refr_mag, ry = vy * refr_mag;
    if (c->has_splay) {
        float a = rx * c->cos_sp - ry * c->sin_sp;
        float b = rx * c->sin_sp + ry * c->cos_sp;
        rx = a; ry = b;
    }

    float col[3] = {0.0f, 0.0f, 0.0f};
    float tmp[3];
    if (c->disp_factor < 0.001f) {
        samp(c->sample_src, c->width, c->height, c->lo_x, c->hi_x, c->lo_y, c->hi_y, fx + rx, fy + ry, 1, 1, 1, col);
    } else {
        for (int k = 0; k < 5; ++k) {
            float s = 1.0f + SPEC_T[k] * c->disp_factor;
            float sx = fx + rx * s, sy = fy + ry * s;
            if (k < 2) {
                samp(c->sample_src, c->width, c->height, c->lo_x, c->hi_x, c->lo_y, c->hi_y, sx, sy, 1, 1, 0, tmp);
                col[0] += tmp[0] * W_R[k]; col[1] += tmp[1] * W_G[k];
            } else if (k == 2) {
                samp(c->sample_src, c->width, c->height, c->lo_x, c->hi_x, c->lo_y, c->hi_y, sx, sy, 1, 1, 1, tmp);
                col[0] += tmp[0] * W_R[2]; col[1] += tmp[1] * W_G[2]; col[2] += tmp[2] * W_B[0];
            } else {
                samp(c->sample_src, c->width, c->height, c->lo_x, c->hi_x, c->lo_y, c->hi_y, sx, sy, 0, 1, 1, tmp);
                col[1] += tmp[1] * W_G[k]; col[2] += tmp[2] * W_B[k - 2];
            }
        }
    }

    if (c->spec_power > 0.001f) {
        float slope = S * 1.6f;
        float Nx = -vx * slope, Ny = -vy * slope;
        float inv_n = 1.0f / sqrtf(Nx * Nx + Ny * Ny + 1.0f);
        Nx *= inv_n; Ny *= inv_n; float Nz = inv_n;
        float n_dot_h = Nx * c->hx + Ny * c->hy + Nz * c->hz;
        float glint = 0.0f;
        if (n_dot_h > 0.0f) {
            float v2 = n_dot_h * n_dot_h, v4 = v2 * v2, v8 = v4 * v4, v16 = v8 * v8, v32 = v16 * v16;
            glint = v32 * v32 * c->spec_power * 0.75f * S;
        }
        float edge_t = t * t;
        float rim = edge_t * (1.0f - Nz) * c->spec_power * 0.35f;
        float sp = glint + rim;
        sp = sp < 0.0f ? 0.0f : (sp > 0.70f ? 0.70f : sp);
        col[0] += sp * (1.0f - col[0] * 0.4f);
        col[1] += sp * (1.0f - col[1] * 0.4f);
        col[2] += sp * (1.0f - col[2] * 0.4f);
    }

    if (c->tint_norm > 0.0f) apply_tint(c, &col[0], &col[1], &col[2]);

    out[0] = clampf(col[0], 0.0f, 1.0f);
    out[1] = clampf(col[1], 0.0f, 1.0f);
    out[2] = clampf(col[2], 0.0f, 1.0f);
}

// ============================================================================
// MAIN RENDERING ENGINE (ec_render)
// ============================================================================

__attribute__((export_name("ec_render")))
int ec_render(int pixels, int width, int height, int params, int nparams, double _time, double _scale) {
    (void)_time;
    if (pixels == 0 || width <= 0 || height <= 0) return -1;

    size_t num_px = (size_t)(unsigned int)width * (size_t)(unsigned int)height;
    size_t total_floats = num_px * 4;

    size_t required_bytes = (size_t)pixels + total_floats * sizeof(float) * 4;
    size_t current_bytes = (size_t)__builtin_wasm_memory_size(0) * 65536;
    if (required_bytes > current_bytes) {
        size_t pages_to_grow = (required_bytes - current_bytes + 65535) / 65536;
        if ((int)__builtin_wasm_memory_grow(0, (int)pages_to_grow) == -1) return -1;
    }

    float light_angle = -45.0f, light_intensity = 100.0f, refraction = 35.0f, depth = 40.0f;
    float dispersion = 20.0f, frost = 0.0f, splay = 0.0f, effect_scale = 1.0f, rim_scale = 1.0f;
    float tint_r = 1.0f, tint_g = 1.0f, tint_b = 1.0f, tint_amount = 0.0f;
    int tint_mode = 0;
    float box_tl_x = 0.0f, box_tl_y = 0.0f, box_br_x = 0.0f, box_br_y = 0.0f;

    if (params != 0 && nparams > 0) {
        const double* p = (const double*)params;
        if (nparams > 0)  light_angle     = (float)p[0];
        if (nparams > 1)  light_intensity = (float)p[1];
        if (nparams > 2)  refraction      = (float)p[2];
        if (nparams > 3)  depth           = (float)p[3];
        if (nparams > 4)  dispersion      = (float)p[4];
        if (nparams > 5)  frost           = (float)p[5];
        if (nparams > 6)  splay           = (float)p[6];
        if (nparams > 9)  effect_scale    = (float)p[9];
        if (nparams > 10) rim_scale       = (float)p[10];
        if (nparams >= 17) {
            tint_r = (float)p[11]; tint_g = (float)p[12]; tint_b = (float)p[13];
            tint_amount = (float)p[15];
            double tm = p[16];
            tint_mode = (__builtin_isnan(tm) || tm < 0.0) ? 0 : (tm > 2.0 ? 2 : (int)tm);
        }
        if (nparams >= 21) {
            box_tl_x = (float)p[17]; box_tl_y = (float)p[18];
            box_br_x = (float)p[19]; box_br_y = (float)p[20];
        }
    }

    float render_scale = (_scale > 0.01 && _scale <= 10.0) ? (float)_scale : 1.0f;

    float tint_norm = tint_amount;
    if (tint_norm > 1.0f) tint_norm *= 0.01f;
    tint_norm = clampf(tint_norm, 0.0f, 1.0f);

    float* dst         = (float*)pixels;
    float* src         = dst + total_floats;
    float* frosted_buf = src + total_floats;
    float* temp_buf    = frosted_buf + total_floats;

    // ---- Active rectangle ----
    float min_x = 0.0f, max_x = (float)(width - 1), min_y = 0.0f, max_y = (float)(height - 1);

    float ex0 = box_tl_x < box_br_x ? box_tl_x : box_br_x;
    float ex1 = box_tl_x > box_br_x ? box_tl_x : box_br_x;
    float ey0 = box_tl_y < box_br_y ? box_tl_y : box_br_y;
    float ey1 = box_tl_y > box_br_y ? box_tl_y : box_br_y;

    if ((ex1 - ex0) > 2.0f && (ey1 - ey0) > 2.0f) {
        min_x = ex0; max_x = ex1; min_y = ey0; max_y = ey1;
    } else {
        int a_min_x = width, a_max_x = -1, a_min_y = height, a_max_y = -1, has_transparent = 0;
        for (int y = 0; y < height; ++y) {
            const float* row = dst + (size_t)y * width * 4;
            for (int x = 0; x < width; ++x) {
                float a = row[x * 4 + 3];
                if (a < 0.99f) has_transparent = 1;
                if (a > 0.05f) {
                    if (x < a_min_x) a_min_x = x;
                    if (x > a_max_x) a_max_x = x;
                    if (y < a_min_y) a_min_y = y;
                    if (y > a_max_y) a_max_y = y;
                }
            }
        }
        if (has_transparent && a_max_x > a_min_x && a_max_y > a_min_y) {
            min_x = (float)a_min_x; max_x = (float)a_max_x;
            min_y = (float)a_min_y; max_y = (float)a_max_y;
        }
    }

    // Integer pixel window clipped to the buffer
    int ix0 = (int)(min_x + 0.999f); if (ix0 < 0) ix0 = 0;
    int ix1 = (int)max_x;            if (ix1 > width - 1) ix1 = width - 1;
    int iy0 = (int)(min_y + 0.999f); if (iy0 < 0) iy0 = 0;
    int iy1 = (int)max_y;            if (iy1 > height - 1) iy1 = height - 1;
    if (ix1 <= ix0 || iy1 <= iy0) return 0;

    float box_w = max_x - min_x + 1.0f;
    float box_h = max_y - min_y + 1.0f;

    // Copy only the rows we will read (window +-1 row) into src
    int cy0 = iy0 > 0 ? iy0 - 1 : 0;
    int cy1 = iy1 < height - 1 ? iy1 + 1 : height - 1;
    memcpy(src + (size_t)cy0 * width * 4, dst + (size_t)cy0 * width * 4,
           (size_t)(cy1 - cy0 + 1) * width * 4 * sizeof(float));

    int do_frost = frost > 0.1f;
    const float* sample_src = src;
    if (do_frost) {
        int total_r = (int)(frost * 0.4f * effect_scale * render_scale + 0.5f);
        if (total_r < 1) total_r = 1;
        if (total_r > 64) total_r = 64;
        int r1 = total_r / 3, r2 = (total_r + 1) / 3, r3 = (total_r + 2) / 3;
        if (r1 < 1) r1 = 1;
        if (r2 < 1) r2 = 1;
        if (r3 < 1) r3 = 1;
        box_blur_region(src, frosted_buf, temp_buf, width, ix0, ix1, iy0, iy1, r1);
        box_blur_region(frosted_buf, frosted_buf, temp_buf, width, ix0, ix1, iy0, iy1, r2);
        box_blur_region(frosted_buf, frosted_buf, temp_buf, width, ix0, ix1, iy0, iy1, r3);
        sample_src = frosted_buf;
    }

    float raw_bw = depth * 0.5f * clampf(rim_scale, 0.2f, 3.0f) * effect_scale * render_scale;
    float max_bw = (box_w < box_h ? box_w : box_h) * 0.35f;
    if (max_bw < 2.0f) max_bw = 2.0f;
    float bevel_width = clampf(raw_bw, 2.0f, max_bw);

    float rad = light_angle * 0.0174532925f;
    float lx = fast_cosf(rad), ly = fast_sinf(rad), lz = 0.65f;
    float l_len = sqrtf(lx * lx + ly * ly + lz * lz);
    lx /= l_len; ly /= l_len; lz /= l_len;
    float hx = lx, hy = ly, hz = lz + 1.0f;
    float h_len = sqrtf(hx * hx + hy * hy + hz * hz);

    Ctx c;
    c.sample_src = sample_src;
    c.width = width; c.height = height;
    c.lo_x = clampf(min_x, 0.0f, (float)(width - 1));
    c.hi_x = clampf(max_x, 0.0f, (float)(width - 1));
    c.lo_y = clampf(min_y, 0.0f, (float)(height - 1));
    c.hi_y = clampf(max_y, 0.0f, (float)(height - 1));
    c.inv_bw = 1.0f / bevel_width;
    c.refr_mult = refraction * 0.8f * effect_scale * render_scale;
    c.has_splay = splay != 0.0f;
    float ts = splay * 0.0174532925f;
    c.cos_sp = fast_cosf(ts); c.sin_sp = fast_sinf(ts);
    c.disp_factor = dispersion * 0.015f * effect_scale;
    c.hx = hx / h_len; c.hy = hy / h_len; c.hz = hz / h_len;
    c.spec_power = light_intensity * 0.01f;
    c.tint_r = tint_r; c.tint_g = tint_g; c.tint_b = tint_b;
    c.tint_norm = tint_norm; c.tint_mode = tint_mode;

    int center_pass = do_frost || tint_norm > 0.0f;

    for (int y = iy0; y <= iy1; ++y) {
        float fy = (float)y;
        float dt = fy - min_y, db = max_y - fy;
        float dy = dt < db ? dt : db;
        float dir_y = (dt < db) ? 1.0f : -1.0f;
        float* row = dst + (size_t)y * width * 4;
        const float* ssrc_row = sample_src + (size_t)y * width * 4;

        // Columns [ix0..lend] and [rstart..ix1] are bevel columns on interior rows;
        // on top/bottom strip rows the whole row is bevel.
        int lend, rstart;
        if (dy < bevel_width) {
            lend = ix1; rstart = ix1 + 1;
        } else {
            lend = (int)(min_x + bevel_width - 0.0001f);
            if (lend < ix0 - 1) lend = ix0 - 1;
            if (lend > ix1) lend = ix1;
            rstart = (int)(max_x - bevel_width) + 1;
            if (rstart <= lend) rstart = lend + 1;
            if (rstart < ix0) rstart = ix0;
        }

        for (int x = ix0; x <= lend; ++x) {
            float fx = (float)x;
            float dl = fx - min_x, dr = max_x - fx;
            float dx = dl < dr ? dl : dr;
            float d = dx < dy ? dx : dy;
            float* o = row + x * 4;
            if (d >= bevel_width) {
                if (center_pass) {
                    const float* s = ssrc_row + x * 4;
                    float r = s[0], g = s[1], b = s[2];
                    if (tint_norm > 0.0f) apply_tint(&c, &r, &g, &b);
                    o[0] = r; o[1] = g; o[2] = b;
                }
                continue;
            }
            bevel_px(&c, o, fx, fy, dx, dy, (dl < dr) ? 1.0f : -1.0f, dir_y, d);
        }

        if (rstart > lend + 1 && center_pass) {
            for (int x = lend + 1; x < rstart; ++x) {
                const float* s = ssrc_row + x * 4;
                float r = s[0], g = s[1], b = s[2];
                if (tint_norm > 0.0f) apply_tint(&c, &r, &g, &b);
                float* o = row + x * 4;
                o[0] = r; o[1] = g; o[2] = b;
            }
        }

        for (int x = rstart; x <= ix1; ++x) {
            if (x <= lend) continue;
            float fx = (float)x;
            float dl = fx - min_x, dr = max_x - fx;
            float dx = dl < dr ? dl : dr;
            float d = dx < dy ? dx : dy;
            float* o = row + x * 4;
            if (d >= bevel_width) {
                if (center_pass) {
                    const float* s = ssrc_row + x * 4;
                    float r = s[0], g = s[1], b = s[2];
                    if (tint_norm > 0.0f) apply_tint(&c, &r, &g, &b);
                    o[0] = r; o[1] = g; o[2] = b;
                }
                continue;
            }
            bevel_px(&c, o, fx, fy, dx, dy, (dl < dr) ? 1.0f : -1.0f, dir_y, d);
        }
    }

    return 0;
}
