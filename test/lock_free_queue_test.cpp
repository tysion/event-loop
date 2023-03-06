#include "core/lock_free_queue.h"

#include <thread>

#include "catch2.hpp"
#include "oxm/task.h"

constexpr const char* kTag = "[oxm::LockFreeQueue]";

TEST_CASE("Single thread tests", kTag) {
  const size_t size = 128;
  oxm::LockFreeQueue<int> queue(size);

  SECTION("Empty queue fails to pop") {
    int elem;
    REQUIRE_FALSE(queue.TryPop(elem));
  }

  SECTION("Pop removes elem from queue") {
    int elem = 0;
    REQUIRE(queue.TryPush(1));
    REQUIRE(queue.TryPop(elem));
    REQUIRE(elem == 1);
    REQUIRE_FALSE(queue.TryPop(elem));
  }

  SECTION("Violate limits") {
    for (int i = 0; i < size; ++i) {
      REQUIRE(queue.TryPush(i));
    }
    REQUIRE_FALSE(queue.TryPush(size));

    int elem;
    for (int i = 0; i < size; ++i) {
      REQUIRE(queue.TryPop(elem));
      REQUIRE(elem == i);
    }
    REQUIRE_FALSE(queue.TryPop(elem));
  }
}
