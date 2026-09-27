#include <iostream>

#include "onnxcc/cli.h"

int main(int argc, char** argv) {
    return onnxcc::run(argc, argv, std::cout, std::cerr);
}