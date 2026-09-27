#include "onnxcc/cli.h"
#include <cxxopts.hpp>
#include<filesystem>
#include<ostream>
#include<string>
#include <string_view>
#include<system_error>
namespace onnxcc{
    namespace{
        constexpr int kExitOk =0;
        constexpr int kExitFailure =1;
        constexpr int kExitUsage =2;

        void print_usage(std::ostream& os){
            os<< "Usage: onnxcc <subcommand> [options]\n"
                 "\n"
                 "Subcommands:\n"
                 " dump    Inspect an ONNX model\n"
                 "\n"
                 "Run 'onnxcc <subcommand> --help' for more information on a subcommand.\n";
        }
        int run_dump(int argc, const char* const* argv,std::ostream& out, std::ostream& err){
            cxxopts::Options options("onnxcc dump","Inspect an ONNX model");
            options.add_options()
                ("m,model","Path to the ONNX model file", cxxopts::value<std::string>(),"PATH")
                ("show-graph","Print the graph structure")
                ("verbose","Print the extra details")
                ("h,help","Show this help message");
            try{
                const auto result = options.parse(argc, argv);
                if (result.count("help") > 0) {
                     out << options.help();
                     return kExitOk;
                }
                if(!result.unmatched().empty()){
                    err << "onnxcc dump: unexpected argument '"
                        << result.unmatched().front() << "'\n"
                        << "Try 'onnxcc dump --help'.\n";
                    return kExitUsage;
                    
                }
                if (result.count("model") == 0){
                    err << "onnxcc dump: missing required option --model\n"
                        <<"Try 'onnxcc dump --help'.\n";
                    return kExitUsage;
                }
                const auto model= result["model"].as<std::string>();
                std::error_code ec;
                if (!std::filesystem::is_regular_file(model, ec)){
                    err << "onnxcc dump: cannot open model file: " << model << "\n";
                    return kExitFailure;


                }

                out<<"model: "<<model<<"\n";
                if (result.count("verbose") > 0){
                    out << "verbose: on\n";

                }
                if (result.count("show-graph") > 0){
                    out << "graph: (not implemented yet)\n";

                }
                return kExitOk;
            }
            catch(const cxxopts::exceptions::exception& e){
                err << "onnxcc dump: " << e.what() << "\n"
                    <<"Try 'onnxcc dump --help'.\n";
                return kExitUsage;

            }
        }
    }//namespace
    int run(int argc, const char* const* argv,std::ostream& out, std::ostream& err){
        if(argc<2){
            print_usage(err);
            return kExitUsage;
        }
        const std::string_view sub = argv[1];
        if(sub=="-h" || sub=="--help"){
            print_usage(out);
            return kExitOk;
        }
        if(sub=="dump"){
            return run_dump(argc - 1, argv + 1, out, err);

        }
        if(sub.starts_with('-')){
            err << "onnxcc: expected a subcommand before options, got '" << sub << "'\n";
        }
        else{
            err << "onnxcc: unknown subcommand '" << sub << "'\n";

        }

        print_usage(err);
        return kExitUsage;
    }

}//namespace onnxcc