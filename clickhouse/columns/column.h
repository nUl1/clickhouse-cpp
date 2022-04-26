#pragma once

#include "../types/types.h"
#include "../columns/itemview.h"
#include "../exceptions.h"

#include <memory>
#include <stdexcept>

namespace clickhouse {

class InputStream;
class OutputStream;

using ColumnRef = std::shared_ptr<class Column>;

/**
 * An abstract base of all columns classes.
 */
class Column : public std::enable_shared_from_this<Column> {
public:
    explicit inline Column(TypeRef type) : type_(type) {}

    virtual ~Column() {}

    /// Downcast pointer to the specific column's subtype.
    template <typename T>
    inline std::shared_ptr<T> As() {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    /// Downcast pointer to the specific column's subtype.
    template <typename T>
    inline std::shared_ptr<const T> As() const {
        return std::dynamic_pointer_cast<const T>(shared_from_this());
    }

    /// Get type object of the column.
    inline TypeRef Type() const { return type_; }
    inline const class Type& GetType() const { return *type_; }

    /// Appends content of given column to the end of current one.
    virtual void Append(ColumnRef column) = 0;

    /// Loads column data from input stream.
    bool Load(InputStream* input, size_t rows);

    virtual bool LoadPrefix(InputStream* input, size_t rows);

    virtual bool LoadBody(InputStream* input, size_t rows);

    /// Saves column data to output stream.
    virtual void Save(OutputStream* output);

    /// Saves column prefix to output stream. Column types with prefixes must implement it
    virtual void SavePrefix(OutputStream* output);

    /// Saves column body to output stream.
    virtual void SaveBody(OutputStream* output);

    /// Saves column suffix to output stream. Column types with suffixes must implement it
    virtual void SaveSuffix(OutputStream* output);

    /// Clear column data .
    virtual void Clear() = 0;

    /// Returns count of rows in the column.
    virtual size_t Size() const = 0;

    /// Makes slice of the current column.
    virtual ColumnRef Slice(size_t begin, size_t len) const = 0;

    virtual void Swap(Column&) = 0;

    /// Get a view on raw item data if it is supported by column, will throw an exception if index is out of range.
    /// Please note that view is invalidated once column items are added or deleted, column is loaded from strean or destroyed.
    virtual ItemView GetItem(size_t) const {
        throw UnimplementedError("GetItem() is not supported for column of " + type_->GetName());
    }

    friend void swap(Column& left, Column& right) {
        left.Swap(right);
    }

protected:
    TypeRef type_;
};

}  // namespace clickhouse
