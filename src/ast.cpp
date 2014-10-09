#include "ast.hpp"
using namespace ctxlang;

std::ostream& ctxlang::operator<<(std::ostream& os, const Value& value) {
    value.print(os);
    return os;
}

boost::optional<ValueRef> Scope::lookup(const Identifier& identifier) const {
    auto it = bindings.find(identifier.name);
    if(it != bindings.end()) {
        return it->second;
    } else if(parent != nullptr) {
        return parent->lookup(identifier.name);
    } else {
        return boost::none;
    }
}
