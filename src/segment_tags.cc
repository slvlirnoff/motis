#include "motis/segment_tags.h"

#include <cstdlib>
#include <charconv>
#include <string_view>

#include "utl/verify.h"

#include "nigiri/timetable.h"

namespace n = nigiri;

namespace motis {

namespace {

std::unique_ptr<n::routing::segment_tags> g_segment_tags;

// Station number of a location: the id of its parent (or its own),
//   "Parentch:1:sloid:4121", "ch:1:sloid:4121:0:1" -> 8504121 (85 + sloid, 5 digits)
//   "Parent8002140", "8002140:0:1"                  -> 8002140
// 0 if the id has no such number. (Swiss GTFS shortcut - a real version
// would take the ids of the file as-is: <tag>_<stop_id>.)
n::routing::segment_tags::station_key_t station_key(n::timetable const& tt,
                                                    n::location_idx_t l) {
  if (auto const p = tt.locations_.parents_[l];
      p != n::location_idx_t::invalid()) {
    l = p;
  }
  auto id = tt.locations_.ids_[l].view();
  if (id.starts_with("Parent")) {
    id.remove_prefix(6U);
  }
  constexpr auto kSloid = std::string_view{"ch:1:sloid:"};
  auto const is_sloid = id.starts_with(kSloid);
  if (is_sloid) {
    id.remove_prefix(kSloid.size());
  }
  auto v = std::uint32_t{0U};
  auto const [ptr, ec] = std::from_chars(id.data(), id.data() + id.size(), v);
  if (ec != std::errc{} || ptr == id.data()) {
    return 0U;
  }
  return is_sloid ? (v < 100'000U ? 8'500'000U + v : 0U) : v;
}

}  // namespace

void load_segment_tags(n::timetable const& tt) {
  auto const path = std::getenv("MOTIS_SEGMENT_TAGS");
  if (path == nullptr || *path == '\0') {
    return;
  }
  g_segment_tags = std::make_unique<n::routing::segment_tags>(
      tt, path, [&](n::location_idx_t const l) { return station_key(tt, l); });
}

std::shared_ptr<n::routing::segment_filter const> make_segment_filter(
    std::optional<std::vector<std::string>> const& allowed_tags) {
  if (!allowed_tags.has_value()) {
    return nullptr;
  }
  utl::verify(g_segment_tags != nullptr,
              "allowedSegmentTags: no segment tags loaded "
              "(start the server with MOTIS_SEGMENT_TAGS=<file>)");
  auto tags = std::vector<std::string_view>{};
  for (auto const& t : *allowed_tags) {
    tags.emplace_back(t);
  }
  return std::make_shared<n::routing::segment_filter const>(*g_segment_tags,
                                                            tags);
}

}  // namespace motis
