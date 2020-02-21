#pragma once
#include <common.hpp>

namespace HalfMesh
{
    // Triple key tuple hash map

    typedef std::tuple<unsigned int, unsigned int, unsigned int> three_type_t;

    struct three_key_hash : public std::unary_function<three_type_t, std::size_t>
    {
        std::size_t operator()(const three_type_t& k) const
        {
            return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
        }
    };

    struct three_key_equal : public std::binary_function<three_type_t, three_type_t, bool>
    {
        bool operator()(const three_type_t& v0, const three_type_t& v1) const
        {
            return (
                    std::get<0>(v0) == std::get<0>(v1) &&
                    std::get<1>(v0) == std::get<1>(v1) &&
                    std::get<2>(v0) == std::get<2>(v1)
            );
        }
    };

    typedef std::unordered_map<const three_type_t, unsigned int,three_key_hash,three_key_equal> special_map_three;


    // Double key tuple hash map
    typedef std::tuple<unsigned int, unsigned int> twin_type_t;

    struct twin_key_hash : public std::unary_function<twin_type_t, std::size_t>
    {
        std::size_t operator()(const twin_type_t& k) const
        {
            return std::get<0>(k) ^ std::get<1>(k);
        }
    };

    struct twin_key_equal : public std::binary_function<twin_type_t, twin_type_t, bool>
    {
        bool operator()(const twin_type_t& v0, const twin_type_t& v1) const
        {
            return (
                    std::get<0>(v0) == std::get<0>(v1) &&
                    std::get<1>(v0) == std::get<1>(v1)
            );
        }
    };

    void print_set(std::unordered_set< unsigned int > _input_set)
    {
        std::string output;
        output.append("   Set: {");
        for(auto iter = _input_set.begin(); iter != _input_set.end(); ++iter)
        {
            output.append(std::to_string(*iter));
            output.append(",");
        }
        output.pop_back();
        output.append("}");
        std::cout << output << std::endl;

    }

    static inline bool is_substring(const std::string &input_string, const std::string &sub_string)
    {
        std::size_t found = input_string.find(sub_string);
        if (found != std::string::npos)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    static inline std::vector<std::string> split_string(const std::string &str, const std::string &delim, const bool trim_empty = false)
    {
        size_t pos, last_pos = 0, len;
        std::vector<std::string> tokens;
        while(true)
        {
            pos = str.find(delim, last_pos);
            if (pos == std::string::npos)
            {
                pos = str.size();
            }
            len = pos-last_pos;
            if ( !trim_empty || len != 0)
            {
                tokens.push_back(str.substr(last_pos, len));
            }
            if (pos == str.size())
            {
                break;
            }
            else
            {
                last_pos = pos + delim.size();
            }
        }
        return tokens;
    }
}