#include "catch2.hpp"
#include "event_loop_context.h"
#include "oxm/task.h"

constexpr const char* kTag = "[oxm::EventLoopContext]";

struct NotifierMock : oxm::BaseNotificator<NotifierMock> {
  int watch_calls_counter = 0;
  int modify_calls_counter = 0;
  int unwatch_calls_counter = 0;
  int wait_calls_counter = 0;

  oxm::EventIds events;

  NotifierMock(uint32_t num_events) {
    events.reserve(num_events);
  }

  void WatchImpl(int fd, oxm::Event::Mask mask, oxm::Event::Id id) {
    ++watch_calls_counter;
    events.emplace_back(mask, id);
  }

  void ModifyImpl(int fd, oxm::Event::Mask mask) {
    ++modify_calls_counter;
  }

  void UnwatchImpl(int fd) {
    ++unwatch_calls_counter;
  }

  void WaitImpl(int timeout, oxm::EventIds* ready_event_ids) {
    ++wait_calls_counter;

    for (auto event : events) {
      ready_event_ids->push_back(event);
    }
  }
};

using EventLoopContextMocked = oxm::EventLoopContext<NotifierMock>;

template <typename T>
oxm::Task* CreateTask(EventLoopContextMocked* ctx, T callback) {
  struct TaskWrapper final : oxm::Task {
    explicit TaskWrapper(T&& callback) : callback_{std::move(callback)} {
    }

    void Execute(oxm::Event::Mask mask) final {
      callback_(mask);
    }

   private:
    T callback_;
  };

  return new (ctx->AllocateTask(sizeof(TaskWrapper))) TaskWrapper(std::move(callback));
}

TEST_CASE("Handling of invalid input", kTag) {
  auto ctx = std::make_unique<oxm::EventLoopContext<NotifierMock>>();
  oxm::Event event;

  SECTION("Throws an exception when trying to bind invalid event id") {
    auto task = CreateTask(ctx.get(), [](oxm::Event::Mask mask) {});
    REQUIRE_THROWS_AS(ctx->Bind(42, task), std::invalid_argument);
  }

  SECTION("Throws an exception when trying to register an event with bad fd") {
    event.fd = -1;
    REQUIRE_THROWS_AS(ctx->RegisterEvent(event), std::invalid_argument);
  }

  SECTION("Throws an exception when trying to register an event with invalid mask") {
    event.fd = 0;
    event.mask.bits = 42;
    REQUIRE_THROWS_AS(ctx->RegisterEvent(event), std::invalid_argument);
  }

  SECTION("Throws an exception when trying to bind nullptr task") {
    event.fd = 0;
    auto id = ctx->RegisterEvent(event);
    REQUIRE_THROWS_AS(ctx->Bind(id, nullptr), std::invalid_argument);
  }

  SECTION("Throws an exception when trying to schedule invalid event id") {
    REQUIRE_THROWS_AS(ctx->Schedule(42), std::invalid_argument);
  }

  SECTION("Throws an exception when trying to schedule event id without bound task") {
    event.fd = 0;
    auto id = ctx->RegisterEvent(event);
    REQUIRE_THROWS_AS(ctx->Schedule(id), std::logic_error);
  }

  SECTION("Throws an exception when trying to de-schedule invalid event id") {
    REQUIRE_THROWS_AS(ctx->Unschedule(42, true), std::invalid_argument);
  }
}

TEST_CASE("Use notifier API properly", kTag) {
  auto ctx = std::make_unique<EventLoopContextMocked>();
  auto* mock = &ctx->GetNotificator();
  int task_calls_count = 0;

  oxm::Event event;
  event.fd = 0;
  event.mask.Set(oxm::Event::Type::Read);

  auto task =
      CreateTask(ctx.get(), [&task_calls_count](oxm::Event::Mask mask) { ++task_calls_count; });

  auto id = ctx->RegisterEvent(event);
  ctx->Bind(id, task);

  SECTION("Increments watch calls counter after scheduling") {
    ctx->Schedule(id);

    REQUIRE(mock->watch_calls_counter == 1);
    REQUIRE(mock->modify_calls_counter == 0);
    REQUIRE(mock->unwatch_calls_counter == 0);
    REQUIRE(mock->wait_calls_counter == 0);
    REQUIRE(task_calls_count == 0);
  }

  SECTION("Increments unwatch calls counter after de-scheduling") {
    ctx->Unschedule(id, true);

    REQUIRE(mock->watch_calls_counter == 0);
    REQUIRE(mock->modify_calls_counter == 0);
    REQUIRE(mock->unwatch_calls_counter == 1);
    REQUIRE(mock->wait_calls_counter == 0);
    REQUIRE(task_calls_count == 0);
  }

  SECTION("Increments wait calls and execute counter after scheduling") {
    ctx->Schedule(id);
    ctx->Poll();

    REQUIRE(mock->watch_calls_counter == 1);
    REQUIRE(mock->modify_calls_counter == 0);
    REQUIRE(mock->unwatch_calls_counter == 0);
    REQUIRE(mock->wait_calls_counter == 1);
    REQUIRE(task_calls_count == 1);
  }
}
