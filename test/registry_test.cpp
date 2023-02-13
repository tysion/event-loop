#include "catch2.hpp"
#include "registry.h"

constexpr const char* kTag = "[oxm::Registry]";

TEST_CASE("Testing registry API", kTag) {
  oxm::Registry registry;
  oxm::Event event;
  event.fd = 0;
  event.mask.Set(oxm::Event::Type::Read);
  oxm::Event::Id id = registry.Add(event);

  SECTION("Event ids increase sequentially as elements are added") {
    REQUIRE(id == 0);

    id = registry.Add({});
    REQUIRE(id == 1);

    id = registry.Add({});
    REQUIRE(id == 2);

    id = registry.Add({});
    REQUIRE(id == 3);
  }

  SECTION("Returns an empty optional if a non-existent event id is passed") {
    auto opt = registry.Find(42);
    REQUIRE(!opt);
  }

  SECTION("Returns an valid and correct optional if a existent event id is passed") {
    auto opt = registry.Find(id);

    REQUIRE(opt);

    auto it = *opt;
    REQUIRE(it->first == id);
    REQUIRE(it->second.event.fd == event.fd);
    REQUIRE(it->second.event.mask.bits == static_cast<uint32_t>(oxm::Event::Type::Read));
  }

  SECTION("Extracts correct reference to event binding from valid iterator") {
    auto& binding = oxm::Registry::ExtractEventBinding(*registry.Find(id));

    REQUIRE(binding.event.fd == event.fd);
    REQUIRE(binding.event.mask.bits == static_cast<uint32_t>(oxm::Event::Type::Read));
  }

  SECTION("Extracts actual reference to event binding from valid iterator") {
    auto& binding_1 = oxm::Registry::ExtractEventBinding(*registry.Find(id));
    binding_1.event.fd = 24;
    auto& binding_2 = oxm::Registry::ExtractEventBinding(*registry.Find(id));

    REQUIRE(binding_2.event.fd == 24);
    REQUIRE(binding_1.event.fd == binding_2.event.fd);
    REQUIRE(binding_1.event.mask.bits == binding_2.event.mask.bits);
  }

  SECTION("Binds a task to the chosen event") {
    auto it = *registry.Find(id);
    auto task = std::make_shared<oxm::Task>([](oxm::Event::Mask mask) {});
  }
}