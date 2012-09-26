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

#include "json.hpp"
#include "puzstring.hpp"
#include <yajl/yajl_parse.h>
#include <stack>

namespace puz {
namespace json {

static const int BUFF_SIZE = 1024;

class ParserData
{
public:
    ParserData() : root(NULL) {}
    ~ParserData() { delete root; }

    void AddValue(Value * val)
    {
        if (root == NULL)
        {
            root = val;
        }
        else if (key.empty())
        {
            valueStack.top()->AsArray()->push_back(val);
        }
        else
        {
            valueStack.top()->AsMap()->Set(key, val);
            key.clear();
        }
        if (val->IsObject())
            valueStack.push(val);
    }

    void OnContainerEnd()
    {
        valueStack.pop();
    }

    void SetKey(const string_t & k)
    {
        key = k;
    }

    Value * GetRoot() { return root; }
    Value * ReleaseRoot() { Value * tmp = root; root = NULL; return tmp; }

protected:
    Value * root;
    std::stack<Value *> valueStack;
    string_t key;
};

// ----------------------------------------------------------------------------
// yajl Callback functions
// ----------------------------------------------------------------------------

// Yeah, this macro makes things look a little funny, but I think
// it also makes things more readable.
#define obj ((ParserData*)ctx)

#define make_string(val, len) decode_utf8(std::string((const char*)val, len))

int on_null(void * ctx)
{
    obj->AddValue(new Null);
    return 1;
}

int on_bool(void * ctx, int boolVal)
{
    obj->AddValue(new Bool(boolVal != 0));
    return 1;
}

int on_number(void * ctx, const char * numberVal, size_t numberLen)
{
    obj->AddValue(new Number(make_string(numberVal, numberLen)));
    return 1;
}

int on_string(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
    obj->AddValue(new String(make_string(stringVal, stringLen)));
    return 1;
}

int on_map_start(void * ctx)
{
    obj->AddValue(new Map);
    return 1;
}

int on_map_key(void * ctx, const unsigned char * key, size_t stringLen)
{
    obj->SetKey(make_string(key, stringLen));
    return 1;
}

int on_map_end(void * ctx)
{
    obj->OnContainerEnd();
    return 1;
}

int on_array_start(void * ctx)
{
    obj->AddValue(new Array);
    return 1;
}

int on_array_end(void * ctx)
{
    obj->OnContainerEnd();
    return 1;
}

#undef make_string
#undef obj

Value * ParseJSON(std::istream & stream)
{
    // The callbacks array
    yajl_callbacks cb = {
        on_null,
        on_bool,
        NULL,
        NULL,
        on_number,
        on_string,
        on_map_start,
        on_map_key,
        on_map_end,
        on_array_start,
        on_array_end
    };

    // Create the json parser
    ParserData data;
    yajl_handle p = yajl_alloc(&cb, NULL, &data);
    yajl_config(p, yajl_allow_trailing_garbage, 1);

    try
    {
        for (;;)
        {
            unsigned char buff[BUFF_SIZE];
            stream.read((char*)(buff), BUFF_SIZE);
            const int bytes_read = stream.gcount();
            yajl_status status = yajl_parse(p, buff, bytes_read);
            if (status != yajl_status_ok)
            {
                if (! data.GetRoot()) // We didn't parse anything
                    throw FileTypeError("json");
                else // This is json, but not well formed
                    throw LoadError(
                        (char *)yajl_get_error(p, 0, buff, BUFF_SIZE));
            }
            if (bytes_read == 0)
            {
                yajl_complete_parse(p);
                break;
            }
        }
    }
    catch (...)
    {
        yajl_free(p);
        throw;
    }
    yajl_free(p);
    // Release ownership of the root pointer and return it
    if (! data.GetRoot())
        throw FileTypeError("json");
    return data.ReleaseRoot();
}

void Parser::LoadPuzzle(Puzzle * puz, std::istream & stream)
{
    std::auto_ptr<Value> root(ParseJSON(stream));
    // JSON document errors will be LoadErrors
    try {
        bool ptr_is_owned = DoLoadPuzzle(puz, root.get());
        if (ptr_is_owned)
        {
            if (root->IsObject()) 
                root->AsObject()->cleanup();
            // The parser subclass owns the pointer
            root.release();
        }
    }
    catch (BaseError & e) {
        throw LoadError(e.what());
    }
}

} // namespace json
} // namespace puz