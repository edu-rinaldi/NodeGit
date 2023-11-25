#include "value.h"

#include <assert.h>

namespace nd
{
value::value(const value& value)
	: m_type(value.m_type), m_boolean(value.m_boolean), m_float_nums(value.m_float_nums), m_int_nums(value.m_int_nums),
	  m_string(value.m_string), m_list(value.m_list), m_dictionary(value.m_dictionary)
{
}

value::value(value&& value)
	: m_type(value.m_type), m_boolean(std::move(value.m_boolean)), m_float_nums(std::move(value.m_float_nums)),
	  m_int_nums(std::move(value.m_int_nums)), m_string(std::move(value.m_string)), m_list(std::move(value.m_list)),
	  m_dictionary(std::move(value.m_dictionary))
{
	value.m_type = value::type::none;
}
// None
value::value() : m_type(type::none) {}
// Boolean
value::value(bool boolean) : m_type(type::boolean), m_boolean(boolean) {}
// Float - Float Array
value::value(float float_num) : m_type(type::float_number), m_float_nums({float_num}) {}
value::value(const std::vector<float>& float_nums) : m_type(type::float_array), m_float_nums(float_nums) {}
value::value(std::vector<float>&& float_nums) : m_type(type::float_array), m_float_nums(std::move(float_nums)) {}
// Int - Int Array
value::value(int int_num) : m_type(type::int_number), m_int_nums({int_num}) {}
value::value(const std::vector<int>& int_nums) : m_type(type::int_array), m_int_nums(int_nums) {}
value::value(std::vector<int>&& int_nums) : m_type(type::int_array), m_int_nums(std::move(int_nums)) {}
// String
value::value(const std::string& string) : m_type(type::string), m_string(string) {}
value::value(std::string&& string) : m_type(type::string), m_string(std::move(string)) {}
// List
value::value(const list& list) : m_type(type::list), m_list(list) {}
value::value(list&& list) : m_type(type::list), m_list(std::move(list)) {}
// Dictionary
value::value(const dictionary& dictionary) : m_type(type::dictionary), m_dictionary(dictionary) {}
value::value(dictionary&& dictionary) : m_type(type::dictionary), m_dictionary(std::move(dictionary)) {}

template <>
bool& value::get()
{
	assert(m_type == type::boolean && "Type requested is different from m_type");
	return m_boolean;
}

template <>
const bool& value::get() const
{
	assert(m_type == type::boolean && "Type requested is different from m_type");
	return m_boolean;
}

template <>
float& value::get()
{
	assert(m_type == type::float_number && "Type requested is different from m_type");
	return m_float_nums[0];
}

template <>
const float& value::get() const
{
	assert(m_type == type::float_number && "Type requested is different from m_type");
	return m_float_nums[0];
}

template <>
std::vector<float>& value::get()
{
	assert(m_type == type::float_array && "Type requested is different from m_type");
	return m_float_nums;
}

template <>
const std::vector<float>& value::get() const
{
	assert(m_type == type::float_array && "Type requested is different from m_type");
	return m_float_nums;
}

template <>
int& value::get()
{
	assert(m_type == type::int_number && "Type requested is different from m_type");
	return m_int_nums[0];
}

template <>
const int& value::get() const
{
	assert(m_type == type::int_number && "Type requested is different from m_type");
	return m_int_nums[0];
}

template <>
std::vector<int>& value::get()
{
	assert(m_type == type::int_array && "Type requested is different from m_type");
	return m_int_nums;
}

template <>
const std::vector<int>& value::get() const
{
	assert(m_type == type::int_array && "Type requested is different from m_type");
	return m_int_nums;
}

template <>
std::string& value::get()
{
	assert(m_type == type::string && "Type requested is different from m_type");
	return m_string;
}

template <>
const std::string& value::get() const
{
	assert(m_type == type::string && "Type requested is different from m_type");
	return m_string;
}

template <>
list& value::get()
{
	assert(m_type == type::list && "Type requested is different from m_type");
	return m_list;
}

template <>
const list& value::get() const
{
	assert(m_type == type::list && "Type requested is different from m_type");
	return m_list;
}

template <>
dictionary& value::get()
{
	assert(m_type == type::dictionary && "Type requested is different from m_type");
	return m_dictionary;
}

template <>
const dictionary& value::get() const
{
	assert(m_type == type::dictionary && "Type requested is different from m_type");
	return m_dictionary;
}

value& value::operator=(const value& value)
{
	m_type = value.m_type;
	switch (value.m_type)
	{
	case type::none: break;
	case type::boolean: m_boolean = value.m_boolean; break;
	case type::float_number:
		m_float_nums.resize(1);
		m_float_nums[0] = value.m_float_nums[0];
		break;
	case type::float_array: m_float_nums = value.m_float_nums; break;
	case type::int_number:
		m_int_nums.resize(1);
		m_int_nums[0] = value.m_int_nums[0];
		break;
	case type::int_array: m_int_nums = value.m_int_nums; break;
	case type::string: m_string = value.m_string; break;
	case type::list: m_list = value.m_list; break;
	case type::dictionary: m_dictionary = value.m_dictionary; break;
	default: assert(false && "Invalid type");
	}
	return *this;
}

value& value::operator=(value&& value)
{
	m_type = value.m_type;
	switch (value.m_type)
	{
	case type::none: break;
	case type::boolean: m_boolean = std::move(value.m_boolean); break;
	case type::float_number:
	case type::float_array: m_float_nums = std::move(value.m_float_nums); break;
	case type::int_number:
	case type::int_array: m_int_nums = std::move(value.m_int_nums); break;
	case type::string: m_string = std::move(value.m_string); break;
	case type::list: m_list = std::move(value.m_list); break;
	case type::dictionary: m_dictionary = std::move(value.m_dictionary); break;
	default: assert(false && "Invalid type");
	}
	return *this;
}

value& value::operator=(bool boolean)
{
	m_type	  = value::type::boolean;
	m_boolean = boolean;
	return *this;
}

value& value::operator=(float float_num)
{
	m_type		 = value::type::float_number;
	m_float_nums = {float_num};
	return *this;
}

value& value::operator=(const std::vector<float>& float_nums)
{
	m_type		 = value::type::float_array;
	m_float_nums = float_nums;
	return *this;
}

value& value::operator=(std::vector<float>&& float_nums)
{
	m_type		 = value::type::float_array;
	m_float_nums = std::move(float_nums);
	return *this;
}

value& value::operator=(int int_num)
{
	m_type	   = value::type::int_number;
	m_int_nums = {int_num};
	return *this;
}

value& value::operator=(const std::vector<int>& int_nums)
{
	m_type	   = value::type::int_array;
	m_int_nums = int_nums;
	return *this;
}

value& value::operator=(std::vector<int>&& int_nums)
{
	m_type	   = value::type::int_array;
	m_int_nums = std::move(int_nums);
	return *this;
}

value& value::operator=(const std::string& string)
{
	m_type	 = value::type::string;
	m_string = string;
	return *this;
}

value& value::operator=(std::string&& string)
{
	m_type	 = value::type::string;
	m_string = std::move(string);
	return *this;
}

value& value::operator=(const list& list)
{
	m_type = value::type::list;
	m_list = list;
	return *this;
}

value& value::operator=(list&& list)
{
	m_type = value::type::list;
	m_list = std::move(list);
	return *this;
}

value& value::operator=(const dictionary& dictionary)
{
	m_type		 = value::type::dictionary;
	m_dictionary = dictionary;
	return *this;
}

value& value::operator=(dictionary&& dictionary)
{
	m_type		 = value::type::dictionary;
	m_dictionary = std::move(dictionary);
	return *this;
}
bool value::operator==(const value& other) const
{
	if (m_type != other.m_type) return false;
	switch (other.m_type)
	{
	case type::none: return true;
	case type::boolean: return m_boolean == other.m_boolean;
	case type::float_number:
	case type::float_array: return m_float_nums == other.m_float_nums;
	case type::int_number:
	case type::int_array: return m_int_nums == other.m_int_nums;
	case type::string: return m_string == other.m_string;
	case type::list: return m_list == other.m_list;
	case type::dictionary: return m_dictionary == other.m_dictionary;
	default: assert(false && "Invalid value type");
	}
	return false;
}
bool value::operator==(bool other) const { return m_type == value::type::boolean && m_boolean == other; }
bool value::operator==(float other) const { return m_type == value::type::float_number && m_float_nums[0] == other; }
bool value::operator==(const std::vector<float>& other) const
{
	return m_type == value::type::float_array && m_float_nums == other;
}
bool value::operator==(int other) const { return m_type == value::type::int_number && m_int_nums[0] == other; }
bool value::operator==(const std::vector<int>& other) const
{
	return m_type == value::type::int_array && m_int_nums == other;
}
bool value::operator==(const char* other) const { return m_type == value::type::string && m_string == other; }
bool value::operator==(const std::string& other) const { return m_type == value::type::string && m_string == other; }
bool value::operator==(const list& other) const { return m_type == value::type::list && m_list == other; }
bool value::operator==(const dictionary& other) const
{
	return m_type == value::type::dictionary && m_dictionary == other;
}
bool value::operator!=(const value& other) const { return !(*this == other); }
bool value::operator!=(bool other) const { return !(*this == other); }
bool value::operator!=(float other) const { return !(*this == other); }
bool value::operator!=(const std::vector<float>& other) const { return !(*this == other); }
bool value::operator!=(int other) const { return !(*this == other); }
bool value::operator!=(const std::vector<int>& other) const { return !(*this == other); }
bool value::operator!=(const char* other) const { return !(*this == other); }
bool value::operator!=(const std::string& other) const { return !(*this == other); }
bool value::operator!=(const list& other) const { return !(*this == other); }
bool value::operator!=(const dictionary& other) const { return !(*this == other); }
}; // namespace nd

///
///	STL
///
namespace std
{
ostream& operator<<(ostream& os, const nd::value& value)
{
	os << nd::json(value);
	return os;
}
}; // namespace std

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
using namespace nd;
void adl_serializer<nd::value>::to_json(nd::json& j, const nd::value& value)
{
	switch (value.type())
	{
	case value::type::boolean: j = value.get<bool>(); break;
	case value::type::float_number: j = value.get<float>(); break;
	case value::type::float_array: j = value.get<std::vector<float>>(); break;
	case value::type::int_number: j = value.get<int>(); break;
	case value::type::int_array: j = value.get<std::vector<int>>(); break;
	case value::type::string: j = value.get<std::string>(); break;
	case value::type::list: j = value.get<nd::list>(); break;
	case value::type::dictionary: j = value.get<nd::dictionary>(); break;
	case value::type::none: j = nullptr;
	}
}
void adl_serializer<nd::value>::from_json(const nd::json& j, nd::value& value)
{
	switch (j.type())
	{
	case json::value_t::boolean: value = j.get<bool>(); break;
	case json::value_t::object: value = j.get<dictionary>(); break;
	case json::value_t::array: {
		size_t array_size = j.size();

		if (array_size >= 1 && j[0].is_number())
		{
			if (j[0].is_number_float()) { value = j.get<std::vector<float>>(); }
			else
			{
				value = j.get<std::vector<int>>();
			}
		}
		else
		{
			value = j.get<list>();
		}

		break;
	}
	case json::value_t::string: value = j.get<std::string>(); break;
	case json::value_t::number_float: value = j.get<float>(); break;
	case json::value_t::number_integer: value = j.get<int>(); break;
	case json::value_t::number_unsigned: value = static_cast<int>(j.get<unsigned int>()); break;
	case json::value_t::null: break;
	default: assert(false && "Type not handled"); break;
	}
}
}; // namespace nlohmann
