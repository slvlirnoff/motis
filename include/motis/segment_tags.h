#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "nigiri/routing/segment_filter.h"

namespace nigiri {
struct timetable;
}

namespace motis {

// PROTOTYPE: process-wide segment tags, loaded once from the CSV file named by
// the environment variable MOTIS_SEGMENT_TAGS (see data::load_tt).
void load_segment_tags(nigiri::timetable const&);

// Per-request filter for the `allowedSegmentTags` parameter.
// nullptr if the parameter is not given; throws if no tags file is loaded.
std::shared_ptr<nigiri::routing::segment_filter const> make_segment_filter(
    std::optional<std::vector<std::string>> const& allowed_tags);

}  // namespace motis
