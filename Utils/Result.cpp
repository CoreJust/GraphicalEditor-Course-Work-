#include "Result.h"

using namespace utils;

Failure::Failure(QString msg)
	: message(std::move(msg)) {

}
