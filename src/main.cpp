#include <Main/Main.h>
#include <Ty/ErrorOr.h>
#include <CLI/ArgumentParser.h>
#include <Core/File.h>
#include <Core/MappedFile.h>

#include "./Source.h"
#include "./Lex.h"
#include "./Parse.h"
#include "./Codegen.h"

ErrorOr<int> Main::main(int argc, c_string argv[])
{
    auto argument_parser = CLI::ArgumentParser();

    auto input_path = StringView();
    TRY(argument_parser.add_positional_argument("input-path", [&](c_string arg) {
        input_path = StringView::from_c_string(arg);
    }));

    auto output_path = StringView();
    TRY(argument_parser.add_option("--output", "-o", "path", "output path", [&](c_string arg) {
        output_path = StringView::from_c_string(arg);
    }));

    bool verbose = false;
    TRY(argument_parser.add_flag("--verbose", "-v", "print verbose output", [&] {
        verbose = true;
    }));

    if (auto result = argument_parser.run(argc, argv); result.is_error()) {
        TRY(result.error().show());
        return 1;
    }

    if (output_path.is_empty()) {
        output_path = "a.cpp"sv;
    }

    auto& stderr = Core::File::stderr();
    if (verbose) {
        TRY(stderr.writeln("input_path: "sv, input_path));
        TRY(stderr.writeln("output_path: "sv, output_path));
    }

    auto input_file = TRY(Core::MappedFile::open(input_path));

    auto source = Source(input_path, input_file.view());
    auto tokens = TRY(lex(source));
    auto tree = TRY(parse(source, tokens.view()));
    auto code = TRY(codegen(source, tree));

    if (output_path == "-"sv) {
        TRY(Core::File::stdout().write(code.view()));
        return 0;
    }
    auto output_file = TRY(Core::File::open_for_writing(output_path, O_TRUNC));
    TRY(output_file.write(code.view()));

    return 0;
}

