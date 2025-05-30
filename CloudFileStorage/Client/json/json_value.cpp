#include <math.h>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>

#include "json_value.h"
#pragma warning (disable : 26495)

#ifdef __MINGW32__
#define wcsncasecmp wcsnicmp
#endif

// Macros to free an array/object
#define FREE_ARRAY(x) { JsonArray::iterator iter; for (iter = x.begin(); iter != x.end(); iter++) { delete *iter; } }
#define FREE_OBJECT(x) { JsonObject::iterator iter; for (iter = x.begin(); iter != x.end(); iter++) { delete (*iter).second; } }

/**
 * Parses a JSON encoded value to a JSONValue object
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the data
 *
 * @return JSONValue* Returns a pointer to a JSONValue object on success, NULL on error
 */
JsonValue* JsonValue::Parse(const wchar_t** data)
{
	// Is it a string?
	if (**data == '"')
	{
		std::wstring str;
		if (!JsonParser::ExtractString(&(++(*data)), str))
		{
			return NULL;
		}
		else
		{
			return new JsonValue(str);
		}
	}
	// Is it a boolean?
	else if ((simplejson_wcsnlen(*data, 4) && wcsncasecmp(*data, L"true", 4) == 0) || (simplejson_wcsnlen(*data, 5) && wcsncasecmp(*data, L"false", 5) == 0))
	{
		bool value = wcsncasecmp(*data, L"true", 4) == 0;
		(*data) += value ? 4 : 5;
		return new JsonValue(value);
	}
	// Is it a null?
	else if (simplejson_wcsnlen(*data, 4) && wcsncasecmp(*data, L"null", 4) == 0)
	{
		(*data) += 4;
		return new JsonValue();
	}
	// Is it a number?
	else if (**data == L'-' || (**data >= L'0' && **data <= L'9'))
	{
		// Negative?
		bool neg = **data == L'-';
		if (neg)
		{
			(*data)++;
		}
		double number = 0.0;

		// Parse the whole part of the number - only if it wasn't 0
		if (**data == L'0')
		{
			(*data)++;
		}
		else if (**data >= L'1' && **data <= L'9')
		{
			number = JsonParser::ParseInt(data);
		}
		else
		{
			return NULL;
		}

		// Could be a decimal now...
		if (**data == '.')
		{
			(*data)++;
			// Not get any digits?
			if (!(**data >= L'0' && **data <= L'9'))
			{
				return NULL;
			}
			// Find the decimal and sort the decimal place out
			// Use ParseDecimal as ParseInt won't work with decimals less than 0.1
			// thanks to Javier Abadia for the report & fix
			double decimal = JsonParser::ParseDecimal(data);

			// Save the number
			number += decimal;
		}

		// Could be an exponent now...
		if (**data == L'E' || **data == L'e')
		{
			(*data)++;
			// Check signage of expo
			bool neg_expo = false;
			if (**data == L'-' || **data == L'+')
			{
				neg_expo = **data == L'-';
				(*data)++;
			}

			// Not get any digits?
			if (!(**data >= L'0' && **data <= L'9'))
			{
				return NULL;
			}

			// Sort the expo out
			double expo = JsonParser::ParseInt(data);
			for (double i = 0.0; i < expo; i++)
			{
				number = neg_expo ? (number / 10.0) : (number * 10.0);
			}
		}
		// Was it neg?
		if (neg)
		{
			number *= -1;
		}
		return new JsonValue(number);
	}

	// An object?
	else if (**data == L'{')
	{
		JsonObject object;

		(*data)++;
		wchar_t ch = **data;
		while (**data != 0)
		{
			// Whitespace at the start?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// Special case - empty object
			if (object.size() == 0 && **data == L'}')
			{
				(*data)++;
				return new JsonValue(object);
			}

			// We want a string now...
			std::wstring name;
			if (!JsonParser::ExtractString(&(++(*data)), name))
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// More whitespace?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// Need a : now
			if (*((*data)++) != L':')
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// More whitespace?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// The value is here
			JsonValue* value = Parse(data);
			if (value == NULL)
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// Add the name:value
			if (object.find(name) != object.end())
			{
				delete object[name];
			}
			object[name] = value;

			// More whitespace?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_OBJECT(object);
				return NULL;
			}

			// End of object?
			if (**data == L'}')
			{
				(*data)++;
				return new JsonValue(object);
			}

			// Want a , now
			if (**data != L',')
			{
				FREE_OBJECT(object);
				return NULL;
			}

			(*data)++;
		}

		// Only here if we ran out of data
		FREE_OBJECT(object);
		return NULL;
	}

	// An array?
	else if (**data == L'[')
	{
		JsonArray array;

		(*data)++;

		while (**data != 0)
		{
			// Whitespace at the start?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_ARRAY(array);
				return NULL;
			}

			// Special case - empty array
			if (array.size() == 0 && **data == L']')
			{
				(*data)++;
				return new JsonValue(array);
			}

			// Get the value
			JsonValue* value = Parse(data);
			if (value == NULL)
			{
				FREE_ARRAY(array);
				return NULL;
			}

			// Add the value
			array.push_back(value);

			// More whitespace?
			if (!JsonParser::SkipWhitespace(data))
			{
				FREE_ARRAY(array);
				return NULL;
			}

			// End of array?
			if (**data == L']')
			{
				(*data)++;
				return new JsonValue(array);
			}

			// Want a , now
			if (**data != L',')
			{
				FREE_ARRAY(array);
				return NULL;
			}

			(*data)++;
		}

		// Only here if we ran out of data
		FREE_ARRAY(array);
		return NULL;
	}

	// Ran out of possibilites, it's bad!
	else
	{
		return NULL;
	}
}

/**
 * Basic constructor for creating a JSON Value of type NULL
 *
 * @access public
 */
JsonValue::JsonValue(/*NULL*/)
{
	type = JSONType_Null;
}

/**
 * Basic constructor for creating a JSON Value of type String
 *
 * @access public
 *
 * @param wchar_t* m_char_value The string to use as the value
 */
JsonValue::JsonValue(const wchar_t* m_char_value)
{
	type = JSONType_String;
	string_value = new std::wstring(std::wstring(m_char_value));
}

/**
 * Basic constructor for creating a JSON Value of type String
 *
 * @access public
 *
 * @param std::wstring m_string_value The string to use as the value
 */
JsonValue::JsonValue(const std::wstring & m_string_value)
{
	type = JSONType_String;
	string_value = new std::wstring(m_string_value);
}

/**
 * Basic constructor for creating a JSON Value of type Bool
 *
 * @access public
 *
 * @param bool m_bool_value The bool to use as the value
 */
JsonValue::JsonValue(bool m_bool_value)
{
	type = JSONType_Bool;
	bool_value = m_bool_value;
}

/**
 * Basic constructor for creating a JSON Value of type Number
 *
 * @access public
 *
 * @param double m_number_value The number to use as the value
 */
JsonValue::JsonValue(double m_number_value)
{
	type = JSONType_Number;
	number_value = m_number_value;
}

/**
 * Basic constructor for creating a JSON Value of type Number
 *
 * @access public
 *
 * @param int m_integer_value The number to use as the value
 */
JsonValue::JsonValue(int m_integer_value)
{
	type = JSONType_Number;
	number_value = (double)m_integer_value;
}

/**
 * Basic constructor for creating a JSON Value of type Array
 *
 * @access public
 *
 * @param JsonArray m_array_value The JsonArray to use as the value
 */
JsonValue::JsonValue(const JsonArray & m_array_value)
{
	type = JSONType_Array;
	array_value = new JsonArray(m_array_value);
}

/**
 * Basic constructor for creating a JSON Value of type Object
 *
 * @access public
 *
 * @param JsonObject m_object_value The JsonObject to use as the value
 */
JsonValue::JsonValue(const JsonObject & m_object_value)
{
	type = JSONType_Object;
	object_value = new JsonObject(m_object_value);
}

/**
 * Copy constructor to perform a deep copy of array / object values
 *
 * @access public
 *
 * @param JSONValue m_source The source JSONValue that is being copied
 */
JsonValue::JsonValue(const JsonValue & m_source)
{
	type = m_source.type;

	switch (type)
	{
	case JSONType_String:
		string_value = new std::wstring(*m_source.string_value);
		break;

	case JSONType_Bool:
		bool_value = m_source.bool_value;
		break;

	case JSONType_Number:
		number_value = m_source.number_value;
		break;

	case JSONType_Array:
	{
		JsonArray source_array = *m_source.array_value;
		JsonArray::iterator iter;
		array_value = new JsonArray();
		for (iter = source_array.begin(); iter != source_array.end(); iter++)
		{
			array_value->push_back(new JsonValue(**iter));
		}
		break;
	}

	case JSONType_Object:
	{
		JsonObject source_object = *m_source.object_value;
		object_value = new JsonObject();
		JsonObject::iterator iter;
		for (iter = source_object.begin(); iter != source_object.end(); iter++)
		{
			std::wstring name = (*iter).first;
			(*object_value)[name] = new JsonValue(*((*iter).second));
		}
		break;
	}

	case JSONType_Null:
		// Nothing to do.
		break;
	}
}

/**
 * The destructor for the JSON Value object
 * Handles deleting the objects in the array or the object value
 *
 * @access public
 */
JsonValue::~JsonValue()
{
	if (type == JSONType_Array)
	{
		JsonArray::iterator iter;
		for (iter = array_value->begin(); iter != array_value->end(); iter++)
		{
			delete * iter;
		}
		delete array_value;
	}
	else if (type == JSONType_Object)
	{
		JsonObject::iterator iter;
		for (iter = object_value->begin(); iter != object_value->end(); iter++)
		{
			delete (*iter).second;
		}
		delete object_value;
	}
	else if (type == JSONType_String)
	{
		delete string_value;
	}
}

JsonType JsonValue::GetType() const
{
	return type;
}

/**
 * Checks if the value is a NULL
 *
 * @access public
 *
 * @return bool Returns true if it is a NULL value, false otherwise
 */
bool JsonValue::IsNull() const
{
	return type == JSONType_Null;
}

/**
 * Checks if the value is a String
 *
 * @access public
 *
 * @return bool Returns true if it is a String value, false otherwise
 */
bool JsonValue::IsString() const
{
	return type == JSONType_String;
}

/**
 * Checks if the value is a Bool
 *
 * @access public
 *
 * @return bool Returns true if it is a Bool value, false otherwise
 */
bool JsonValue::IsBool() const
{
	return type == JSONType_Bool;
}

/**
 * Checks if the value is a Number
 *
 * @access public
 *
 * @return bool Returns true if it is a Number value, false otherwise
 */
bool JsonValue::IsNumber() const
{
	return type == JSONType_Number;
}

/**
 * Checks if the value is an Array
 *
 * @access public
 *
 * @return bool Returns true if it is an Array value, false otherwise
 */
bool JsonValue::IsArray() const
{
	return type == JSONType_Array;
}

/**
 * Checks if the value is an Object
 *
 * @access public
 *
 * @return bool Returns true if it is an Object value, false otherwise
 */
bool JsonValue::IsObject() const
{
	return type == JSONType_Object;
}

/**
 * Retrieves the String value of this JSONValue
 * Use IsString() before using this method.
 *
 * @access public
 *
 * @return std::wstring Returns the string value
 */
const std::wstring& JsonValue::AsString() const
{
	return (*string_value);
}

/**
 * Retrieves the Bool value of this JSONValue
 * Use IsBool() before using this method.
 *
 * @access public
 *
 * @return bool Returns the bool value
 */
bool JsonValue::AsBool() const
{
	return bool_value;
}

/**
 * Retrieves the Number value of this JSONValue
 * Use IsNumber() before using this method.
 *
 * @access public
 *
 * @return double Returns the number value
 */
double JsonValue::AsNumber() const
{
	return number_value;
}

/**
 * Retrieves the Array value of this JSONValue
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return JsonArray Returns the array value
 */
const JsonArray& JsonValue::AsArray() const
{
	return (*array_value);
}

/**
 * Retrieves the Object value of this JSONValue
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return JsonObject Returns the object value
 */
const JsonObject& JsonValue::AsObject() const
{
	return (*object_value);
}

/**
 * Retrieves the number of children of this JSONValue.
 * This number will be 0 or the actual number of children
 * if IsArray() or IsObject().
 *
 * @access public
 *
 * @return The number of children.
 */
std::size_t JsonValue::CountChildren() const
{
	switch (type)
	{
	case JSONType_Array:
		return array_value->size();
	case JSONType_Object:
		return object_value->size();
	default:
		return 0;
	}
}

/**
 * Checks if this JSONValue has a child at the given index.
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return bool Returns true if the array has a value at the given index.
 */
bool JsonValue::HasChild(std::size_t index) const
{
	if (type == JSONType_Array)
	{
		return index < array_value->size();
	}
	else
	{
		return false;
	}
}

/**
 * Retrieves the child of this JSONValue at the given index.
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return JSONValue* Returns JSONValue at the given index or NULL
 *                    if it doesn't exist.
 */
JsonValue* JsonValue::Child(std::size_t index)
{
	if (index < array_value->size())
	{
		return (*array_value)[index];
	}
	else
	{
		return NULL;
	}
}

/**
 * Checks if this JSONValue has a child at the given key.
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return bool Returns true if the object has a value at the given key.
 */
bool JsonValue::HasChild(const wchar_t* name) const
{
	if (type == JSONType_Object)
	{
		return object_value->find(name) != object_value->end();
	}
	else
	{
		return false;
	}
}

/**
 * Retrieves the child of this JSONValue at the given key.
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return JSONValue* Returns JSONValue for the given key in the object
 *                    or NULL if it doesn't exist.
 */
JsonValue* JsonValue::Child(const wchar_t* name)
{
	JsonObject::const_iterator it = object_value->find(name);
	if (it != object_value->end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}

/**
 * Retrieves the keys of the JSON Object or an empty vector
 * if this value is not an object.
 *
 * @access public
 *
 * @return std::vector<std::wstring> A vector containing the keys.
 */
std::vector<std::wstring> JsonValue::ObjectKeys() const
{
	std::vector<std::wstring> keys;

	if (type == JSONType_Object)
	{
		JsonObject::const_iterator iter = object_value->begin();
		while (iter != object_value->end())
		{
			keys.push_back(iter->first);

			iter++;
		}
	}

	return keys;
}

/**
 * Creates a JSON encoded string for the value with all necessary characters escaped
 *
 * @access public
 *
 * @param bool prettyprint Enable prettyprint
 *
 * @return std::wstring Returns the JSON string
 */
std::wstring JsonValue::Stringify(bool const prettyprint) const
{
	size_t const indentDepth = prettyprint ? 1 : 0;
	return StringifyImpl(indentDepth);
}


/**
 * Creates a JSON encoded string for the value with all necessary characters escaped
 *
 * @access private
 *
 * @param size_t indentDepth The prettyprint indentation depth (0 : no prettyprint)
 *
 * @return std::wstring Returns the JSON string
 */
std::wstring JsonValue::StringifyImpl(size_t const indentDepth) const
{
	std::wstring ret_string;
	size_t const indentDepth1 = indentDepth ? indentDepth + 1 : 0;
	std::wstring const indentStr = Indent(indentDepth);
	std::wstring const indentStr1 = Indent(indentDepth1);

	switch (type)
	{
	case JSONType_Null:
		ret_string = L"null";
		break;

	case JSONType_String:
		ret_string = StringifyString(*string_value);
		break;

	case JSONType_Bool:
		ret_string = bool_value ? L"true" : L"false";
		break;

	case JSONType_Number:
	{
		if (isinf(number_value) || isnan(number_value))
			ret_string = L"null";
		else
		{
			std::wstringstream ss;
			ss.precision(15);
			ss << number_value;
			ret_string = ss.str();
		}
		break;
	}

	case JSONType_Array:
	{
		ret_string = indentDepth ? L"[\n" + indentStr1 : L"[";
		JsonArray::const_iterator iter = array_value->begin();
		while (iter != array_value->end())
		{
			ret_string += (*iter)->StringifyImpl(indentDepth1);

			// Not at the end - add a separator
			if (++iter != array_value->end())
			{
				ret_string += L",";
			}
		}
		ret_string += indentDepth ? L"\n" + indentStr + L"]" : L"]";
		break;
	}

	case JSONType_Object:
	{
		ret_string = indentDepth ? L"{\n" + indentStr1 : L"{";
		JsonObject::const_iterator iter = object_value->begin();
		while (iter != object_value->end())
		{
			ret_string += StringifyString((*iter).first);
			ret_string += L":";
			ret_string += (*iter).second->StringifyImpl(indentDepth1);

			// Not at the end - add a separator
			if (++iter != object_value->end())
			{
				ret_string += L",";
			}
		}
		ret_string += indentDepth ? L"\n" + indentStr + L"}" : L"}";
		break;
	}
	}

	return ret_string;
}

/**
 * Creates a JSON encoded string with all required fields escaped
 * Works from http://www.ecma-internationl.org/publications/files/ECMA-ST/ECMA-262.pdf
 * Section 15.12.3.
 *
 * @access private
 *
 * @param std::wstring str The string that needs to have the characters escaped
 *
 * @return std::wstring Returns the JSON string
 */
std::wstring JsonValue::StringifyString(const std::wstring & str)
{
	std::wstring str_out = L"\"";
	std::wstring::const_iterator iter = str.begin();
	while (iter != str.end())
	{
		wchar_t chr = *iter;

		if (chr == L'"' || chr == L'\\' || chr == L'/')
		{
			str_out += L'\\';
			str_out += chr;
		}
		else if (chr == L'\b')
		{
			str_out += L"\\b";
		}
		else if (chr == L'\f')
		{
			str_out += L"\\f";
		}
		else if (chr == L'\n')
		{
			str_out += L"\\n";
		}
		else if (chr == L'\r')
		{
			str_out += L"\\r";
		}
		else if (chr == L'\t')
		{
			str_out += L"\\t";
		}
		else if (chr < L' ' || chr > 126)
		{
			str_out += L"\\u";
			for (int i = 0; i < 4; i++)
			{
				int value = (chr >> 12) & 0xf;
				if (value >= 0 && value <= 9)
				{
					str_out += (wchar_t)('0' + value);
				}
				else if (value >= 10 && value <= 15)
				{
					str_out += (wchar_t)('A' + (value - 10));
				}
				chr <<= 4;
			}
		}
		else
		{
			str_out += chr;
		}
		iter++;
	}
	str_out += L"\"";
	return str_out;
}

/**
 * Creates the indentation string for the depth given
 *
 * @access private
 *
 * @param size_t indent The prettyprint indentation depth (0 : no indentation)
 *
 * @return std::wstring Returns the string
 */
std::wstring JsonValue::Indent(size_t depth)
{
	const size_t indent_step = 2;
	depth ? --depth : 0;
	std::wstring indentStr(depth * indent_step, ' ');
	return indentStr;
}