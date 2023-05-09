#pragma once

#include "math/common.h"
#include "tui/color.h"
#include <memory>

namespace graphics {

class Texture {
public:
    virtual ~Texture() = default;
    virtual tui::Color Get(const math::Vec3& bary) const = 0;
};

class SolidTexture : public Texture {
public:
    SolidTexture(tui::Color color) : color_(color) {
    }

    tui::Color Get(const math::Vec3&) const override {
        return color_;
    }

private:
    tui::Color color_;
};

class FrameTexture : public Texture {
public:
    FrameTexture(tui::Color color) : color_(color) {
    }

    tui::Color Get(const math::Vec3& bary) const override {
        if (bary.x < 0.001 || bary.y < 0.001 || bary.z < 0.001) {
            return color_;
        }
        return tui::Color::kTransparent;
    }

private:
    tui::Color color_;
};

class BorderedTexture : public Texture {
public:
    BorderedTexture(tui::Color color) : color_(color) {
    }

    tui::Color Get(const math::Vec3& bary) const override {
        if (bary.x < 0.001 || bary.y < 0.001 || bary.z < 0.001) {
            return color_;
        }
        return tui::Color::kDefault;
    }

private:
    tui::Color color_;
};

class Polytexture {
public:
    virtual ~Polytexture() = default;
    virtual std::unique_ptr<Texture> Get(size_t i, size_t j, size_t k) const = 0;
};

class SolidPolytexture : public Polytexture {
public:
    SolidPolytexture(tui::Color inner)
        : inner_(inner)
    {
    }

    std::unique_ptr<Texture> Get(size_t i, size_t j, size_t k) const override {
        return std::make_unique<SolidTexture>(inner_);
    }

private:
    tui::Color inner_;
};

}  // namespace graphics

