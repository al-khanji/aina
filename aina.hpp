#ifndef _AINA_HPP_
#define _AINA_HPP_

#include <stdlib.h>
#include <stdio.h>

#include <string>

namespace aina {

typedef struct cell *value_t;

enum {
    port_rdonly  = 0x01,
    port_wronly  = 0x02,
    port_rdwr    = port_rdonly | port_wronly,
    port_noclose = 0x04
};

bool is_null(value_t v);
value_t nil();

bool is_pair(value_t v);
value_t cons(value_t a, value_t b);

bool is_symbol(value_t v);
value_t symbol(const char *name);

bool is_vector(value_t v);
value_t vector(size_t size = 0, value_t fill = nil());

bool is_slot(value_t v);
value_t slot(value_t name, value_t value);

bool is_environment(value_t v);
value_t environment(value_t outer = nil());

bool is_port(value_t v);
bool is_eof_object(value_t v);
value_t open_file(FILE *fd, int port_flags);
value_t read(value_t port = nil());
value_t display(value_t obj, value_t port = nil());
value_t newline(value_t port = nil());

std::string to_string(value_t v);

} // namespace aina

#endif // _AINA_HPP_
