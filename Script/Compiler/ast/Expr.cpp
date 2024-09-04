#include "Expr.h"

Expr::Expr(Type type)
	: m_type(std::move(type)) {

}

bool Expr::isConstNumber() {
    return false;
}

const Type& Expr::getType() {
	return m_type;
}

Type getCommonType(const Type& a, const Type& b) {
	if (a.size() != 1 || b.size() != 1) {
		throw QString("Cannot get a common type of tuples");
	}

	if (int(a[0]) > int(b[0])) {
		return a;
	}

	return b;
}
