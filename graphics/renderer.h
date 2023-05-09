#pragma once

#include "graphics/texture.h"
#include "math/common.h"
#include "tui/color.h"

#include <map>
#include <cassert>
#include <iostream>
#include <ostream>
#include <set>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace graphics {

class Renderer {
public:
    struct Pixel {
        float z = 1000.0;
        tui::Color color = tui::Color::kDefault;

        bool Defined() const {
            return z < 999.0 && color != tui::Color::kDefault;
        }
    };

    Renderer(int h, int w)
        : h_(h)
        , w_(w)
        , pixels_(h, std::vector<Pixel>(w))
    {
        assert(h_ > 0 && w_ > 0);
    }

    int Width() const {
        return w_;
    }
 
    int Height() const {
        return h_;
    }

    const Pixel& Get(int y, int x) const {
        return pixels_.at(y).at(x);
    }

    void DrawDot(const math::Vec4& dot, tui::Color color) {
        auto pos = Round(Remap(dot));
        Set(pos, color);
    }

    void DrawSegment(math::Vec4 a, math::Vec4 b, tui::Color color) {
        auto points = DumpSegmentPoints(a, b);
        for (const auto pt : points) {
            Set(pt, color);
        }
    }

    void DrawTriangle(math::Vec4 a, math::Vec4 b, math::Vec4 c) {
        DrawTriangle(a, b, c, SolidTexture(tui::Color::kWhite), {});
    }

    void DrawPolygon(std::vector<math::Vec4> verts, tui::Color outer, tui::Color fill = tui::Color::kDefault) {
        DrawPolygon(std::move(verts), outer, SolidPolytexture(fill));
    }

    void DrawPolygon(std::vector<math::Vec4> verts, tui::Color outer, const Polytexture& polytexture) {
        std::set<std::pair<int, int>> boundary;
        for (size_t i = 0; i < verts.size(); ++i) {
            size_t j = i + 1;
            if (j == verts.size()) {
                j = 0;
            }
            for (const auto& pt : DumpSegmentPoints(verts[i], verts[j])) {
                boundary.emplace(pt.y, pt.x);
                Set(pt, outer);
            }
        }

        for (size_t i = 1; i + 1 < verts.size(); ++i) {
            DrawTriangle(verts[0], verts[i], verts[i + 1], *polytexture.Get(0, i, i + 1), boundary);
        }
    }

    void DrawTriangle(math::Vec4 va, math::Vec4 vb, math::Vec4 vc, const Texture& texture) {
        int min_x = std::min({va.x, vb.x, vc.x});
        int min_y = std::min({va.y, vb.y, vc.y});
        int max_x = std::max({va.x, vb.x, vc.x});
        int max_y = std::max({va.y, vb.y, vc.y});

        std::set<std::pair<int, int>> boundary;
        for (const auto& pt : DumpSegmentPoints(va, vb)) {
            boundary.emplace(pt.y, pt.x);
        }
        for (const auto& pt : DumpSegmentPoints(vc, va)) {
            boundary.emplace(pt.y, pt.x);
        }
        for (const auto& pt : DumpSegmentPoints(vb, vc)) {
            boundary.emplace(pt.y, pt.x);
        }

        DrawTriangle(va, vb, vc, texture, boundary);
    }

    class BaryTexture : public Texture {
    public:
        BaryTexture(const Texture& slave, math::Vec3 bary_a, math::Vec3 bary_b, math::Vec3 bary_c)
            : slave_(slave)
            , bary_a_(bary_a)
            , bary_b_(bary_b)
            , bary_c_(bary_c)
        {
        }

        tui::Color Get(const math::Vec3& bary) const override {
            return slave_.Get(bary.x * bary_a_ + bary.y * bary_b_ + bary.z * bary_c_);
        }

    private:
        const Texture& slave_;
        math::Vec3 bary_a_;
        math::Vec3 bary_b_;
        math::Vec3 bary_c_;
    };

#define piece(a, b, c, text) \
        if (a.w < 1e-5) {\
            if (b.w < c.w) {\
                float ratio = (2 * 1e-5 - a.w) / (c.w - a.w);\
                auto p = math::Blend(a, c, ratio);\
                assert(p.w > 1e-5);\
                DrawTriangle(b, p, c, BaryTexture(text, {0, 1, 0}, {1 - ratio, 0, ratio}, {0, 0, 1}), forbidden);\
                DrawTriangle(b, p, a, BaryTexture(text, {0, 1, 0}, {1 - ratio, 0, ratio}, {1, 0, 0}), forbidden);\
                return;\
            } else {\
                float ratio = (2 * 1e-5 - a.w) / (b.w - a.w);\
                auto p = math::Blend(a, b, (2 * 1e-5 - a.w) / (b.w - a.w));\
                assert(p.w > 1e-5);\
                DrawTriangle(c, p, b, BaryTexture(text, {0, 0, 1}, {1 - ratio, ratio, 0}, {0, 1, 0}), forbidden);\
                DrawTriangle(c, p, a, BaryTexture(text, {0, 0, 1}, {1 - ratio, ratio, 0}, {1, 0, 0}), forbidden);\
                return;\
            }\
        }

    void DrawTriangle(math::Vec4 va, math::Vec4 vb, math::Vec4 vc, const Texture& texture,
            const std::set<std::pair<int, int>>& forbidden) {
        if (va.w < 1e-4 && vb.w < 1e-4 && vc.w < 1e-4) {
            return;
        }
        piece(va, vb, vc, texture);
        piece(vb, vc, va, BaryTexture(texture, {0, 1, 0}, {0, 0, 1}, {1, 0, 0}));
        piece(vc, va, vb, BaryTexture(texture, {0, 0, 1}, {1, 0, 0}, {0, 1, 0}));
        assert(va.w > 0 && vb.w > 0 && vc.w > 0);
        std::set<std::pair<int, int>> boundary;
        for (const auto& pt : DumpSegmentPoints(va, vb)) {
            boundary.emplace(pt.y, pt.x);
        }
        for (const auto& pt : DumpSegmentPoints(vc, va)) {
            boundary.emplace(pt.y, pt.x);
        }
        for (const auto& pt : DumpSegmentPoints(vb, vc)) {
            boundary.emplace(pt.y, pt.x);
        }
        if (boundary.empty()) {
            return;
        }
        va = Remap(va);
        vb = Remap(vb);
        vc = Remap(vc);
        int min_y = boundary.begin()->first;
        int max_y = boundary.rbegin()->first;

        auto area = [](const math::Vec4& a, const math::Vec4& b, const math::Vec4& c) {
            math::Vec2 d1 = {static_cast<float>(a.x - b.x), static_cast<float>(a.y - b.y)};
            math::Vec2 d2 = {static_cast<float>(c.x - b.x), static_cast<float>(c.y - b.y)};
            return d1.x * d2.y - d1.y * d2.x;
        };
        auto point_inside = [&](const math::Vec4& a, const math::Vec4& b, const math::Vec4& c, const math::Vec4& pt) {
                auto abc_area = area(va, vb, vc);
                return area(pt, vb, vc) / abc_area > 0 && area(va, pt, vc) / abc_area > 0 && area(va, vb, pt) / abc_area > 0;
        };
        auto abc_area = area(va, vb, vc);
        for (int y = min_y; y <= max_y; ++y) {
            int min_x = -500;
            int max_x = -500;
            auto lower_iter = boundary.lower_bound({y, -1});
            if (lower_iter == boundary.end() || lower_iter->first != y) {
                min_x = 0;
                max_x = w_ - 1;
            } else {
                auto iter = (--boundary.lower_bound({y + 1, -1}));
                bool s_podvohom = false;
                for (int i = lower_iter->second; i < iter->second; ++i) {
                    if (!boundary.contains({lower_iter->first, i})) {
                        s_podvohom = true;
                        break;
                    }
                }
                if (!s_podvohom) {
                    if (point_inside(va, vb, vc, {1.0, static_cast<float>(y)})) {
                        min_x = 0;
                        max_x = lower_iter->second;
                    } else if (point_inside(va, vb, vc, {static_cast<float>(w_ - 1.0), static_cast<float>(y)})) {
                        min_x = lower_iter->second;
                        max_x = w_ - 1;
                    } else {
                        // assert(false);
                        min_x = max_x = lower_iter->second;
                        // min_x = max_x = w_ - 5;
                    }
                } else {
                    assert(iter->first == y && iter->second != lower_iter->second);
                    min_x = lower_iter->second;
                    max_x = iter->second;
                    assert(min_x < max_x);
                    Set({w_ - 10, y, 0}, tui::Color::kGreen);
                }
            }
            assert(min_x != -500 && max_x != -500);
            /*
            assert(lower_iter->first == y);
            auto lower_x = std::max(0, lower_iter->second);
            auto iter = (--boundary.lower_bound({y + 1, -1}));
            assert(iter->first == y);
            auto upper_x = std::min(w_ - 1, iter->second);
            */

            for (int x = min_x; x <= max_x; ++x) {
                assert(x >= -10 && x <= w_ + 10);
                if (forbidden.contains({y, x})) {
                    continue;
                }
                float fx = x;
                float fy = y;
                math::Vec3 bary{
                    .x = area({fx, fy}, vb, vc) / abc_area,
                    .y = area(va, {fx, fy}, vc) / abc_area,
                    .z = area(va, vb, {fx, fy}) / abc_area,
                };
                auto z = bary.x * va.z + bary.y * vb.z + bary.z * vc.z;
                auto w = bary.x * va.w + bary.y * vb.w + bary.z * vc.w;
                Set({x, y, z, w}, texture.Get(bary));
            }
        }
    }

    std::vector<std::vector<tui::Color>> BuildDownsampledColormap(int ys, int xs) const {
        assert(xs > 0 && ys > 0);
        assert(w_ % xs == 0 && h_ % ys == 0);

        std::vector<std::vector<tui::Color>> result(h_ / ys, std::vector<tui::Color>(w_ / xs, tui::Color::kDefault));
        for (int i = 0; i < result.size(); ++i) {
            for (int j = 0; j < result[0].size(); ++j) {
                /*
                std::unordered_map<tui::Color, int> occs;
                for (int di = 0; di < ys; ++di) {
                    for (int dj = 0; dj < xs; ++dj) {
                        if (Get(i * ys + di, j * xs + dj).Defined()) {
                            ++occs[Get(i * ys + di, j * xs + dj).color];
                        }
                    }
                }
                auto max = 0;
                auto argmax = tui::Color::kDefault;
                for (const auto& [k, v] : occs) {
                    if (v > max || (v == max && argmax < k)) {
                        argmax = k;
                        max = v;
                    }
                }
                result[i][j] = argmax;
                */
                auto min = std::numeric_limits<double>::max();
                auto closest = tui::Color::kDefault;
                for (int di = 0; di < ys; ++di) {
                    for (int dj = 0; dj < xs; ++dj) {
                        const auto& pixel = Get(i * ys + di, j * xs + dj);
                        if (pixel.Defined() && pixel.z < min) {
                            closest = pixel.color;
                            min = pixel.z;
                        }
                    }
                }
                result[i][j] = closest;
            }
        }
        return result;
    }

private:
    struct Pos {
        int x;
        int y;
        float z;
        float w;
    };

    void Set(Pos pos, tui::Color color) {
        if (color == tui::Color::kTransparent) {
            return;
        }
        if (pos.z < -pos.w || pos.z > pos.w) {
            return;
        }
        if (!(pos.x >= 0 && pos.x < w_ && pos.y >= 0 && pos.y < h_)) {
            return;
        }
        pos.z /= pos.w;
        auto& pt = pixels_.at(pos.y).at(pos.x);
        if (pos.z < pt.z || (pos.z - 1e-5 < pt.z && pt.color == tui::Color::kDefault)) {
            pt.z = pos.z;
            pt.color = color;
        }
    }

    Pos Round(math::Vec4 floats) {
        return {
            .x = static_cast<int>(round(floats.x)),
            .y = static_cast<int>(round(floats.y)),
            .z = floats.z,
            .w = floats.w,
        };
    }

    math::Vec4 Remap(math::Vec4 floats) {
        return {
            .x = std::round((floats.x / floats.w + 1) / 2 * w_),
            .y = std::round((floats.y / floats.w + 1) / 2 * h_),
            .z = floats.z,
            .w = floats.w,
        };
    }

    std::vector<Pos> DumpSegmentPoints(math::Vec4 a, math::Vec4 b) {
        if (a.w < 1e-5 && b.w < 1e-5) {
            return {};
        }
        if (a.w < 1e-5) {
            a = math::Blend(a, b, (1e-5 - a.w) / (b.w - a.w));
        }
        if (b.w < 1e-5) {
            b = math::Blend(b, a, (1e-5 - b.w) / (a.w - b.w));
        }
        a = Remap(a);
        b = Remap(b);
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (a.x < 0) {
            a = math::Blend(a, b, -a.x / (b.x - a.x));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (a.y < 0) {
            a = math::Blend(a, b, -a.y / (b.y - a.y));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (b.x < 0) {
            b = math::Blend(b, a, -b.x / (-b.x + a.x));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (b.y < 0) {
            b = math::Blend(b, a, -b.y / (-b.y + a.y));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (a.x > w_) {
            a = math::Blend(a, b, (w_ - a.x) / (b.x - a.x));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (b.x > w_) {
            b = math::Blend(b, a, (w_ - b.x) / (a.x - b.x));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (a.y > h_) {
            a = math::Blend(a, b, (h_ - a.y) / (b.y - a.y));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }
        if (b.y > h_) {
            b = math::Blend(b, a, (h_ - b.y) / (a.y - b.y));
        }
        if (a.x < 0 && b.x < 0 || a.y < 0 && b.y < 0 || a.x > w_ && b.x > w_ || a.y > h_ && b.y > h_) {
            return {};
        }

        std::vector<Pos> result;
        if (std::abs(a.x - b.x) > std::abs(a.y - b.y)) {
            int min = round(std::min(a.x, b.x));
            int max = round(std::max(a.x, b.x));
            if (a.x > b.x) {
                std::swap(a, b);
            }
            if (std::abs(min - max) < 1e-5) {
                return {Round(a)};
            }
            for (int x = min; x <= max; ++x) {
                auto ratio = (float) (x - min) / (max - min);
                auto pos = Round(math::Blend(a, b, ratio));
                if (std::abs(pos.x) + std::abs(pos.y) > 1'000) {
                    // assert(false);
                }
                result.push_back(pos);
            }
        } else {
            int min = round(std::min(a.y, b.y));
            int max = round(std::max(a.y, b.y));
            if (a.y > b.y) {
                std::swap(a, b);
            }
            if (std::abs(min - max) < 1e-5) {
                return {Round(a)};
            }
            for (int y = min; y <= max; ++y) {
                auto ratio = (float) (y - min) / (max - min);
                auto pos = Round(math::Blend(a, b, ratio));
                if (std::abs(pos.x) + std::abs(pos.y) > 1'000) {
                    // assert(false);
                }
                result.push_back(pos);
            }
        }
        return result;
    }

    int h_;
    int w_;

    std::vector<std::vector<Pixel>> pixels_;
};

}  // namespace graphics

