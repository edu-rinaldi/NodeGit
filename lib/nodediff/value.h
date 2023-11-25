#pragma once
#include "utility/types.h"

#include <unordered_map>
#include <variant>
#include <vector>

// Variadic type value declaration
namespace nd
{
// Forward decl.
struct value;

// Dictionary of values
struct dictionary : public std::unordered_map<std::string, value>
{
	using std::unordered_map<std::string, value>::unordered_map;
};

// List of values
struct list : public std::vector<value>
{
	using std::vector<value>::vector;
};

// Variadic type value
class value
{
  public:
	enum struct type
	{
		none,
		boolean,
		float_number,
		float_array,
		int_number,
		int_array,
		string,
		list,
		dictionary
	};

  private:
	type m_type = type::none; // current value's type

	bool m_boolean					= false;
	std::vector<float> m_float_nums = {};
	std::vector<int> m_int_nums		= {};
	std::string m_string			= "";
	list m_list						= {};
	dictionary m_dictionary			= {};

  public:
	// Default constructor disabled, use custom ones
	value(type, bool, std::vector<float>, std::vector<int>, std::string, list, dictionary) = delete;

	// Copy constructor
	value(const value& value);
	// Move constructor
	value(value&& value);

	// Constructors for each value type
	value();
	value(bool boolean);
	value(float float_num);
	value(const std::vector<float>& float_nums);
	value(std::vector<float>&& float_nums);
	value(int int_num);
	value(const std::vector<int>& int_nums);
	value(std::vector<int>&& int_nums);
	value(const std::string& string);
	value(std::string&& string);
	value(const list& list);
	value(list&& list);
	value(const dictionary& dictionary);
	value(dictionary&& dictionary);

	// Getters
	/*
	* It returns the currently value stored by the struct but casted to its current type associated.
	* Example, nd::value v storing an integer: int a = v.get<int>();
	*/
	template <typename T>
	T& get();
	template <typename T>
	const T& get() const;

	/*
	* Returns the value::type currently stored by the value.
	*/
	[[nodiscard]] inline type type() const { return m_type; }

	// Assignment operators
	value& operator=(const value& value);
	value& operator=(value&& value);

	value& operator=(bool boolean);

	value& operator=(float float_num);
	value& operator=(const std::vector<float>& float_nums);
	value& operator=(std::vector<float>&& float_nums);

	value& operator=(int int_num);
	value& operator=(const std::vector<int>& int_nums);
	value& operator=(std::vector<int>&& int_nums);

	value& operator=(const std::string& string);
	value& operator=(std::string&& string);

	value& operator=(const list& list);
	value& operator=(list&& list);

	value& operator=(const dictionary& dictionary);
	value& operator=(dictionary&& dictionary);

	// operator==
	bool operator==(const value& other) const;
	bool operator==(bool other) const;
	bool operator==(float other) const;
	bool operator==(const std::vector<float>& other) const;
	bool operator==(int other) const;
	bool operator==(const std::vector<int>& other) const;
	bool operator==(const char* other) const;
	bool operator==(const std::string& other) const;
	bool operator==(const list& other) const;
	bool operator==(const dictionary& other) const;

	// operator!=
	bool operator!=(const value& other) const;
	bool operator!=(bool other) const;
	bool operator!=(float other) const;
	bool operator!=(const std::vector<float>& other) const;
	bool operator!=(int other) const;
	bool operator!=(const std::vector<int>& other) const;
	bool operator!=(const char* other) const;
	bool operator!=(const std::string& other) const;
	bool operator!=(const list& other) const;
	bool operator!=(const dictionary& other) const;
};
}; // namespace nd

///
///	STL
///
namespace std
{
ostream& operator<<(ostream& os, const nd::value& value);
}

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
template <>
struct adl_serializer<nd::value>
{
	static void to_json(nd::json& j, const nd::value& value);
	static void from_json(const nd::json& j, nd::value& value);
};
} // namespace nlohmann