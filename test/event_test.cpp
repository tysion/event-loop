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
  REQUIRE(event.mask == 0);
  REQUIRE(event.fd == -1);
}

TEST_CASE("After setting trigger, bits are correct", kTag) {
  oxm::Event event;

  SECTION("Set Read bit") {
    event.TriggerOn(oxm::Event::Type::Read);
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::Read));
  }

  SECTION("Set Write bit") {
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::Write));
  }

  SECTION("Set FileDescriptorError bit") {
    event.TriggerOn(oxm::Event::Type::FileDescriptorError);
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::FileDescriptorError));
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.TriggerOn(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::RemoteConnectionClosed));
  }

  SECTION("Set Read and Write bits") {
    event.TriggerOn(oxm::Event::Type::Read);
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::Read));
    REQUIRE(oxm::Has(event.mask, oxm::Event::Type::Write));
  }
}

TEST_CASE("HasError returns true only on error bits", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(oxm::HasError(event.mask));
  }

  SECTION("Set Read bit") {
    event.TriggerOn(oxm::Event::Type::Read);
    REQUIRE_FALSE(oxm::HasError(event.mask));
  }

  SECTION("Set Write bit") {
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE_FALSE(oxm::HasError(event.mask));
  }

  SECTION("Set FileDescriptorError bit") {
    event.TriggerOn(oxm::Event::Type::FileDescriptorError);
    REQUIRE(oxm::HasError(event.mask));
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.TriggerOn(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE(oxm::HasError(event.mask));
  }
}

TEST_CASE("CanRead returns true only on Read bit", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(oxm::CanRead(event.mask));
  }

  SECTION("Set Read bit") {
    event.TriggerOn(oxm::Event::Type::Read);
    REQUIRE(oxm::CanRead(event.mask));
  }

  SECTION("Set Write bit") {
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE_FALSE(oxm::CanRead(event.mask));
  }

  SECTION("Set FileDescriptorError bit") {
    event.TriggerOn(oxm::Event::Type::FileDescriptorError);
    REQUIRE_FALSE(oxm::CanRead(event.mask));
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.TriggerOn(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE_FALSE(oxm::CanRead(event.mask));
  }

  SECTION("Set Read and Write bits") {
    event.TriggerOn(oxm::Event::Type::Read);
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE(oxm::CanRead(event.mask));
  }
}

TEST_CASE("CanWrite returns true only on Write bit", kTag) {
  oxm::Event event;

  SECTION("Set no bit") {
    REQUIRE_FALSE(oxm::CanWrite(event.mask));
  }

  SECTION("Set Read bit") {
    event.TriggerOn(oxm::Event::Type::Read);
    REQUIRE_FALSE(oxm::CanWrite(event.mask));
  }

  SECTION("Set Write bit") {
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE(oxm::CanWrite(event.mask));
  }

  SECTION("Set FileDescriptorError bit") {
    event.TriggerOn(oxm::Event::Type::FileDescriptorError);
    REQUIRE_FALSE(oxm::CanWrite(event.mask));
  }

  SECTION("Set RemoteConnectionClosed bit") {
    event.TriggerOn(oxm::Event::Type::RemoteConnectionClosed);
    REQUIRE_FALSE(oxm::CanWrite(event.mask));
  }

  SECTION("Set Read and Write bits") {
    event.TriggerOn(oxm::Event::Type::Read);
    event.TriggerOn(oxm::Event::Type::Write);
    REQUIRE(oxm::CanWrite(event.mask));
  }
}
