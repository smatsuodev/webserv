#include "value.hpp"
#include "utils/types/result.hpp"
#include "utils/types/try.hpp"

namespace toml {
    /* Value */
    Value::Value() : type_(kUnknown), integerValue_(0), booleanValue_(false) {}
    Value::Value(const ValueType type) : type_(type), integerValue_(0), booleanValue_(false) {}
    Value::Value(const std::string &value)
        : type_(kString), stringValue_(value), integerValue_(0), booleanValue_(false) {}
    Value::Value(const char *value) : type_(kString), stringValue_(value), integerValue_(0), booleanValue_(false) {}
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
            default:
                // do nothing
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
                default:
                    // do nothing
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

    Table &Value::getTableRef() {
        return tableValue_;
    }

    Array &Value::getArrayRef() {
        return arrayValue_;
    }

    /* Array */
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

    size_t Array::size() const {
        return elements_.size();
    }

    Value &Array::getElementRef(size_t index) {
        return elements_[index];
    }

    std::vector<Value> &Array::getElements() {
        return elements_;
    }

    /* Table */
    Table::Table() : isEditable_(true) {}

    Table::Table(const std::map<std::string, Value> &values) : values_(values), isEditable_(true) {}

    Table::Table(const std::map<std::string, Value> &values, bool isEditable)
        : values_(values), isEditable_(isEditable) {}

    Table::Table(const Table &other) : values_(other.values_), isEditable_(other.isEditable_) {}

    Table::~Table() {}

    Table &Table::operator=(const Table &other) {
        if (this != &other) {
            values_ = other.values_;
            isEditable_ = other.isEditable_;
        }
        return *this;
    }

    bool Table::operator==(const Table &other) const {
        return values_ == other.values_;
    }

    Table Table::readOnly() const {
        return Table(values_, false);
    }

    Option<Value> Table::setValue(const std::string &key, const Value &value) {
        if (!isEditable_ || values_.find(key) != values_.end()) {
            return None;
        }
        values_[key] = value;
        return Some(value);
    }

    Option<Value> Table::getValue(const std::string &key) const {
        const std::map<std::string, Value>::const_iterator it = values_.find(key);
        if (it != values_.end()) {
            return Some<Value>(it->second);
        }
        return None;
    }

    Value &Table::getValueRef(const std::string &key) {
        return values_[key];
    }

    const std::map<std::string, Value> &Table::getValues() const {
        return values_;
    }

    Result<Table *, error::AppError> Table::findOrCreateTablePath(const std::vector<std::string> &keys) {
        if (keys.empty()) {
            return Err(error::AppError::kParseUnknown);
        }

        const size_t endIndex = keys.size() - 1;
        Table *currentTable = this;

        for (size_t i = 0; i < endIndex; ++i) {
            const std::string &key = keys[i];
            Option<Value> nextValue = currentTable->getValue(key);

            if (nextValue.isSome()) {
                Value &value = currentTable->getValueRef(key);
                if (value.getType() == Value::kTable) {
                    currentTable = &value.getTableRef();
                } else if (value.getType() == Value::kArray) {
                    Array &array = value.getArrayRef();
                    if (array.size() == 0) {
                        Table newTable;
                        array.addElement(Value(newTable));
                        currentTable = &array.getElementRef(0).getTableRef();
                    } else {
                        Value &lastElement = array.getElementRef(array.size() - 1);
                        if (lastElement.getType() == Value::kTable) {
                            currentTable = &lastElement.getTableRef();
                        } else {
                            return Err(error::AppError::kParseUnknown);
                        }
                    }
                } else {
                    return Err(error::AppError::kParseUnknown);
                }
            } else {
                Table newTable;
                currentTable->setValue(key, Value(newTable));
                currentTable = &currentTable->getValueRef(key).getTableRef();
            }
        }

        return Ok(currentTable);
    }

    bool Table::hasKey(const std::string &key) const {
        return values_.find(key) != values_.end();
    }

    std::vector<std::string> Table::getKeys() {
        std::vector<std::string> keys;
        for (std::map<std::string, Value>::iterator it = values_.begin(); it != values_.end(); ++it) {
            keys.push_back(it->first);
        }
        return keys;
    }
}
