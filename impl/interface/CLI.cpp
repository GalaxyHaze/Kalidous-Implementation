
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

#include "Nova/nova.h"

const char* nova_version = NOVA_VERSION;

// ============================================================================
// Helpers internos (C++ apenas)
// ============================================================================

static void print_not_implemented(const char* command) {
    std::cerr << "\n⚠️  Command '" << command << "' is not implemented yet.\n";
    std::cerr << "   This feature will be available in a future version of Nova.\n";
    std::cerr << "   Track progress at: https://github.com/your-username/Nova\n\n";
}

static void print_success(const char* action, const char* target) {
    std::cout << "✅ " << action << " successful: " << target << "\n";
}

static void print_error(const char* msg) {
    std::cerr << "❌ Error: " << msg << "\n";
}

// ============================================================================
// Comandos (cada um é uma função independente)
// ============================================================================

static int cmd_compile(const std::string& input_file, const std::string& output_file,
                       const std::string& mode_str, bool verbose) {
    if (verbose) {
        std::cout << "🔍 Compiling '" << input_file << "' in " << mode_str << " mode...\n";
    }

    NovaArena* arena = nova_arena_create(64 * 1024);
    if (!arena) {
        print_error("Failed to create memory arena");
        return 1;
    }

    size_t file_size = 0;
    const char* source = nova_load_file_to_arena(arena, input_file.c_str(), &file_size);
    if (!source) {
        print_error(("Failed to load file: " + input_file).c_str());
        nova_arena_destroy(arena);
        return 1;
    }

    const NovaTokenStream tokens = nova_tokenize(arena, source, file_size);
    if (!tokens.data) {
        // nova_tokenize aborts on error, but just in case:
        nova_arena_destroy(arena);
        return 1;
    }

    if (verbose) {
        std::cout << "✨ Tokenized " << tokens.len << " tokens\n";
    }

    /*if (const auto ast = nova_parse(arena, tokens); !ast) {
        print_error("AST parsing failed");
        nova_arena_destroy(arena);
        return 1;
    }

    if (verbose) {
        std::cout << "🌳 AST built successfully\n";
    }*/

    // TODO: Code generation, LLVM IR emission, etc.
    // For now, just simulate success
    nova_arena_destroy(arena);

    const char* out = output_file.empty() ? "a.out" : output_file.c_str();
    print_success("Compilation", out);
    return 0;
}

static int cmd_run(const std::string& input_file, const std::string& mode_str, bool verbose) {
    print_not_implemented("run");
    std::cerr << "💡 Tip: Use 'nova compile' followed by './output' for now.\n";
    return 1;
}

static int cmd_test(const std::string& input_file, bool verbose) {
    print_not_implemented("test");
    std::cerr << "💡 Tip: Write tests manually and compile with 'nova compile'.\n";
    return 1;
}

static int cmd_fmt(const std::string& input_file, bool check_only, bool verbose) {
    print_not_implemented("fmt");
    std::cerr << "💡 Tip: Use your editor's formatter for now (e.g., clang-format).\n";
    return 1;
}

static int cmd_check(const std::string& input_file, bool verbose) {
    // Reuse compile logic but stop after semantic analysis
    return cmd_compile(input_file, "", "debug", verbose);
}

static int cmd_docs(const std::string& input_file, const std::string& output_dir, bool verbose) {
    print_not_implemented("docs");
    std::cerr << "💡 Tip: Write documentation in comments for now.\n";
    return 1;
}

static int cmd_repl(bool verbose) {
    print_not_implemented("repl");
    std::cerr << "💡 Tip: Use 'nova run' with small scripts for quick testing.\n";
    return 1;
}

static int cmd_version() {
    std::cout << "Nova Programming Language\n";
    std::cout << "Version: " << nova_version << "\n";
    std::cout << "Compiler: " << __VERSION__ << "\n";
    std::cout << "Target: " << __GNUG__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
    return 0;
}

static int cmd_help() {
    // CLI11 will handle --help automatically, but we provide a custom banner
    std::cout << R"(Nova - A low-level general-purpose language

USAGE:
    nova [OPTIONS] <COMMAND> [ARGS]

COMMANDS:
    compile    Compile source file to binary (default)
    run        Compile and run immediately
    test       Run tests in source file
    fmt        Format source code
    check      Type-check without code generation
    docs       Generate documentation
    repl       Start interactive REPL
    version    Show version information
    help       Show this help message

OPTIONS:
    -m, --mode <debug|dev|release|fast|test>    Build mode [default: debug]
    -o, --output <FILE>                         Output file [default: a.out]
    -I, --include <DIR>                         Add include directory
    --emit <ast|ir|asm|obj|bin>                Emit intermediate representation
    --target <TRIPLE>                           Target triple (e.g., x86_64-linux-gnu)
    -v, --verbose                               Verbose output
    -h, --help                                  Show help
    --version                                   Show version

EXAMPLES:
    nova compile main.nova -o main
    nova run script.nova --mode release
    nova fmt --check src/
    nova check lib.nova -v

LEARN MORE:
    Source: https://github.com/GalaxyHaze/Nova
    Docs:   https://nova-lang.dev (coming soon)
)";
    return 0;
}

// ============================================================================
// Dispatch central: nova_run (C API)
// ============================================================================

extern "C" int nova_run(const int argc, const char* argv[]) {
    CLI::App app{"Nova - A low-level general-purpose language"};

    // Global options (apply to all commands)
    std::string mode_str = "debug";
    std::string output_file;
    std::vector<std::string> include_dirs;
    std::string emit_target;
    std::string target_triple;
    bool verbose = false;

    app.add_option("-m,--mode", mode_str,
        "Build mode: debug, dev, release, fast, test")
       ->transform(CLI::IsMember({"debug", "dev", "release", "fast", "test"}))
       ->default_str("debug");

    app.add_option("-o,--output", output_file, "Output file path")
       ->default_str("a.out");

    app.add_option("-I,--include", include_dirs, "Include directories for imports");

    app.add_option("--emit", emit_target,
        "Emit intermediate representation: ast, ir, asm, obj, bin");

    app.add_option("--target", target_triple, "Target triple (e.g., x86_64-linux-gnu)");

    app.add_flag("-v,--verbose", verbose, "Verbose output");

    // Subcomandos
    auto* compile_cmd = app.add_subcommand("compile", "Compile source file to binary (default)");
    auto* run_cmd = app.add_subcommand("run", "Compile and run immediately");
    auto* test_cmd = app.add_subcommand("test", "Run tests in source file");
    auto* fmt_cmd = app.add_subcommand("fmt", "Format source code");
    auto* check_cmd = app.add_subcommand("check", "Type-check without code generation");
    auto* docs_cmd = app.add_subcommand("docs", "Generate documentation");
    auto* repl_cmd = app.add_subcommand("repl", "Start interactive REPL");
    auto* version_cmd = app.add_subcommand("version", "Show version information");
    auto* help_cmd = app.add_subcommand("help", "Show help message");

    // Argumentos por comando
    std::string input_file;
    bool fmt_check = false;
    std::string docs_output = "docs";

    compile_cmd->add_option("input", input_file, "Input source file (.nova)")
                ->required()
                ->check(CLI::ExistingFile);

    run_cmd->add_option("input", input_file, "Input source file (.nova)")
           ->required()
           ->check(CLI::ExistingFile);

    test_cmd->add_option("input", input_file, "Input source file (.nova)")
            ->required()
            ->check(CLI::ExistingFile);

    fmt_cmd->add_option("input", input_file, "Input source file or directory")
           ->required();
    fmt_cmd->add_flag("--check", fmt_check, "Check formatting without modifying files");

    check_cmd->add_option("input", input_file, "Input source file (.nova)")
             ->required()
             ->check(CLI::ExistingFile);

    docs_cmd->add_option("input", input_file, "Input source file (.nova)")
            ->required()
            ->check(CLI::ExistingFile);
    docs_cmd->add_option("-o,--output", docs_output, "Output directory for docs")
            ->default_str("docs");

    // Parse
    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp&) {
        cmd_help();
        return 0;
    } catch (const CLI::CallForVersion&) {
        return cmd_version();
    } catch (const CLI::ParseError& e) {
        if (e.get_name() == "ExtrasError" && argc > 1 && std::string(argv[1]) == "help") {
            return cmd_help();
        }
        std::cerr << "❌ CLI Error: " << e.what() << "\n\n";
        std::cerr << app.help();
        return 1;
    }

    // Dispatch
    if (*help_cmd) return cmd_help();
    if (*version_cmd) return cmd_version();
    if (*repl_cmd) return cmd_repl(verbose);
    if (*docs_cmd) return cmd_docs(input_file, docs_output, verbose);
    if (*fmt_cmd) return cmd_fmt(input_file, fmt_check, verbose);
    if (*test_cmd) return cmd_test(input_file, verbose);
    if (*run_cmd) return cmd_run(input_file, mode_str, verbose);
    if (*check_cmd) return cmd_check(input_file, verbose);
    if (*compile_cmd || app.get_subcommands().empty()) {
        // Default command: compile
        if (input_file.empty() && argc > 1) {
            // Support positional argument without "compile" subcommand
            input_file = argv[1];
            if (!nova_file_exists(input_file.c_str())) {
                print_error(("File not found: " + input_file).c_str());
                return 1;
            }
        }
        return cmd_compile(input_file, output_file, mode_str, verbose);
    }

    print_error("No command specified");
    std::cout << app.help();
    return 1;
}