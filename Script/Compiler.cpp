#include "Compiler.h"
#include <QFile>
#include "Compiler/Lexer.h"
#include "Compiler/Parser.h"
#include "Compiler/ByteCodeBuilder.h"

utils::Result<Void> Compiler::compile(QString scriptFilePath) {
    // Checking for correct file extension
    if (!scriptFilePath.endsWith(".cw2")) {
        return utils::Failure("Not a CW2 script file: " + scriptFilePath);
    }

    // Opening the file
    QFile file(scriptFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return utils::Failure("Failed to open file: " + scriptFilePath);
    }

    g_builder = ByteCodeBuilder();

    // Reading the file
    std::string program = file.readAll().toStdString();

    // Splitting the script's text into tokens
    utils::Result<QList<Token>> tokens = Lexer(std::move(program)).tokenize();

    if (!tokens.isOk()) {
        return tokens.extractError();
    }

    // Parsing the tokens into AST
    utils::Result<std::vector<std::unique_ptr<State>>> ast = Parser(tokens.extract()).parse();

    if (!ast.isOk()) {
        return ast.extractError();
    }

    // Compiling ast into bytecode
    try {
        auto astVec = ast.extract();
        for (auto& state : astVec) {
            state->generate();
        }

        g_builder.addInst(Instruction(InstructionType::HALT));
    } catch (QString error) {
        return utils::Failure(error);
    }

    // Processing the bytecode and storing it to file
    g_builder.buildAndStore(scriptFilePath + "c");

    return utils::Success();
}
