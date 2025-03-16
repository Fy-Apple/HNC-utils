//
// Created by Orange on 2024/11/29.
//
#pragma once
#include <regex>
#include <unordered_map>
#include <charconv>
#include <variant>
#include <optional>
#include <string_view>

#include "print.h"

/**
 * @brief 使用string_view 的 json 解析
 */
namespace hnc_json{
// 前向声明
struct JSONObject;
using JSONDict = std::unordered_map<std::string, JSONObject>;
using JSONList = std::vector<JSONObject>;

struct JSONObject {
  std::variant<
    std::nullptr_t,
    bool,
    int,
    double,
    std::string,
    JSONList,
    JSONDict
  > inner;
  // 万能print
  void do_print() const {
    printnl(inner);
  }
  template <typename T>
  [[nodiscard]] bool is() const {
    return std::holds_alternative<T>(inner);
  }

  template <typename T>
  T const &get() const {
    return std::get<T>(inner);
  }

  template <typename T>
  T &get() {
    return std::get<T>(inner);
  }
};

inline char unescaped_char(const char c) {
  switch (c) {
    case 'n': return '\n';
    case 'r': return '\r';
    case '0': return '\0';
    case 't': return '\t';
    case 'v': return '\v';
    case 'f': return '\f';
    case 'b': return '\b';
    case 'a': return '\a';
    case '\\': return '\\';
    default: return c;
  }
}

inline std::pair<JSONObject, size_t> parse_bool_or_null(const std::string_view json) {
    if (json.starts_with("true")) {
        return {JSONObject{true}, 4};
    }
    if (json.starts_with("false")) {
        return {JSONObject{false}, 5};
    }
    if (json.starts_with("null")) {
        return {JSONObject{std::nullptr_t{}}, 4};
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}

template <class T>
std::optional<T>  try_parse_num(std::string_view str) {
  T value;
  auto res = std::from_chars(str.data(), str.data() + str.size(), value);
  if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
    return value;
  }
  return std::nullopt;
}
inline std::pair<JSONObject, size_t> parse_number(const std::string_view json) {
    size_t i = 0;
    while (i < json.size() && (isdigit(json[i]) || json[i] == '.' || json[i] == 'e' || json[i] == 'E' || json[i] == '+' || json[i] == '-')) {
        ++i;
    }
    auto str = json.substr(0, i);
    if (auto num = try_parse_num<int>(str)) {
        return {JSONObject{*num}, i};
    }
    if (auto num = try_parse_num<double>(str)) {
        return {JSONObject{*num}, i};
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}

inline std::pair<JSONObject, size_t> parse_string(const std::string_view json) {
    if (json[0] != '"') {
        return {JSONObject{std::nullptr_t{}}, 0};
    }
    std::string str;
    for (size_t i = 1; i < json.size(); ++i) {
        if (json[i] == '"') {
            return {JSONObject{str}, i + 1};
        }
        if (json[i] == '\\') {
            ++i;
            str.push_back(unescaped_char(json[i]));
        } else {
            str.push_back(json[i]);
        }
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}
inline std::pair<JSONObject, size_t> parse_array(std::string_view json);
inline std::pair<JSONObject, size_t> parse_object(std::string_view json);

inline std::pair<JSONObject, size_t> parse(std::string_view json) {
  size_t offset = json.find_first_not_of(" \n\r\t\v\f");
  if (offset == std::string_view::npos) {
    return {JSONObject{std::nullptr_t{}}, 0};
  }
  json.remove_prefix(offset);

  if (auto [obj, size] = parse_bool_or_null(json); size != 0) {
    return {std::move(obj), size};
  }
  if (auto [obj, size] = parse_number(json); size != 0) {
    return {std::move(obj), size};
  }
  if (auto [obj, size] = parse_string(json); size != 0) {
    return {std::move(obj), size};
  }
  if (json[0] == '[') {
    return parse_array(json);
  }
  if (json[0] == '{') {
    return parse_object(json);
  }

  return {JSONObject{std::nullptr_t{}}, 0};
}


inline std::pair<JSONObject, size_t> parse_array(const std::string_view json) {
    // if (json.empty() || json[0] != '[') {
    //     return {JSONObject{std::nullptr_t{}}, 0};
    // }

    JSONList list;
    size_t i = 1; // Skip '['
    while (i < json.size()) {
        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");
        if (i >= json.size() || json[i] == ']') {
            ++i; // Skip ']'
            return {JSONObject{std::move(list)}, i};
        }

        auto [element, consumed] = parse(json.substr(i));
        if (consumed == 0) {
            return {JSONObject{std::nullptr_t{}}, 0};
        }

        list.emplace_back(std::move(element));
        i += consumed;

        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");
        if (i < json.size() && json[i] == ',') {
            ++i; // Skip ','
        }
        // } else if (i < json.size() && json[i] == ']') {
        //     ++i; // Skip ']'
        //     return {JSONObject{std::move(list)}, i};
        // }
    }

    return {JSONObject{std::nullptr_t{}}, 0};
}

inline std::pair<JSONObject, size_t> parse_object(const std::string_view json) {
    // if (json.empty() || json[0] != '{') {
    //     return {JSONObject{std::nullptr_t{}}, 0};
    // }

    JSONDict dict;
    size_t i = 1; // Skip '{'
    while (i < json.size()) {
        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");
        if (i >= json.size() || json[i] == '}') {
            ++i; // Skip '}'
            return {JSONObject{std::move(dict)}, i};
        }

        // Parse
        auto [key, key_consumed] = parse_string(json.substr(i));
        if (key_consumed == 0 || !key.is<std::string>()) {
            return {JSONObject{std::nullptr_t{}}, 0};
        }
        i += key_consumed;

        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");
        if (i >= json.size() || json[i] != ':') {
            return {JSONObject{std::nullptr_t{}}, 0};
        }
        ++i; // Skip ':'

        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");

        // Parse value
        auto [value, value_consumed] = parse(json.substr(i));
        if (value_consumed == 0) {
            return {JSONObject{std::nullptr_t{}}, 0};
        }
        i += value_consumed;

        dict.emplace(std::get<std::string>(key.inner), std::move(value));

        i += json.substr(i).find_first_not_of(" \n\r\t\v\f");
        if (i < json.size() && json[i] == ',') {
            ++i; // Skip ','
        }
        // } else if (i < json.size() && json[i] == '}') {
        //     ++i; // Skip '}'
        //     return {JSONObject{std::move(dict)}, i};
        // }
    }

    return {JSONObject{std::nullptr_t{}}, 0};
}

}