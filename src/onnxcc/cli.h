#pragma once
#include <ostream>
//runs the onnxcc cli
//   argc, argv : the command line exactly as main() receives it
//                (argv[0] is the program name).
//   out        : normal output   (std::cout in the real program).
//   err        : error messages  (std::cerr in the real program).
//
// Returns the process exit code:
//   0 = success, 1 = runtime failure, 2 = usage error.

namespace onnxcc {
    [[nodiscard]] int run(int argc, const char* const* argv, std::ostream& out,std::ostream& err);
}