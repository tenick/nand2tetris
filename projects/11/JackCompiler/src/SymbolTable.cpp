#include "JackCompiler/SymbolTable.hpp"
#include "JackCompiler/SymbolKind.hpp"
#include <stdexcept>
#include <string>

namespace JackCompiler {
    SymbolTable::SymbolTable() {}

    void SymbolTable::startSubroutine() {
        subroutineScopeTable_.clear();
        subroutineScopeKindCount_.clear();
    }

    void SymbolTable::define(const std::string& name, const std::string& type, SymbolKind kind) {
        switch (kind) {
            case SymbolKind::Static:
            case SymbolKind::Field:
                if (classScopeTable_.find(name) != classScopeTable_.end()) {
                    throw std::runtime_error("Symbol '" + name + "' already declared.\n");
                }
                classScopeTable_[name] = SymbolTableValue{kind, type, varCount(kind)};
                classScopeKindCount_[kind]++;
                break;
            case SymbolKind::Arg:
            case SymbolKind::Var:
                if (subroutineScopeTable_.find(name) != subroutineScopeTable_.end()) {
                    throw std::runtime_error("Symbol '" + name + "' already declared.\n");
                }
                subroutineScopeTable_[name] = SymbolTableValue(kind, type, varCount(kind));
                subroutineScopeKindCount_[kind]++;
                break;
            default:
                throw std::runtime_error("Unsupported symbol kind '" + std::string(symbolKindToString(kind)) + "'.\n");
        }
    }

    int SymbolTable::varCount(SymbolKind kind) {
        switch (kind) {
            case SymbolKind::Static:
            case SymbolKind::Field:
                return classScopeKindCount_[kind];
            case SymbolKind::Arg:
            case SymbolKind::Var:
                return subroutineScopeKindCount_[kind];
            default:
                throw std::runtime_error("Unsupported symbol kind '" + std::string(symbolKindToString(kind)) + "'.\n");
        }
    }

    SymbolKind SymbolTable::kindOf(const std::string& name) {
        if (subroutineScopeTable_.find(name) != subroutineScopeTable_.end())
            return subroutineScopeTable_[name].kind;
        if (classScopeTable_.find(name) != classScopeTable_.end())
            return classScopeTable_[name].kind;
        throw std::runtime_error("Symbol '" + name + "' not found.\n");
    }

    std::string SymbolTable::typeOf(const std::string& name) {
        if (subroutineScopeTable_.find(name) != subroutineScopeTable_.end())
            return subroutineScopeTable_[name].type;
        if (classScopeTable_.find(name) != classScopeTable_.end())
            return classScopeTable_[name].type;
        throw std::runtime_error("Symbol '" + name + "' not found.\n");
    }

    int SymbolTable::indexOf(const std::string& name) {
        if (subroutineScopeTable_.find(name) != subroutineScopeTable_.end())
            return subroutineScopeTable_[name].index;
        if (classScopeTable_.find(name) != classScopeTable_.end())
            return classScopeTable_[name].index;
        throw std::runtime_error("Symbol '" + name + "' not found.\n");
    }
    bool SymbolTable::exists(const std::string_view name) {
        if (subroutineScopeTable_.find(std::string(name)) != subroutineScopeTable_.end())
            return true;
        if (classScopeTable_.find(std::string(name)) != classScopeTable_.end())
            return true;
        return false;
    }
}
