#include "core/server.hpp"

int main() {
    Server s("0.0.0.0", 8080);
    s.start();
}
