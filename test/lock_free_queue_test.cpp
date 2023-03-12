#include "multithreading/lock_free_queue.h"

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

  SECTION("TryPop removes elem from queue") {
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

TEST_CASE("Multiple threads", kTag) {
  const size_t size = 1024 * 128;
  oxm::LockFreeQueue<int> queue(size);

  std::thread producer_1([&]() {
    int i = 0;
    while (i < size) {
      if (queue.TryPush(i)) {
        i += 2;
      }
    }
  });

  std::thread producer_2([&]() {
    int i = 1;
    while (i < size) {
      if (queue.TryPush(i)) {
        i += 2;
      }
    }
  });

  std::thread consumer([&]() {
    int counter = 0;
    int value;
    std::vector<bool> bitset(size, false);

    while (counter < size) {
      if (queue.TryPop(value)) {
        REQUIRE(bitset[value] == false);
        bitset[value] = true;
        ++counter;
      }
    }

    REQUIRE(std::all_of(bitset.begin(), bitset.end(), [](bool res) { return res; }));
  });

  producer_1.join();
  producer_2.join();
  consumer.join();
}
