#include <sys/ioctl.h>
#include <unistd.h>
#include "wordWrap.h"
using namespace std;

static int screenWidth();

void wordWrap(stringstream &ss, ostream &os, int indentation)
{
    size_t width = screenWidth();
    int column = 0;
    string current;
    
    while (ss >> current) {
        if (column + current.size() >= width) {
            os << endl;
            column = 0;
        }

        if (column == 0) {
            for (int i = 0; i < indentation; i++)
                os << " ";

            column = indentation;
        }

        os << current << " ";
        column += current.size() + 1;
    }

    os << endl;
    ss.clear();
}

int screenWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}