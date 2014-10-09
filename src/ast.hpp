#pragma once
#include <memory>
#include <initializer_list>
#include <iostream>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <map>
#include <functional>
#include <sstream>

namespace ctxlang {
    class Identifier;
    
    class Location {
    public:
        size_t byte_start;
        size_t byte_end;
        Location(size_t byte_start, size_t byte_end) : byte_start(byte_start), byte_end(byte_end) {}
    };

    class Value;
    typedef std::shared_ptr<const Value> ValueRef;

    class Scope {
    public:
        std::shared_ptr<Scope> parent;
        std::map<std::string, ValueRef> bindings;
        Scope(std::shared_ptr<Scope> parent=nullptr) : parent(parent) {}
        boost::optional<ValueRef> lookup(const Identifier& identifier) const;
    };
    
    class Value {
    public:
        virtual void print(std::ostream& os) const {
            throw std::runtime_error("Print not implemented");
        }
        virtual ValueRef evaluate(Scope& scope) const {
            throw std::runtime_error("Evaluate not implemented");
        }
        virtual ValueRef call(const std::vector<ValueRef>& arguments, Scope& scope) const {
            throw std::runtime_error("Not callable");
        }
        virtual bool is_identifier() const {
            return false;
        }
    };

    std::ostream& operator<<(std::ostream& os, const Value& value);

    class Expression : public Value {
    public:
        boost::optional<Location> location;
        Expression(const Location& location) : location(location) {}
        Expression() : location(boost::none) {}
    };

    class Atom : public Expression {
    public:
        
    };

    class Integer : public Atom {
    public:
        size_t bits;
        bool is_signed;
        union {
            int8_t int8;
            int16_t int16;
            int32_t int32;
            int64_t int64;
            uint8_t uint8;
            uint16_t uint16;
            uint32_t uint32;
            uint64_t uint64;
        } value;
        Integer(int8_t value) : bits(8), is_signed(true) { this->value.int8 = value; }
        Integer(int16_t value) : bits(16), is_signed(true) { this->value.int16 = value; }
        Integer(int32_t value) : bits(32), is_signed(true) { this->value.int32 = value; }
        Integer(int64_t value) : bits(64), is_signed(true) { this->value.int64 = value; }
        Integer(uint8_t value) : bits(8), is_signed(false) { this->value.uint8 = value; }
        Integer(uint16_t value) : bits(16), is_signed(false) { this->value.uint16 = value; }
        Integer(uint32_t value) : bits(32), is_signed(false) { this->value.uint32 = value; }
        Integer(uint64_t value) : bits(64), is_signed(false) { this->value.uint64 = value; }

        virtual void print(std::ostream& os) const {
            if(is_signed) {
                switch(bits) {
                    case 8:
                        os << value.int8;
                        break;
                    case 16:
                        os << value.int16;
                        break;
                    case 32:
                        os << value.int32;
                        break;
                    case 64:
                        os << value.int64;
                        break;
                    default:
                        throw std::runtime_error("Invalid number of bits");
                }
            } else {
                switch(bits) {
                    case 8:
                        os << value.uint8;
                        break;
                    case 16:
                        os << value.uint16;
                        break;
                    case 32:
                        os << value.uint32;
                        break;
                    case 64:
                        os << value.uint64;
                        break;
                    default:
                        throw std::runtime_error("Invalid number of bits");
                }
            }
        }
        virtual ValueRef evaluate(Scope& scope) const {
            return std::make_shared<Integer>(*this);
        }
    };

    class FloatingPoint : public Atom {
    public:
        size_t bits;
        union {
            float float32;
            double float64;
        } value;
        FloatingPoint(float value) : bits(32) { this->value.float32 = value; }
        FloatingPoint(double value) : bits(64) { this->value.float64 = value; }

        virtual void print(std::ostream& os) const {
            switch(bits) {
                case 32:
                    os << value.float32;
                    break;
                case 64:
                    os << value.float64;
                    break;
                default:
                    throw std::runtime_error("Invalid number of bits");
            }
        }
        virtual ValueRef evaluate(Scope& scope) const {
            return std::make_shared<FloatingPoint>(*this);
        }
    };

    class String : public Atom {
    public:
        std::string contents;
        String(const std::string& contents) : contents(contents) {}
        virtual void print(std::ostream& os) const {
            // TODO: properly escape string contents
            os << "\"" << contents << "\"";
        }
        virtual ValueRef evaluate(Scope& scope) const {
            return std::make_shared<String>(*this);
        }
    };

    class Keyword : public Atom {
    public:
        std::string name;
        Keyword(const std::string& name) : name(name) {}
        virtual void print(std::ostream& os) const {
            os << ":" << name;
        }
        virtual ValueRef evaluate(Scope& scope) const {
            return std::make_shared<Keyword>(*this);
        }
    };

    class Identifier : public Expression {
    public:
        std::string name;
        Identifier(const std::string& name) : name(name) {}
        virtual void print(std::ostream& os) const {
            os << name;
        }
        virtual bool is_identifier() const {
            return true;
        }
        virtual ValueRef evaluate(Scope& scope) const {
            auto value = scope.lookup(*this);
            if(value) {
                return *value;
            } else {
                std::stringstream ss;
                ss << "Undefined identifier \"" << *static_cast<const Value *>(this) << "\"";
                throw std::runtime_error(ss.str());
            }
        }
    };

    class Tuple : public Expression {
    public:
        std::vector<ValueRef> elements;
        Tuple(const std::vector<ValueRef>& elements) : elements(elements) {}
        virtual void print(std::ostream& os) const {
            os << "(";
            if(elements.size() > 0) {
                os << *elements[0];
            }
            for(size_t i = 1; i < elements.size(); ++i) {
                os << " " << *elements[i];
            }
            os << ")";
        }
        virtual ValueRef evaluate(Scope& scope) const {
            if(elements.size() == 0) {
                return std::shared_ptr<const Tuple>(this);
            }
            ValueRef first = elements[0]->evaluate(scope);
            return first->call(std::vector<ValueRef>(elements.begin()+1, elements.end()), scope);
        }
    };

    class Function : public Atom {
    public:
        Identifier name;
        std::vector<Identifier> argument_names;
        Function(const Identifier& name, const std::vector<Identifier> argument_names) : name(name), argument_names(argument_names) {}
        virtual void print(std::ostream& os) const {
            os << "<function \"" << name << "\">";
        }
    };

    class BuiltinFunction : public Function {
    public:
        std::function<ValueRef(const std::vector<ValueRef>&, Scope&)> function;
        BuiltinFunction(const Identifier& name, const std::vector<Identifier>& argument_names, std::function<ValueRef(const std::vector<ValueRef>&, Scope&)> function) : Function(name, argument_names), function(function) {}
        virtual ValueRef call(const std::vector<ValueRef>& arguments, Scope& scope) const {
            return function(arguments, scope);
        }
        virtual ValueRef evaluate(Scope& scope) const {
            return std::make_shared<BuiltinFunction>(*this);
        }
    };
    
    
}
