#pragma once
#include "macro.h"
#include "types.h"

#include <unordered_map>

namespace nd
{
/*
* It combines hash of multiple objects (similar concept to hashCombine in Java or boost::hash_combine).
* Example:
* struct point2f { int x, int y; } p;
* size_t seed = 0;
* nd::hash_combine(seed, p.x, py);
* 
* In this case the function combine hash(p.x) and hash(p.y) into a single one, hence result can be used as hash value for point2f.
* 
* from: https://stackoverflow.com/a/57595105
*/
template <typename T, typename... Rest>
inline void hash_combine(out_var size_t& seed, const T& v, const Rest&... rest)
{
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
};

/*
* Updates std::unordered_map values with the one associated to common keys in m2.
* Example:
* auto m1 = {0: "hello", 1: "world"};
* auto m2 = {1: "earth"};
* nd::update(m1, m2); => m1 <- {0: "hello", 1: "earth"}
*/
template <typename K, typename V>
inline void update(std::unordered_map<K, V>& m1, const std::unordered_map<K, V>& m2)
{
	for (const auto& [k2, v2] : m2)
	{
		m1[k2] = v2;
	}
}

/*
* Given a std::vector<T> v, and an element T e, it returns the first index of v in which is located e.
* In case e is not in v, it just returns -1.
* T is the type of the element to find.
*/
template <typename T>
[[nodiscard]] inline int index_of(const std::vector<T>& v, const T& e)
{
	auto it = std::find(v.begin(), v.end(), e);
	return it != v.end() ? it - v.begin() : -1;
}

/*
* Given an nd::json array, it resize it to 'size'.
*/
inline void resize_json_array(nd::json& array, size_t size) { array.get_ref<json::array_t&>().resize(size); }

/*
* It loads a json file located at fp std::string file_path and 
* store it as an nd::json object (which is given as parameter).
*/
bool load_json(const std::string& file_path, out_var nd::json& json);
/*
* It saves an nd::json object at std::string file_path. 
* The function also allows to specify indentation size in spaces (default value: 4)
*/
bool save_json(const nd::json& json, const std::string& file_path, int indent = 4);
}; // namespace nd

namespace nd
{
/*
* Python-like enumerator. 
* Inspired by https://stackoverflow.com/a/11329249/8927036
*/
template <typename Iterable>
class enumerate_object
{
  private:
	Iterable m_iter;
	std::size_t m_size;
	decltype(std::begin(m_iter)) m_begin;
	const decltype(std::end(m_iter)) m_end;

  public:
	enumerate_object(Iterable iter) : m_iter(iter), m_size(0), m_begin(std::begin(iter)), m_end(std::end(iter)) {}

	inline const enumerate_object& begin() const { return *this; }
	inline const enumerate_object& end() const { return *this; }

	inline bool operator!=(const enumerate_object&) const { return m_begin != m_end; }

	inline void operator++()
	{
		++m_begin;
		++m_size;
	}

	inline auto operator*() const -> std::pair<std::size_t, decltype(*m_begin)> { return {m_size, *m_begin}; }
};

/*
* Python-like enumerate function.
* Example:
* std::vector<std::string> v = {"hello", "world"};
* for(const auto& [idx, word] : enumerate(v)) { ... }
*/
template <typename Iterable>
inline enumerate_object<Iterable> enumerate(Iterable&& iter)
{
	return {std::forward<Iterable>(iter)};
}
}; // namespace nd