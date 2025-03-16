#ifndef SRC_LIB_CONFIG_TOML_VALUE_HPP
#define SRC_LIB_CONFIG_TOML_VALUE_HPP

#include "utils/types/error.hpp"
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
        size_t size() const;
        Value &getElementRef(size_t index);
        std::vector<Value> &getElements();

    private:
        std::vector<Value> elements_;
    };

    class Table {
    public:
        Table();
        explicit Table(const std::map<std::string, Value> &values);
        Table(const std::map<std::string, Value> &values, bool isEditable);
        Table(const Table &other);
        ~Table();

        Table &operator=(const Table &other);
        bool operator==(const Table &other) const;

        Table readOnly() const;

        Option<Value> setValue(const std::string &key, const Value &value);

        Option<Value> getValue(const std::string &key) const;
        Value &getValueRef(const std::string &key);
        const std::map<std::string, Value> &getValues() const;
        Result<Table *, error::AppError> findOrCreateTablePath(const std::vector<std::string> &keys);
        bool hasKey(const std::string &key) const;
        std::vector<std::string> getKeys();

    private:
        std::map<std::string, Value> values_;
        bool isEditable_;
    };

    class Value {
    public:
        enum ValueType { kUnknown, kString, kInteger, kBoolean, kArray, kTable };

        Value();
        explicit Value(ValueType type);
        explicit Value(const std::string &value);
        explicit Value(const char *value);
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
        Table &getTableRef();
        Array &getArrayRef();

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
