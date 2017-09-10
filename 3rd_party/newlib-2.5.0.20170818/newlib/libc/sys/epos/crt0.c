#include <fcntl.h>

extern void exit(int code);
extern int main (int argc, int argv);

void _start(int argc, int argv) {

    int ex = main(argc, argv);
    exit(ex);
}
