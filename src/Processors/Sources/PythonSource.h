#pragma once

#include <cstddef>
#include <Core/Block.h>

#include <Core/ExternalResultDescription.h>
#include <Processors/ISource.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <Poco/Logger.h>
#include <Common/PythonUtils.h>

namespace DB
{

namespace py = pybind11;

class PyReader;


class PythonSource : public ISource
{
public:
    PythonSource(
        py::object & data_source_,
        const Block & sample_block_,
        PyColumnVecPtr column_cache,
        size_t data_source_row_count,
        size_t max_block_size_,
        size_t stream_index,
        size_t num_streams);

    ~PythonSource() override = default;

    String getName() const override { return "Python"; }
    Chunk genChunk(size_t & num_rows, PyObjectVecPtr data);
    Chunk generate() override;


private:
    py::object & data_source; // Do not own the reference

    Block sample_block;
    PyColumnVecPtr column_cache;
    size_t data_source_row_count;
    const UInt64 max_block_size;
    // Caller will only pass stream index and total stream count
    // to the constructor, we need to calculate the start offset and end offset.
    const size_t stream_index;
    const size_t num_streams;
    size_t cursor;

    Poco::Logger * logger = &Poco::Logger::get("TableFunctionPython");
    ExternalResultDescription description;

    PyObjectVecPtr scanData(const py::object & data, const std::vector<std::string> & col_names, size_t & cursor, size_t count);
    template <typename T>
    ColumnPtr convert_and_insert_array(const ColumnWrapper & col_wrap, size_t & cursor, size_t count, UInt32 scale = 0);
    template <typename T>
    ColumnPtr convert_and_insert(const py::object & obj, UInt32 scale = 0);
    template <typename T>
    void insert_from_ptr(const void * ptr, const MutableColumnPtr & column, size_t offset, size_t row_count);

    void convert_string_array_to_block(PyObject ** buf, const MutableColumnPtr & column, size_t offset, size_t row_count);


    template <typename T>
    void insert_from_list(const py::list & obj, const MutableColumnPtr & column);

    void insert_string_from_array(py::handle obj, const MutableColumnPtr & column);

    void insert_string_from_array_raw(PyObject ** buf, const MutableColumnPtr & column, size_t offset, size_t row_count);
    void prepareColumnCache(Names & names, Columns & columns);
    Chunk scanDataToChunk();
    void destory(PyObjectVecPtr & data);
};
}
