//
// Created by edz on 2022/4/6.
//


#include "javah.h"
#include <getopt.h>


static void usage(const char *argv0) noexcept
{
    printf("usage: %s [options] <class_file_path>\n", argv0);
    printf("[options]:\n");
    printf("  -e --empty                 print nothing if no native methods found\n");
    printf("  -m <mode> --mode <mode>    explicit the output format:\n");
    printf("    list                     just list all native methods\n");
    printf("    h                        output format is .h\n");
    printf("    cpp                      output format is .cpp\n");
    printf("  -o --output <file_path>    sava as\n");
}

static constexpr struct option options[] = {
        { "empty", no_argument, nullptr, 'e' },
        { "mode", required_argument, nullptr, 'm' },
        { "output", required_argument, nullptr, 'o' },
        {nullptr, 0, nullptr, 0 },
};

static int do_main(args_t &) noexcept;


int main(int argc, char *argv[])
{
    args_t args;

    int ch;
    while ((ch = getopt_long(argc, argv, "em:o:", options, nullptr)) != -1) {
        switch (ch) {
            case 'e':
                args.return_if_empty = 1;
                break;
            case 'm':
                if ((args.mode = args_t::get_mode(optarg)) == args_t::none) {
                    fprintf(stderr, "invalid [mode] option\n");
                    return 1;
                }
                break;
            case 'o':
                args.output = optarg;
                break;
            default:
                return 1;
        }
    }
    if (optind < 0 || optind >= argc) {
        usage(argv[0]);
        return 1;
    }
    args.file = argv[optind];
    return do_main(args);
}


static int do_main(args_t &args) noexcept
{
    context_t context;
    if (context.open(args.file) != 0) {
        return 1;
    }

    if (args.return_if_empty && context.methods.empty()) {
        return 0;
    }

    // 打开输出文件
    FILE *fout = stdout;

    if (args.output != nullptr && args.output[0] != '\0') {
        fout = fopen(args.output, "wb");
        if (fout == nullptr) {
            fprintf(stderr, "failed to open output file '%s'\n", args.output);
            return 3;
        }
    }

    switch (args.mode) {
        case args_t::h: context.print_as_h(fout); break;
        case args_t::cpp: context.print_as_cpp(fout); break;
        case args_t::list: context.print_as_list(fout); break;
        default: break;
    }

    if (fout != stdout) {
        fclose(fout);
    }
    return 0;
}