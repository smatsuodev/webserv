#include "value.hpp"

namespace toml {
    /* Value */
    Value::Value(const ValueType type) : type_(type), integerValue_(0), booleanValue_(false) {}
    Value::Value(const std::string &value)
        : type_(kString), stringValue_(value), integerValue_(0), booleanValue_(false) {}
    Value::Value(const long value) : type_(kInteger), integerValue_(value), booleanValue_(false) {}
    Value::Value(const bool value) : type_(kBoolean), integerValue_(0), booleanValue_(value) {}
    Value::Value(const Array &value) : type_(kArray), integerValue_(0), booleanValue_(false), arrayValue_(value) {}
    Value::Value(const Table &value) : type_(kTable), integerValue_(0), booleanValue_(false), tableValue_(value) {}

    Value::Value(const Value &other) : type_(other.type_) {
        switch (type_) {
            case kString:
                stringValue_ = other.stringValue_;
                break;
            case kInteger:
                integerValue_ = other.integerValue_;
                break;
            case kBoolean:
                booleanValue_ = other.booleanValue_;
                break;
            case kArray:
                arrayValue_ = other.arrayValue_;
                break;
            case kTable:
                tableValue_ = other.tableValue_;
                break;
        }
    }

    Value::~Value() {}

    Value &Value::operator=(const Value &other) {
        if (this != &other) {
            type_ = other.type_;
            switch (type_) {
                case kString:
                    stringValue_ = other.stringValue_;
                    break;
                case kInteger:
                    integerValue_ = other.integerValue_;
                    break;
                case kBoolean:
                    booleanValue_ = other.booleanValue_;
                    break;
                case kArray:
                    arrayValue_ = other.arrayValue_;
                    break;
                case kTable:
                    tableValue_ = other.tableValue_;
                    break;
            }
        }
        return *this;
    }

    bool Value::operator==(const Value &other) const {
        switch (type_) {
            case kString:
                return stringValue_ == other.stringValue_;
            case kInteger:
                return integerValue_ == other.integerValue_;
            case kBoolean:
                return booleanValue_ == other.booleanValue_;
            case kArray:
                return arrayValue_ == other.arrayValue_;
            case kTable:
                return tableValue_ == other.tableValue_;
            default:
                return false;
        }
    }

    void Value::setString(const std::string &value) {
        type_ = kString;
        stringValue_ = value;
    }

    void Value::setInteger(const long value) {
        type_ = kInteger;
        integerValue_ = value;
    }

    void Value::setBoolean(const bool value) {
        type_ = kBoolean;
        booleanValue_ = value;
    }

    void Value::setArray(const Array &value) {
        type_ = kArray;
        arrayValue_ = value;
    }

    void Value::setTable(const Table &value) {
        type_ = kTable;
        tableValue_ = value;
    }

    Value::ValueType Value::getType() const {
        return type_;
    }

    Option<std::string> Value::getString() const {
        if (type_ == kString) {
            return Some(stringValue_);
        }
        return None;
    }

    Option<long> Value::getInteger() const {
        if (type_ == kInteger) {
            return Some(integerValue_);
        }
        return None;
    }

    Option<bool> Value::getBoolean() const {
        if (type_ == kBoolean) {
            return Some(booleanValue_);
        }
        return None;
    }

    Option<Array> Value::getArray() const {
        if (type_ == kArray) {
            return Some(arrayValue_);
        }
        return None;
    }

    Option<Table> Value::getTable() const {
        if (type_ == kTable) {
            return Some(tableValue_);
        }
        return None;
    }

    Option<Array *> Value::getArrayPointer() {
        if (type_ == kArray) {
            return Some(&arrayValue_);
        }
        return None;
    }

    Option<Table *> Value::getTablePointer() {
        if (type_ == kArray) {
            return Some(&tableValue_);
        }
        return None;
    }

    /* Array */
    Array::Array() {}

    Array::Array(const std::vector<Value> &elements) : elements_(elements) {}

    Array::Array(const Array &other) : elements_(other.elements_) {}

    Array::~Array() {}

    Array &Array::operator=(const Array &other) {
        if (this != &other) {
            elements_ = other.elements_;
        }
        return *this;
    }

    bool Array::operator==(const Array &other) const {
        return elements_ == other.elements_;
    }

    void Array::addElement(const Value &value) {
        elements_.push_back(value);
    }

    std::vector<Value> &Array::getElements() {
        return elements_;
    }

    /* Table */
    Table::Table() {}

    Table::Table(const std::map<std::string, Value> &values) : values_(values) {}

    Table::Table(const Table &other) : values_(other.values_) {}

    Table::~Table() {}

    Table &Table::operator=(const Table &other) {
        if (this != &other) {
            values_ = other.values_;
        }
        return *this;
    }

    bool Table::operator==(const Table &other) const {
        return values_ == other.values_;
    }

    void Table::setValue(const std::string &key, const Value &value) {
        values_[key] = value;
    }

    Option<Value> Table::getValue(const std::string &key) const {
        const std::map<std::string, Value>::const_iterator it = values_.find(key);
        if (it != values_.end()) {
            return Some<Value>(it->second);
        }
        return None;
    }

    std::map<std::string, Value> &Table::getValues() {
        return values_;
    }
}
