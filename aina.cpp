#include "aina.hpp"

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <functional>
#include <unordered_map>
#include <vector>
#include <numeric>

namespace aina {

enum cell_type : uint8_t {
    T_FREE,
    T_NIL,
    T_EOF_OBJECT,
    T_PAIR,
    T_SYMBOL,
    T_VECTOR,
    T_SLOT,
    T_ENVIRONMENT,
    T_PORT
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

        struct {
            std::vector<value_t> data;
        } vector;

        struct {
            FILE *file;
            int flags;
        } port;

    } object;
};

static auto & c_type(struct cell *c)
{ return c->type; }

static auto & c_uniq(struct cell *c)
{ return c->object.uniq; }

static auto & c_pair(struct cell *c)
{ return c->object.pair; }

static auto & c_car(struct cell *c)
{ return c_pair(c).car; }

static auto & c_cdr(struct cell *c)
{ return c_pair(c).cdr; }

static auto & c_symbol(struct cell *c)
{ return c->object.symbol; }

static auto & c_vector(struct cell *c)
{ return c->object.vector; }

static auto & c_port(struct cell *c)
{ return c->object.port; }

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
static value_t g_eof_object = make_unique_object("<#eof-object>", T_EOF_OBJECT);
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
    c_car(v) = a;
    c_cdr(v) = b;
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

bool is_vector(value_t v)
{
    return T_VECTOR == c_type(v);
}

value_t vector(size_t size, value_t fill)
{
    auto v = new_cell();
    c_type(v) = T_VECTOR;
    new (&c_vector(v).data) std::vector<value_t>(size, fill);
    return v;
}

bool is_slot(value_t v)
{
    return T_SLOT == c_type(v);
}

value_t slot(value_t name, value_t value)
{
    auto v = new_cell();
    c_type(v) = T_SLOT;
    c_car(v) = name;
    c_cdr(v) = value;
    return v;
}

bool is_environment(value_t v)
{
    return T_ENVIRONMENT == c_type(v);
}

value_t environment(value_t outer)
{
    auto v = new_cell();
    c_type(v) = T_ENVIRONMENT;
    c_car(v) = vector();
    c_cdr(v) = outer;
    return v;
}

bool is_port(value_t v)
{
    return T_PORT == c_type(v);
}

bool is_eof_object(value_t v)
{
    return g_eof_object == v;
}

value_t open_file(FILE *fd, int port_flags)
{
    auto v = new_cell();
    c_type(v) = T_PORT;
    c_port(v).file = fd;
    c_port(v).flags = port_flags;
    return v;
}

static int fpeekc(FILE *stream)
{
    int c = fgetc(stream);

    if (EOF != c)
        ungetc(c, stream);

    return c;
}

static int skipspace(FILE *stream)
{
    int c;

    while (isspace(c = fpeekc(stream)))
        fgetc(stream);

    return c;
}

static bool is_initial(int c)
{
    const char *special_initials = "!$%&*/:<=>?^_~@";
    return isalpha(c) || strchr(special_initials, c);
}

static bool is_subsequent(int c)
{
    const char *special_subsequent = "+-.@";
    return is_initial(c) || isdigit(c) || strchr(special_subsequent, c);
}

static value_t read_symbol(value_t port)
{
    std::string name;
    FILE *file = c_port(port).file;

    int c = fgetc(file);
    assert(is_initial(c));

    name += char(c);

    while (is_subsequent(fpeekc(file)))
        name += char(fgetc(file));

    return symbol(name.c_str());
}

static value_t read_list(value_t port)
{
    std::vector<value_t> elems;
    FILE *file = c_port(port).file;

    int c = fgetc(file);
    assert(c == '(');

    while (true) {
        int c = skipspace(file);

        if (')' == c) {
            fgetc(file);
            break;
        }

        elems.push_back(read(port));
    }

    auto result = nil();
    for (auto it = elems.crbegin(); it != elems.crend(); ++it)
        result = cons(*it, result);

    return result;
}

value_t read(value_t port)
{
    if (is_null(port))
        port = open_file(stdin, port_rdonly | port_noclose);

    FILE *stream = c_port(port).file;
    int c = skipspace(stream);

    if (is_initial(c))
        return read_symbol(port);
    
    switch (c) {
    case '(':
        return read_list(port);

    case EOF:
        return g_eof_object;
    }

    assert(false);
}

value_t display(value_t obj, value_t port)
{
    if (is_null(port))
        port = open_file(stdout, port_wronly | port_noclose);

    auto repr = to_string(obj);
    fprintf(c_port(port).file, "%s", repr.c_str());

    return port;
}

value_t newline(value_t port)
{
    if (is_null(port))
        port = open_file(stdout, port_wronly | port_noclose);

    fprintf(c_port(port).file, "\n");
    return port;
}

std::string to_string(value_t v)
{
    switch (c_type(v)) {
    case T_FREE:
        return "#<freed object>";

    case T_NIL:
    case T_EOF_OBJECT:
        return c_uniq(v).name;

    case T_PAIR:
        return "(" + to_string(c_car(v)) + " . " + to_string(c_cdr(v)) + ")";

    case T_SYMBOL:
        return c_symbol(v).name;

    case T_VECTOR:
        return "#("
               + std::accumulate(c_vector(v).data.cbegin(),
                                 c_vector(v).data.cend(),
                                 std::string{},
                                 [] (const std::string &acc, value_t elem) {
                                     if (acc.empty())
                                        return to_string(elem);
                                     else
                                        return acc + " " + to_string(elem);
                                 })
               + ")";        

    case T_SLOT:
        return "[" + to_string(c_car(v)) + " -> " + to_string(c_cdr(v)) + "]";

    case T_ENVIRONMENT:
        return "<#environment>";

    case T_PORT:
        return "<#port>";
    }

    return "<#unprintable type>";
}

} // namespace aina































