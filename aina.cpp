#include "aina.hpp"

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <functional>
#include <unordered_map>

namespace aina {

enum cell_type : uint8_t {
    T_FREE,
    T_NIL,
    T_PAIR,
    T_SYMBOL,
};

struct cell {
    cell_type type;

    union {

        struct {
            const char *name;
            size_t len;
        } uniq;

        struct {
            value_t car, cdr;
        } pair;

        struct {
            const char *name;
            size_t len, hash;
        } symbol;

    } object;
};

static auto & c_type(struct cell *c)
{ return c->type; }

static auto & c_uniq(struct cell *c)
{ return c->object.uniq; }

static auto & c_pair(struct cell *c)
{ return c->object.pair; }

static auto & c_symbol(struct cell *c)
{ return c->object.symbol; }

static value_t make_unique_object(const char *name, cell_type type)
{
    auto v = static_cast<value_t>(calloc(1, sizeof(struct cell)));
    c_type(v) = type;
    c_uniq(v).name = strdup(name);
    c_uniq(v).len = strlen(name);
    return v;
}

static value_t new_cell()
{
    return static_cast<value_t>(calloc(1, sizeof(struct cell)));
}

static value_t g_nil = make_unique_object("()", T_NIL);
static std::unordered_map<size_t, value_t> g_symbols;

bool is_null(value_t v)
{
    return v == g_nil;
}

value_t nil()
{
    return g_nil;
}

bool is_pair(value_t v)
{
    return T_PAIR == c_type(v);
}

value_t cons(value_t a, value_t b)
{
    auto v = new_cell();
    c_type(v) = T_PAIR;
    c_pair(v).car = a;
    c_pair(v).cdr = b;
    return v;
}

bool is_symbol(value_t v)
{
    return T_SYMBOL == c_type(v);
}

value_t symbol(const char *name)
{
    const std::string str(name);
    const size_t hashval = std::hash<std::string>{}(str);

    auto it = g_symbols.find(hashval);
    if (it != g_symbols.end())
        return it->second;

    auto v = new_cell();
    c_type(v) = T_SYMBOL;
    c_symbol(v).name = strdup(name);
    c_symbol(v).len = str.length();
    c_symbol(v).hash = hashval;

    g_symbols[hashval] = v;
    return v;
}


} // namespace aina































