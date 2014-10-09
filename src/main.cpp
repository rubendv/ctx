#include "ast.hpp"
#include <iostream>

int main(int argc, const char ** argv) {
    using namespace ctxlang;

    Scope root_scope;    
    root_scope.bindings["print"] = std::make_shared<BuiltinFunction>(
            Identifier("print"),
            std::vector<Identifier>({Identifier("expression")}),
            [](const std::vector<ValueRef>& arguments, Scope& scope) -> ValueRef {
                if(arguments.size() != 1) {
                    throw std::runtime_error("Expecting 1 argument");
                }
                ValueRef expression = arguments[0]->evaluate(scope);
                std::cout << *expression << std::endl;
                return std::make_shared<Tuple>(std::vector<ValueRef>());
            }
    );
    ValueRef expression = std::make_shared<Tuple>(std::vector<ValueRef>({std::make_shared<Identifier>("print"), std::make_shared<String>("Hello, world!")}));
    ValueRef result = expression->evaluate(root_scope);
    std::cout << *result << std::endl;

    return 0;
}
