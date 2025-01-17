#include <rose.h>
#include <rosePublicConfig.h>
#include <Sawyer/CommandLine.h>
#include <BinarySymbolicExprParser.h>

#ifdef ROSE_HAVE_LIBREADLINE
# include <readline/readline.h>
# include <readline/history.h>
#else
# include <rose_getline.h>
#endif

using namespace rose::BinaryAnalysis;

static SymbolicExprParser
symbolicParser() {
    return SymbolicExprParser();
}

static void
parseCommandLine(int argc, char *argv[]) {
    using namespace Sawyer::CommandLine;
    Parser parser;
    parser
        .purpose("test symbolic simplification")
        .version(std::string(ROSE_SCM_VERSION_ID).substr(0, 8), ROSE_CONFIGURE_DATE)
        .chapter(1, "ROSE Command-line Tools")
        .doc("Synopsis", "@rop{programName} [@v{switches}]")
        .doc("Description",
             "Parses symbolic expressions from standard input and prints the resulting expression trees. These trees "
             "undergo basic simplifications in ROSE before they're printed.")
        .doc("Symbolic expression syntax", symbolicParser().docString())
        .with(CommandlineProcessing::genericSwitches());

    if (!parser.parse(argc, argv).apply().unreachedArgs().empty())
        throw std::runtime_error("incorrect usage; see --help");
}

int
main(int argc, char *argv[]) {
    parseCommandLine(argc, argv);
    unsigned lineNumber = 0;
    while (1) {

        // Prompt for and read a line containing one expression
        ++lineNumber;
#ifdef ROSE_HAVE_LIBREADLINE
        char *line = readline("> ");
        if (!line)
            break;
        add_history(line);
#else
        char *line = NULL;
        size_t linesz = 0;
        printf("> ");
        ssize_t nread = rose_getline(&line, &linesz, stdin);
        if (nread <= 0)
            break;
        while (nread > 0 && isspace(line[nread-1]))
            line[--nread] = '\0';
#endif

        // Parse the expression
        try {
            SymbolicExpr::Ptr expr = symbolicParser().parse(line);
            std::cout <<"Parsed value = " <<*expr <<"\n\n";
        } catch (const SymbolicExprParser::SyntaxError &e) {
            std::cerr <<e <<"\n";
            if (e.lineNumber != 0) {
                std::cerr <<"    input: " <<line <<"\n"
                          <<"    here---" <<std::string(e.columnNumber, '-') <<"^\n\n";
            }
        } catch (const rose::FailedAssertion &e) {
            std::cerr <<"\n"; // message has already been printed.
        }

        free(line);
    }
}
