#include "memory/buddy_allocator.h"

#include "catch2.hpp"

constexpr const char* kTag = "[oxm::BuddyAllocator]";

int64_t CalculateDistance(void* first, void* last) {
  return std::distance(static_cast<const uint8_t*>(first), static_cast<const uint8_t*>(last));
}

TEST_CASE("Interface", kTag) {
  auto dummy = std::make_shared<oxm::DummyAllocator>(64);
  auto buddy = std::make_shared<oxm::BuddyAllocator>(dummy.get(), 128, 32);

  REQUIRE(buddy->GetBlockCount() == 7);
  REQUIRE(buddy->GetLevelCount() == 3);

  SECTION("Return nullptr if requested number of bytes is greater than pool size") {
    auto* ptr1 = buddy->Allocate(256);
    REQUIRE(ptr1 == nullptr);

    auto* ptr2 = buddy->Allocate(129);
    REQUIRE(ptr2 == nullptr);
  }

  SECTION("Return nullptr if all poll is used") {
    void* ptr_128_0 = buddy->Allocate(128);
    REQUIRE(ptr_128_0 != nullptr);

    void* ptr_32_0 = buddy->Allocate(32);
    REQUIRE(ptr_32_0 == nullptr);
  }

  SECTION("Able to allocate again after freeing memory") {
    void* ptr_64_0 = buddy->Allocate(64);
    REQUIRE(ptr_64_0 != nullptr);

    void* ptr_64_1 = buddy->Allocate(64);
    REQUIRE(ptr_64_1 != nullptr);

    buddy->Deallocate(ptr_64_0);

    void* ptr_32_0 = buddy->Allocate(32);
    REQUIRE(ptr_32_0 != nullptr);

    buddy->Deallocate(ptr_64_1);
    buddy->Deallocate(ptr_32_0);
  }

  SECTION("Sequence +32 +64 +32") {
    void* ptr_32_0 = buddy->Allocate(32);
    REQUIRE(ptr_32_0 != nullptr);

    void* ptr_64_0 = buddy->Allocate(64);
    REQUIRE(ptr_64_0 != nullptr);

    void* ptr_32_1 = buddy->Allocate(32);
    REQUIRE(ptr_32_1 != nullptr);

    REQUIRE(CalculateDistance(ptr_32_0, ptr_64_0) == 64);
    REQUIRE(CalculateDistance(ptr_32_1, ptr_64_0) == 32);
    REQUIRE(CalculateDistance(ptr_32_0, ptr_32_1) == 32);
  }

  SECTION("Sequence +64 +32 +32 -64 +32") {
    void* ptr_64_0 = buddy->Allocate(64);
    REQUIRE(ptr_64_0 != nullptr);

    void* ptr_32_0 = buddy->Allocate(32);
    REQUIRE(ptr_32_0 != nullptr);

    void* ptr_32_1 = buddy->Allocate(32);
    REQUIRE(ptr_32_1 != nullptr);

    buddy->Deallocate(ptr_64_0);

    void* ptr_32_2 = buddy->Allocate(32);
    REQUIRE(ptr_32_2 != nullptr);

    REQUIRE(CalculateDistance(ptr_32_2, ptr_64_0) == 0);
    REQUIRE(CalculateDistance(ptr_32_2, ptr_32_0) == 64);
    REQUIRE(CalculateDistance(ptr_32_2, ptr_32_1) == 96);
  }
}
