#include "catch2.hpp"

#include "oxm/event.h"

constexpr const char* kTag = "[oxm::Event]";

TEST_CASE("Event::Type has permanent values", kTag) {
  REQUIRE(static_cast<uint32_t>(oxm::Event::Type::Read) == 1);
  REQUIRE(static_cast<uint32_t>(oxm::Event::Type::Write) == 2);
  REQUIRE(static_cast<uint32_t>(oxm::Event::Type::FileDescriptorError) == 4);
  REQUIRE(static_cast<uint32_t>(oxm::Event::Type::RemoteConnectionClosed) == 8);
}

TEST_CASE("Default state is None", kTag) {
  oxm::Event event;
  REQUIRE(event.mask.bits == 0);
  REQUIRE(event.fd == -1);
}

TEST_CASE("After setting trigger, bits are correct", kTag) {
  oxm::Event event;

  SECTION("Set Read bit") {
    event.mask.Set(oxm::Event::Type::Read);
    REQUIRE(event.mask.Has(oxm::Event::Type::Read));
  }

  SECTION("Set Write bit") {
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE(event.mask.Has(oxm::Event::Type::Write));
  }

  SECTION("Set FileDescriptorError bit") {
    event.mask.Set(oxm::Event::Type::FileDescriptorError);
    REQUIRE(event.mask.Has(oxm::Event::Type::FileDescriptorError));
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.mask.Set(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE(event.mask.Has(oxm::Event::Type::RemoteConnectionClosed));
  }

  SECTION("Set Read and Write bits") {
    event.mask.Set(oxm::Event::Type::Read);
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE(event.mask.Has(oxm::Event::Type::Read));
    REQUIRE(event.mask.Has(oxm::Event::Type::Write));
  }
}

TEST_CASE("HasError returns true only on error bits", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(event.mask.HasError());
  }

  SECTION("Set Read bit") {
    event.mask.Set(oxm::Event::Type::Read);
    REQUIRE_FALSE(event.mask.HasError());
  }

  SECTION("Set Write bit") {
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE_FALSE(event.mask.HasError());
  }

  SECTION("Set FileDescriptorError bit") {
    event.mask.Set(oxm::Event::Type::FileDescriptorError);
    REQUIRE(event.mask.HasError());
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.mask.Set(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE(event.mask.HasError());
  }
}

TEST_CASE("CanRead returns true only on Read bit", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(event.mask.CanRead());
  }

  SECTION("Set Read bit") {
    event.mask.Set(oxm::Event::Type::Read);
    REQUIRE(event.mask.CanRead());
  }

  SECTION("Set Write bit") {
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE_FALSE(event.mask.CanRead());
  }

  SECTION("Set FileDescriptorError bit") {
    event.mask.Set(oxm::Event::Type::FileDescriptorError);
    REQUIRE_FALSE(event.mask.CanRead());
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.mask.Set(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE_FALSE(event.mask.CanRead());
  }

  SECTION("Set Read and Write bits") {
    event.mask.Set(oxm::Event::Type::Read);
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE(event.mask.CanRead());
  }
}

TEST_CASE("CanWrite returns true only on Write bit", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(event.mask.CanWrite());
  }

  SECTION("Set Read bit") {
    event.mask.Set(oxm::Event::Type::Read);
    REQUIRE_FALSE(event.mask.CanWrite());
  }

  SECTION("Set Write bit") {
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE(event.mask.CanWrite());
  }

  SECTION("Set FileDescriptorError bit") {
    event.mask.Set(oxm::Event::Type::FileDescriptorError);
    REQUIRE_FALSE(event.mask.CanWrite());
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.mask.Set(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE_FALSE(event.mask.CanWrite());
  }

  SECTION("Set Read and Write bits") {
    event.mask.Set(oxm::Event::Type::Read);
    event.mask.Set(oxm::Event::Type::Write);
    REQUIRE(event.mask.CanWrite());
  }
}
