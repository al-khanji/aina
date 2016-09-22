#include <stdio.h>
#include "aina.hpp"
using namespace aina;

int main(int argc, char *argv[])
{
    if (argc > 1) {
        auto inport = open_file(fopen(argv[1], "r"), port_rdonly);
        while (true) {
            auto exp = read(inport);

            if (is_eof_object(exp))
                break;
        }
    } else {
        auto inport = open_file(stdin, port_rdonly | port_noclose);
        auto outport = open_file(stdout, port_wronly | port_noclose);

        while (true) {
            fprintf(stdout, ":: ");
            fflush(stdout);

            auto exp = read(inport);

            if (is_eof_object(exp))
                break;

            display(exp, outport);
            newline(outport);
        }
    }

    return 0;
}
