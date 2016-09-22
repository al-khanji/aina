#ifndef _AINA_HPP_
#define _AINA_HPP_

namespace aina {

typedef struct cell *value_t;

bool is_null(value_t v);
value_t nil();

bool is_pair(value_t v);
value_t cons(value_t a, value_t b);

bool is_symbol(value_t v);
value_t symbol(const char *name);

} // namespace aina

#endif // _AINA_HPP_
