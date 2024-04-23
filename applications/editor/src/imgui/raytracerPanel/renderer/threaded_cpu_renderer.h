#pragma once

#include "i_renderer.h"
#include "scene/i_scene.h"
#include "utils.h"
#include <execution>
#include <future>
#include <queue>
#include <thread>


class ThreadPool
{
public:
    explicit ThreadPool(size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                for (;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }

    template<class F>
    void enqueue(F&& f)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker: workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;

    bool stop;
};

class threaded_cpu_renderer final : public i_renderer
{
public:
    void get_compute_unit(uint32_t start, uint32_t end)
    {
        // Compute unit work
        for (uint32_t i = start; i < end; i++)
        {
            ray ray{scene.trace_camera_ray(precomputed_directions[i])};
            color3 pixel_color = scene.color_at(ray);
            uint32_t pixel_index = i;
            scene.add_color_to_image(pixel_color, pixel_index);
        }
    }
    explicit threaded_cpu_renderer(const i_scene&
                                           scene

                                   ) : scene(scene), pool(std::thread::hardware_concurrency())

    {
        precompute_directions();


        const uint32_t nb_compute_units = scene.horizontal_pixel_count() * scene.vertical_pixel_count() / (8 * 8);


        for (size_t i = 0; i < nb_compute_units; ++i)
        {
            uint32_t start = i * work_unit_pixels * work_unit_pixels;
            uint32_t end = start + work_unit_pixels * work_unit_pixels;
            compute_units.emplace([=, this] { get_compute_unit(start, end); });
        }
    }

    bool render(uint32_t& current_compute_unit) override
    {


        while (!compute_units.empty())
        {
            pool.enqueue(compute_units.front());
            compute_units.pop();
        }


        return 1;
    }
    void precompute_directions()
    {
        for (uint32_t y = scene.vertical_pixel_count(); y > 0; --y)
        {
            for (uint32_t x = 0; x < scene.horizontal_pixel_count(); ++x)
            {
                vec3 direction;
                direction.x = (x + 0.5f) / scene.horizontal_pixel_count();
                direction.y = (y + 0.5f) / scene.vertical_pixel_count();
                precomputed_directions.push_back(direction);
            }
        }
    }

private:
    size_t max_threads = std::thread::hardware_concurrency() -2;

    std::queue<std::function<void()>> compute_units;
    static const uint32_t work_unit_pixels{8};
    const i_scene& scene;
    std::vector<vec3> precomputed_directions;

    ThreadPool pool;
};
