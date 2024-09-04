#pragma once
#include "../Utils/Result.h"

// Compiles a script into a compiled bytecode file
class Compiler final {
public:
	Compiler() = default;

	utils::Result<Void> compile(QString scriptFilePath);
};