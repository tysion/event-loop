#include <chrono>
#include <thread>
#include <utility>

#include "catch2.hpp"
#include "multithreading/task_executor.h"

using namespace std::chrono_literals;

constexpr const char* kTag = "[oxm::Executor]";

TEST_CASE("tmp", kTag) {
  auto executor = oxm::MakeExecutor(4, 128);

  SECTION("Single task wait") {
    struct SimpleTask : oxm::Task {
      void Execute(oxm::Event::Mask mask) override {
        // pretend to work hard
        std::this_thread::sleep_for(200ns);
        is_ready.store(true);
      }

      [[nodiscard]] bool IsReady() const {
        return is_ready.load();
      }

     private:
      std::atomic<bool> is_ready = false;
    };

    SimpleTask future;
    executor->PushTask(&future, {});
    executor->Wait([&future]() -> bool { return future.IsReady(); });

    REQUIRE(future.IsReady());
  }

  SECTION("Spawn simple task") {
    struct SimpleTask : oxm::Task {
      explicit SimpleTask(std::atomic<uint64_t>* counter) : counter_{counter} {
      }

      void Execute(oxm::Event::Mask mask) override {
        counter_->fetch_add(1);
      }

      std::atomic<uint64_t>* const counter_;
    };

    std::vector<std::unique_ptr<SimpleTask>> tasks(10000);
    std::atomic<uint64_t> counter = 0;
    for (auto& task : tasks) {
      task = std::make_unique<SimpleTask>(&counter);
    }

    for (const auto& task : tasks) {
      executor->PushTask(task.get(), {});
    }

    executor->Wait([&counter, desired = tasks.size()]() -> bool { return counter == desired; });

    REQUIRE(counter == tasks.size());
  }

  SECTION("Spawn tasks") {
    struct SpawnTask : oxm::Task {
      explicit SpawnTask(std::atomic<uint64_t>* counter, oxm::ExecutorPtr ex)
          : counter_{counter}, executor_{std::move(ex)} {
      }

      void Execute(oxm::Event::Mask mask) override {
        if (spawn_) {
          spawn_ = false;
          executor_->PushTask(this, {});
        }
        counter_->fetch_add(1);
      }

      std::atomic<uint64_t>* const counter_;
      oxm::ExecutorPtr executor_;
      bool spawn_ = true;
    };

    std::vector<std::unique_ptr<SpawnTask>> tasks(10000);
    std::atomic<uint64_t> counter = 0;
    for (auto& task : tasks) {
      task = std::make_unique<SpawnTask>(&counter, executor);
    }

    for (const auto& task : tasks) {
      executor->PushTask(task.get(), {});
    }

    executor->Wait([&counter, desired = tasks.size() * 2]() -> bool { return counter == desired; });

    REQUIRE(counter == tasks.size() * 2);
    REQUIRE(
        std::all_of(tasks.begin(), tasks.end(), [](auto& task) { return task->spawn_ == false; }));
  }
}