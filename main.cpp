#include "braille/canvas.h"
#include "graphics/renderer.h"
#include "input/input.h"
#include "tui/plates.h"
#include "tui/utils.h"
#include "tui/view_port.h"
#include "math/3d.h"

#include <cstring>
#include <complex>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "fcntl.h"
#include "tui/utf8.h"

uint8_t happy[] = { 0xe2, 0x98, 0xba };  /* U+263A */

float RandomFloat() {
    return (rand() % 100 - 50) / 50.0;
}

auto mvp = math::Perspective(M_PI / 6, 1, 0.1, 100.0) * math::LookAt({static_cast<float>(0.8), 20.7, 0.8}, {.0, 20.05, .05}, {0.0, 1.0, 0.0});

void Draw(graphics::Renderer& renderer, math::Vec4 first, math::Vec4 second, math::Vec4 third, tui::Color color) {
    auto f = (mvp * first);
    auto s = (mvp * second);
    auto t = (mvp * third);
    renderer.DrawTriangle(f, s, t, graphics::SolidTexture(tui::Color::kWhite));
}

void Draw(graphics::Renderer& renderer, math::Vec4 first, math::Vec4 second, tui::Color color) {
    auto f = (mvp * first);
    auto s = (mvp * second);
    // renderer.DrawSegment(f, s, static_cast<tui::Color>(rand() % (52 - 46) + 46));
    renderer.DrawSegment(f, s, tui::Color::kWhite);
}

class SquarePolytexture : public graphics::Polytexture {
public:
    SquarePolytexture(tui::Color inner)
        : inner_(inner)
    {
    }

    std::unique_ptr<graphics::Texture> Get(size_t i, size_t j, size_t k) const override {
        math::Vec3 bounds{0.2, 0.2, -0.1};
        if ((i ^ j) % 2) {
            std::swap(j, k);
            std::swap(bounds.y, bounds.z);
        }
        if ((i ^ j) % 2) {
            std::swap(i, k);
            std::swap(bounds.x, bounds.z);
        }
        return std::make_unique<TriTexture>(inner_, bounds);
    }

private:
    class TriTexture : public graphics::Texture {
    public:
        TriTexture(tui::Color inner, math::Vec3 bounds)
            : inner_(inner)
            , bounds_(bounds)
        {
        }

        tui::Color Get(const math::Vec3& bary) const override {
            return bary.x > bounds_.x && bary.y > bounds_.y && bary.z > bounds_.z ? inner_ : tui::Color::kDefault;
            // return tui::Color::kDefault;
        }

    private:
        tui::Color inner_;
        math::Vec3 bounds_;
    };

    tui::Color inner_;
};

void DrawPolygon(graphics::Renderer& renderer, const std::vector<math::Vec4>& points, const math::Mat4& mvp, tui::Color outer, const graphics::Polytexture& texture) {
    std::vector<math::Vec4> input;
    for (auto& pt : points) {
        input.push_back((mvp * pt));
    }
    renderer.DrawPolygon(input, outer, texture);
}

void TransferToCanvas(const graphics::Renderer& renderer, braille::Canvas& canvas) {
    assert(renderer.Width() == 2 * canvas.Width());
    assert(renderer.Height() == 4 * canvas.Height());

    for (int i = 0; i < renderer.Height(); ++i) {
        for (int j = 0; j < renderer.Width(); ++j) {
            if (renderer.Get(i, j).Defined()) {
                canvas.Set(i, j, renderer.Get(i, j).Defined());
            }
        }
    }

    auto colormap = renderer.BuildDownsampledColormap(4, 2);
    for (int i = 0; i < colormap.size(); ++i) {
        for (int j = 0; j < colormap.at(0).size(); ++j) {
            canvas.SetColor(i, j, colormap.at(i).at(j));
        }
    }
}

class ExampleTexture : public graphics::Polytexture {
public:
    std::unique_ptr<graphics::Texture> Get(size_t i, size_t j, size_t k) const override {
        auto get = [](size_t i) -> math::Vec2 {
            if (i == 0) {
                return {1., 1.};
            }
            if (i == 1) {
                return {1., -1.};
            }
            if (i == 2) {
                return {-1., -1.};
            }
            if (i == 3) {
                return {-1., 1.};
            }
            assert(false);
        };
        return std::make_unique<Subtexture>(get(i), get(j), get(k));
    };

private:
    struct Subtexture : public graphics::Texture {
        Subtexture(math::Vec2 a, math::Vec2 b, math::Vec2 c)
            : a(a)
            , c(c)
            , b(b)
        {
        }

        tui::Color Get(const math::Vec3& bary) const override {
            /*
            std::wcerr << a.x << '\t' << b.x << '\t' << c.x << std::endl;
            std::wcerr << a.y << '\t' << b.y << '\t' << c.y << std::endl;
            std::wcerr << bary.x << '\t' << bary.y << '\t' << bary.z << std::endl;
            auto res = a * bary.x + b * bary.y + c * bary.z;
            std::wcerr << res.x << '\t' << res.y << std::endl;
            std::wcerr << "--------------------\n";
            */
            auto posx = (a.x * bary.x + b.x * bary.y + c.x * bary.z);
            auto posy = (a.y * bary.x + b.y * bary.y + c.y * bary.z);
            auto lies = [&](float x, float y) {
                y *= 1.2;
                x = x * 1.2 - 0.5;
                std::complex<double> c(x, y);
                std::complex<double> z;
                for (int i = 0; i < 100; ++i) {
                    z = z * z + c;
                }
                return std::abs(z) < 2;
            };
            int quad_i = 5;
            int quad_j = (2 * posx + posy + 3) * 6 / 6;
            auto color = static_cast<tui::Color>(16 + 6 * quad_j + quad_i);
            return (lies(posx, posy)) ? color : tui::Color::kDefault;
        }

        math::Vec2 a, b, c;
    };
};

void Example() {
    tui::ViewPort view(30, 60);
    /*
    view.PlaceObject(-8, 40, tui::Border(20, 60, tui::Color::kBlue));
    view.PlaceObject(0, 0, tui::Border(20, 60, tui::Color::kGreen));
    view.PlaceObject(8, 50, tui::Border(20, 60, tui::Color::kRed));
    view.PlaceObject(1, 2, tui::Border(10, 20, tui::Color::kYellow));
    view.PlaceObject(7, 14, tui::Border(10, 20, tui::Color::kRed));
    view.PlaceObject(3, 5, tui::Border(5, 5, tui::Color::kBlue));
    view.Render(true);
    int a;
    */
    auto height = view.Height() * 4 - 8;
    auto width = view.Width() * 2 - 4;
    braille::Canvas canvas(height, width);
    /*
    auto lies = [&](int i, int j) {
        double y = 2.0 * i / (height) - 1.0;
        double x = -(2.0 * j / width - 1.0);
        y *= 1.2;
        x = x * 1.2 - 0.5;
        std::complex<double> c(x, y);
        std::complex<double> z;
        for (int i = 0; i < 100; ++i) {
            z = z * z + c;
        }
        return std::abs(z) < 2;
    };
    for (int i = 0; i < height - 1; ++i) {
        auto prev = false;
        for (int j = 0; j < width; ++j) {
            auto t = lies(i, j);
            if (t) {
            // if (t && !(lies(i + 1, j) && lies(i - 1, j) && lies(i, j + 1) && lies(i, j - 1))) {
                canvas.Set(i, j, true);
            }
        }
    }
    for (int i = 0; i < canvas.Height(); ++i) {
        for (int j = 0; j < canvas.Width(); ++j) {
            // auto quad_i = i * 6 / canvas.Height();
            auto quad_i = 5;
            auto quad_j = (i + j) * 6 / (canvas.Height() + canvas.Width());
            canvas.SetColor(i, j, static_cast<tui::Color>(16 + 6 * quad_j + quad_i));
        }
    }
    view.PlaceObject(0, 0, tui::Border(view.Height(), view.Width(), tui::Color::kYellow));
    view.PlaceObject(1, 1, canvas);
    view.Render();
    */
    auto mvp = math::Perspective(M_PI / 9, 1, 0.1, 100.0) * math::LookAt({static_cast<float>(0.8), 0.4, 0.8}, {.0, 0.10, -0.20}, {0.0, -1.0, 0.0});
    auto vec = [=](uint8_t code) {
        math::Vec4 result{};
        double scale = 0.3;
        if (code & 1) {
            result[0] = scale;
        }
        if (code & 2) {
            result[1] = scale;
        }
        if (code & 4) {
            result[2] = scale;
        }
        result[3] = 1.0;
        return mvp * result;
    };
    graphics::Renderer renderer(view.Height() * 4 - 8, view.Width() * 2 - 4);
    /*
    int color = 10;
    for (uint8_t i = 0; i < 8; ++i) {
        Draw(renderer, vec(i), vec(i ^ 1), static_cast<tui::Color>(50));
        Draw(renderer, vec(i), vec(i ^ 2), static_cast<tui::Color>(46));
        Draw(renderer, vec(i), vec(i ^ 4), static_cast<tui::Color>(226));
    }
    */
    renderer.DrawPolygon({vec(2), vec(0), vec(1), vec(3)}, tui::Color::kYellow, ExampleTexture());
    TransferToCanvas(renderer, canvas);
    view.PlaceObject(0, 0, tui::Border(view.Height(), view.Width(), tui::Color::kWhite));
    view.PlaceObject(1, 1, canvas);
    view.Render(true);

    int a;
    std::cin >> a;
}

bool world[20][20][20] = {};
math::Vec3 camPos = {1.0, 2.7, 1.0};
math::Vec4 direction = {-1.0, -0.7, -1.0, 1.0};

void RenderTo(graphics::Renderer& renderer, const math::Mat4& mvp, bool cool_colors, std::vector<int> block_s_podvohom = {}) {
    for (int layer = 1; layer < 19; ++layer) {
        for (int x = 1; x < 19; ++x) {
            for (int z = 1; z < 19; ++z) {
                if (!world[layer][x][z]) {
                    continue;
                }
                auto vec = [=](uint8_t code) {
                    math::Vec4 result{};
                    double scale = 0.2;
                    if (code & 1) {
                        result[0] = scale;
                    }
                    if (code & 2) {
                        result[1] = scale;
                    }
                    if (code & 4) {
                        result[2] = scale;
                    }
                    result[0] += scale * x;
                    result[1] += scale * layer;
                    result[2] += scale * z;
                    result[3] = 1.0;
                    return result;
                };
                for (uint8_t i = 0; i < 8; ++i) {
                    for (auto mask : {std::pair{1, 2}, std::pair{2, 4}, std::pair{4, 1}}) {
                        if ((i ^ mask.first ^ mask.second) < i || (i ^ mask.first) < i || (i ^ mask.second) < i) {
                            continue;
                        }
                        auto color = tui::Color::kYellow;
                        if (cool_colors) {
                            uint32_t code = x | (layer << 8) | (z << 16);
                            uint32_t dir = 7 ^ mask.first ^ mask.second;
                            if (i & dir) {
                                dir |= 8;
                            }
                            code |= (dir << 24);
                            color = static_cast<tui::Color>(code);
                            // std::wcerr << " " << code << "         " << std::endl;
                        }
                        auto outer = tui::Color::kDefault;
                        if (!block_s_podvohom.empty()) {
                            if (block_s_podvohom[0] == x && block_s_podvohom[1] == layer && block_s_podvohom[2] == z) {
                                outer = tui::Color::kRed;
                            }
                        }
                        if (cool_colors) {
                            outer = color;
                        }
                        auto t1 = graphics::SolidPolytexture(color);
                        auto t2 = SquarePolytexture(color);
                        if (cool_colors) {
                            DrawPolygon(renderer, {
                                vec(i),
                                vec(i ^ mask.first),
                                vec(i ^ mask.first ^ mask.second),
                                vec(i ^ mask.second),
                            // }, tui::Color::kDefault, static_cast<tui::Color>(i * 10 + mask.first));
                            }, mvp, outer, t1);
                        } else {
                            DrawPolygon(renderer, {
                                vec(i),
                                vec(i ^ mask.first),
                                vec(i ^ mask.first ^ mask.second),
                                vec(i ^ mask.second),
                            // }, tui::Color::kDefault, static_cast<tui::Color>(i * 10 + mask.first));
                            }, mvp, outer, t2);
                        }
                    }
                }
                if (cool_colors) {
                    continue;
                }
                for (uint8_t i = 0; i < 8; ++i) {
                    if ((i ^ 1) > i) {
                        auto other_x = x;
                        // auto other_x = z + (i & 1 ? 1 : -1);
                        auto other_y = layer + (i & 2 ? 1 : -1);
                        auto other_z = z + (i & 4 ? 1 : -1);
                        if (world[other_y][other_x][other_z] || !(world[other_y][other_x][z] ^ world[layer][other_x][other_z])) {
                            Draw(renderer, vec(i), vec(i ^ 1), static_cast<tui::Color>(i + 1));
                        }
                    }
                    if ((i ^ 2) > i) {
                        auto other_x = x + (i & 1 ? 1 : -1);
                        auto other_y = layer;
                        auto other_z = z + (i & 4 ? 1 : -1);
                        if (world[other_y][other_x][other_z] || !(world[other_y][other_x][z] ^ world[other_y][x][other_z])) {
                            Draw(renderer, vec(i), vec(i ^ 2), static_cast<tui::Color>(i + 1));
                        }
                    }
                    if ((i ^ 4) > i) {
                        auto other_x = x + (i & 1 ? 1 : -1);
                        auto other_y = layer + (i & 2 ? 1 : -1);
                        auto other_z = z;
                        if (world[other_y][other_x][other_z] || !(world[layer][other_x][other_z] ^ world[other_y][x][other_z])) {
                            Draw(renderer, vec(i), vec(i ^ 4), static_cast<tui::Color>(i + 1));
                        }
                    }
                }
            }
        }
    }
}

int main() {
    Example();
    // return 0;
    tui::ViewPort view(59, 119);
    view.Clear();
    view.PlaceObject(0, 0, tui::Border(view.Height(), view.Width(), tui::Color::kGreen));

    world[10][10][10] = true;
    input::EventPoller poller;
    for (double angle = 0; ; angle += 0.05) {
        bool got_event = false;
        bool will_place = false;
        bool will_destroy = false;
        while (auto event = poller.Poll()) {
            got_event = true;
            math::Vec3 up = {0.0, 1.0, 0.0};
            math::Vec3 left = 0.1 * math::Vec3::Cross(math::FromProjective(direction), up);
            math::Vec3 forward = math::Vec3::Cross(up, left);
            if (event->key == input::Key::W) {
                camPos += forward;
            }
            if (event->key == input::Key::S) {
                camPos += -forward;
            }
            if (event->key == input::Key::A) {
                camPos += left;
            }
            if (event->key == input::Key::D) {
                camPos += -left;
            }
            if (event->key == input::Key::J) {
                will_place = true;
            }
            if (event->key == input::Key::U) {
                will_destroy = true;
            }
            if (event->key == input::Key::Up) {
                // assert(false);
                direction = math::Rotate(M_PI / 30, math::Vec3::Cross(math::FromProjective(direction), {0.0, 1.0, 0.0})) * direction;
            }
            if (event->key == input::Key::Down) {
                // assert(false);
                direction = math::Rotate(-M_PI / 30, math::Vec3::Cross(math::FromProjective(direction), {0.0, 1.0, 0.0})) * direction;
            }
            if (event->key == input::Key::Left) {
                // assert(false);
                direction = math::Rotate(M_PI / 30, {0.0, 1.0, 0.0}) * direction;
            }
            if (event->key == input::Key::Right) {
                // assert(false);
                direction = math::Rotate(-M_PI / 30, {0.0, 1.0, 0.0}) * direction;
            }
        }
        if (!got_event) {
            usleep(10000);
            continue;
        }

        direction.Normalize();
        graphics::Renderer renderer(view.Height() * 4 - 8, view.Width() * 2 - 4);
        graphics::Renderer renderer2(view.Height() * 4 - 8, view.Width() * 2 - 4);
        mvp = math::Perspective(M_PI / 2.5, 1, 0.1, 100.0) * math::LookAt(camPos, camPos + math::FromProjective(direction), {0.0, -1.0, 0.0});
        RenderTo(renderer2, mvp, true);
        uint32_t code = static_cast<uint32_t>(renderer2.Get(renderer2.Height() / 2, renderer2.Width() / 2).color);
        auto x = code & 0xff;
        auto y = (code & 0xff00) >> 8;
        auto z = (code & 0xff0000) >> 16;
        auto dir = (code & 0xff000000) >> 24;
        if (will_place) {
            auto mult = dir < 8 ? -1 : 1;
            dir &= 7;
            if (dir == 1) {
                x += mult;
            } else if (dir == 2) {
                y += mult;
            } else if (dir == 4) {
                z += mult;
            }
            world[y][x][z] = true;
        }
        if (will_destroy) {
            world[y][x][z] = false;
            graphics::Renderer renderer2(view.Height() * 4 - 8, view.Width() * 2 - 4);
            RenderTo(renderer2, mvp, true);
            uint32_t code = static_cast<uint32_t>(renderer2.Get(renderer2.Height() / 2, renderer2.Width() / 2).color);
            x = code & 0xff;
            y = (code & 0xff00) >> 8;
            z = (code & 0xff0000) >> 16;
            dir = (code & 0xff000000) >> 24;
        }
        RenderTo(renderer, mvp, false, {x, y, z});
        // Draw(renderer, vec(0), vec(1), tui::Color::kWhite);
        // renderer.DrawTriangle({0.0, 0.7}, {0.7, 0.0}, {-0.7, 0.0});
        // renderer.DrawSegment({0.0, 0.7}, {0.7, 0.0}, tui::Color::kWhite);
        braille::Canvas canvas(renderer.Height(), renderer.Width());
        TransferToCanvas(renderer, canvas);
        view.PlaceObject(1, 1, canvas);
        view.PlaceObject(view.Height() / 2, view.Width() / 2, tui::Textbox("x", tui::Color::kRed));
        view.Render(false);
        std::wcerr << x << "\t\n" << y << "\t\n" << z << "          " << std::endl;
        mvp = mvp * math::Rotate(M_PI / 20, {0.4, 0.4, 0.4});
        usleep(50000);
        // usleep(1000000);
    }
}

