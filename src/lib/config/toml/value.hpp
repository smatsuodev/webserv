#ifndef SRC_LIB_CONFIG_TOML_VALUE_HPP
#define SRC_LIB_CONFIG_TOML_VALUE_HPP

#include "utils/types/option.hpp"
#include <string>
#include <vector>
#include <map>

namespace toml {
    class Value;

    class Array {
    public:
        Array();
        explicit Array(const std::vector<Value> &elements);
        Array(const Array &other);
        ~Array();

        Array &operator=(const Array &other);
        bool operator==(const Array &other) const;

        void addElement(const Value &value);
        // parse が楽になるように、可変参照を返してる
        std::vector<Value> &getElements();

    private:
        std::vector<Value> elements_;
    };

    class Table {
    public:
        Table();
        explicit Table(const std::map<std::string, Value> &values);
        Table(const Table &other);
        ~Table();

        Table &operator=(const Table &other);
        bool operator==(const Table &other) const;

        void setValue(const std::string &key, const Value &value);

        Option<Value> getValue(const std::string &key) const;
        // parse が楽になるように、可変参照を返してる
        std::map<std::string, Value> &getValues();

    private:
        std::map<std::string, Value> values_;
    };

    class Value {
    public:
        enum ValueType { kUnknown, kString, kInteger, kBoolean, kArray, kTable };

        Value();
        explicit Value(ValueType type);
        explicit Value(const std::string &value);
        explicit Value(long value);
        explicit Value(bool value);
        explicit Value(const Array &value);
        explicit Value(const Table &value);
        Value(const Value &other);
        ~Value();

        Value &operator=(const Value &other);
        bool operator==(const Value &other) const;

        void setString(const std::string &value);
        void setInteger(long value);
        void setBoolean(bool value);
        void setArray(const Array &value);
        void setTable(const Table &value);

        ValueType getType() const;
        Option<std::string> getString() const;
        Option<long> getInteger() const;
        Option<bool> getBoolean() const;
        Option<Array> getArray() const;
        Option<Table> getTable() const;

        // parse が楽になるように、ポインタを取得できるようにする
        Option<Array *> getArrayPointer();
        Option<Table *> getTablePointer();

    private:
        ValueType type_;
        std::string stringValue_;
        long integerValue_;
        bool booleanValue_;
        Array arrayValue_;
        Table tableValue_;
    };
}

#endif
