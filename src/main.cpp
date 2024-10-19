#include <array>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <random>
#include <three/check_gl.h>

struct Star
{
    glm::vec2 pos;
    glm::vec2 vel;
};

constexpr int n_stars = 3;
std::array<Star, n_stars> stars;
std::array<glm::vec3, n_stars> colors;
std::array<float, n_stars> masses;

// 万有引力常数
constexpr float G = 1e-11f;

void init()
{
    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND));
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.8f, 0.8f);
    std::uniform_real_distribution<float> vel(-0.1f, 0.1f);
    std::uniform_real_distribution<float> color(0.2f, 1.0f);
    std::uniform_real_distribution<float> mass(1e3, 1e5);

    for (auto &star : stars)
    {
        star.pos = glm::vec2(dis(gen), dis(gen));
        star.vel = glm::vec2(vel(gen), vel(gen));
    }

    for (auto &c : colors)
    {
        c = glm::vec3(color(gen), color(gen), color(gen));
    }

    for (auto &m : masses)
    {
        m = mass(gen);
    }
}

void render(float dt)
{
    // 每次渲染一个根据时间变化的背景色
    const float fade = 1.5f;
    CHECK_GL(glColor4f(0.0f, 0.0f, 0.1f, 1.0f - std::exp(-fade * dt)));
    CHECK_GL(glRectf(-1.0f, -1.0f, 1.0f, 1.0f));

    for (std::size_t i = 0; i < stars.size(); ++i)
    {
        CHECK_GL(glPointSize(std::log(masses[i])));

        glBegin(GL_POINTS);
        glm::vec2 pos = stars[i].pos;
        glm::vec3 color = colors[i];
        glColor3fv(&color[0]);
        glVertex2f(pos.x, pos.y);
        CHECK_GL(glEnd());
    }
}

void free(float dt)
{
    // 反弹阻尼 + 空气阻力
    const float bounce = 0.7f;
    const float friction = 0.99f;
    const float gravity = 0.8f;

    for (auto &star : stars)
    {
        star.pos += star.vel * dt;
        star.vel.y -= gravity * dt;

        if (star.pos.x < -1.0f && star.vel.x < 0.0f || star.pos.x > 1.0f && star.vel.x > 0.0f)
        {
            // 反弹阻尼
            star.vel.x = -star.vel.x * bounce;
        }

        if (star.pos.y < -1.0f && star.vel.y < 0.0f || star.pos.y > 1.0f && star.vel.y > 0.0f)
        {
            star.vel.y = -star.vel.y * bounce;
        }

        // 空气阻力
        star.vel *= std::exp(-friction * dt);
    }
}

void euler(float dt)
{
    for (auto &star : stars)
    {
        star.pos += star.vel * dt;
    }

    // 计算万有引力
    for (std::size_t i = 0; i < stars.size(); ++i)
    {
        for (std::size_t j = 0; j < stars.size(); ++j)
        {
            if (i == j)
                continue;

            glm::vec2 dist = stars[j].pos - stars[i].pos;
            float dist_sq = std::max(0.05f, glm::dot(dist, dist));
            float mag = G * masses[i] * masses[j] / dist_sq;
            glm::vec2 force = glm::normalize(dist) * mag;
            stars[i].vel += force * dt;
            stars[j].vel -= force * dt;
        }
    }

    // 碰撞反弹
    for (auto &star : stars)
    {
        if (star.pos.x < -1.0f && star.vel.x < 0.0f || star.pos.x > 1.0f && star.vel.x > 0.0f)
        {
            star.vel.x = -star.vel.x;
        }

        if (star.pos.y < -1.0f && star.vel.y < 0.0f || star.pos.y > 1.0f && star.vel.y > 0.0f)
        {
            star.vel.y = -star.vel.y;
        }
    }
}

float energy()
{
    // 引力势能
    auto potE = 0.0f;
    for (std::size_t i = 0; i < stars.size(); ++i)
    {
        for (std::size_t j = 0; j < stars.size(); ++j)
        {
            if (i == j)
                continue;

            glm::vec2 dist = stars[j].pos - stars[i].pos;
            float dist_sq = glm::dot(dist, dist);
            float mag = G * masses[i] * masses[j] / dist_sq;
            potE += mag;
        }
    }

    // 动能
    auto movE = 0.0f;
    for (std::size_t i = 0; i < stars.size(); ++i)
    {
        glm::vec2 vel = stars[i].vel;
        movE += 0.5f * masses[i] * glm::dot(vel, vel);
    }

    // 总能量
    return movE - potE;
}

void advance(float dt)
{
    // 细分提高 Euler 法精度
    constexpr int n = 100;
    for (int i = 0; i < n; ++i)
    {
        euler(dt / n);
    }
    // std::cout << "Energy: " << energy() << std::endl;
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "Three Stars", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    init();
    double t0 = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // glClear(GL_COLOR_BUFFER_BIT);

        // 根据时间更新，即使渲染卡顿也能保持一致的动画效果
        double t1 = glfwGetTime();
        render(t1 - t0);
        advance(t1 - t0);
        t0 = t1;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
