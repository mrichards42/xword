// This file is part of XWord
// Copyright (C) 2011 Mike Richards ( mrichards42@gmx.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef PUZ_JSON_H
#define PUZ_JSON_H

#include <map>
#include <vector>
#include <set>
#include <fstream>

#include "Puzzle.hpp"
#include "puzstring.hpp"

namespace puz {
namespace json {

class Value;
class Object;
class Map;
class Array;

class Parser
{
public:
    Parser() {}
    virtual ~Parser() {}

    virtual void LoadPuzzle(Puzzle * puz, std::istream & stream);

    // Override this to load the actual puzzle given a json::Value.
    // Any values that are used by the puzzle should be pop'd so that
    // the remaining values can be stored as extra data in the puzzle object.
    // Return true if your parser subclass will take responsibility for the
    // root pointer (e.g. the pointer is stored as Puzzle::FormatData)
    virtual bool DoLoadPuzzle(Puzzle * puz, Value * root) =0;
};

// ----------------------------------------------------------------------------
// JSON object representation
// ----------------------------------------------------------------------------
// This is similar to yajl_tree
enum json_t
{
    j_string,
    j_number,
    j_bool,
    j_null,
    j_map,
    j_array
};

class BaseError : public std::exception
{
public:
    explicit BaseError(const std::string & msg)
        : message(msg)
    {}
	virtual ~BaseError() throw() {}
    std::string message;
    virtual const char * what() const throw() { return message.c_str(); }
};

class TypeError : public BaseError
{
public:
    TypeError(const std::string & kind)
        : BaseError((std::string("Expected json type: ") + kind).c_str())
    {}
};

class KeyError : public BaseError
{
public:
    KeyError(const std::string & key)
        : BaseError((std::string("Invalid object key: ") + key).c_str())
    {}
};


class IndexError : public BaseError
{
public:
    IndexError(size_t index)
        : BaseError(
            (std::string("Index out of range: ")
            + encode_utf8(ToString(index))).c_str())
    {}
};

class Value
{
public:
    Value() {}
    virtual ~Value() {}

    // Type check
    virtual bool IsString() const { return false; }
    virtual bool IsNumber() const { return false; }
    virtual bool IsBool()   const { return false; }
    virtual bool IsNull()   const { return false; }
    virtual bool IsMap()    const { return false; }
    virtual bool IsArray()  const { return false; }
    virtual bool IsObject() const { return false; }
    virtual bool IsSimple() const { return ! IsObject(); }

    // Getters
    virtual const string_t & AsString() const { throw TypeError("string"); }
    virtual const string_t & AsNumber()  const { throw TypeError("number"); }
    virtual bool AsBool() const { throw TypeError("bool"); }
    virtual Map * AsMap() { throw TypeError("map"); }
    virtual Array * AsArray() { throw TypeError("array"); }
    virtual Object * AsObject() { throw TypeError("object"); }
};


class String : public Value
{
public:
    String(const string_t & str) : Value(), m_str(str) {}
    virtual ~String () {}

    virtual const string_t & AsString() const { return m_str; }
    bool IsString() const { return true; }

protected:
    string_t m_str;
};


class Number : public Value
{
public:
    Number(const string_t & num) : m_str(num) {}
    Number(const int num) : m_str(ToString(num)) {}
    virtual ~Number () {}

    virtual const string_t & AsString() const { return m_str; }
    virtual const string_t & AsNumber() const { return m_str; }
    bool IsNumber() const { return true; }

protected:
    string_t m_str;
};


class Bool : public Value
{
public:
    Bool(bool val) : m_val(val) {}
    virtual ~Bool () {}

    virtual bool AsBool() const { return m_val; }
    bool IsBool() const { return true; }

protected:
    bool m_val;
};


class Null : public Value
{
public:
    Null() {}
    virtual ~Null () {}
    bool IsNull() const { return true; }
};


class Object : public Value
{
public:
    Object() {}
    virtual ~Object ()
    {
        cleanup();
    }

    Object * AsObject() { return this; }
    bool IsObject() const { return true; }

    virtual void cleanup()
    {
        std::set<Value *>::iterator it;
        for (it = m_to_delete.begin(); it != m_to_delete.end(); ++it)
            delete *it;
        m_to_delete.clear();
    }

protected:
    virtual void mark_for_deletion(Value * ptr)
    {
        m_to_delete.insert(ptr);
    }
    
    std::set<Value *> m_to_delete;
};



template <typename KEY, typename CONTAINER>
class Object_Template : public Object
{
public:
    Object_Template() {}
    virtual ~Object_Template();

    typedef KEY key_t;
    typedef CONTAINER container_t;
    // Iteration
    typedef typename container_t::iterator iterator;
    iterator begin() { return m_container.begin(); }
    iterator end()   { return m_container.end(); }
    // Size
    size_t size() const { return m_container.size(); }

    // Getters
    Value * Get(key_t key) { return get_value(find_err(key)); }
    Value * operator[](key_t key) { return Get(key); }
    bool Contains(key_t key) { return find(key) != end(); }

    const string_t & GetString(key_t key) { return Get(key)->AsString(); }
    const string_t & GetNumber(key_t key) { return Get(key)->AsNumber(); }
    bool GetBool(key_t key)     { return Get(key)->AsBool(); }
    bool GetNull(key_t key)     { return Get(key)->IsNull(); }
    Map * GetMap(key_t key)     { return Get(key)->AsMap(); }
    Array * GetArray(key_t key) { return Get(key)->AsArray(); }

    const string_t & GetString(key_t key, const string_t & def);
    const string_t & GetNumber(key_t key, const string_t & def);
    bool GetBool(key_t key, bool def);

    // Return a value and relinquish ownership of it.
    Value * Detach(key_t key);
    // Return a value and mark it for later deletion.
    Value * Pop(key_t key);

    const string_t & PopString(key_t key) { return Pop(key)->AsString(); }
    const string_t & PopNumber(key_t key) { return Pop(key)->AsNumber(); }
    bool PopBool(key_t key) { return Pop(key)->AsBool(); }
    bool PopNull(key_t key) { return Pop(key)->IsNull(); }
    Map * PopMap(key_t key) { return Pop(key)->AsMap(); }
    Array * PopArray(key_t key) { return Pop(key)->AsArray(); }

    const string_t & PopString(key_t key, const string_t & def);
    const string_t & PopNumber(key_t key, const string_t & def);
    bool PopBool(key_t key, bool def);

protected:
    // These functions *must* be specialized for each class
    // They're kind of like virtual functions.
    iterator find(key_t key);
    Value * get_value(iterator it);
    void throw_out_of_range(key_t key);

    iterator find_err(key_t key)
    {
        iterator it = find(key);
        if (it == end())
            throw_out_of_range(key);
        return it;
    }

	container_t m_container;
};




class Map
    : public Object_Template< const string_t &, std::map<string_t, Value*> >
{
public:
    Map() {}
    virtual ~Map () {}

    Map * AsMap() { return this; }
    bool IsMap() const { return true; }

    // Setters
    void Set(key_t key, Value * val);
};



class Array : public Object_Template< size_t, std::vector<Value*> >
{
public:
    Array() {}
    virtual ~Array () {}

    Array * AsArray() { return this; }
    bool IsArray() const { return true; }

    // Size
    void reserve(size_t count) { m_container.reserve(count); }

    // Get and set
    Value * at(size_t index) { return m_container.at(index); }
    void push_back(Value * val) { m_container.push_back(val); }
};


// --------------------------------------------------------------------------
// Object_Template methods
// --------------------------------------------------------------------------

template <typename KEY, typename CONTAINER>
inline Object_Template<KEY, CONTAINER>::~Object_Template()
{
    for (iterator it = begin(); it != end(); ++it)
        delete get_value(it);
}


template <typename KEY, typename CONTAINER>
inline const string_t &
Object_Template<KEY, CONTAINER>::GetString(KEY key, const string_t & def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        if (! val->IsNull())
            return val->AsString();
    }
    return def;
}

template <typename KEY, typename CONTAINER>
inline const string_t &
Object_Template<KEY, CONTAINER>::GetNumber(KEY key, const string_t & def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        if (! val->IsNull())
            return val->AsNumber();
    }
    return def;
}

template <typename KEY, typename CONTAINER>
inline bool
Object_Template<KEY, CONTAINER>::GetBool(KEY key, bool def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        if (! val->IsNull())
            return val->AsBool();
    }
    return def;
}

template <typename KEY, typename CONTAINER>
inline Value * Object_Template<KEY, CONTAINER>::Detach(KEY key)
{
    iterator it = find_err(key);
    Value * val = get_value(it);
    m_container.erase(it);
    return val;
}

// Return a value and mark it for later deletion.
template <typename KEY, typename CONTAINER>
inline Value * Object_Template<KEY, CONTAINER>::Pop(KEY key)
{
    Value * val = Detach(key);
    mark_for_deletion(val);
    return val;
}

template <typename KEY, typename CONTAINER>
inline const string_t &
Object_Template<KEY, CONTAINER>::PopString(KEY key, const string_t & def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        m_container.erase(it);
        mark_for_deletion(val);
        if (! val->IsNull())
            return val->AsString();
    }
    return def;
}

template <typename KEY, typename CONTAINER>
inline const string_t &
Object_Template<KEY, CONTAINER>::PopNumber(KEY key, const string_t & def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        m_container.erase(it);
        mark_for_deletion(val);
        if (! val->IsNull())
            return val->AsNumber();
    }
    return def;
}

template <typename KEY, typename CONTAINER>
inline bool
Object_Template<KEY, CONTAINER>::PopBool(KEY key, bool def)
{
    iterator it = find(key);
    if (it != end())
    {
        Value * val = get_value(it);
        m_container.erase(it);
        mark_for_deletion(val);
        if (! val->IsNull())
            return val->AsBool();
    }
    return def;
}

// --------------------------------------------------------------------------
// Map methods
// --------------------------------------------------------------------------

// template specialization for Object_Template:: find and get_value
template<> inline Map::iterator
Object_Template<Map::key_t, Map::container_t>::find(const string_t & key)
{
    return m_container.find(key);
}

template<> inline Value *
Object_Template<Map::key_t, Map::container_t>::get_value(Map::iterator it)
{
    return it->second;
}

template<> inline void
Object_Template<Map::key_t, Map::container_t>::throw_out_of_range(Map::key_t key)
{
    throw KeyError(encode_utf8(key));
}



// I think this method needs to be defined *after* the template
// specializations above.
inline void Map::Set(const string_t & key, Value * val)
{
    iterator it = find(key);
    if (it != end())
        mark_for_deletion(get_value(it));
    m_container[key] = val;
}


// --------------------------------------------------------------------------
// Array methods
// --------------------------------------------------------------------------
// template specialization for Object_Template:: find and get_value
template<> inline Array::iterator
Object_Template<Array::key_t, Array::container_t>::find(size_t index)
{
    if (index >= size())
        return end();
    return begin() + index;
}

template<> inline Value *
Object_Template<Array::key_t, Array::container_t>::get_value(Array::iterator it)
{
    return *it;
}

template<> inline void
Object_Template<Array::key_t, Array::container_t>::throw_out_of_range(size_t index)
{
    throw IndexError(index);
}

} // namespace json
} // namespace puz

#endif // PUZ_JSON_H