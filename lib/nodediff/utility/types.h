#pragma once

#include <nlohmann_json/json.hpp>
#include <unordered_set>

namespace nd
{
/*
* Following lines can be used for allowing nlohmann::json to use unordered_map as object container.
*/
// template <class K, class V, class...>
// using default_unordered_map = std::unordered_map<K, V>;
// using json = nlohmann::basic_json<default_unordered_map>;

/*
* nd::json object corresponds to nlohmann::json.
*/
using json = nlohmann::json;
}; // namespace nd

///
/// Static type checks
///
namespace nd
{
/*
* Type traits for statically checking if an object is a pair.
*/
template <typename T>
struct is_pair : std::false_type
{
};
template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type
{
};
template <typename T>
constexpr bool is_pair_v = is_pair<T>::value;

/*
* Type traits for statically checking if an object is a map.
* Source: https://stackoverflow.com/questions/45042233/detect-if-type-is-a-mapping
*/
template <typename, typename = void>
struct is_mapping : std::false_type
{
};
template <typename Container>
struct is_mapping<Container,
				  std::enable_if_t<is_pair_v<typename std::iterator_traits<typename Container::iterator>::value_type>>>
	: std::true_type
{
};
template <typename T>
constexpr bool is_mapping_v = is_mapping<T>::value;
}; // namespace nd