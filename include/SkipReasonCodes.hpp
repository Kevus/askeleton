#pragma once

namespace skip_reason {
inline constexpr char unsupported_type[] = "unsupported_type";
inline constexpr char unsupported_array_shape[] = "unsupported_array_shape";
inline constexpr char unsupported_template_parameter[] =
    "unsupported_template_parameter";
inline constexpr char unsupported_indirection[] = "unsupported_indirection";
inline constexpr char unsupported_pointer_pointee[] =
    "unsupported_pointer_pointee";
inline constexpr char incomplete_type[] = "incomplete_type";
inline constexpr char incomplete_record[] = "incomplete_record";
inline constexpr char abstract_record[] = "abstract_record";
inline constexpr char non_public_lifecycle[] = "non_public_lifecycle";
inline constexpr char missing_fixture_strategy[] = "missing_fixture_strategy";
inline constexpr char missing_instance_strategy[] = "missing_instance_strategy";
inline constexpr char coverage_policy_mutable_parameter[] =
    "coverage_policy_mutable_parameter";
inline constexpr char coverage_policy_instance_construction[] =
    "coverage_policy_instance_construction";
inline constexpr char unusable_constructor[] = "unusable_constructor";
inline constexpr char unsupported_framework_feature[] =
    "unsupported_framework_feature";
} // namespace skip_reason
