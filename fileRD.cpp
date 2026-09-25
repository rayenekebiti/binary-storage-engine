#include "fileRD.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

binary_file::binary_file(const std::string& file_namer, const file_header& header_cord)
    : file_name(file_namer), file_record_buffer(0)
{
    file_header head_check{};
    allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);

    if (!allocator.is_open())
    {
        std::ofstream create_file(file_name, std::ios::out | std::ios::binary);
        create_file.write(reinterpret_cast<const char*>(&header_cord), sizeof(header_cord));
        if (!create_file)
        {
            throw std::runtime_error("FATAL ERROR: unable to create the file");
        }
        create_file.close();
        allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    }

    allocator.read(reinterpret_cast<char*>(&head_check), sizeof(head_check));
    if (header_cord.header != head_check.header || header_cord.version != head_check.version)
    {
        allocator.close();
        std::ofstream rewrite_file(file_name, std::ios::binary | std::ios::trunc);
        rewrite_file.write(reinterpret_cast<const char*>(&header_cord), sizeof(header_cord));
        if (!rewrite_file)
        {
            throw std::runtime_error("fatal error, can't rewrite in the file, error:111,4");
        }
        rewrite_file.close();
        allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    }

    file_record_buffer = file_records();
    for (uint16_t i{}; i < file_record_buffer; ++i)
    {
        record temp{};
        const std::streampos offset = i * sizeof(record) + sizeof(file_header);
        allocator.seekg(offset, std::ios::beg);
        allocator.read(reinterpret_cast<char*>(&temp), sizeof(temp));
        if (temp.flags == 1)
        {
            id_map[temp.id] = offset;
        }
    }
    allocator.close();
}

binary_file::~binary_file()
{
    if (allocator.is_open())
    {
        allocator.close();
    }
}

uint16_t binary_file::allocate_id()
{
    file_header head{};
    allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!allocator)
    {
        throw std::runtime_error("can't open file, error:13,1");
    }

    allocator.seekg(0, std::ios::beg);
    allocator.read(reinterpret_cast<char*>(&head), sizeof(head));
    const uint16_t id = head.next_id;
    ++head.next_id;
    allocator.seekp(0, std::ios::beg);
    allocator.write(reinterpret_cast<const char*>(&head), sizeof(head));
    allocator.close();
    return id;
}

std::streampos binary_file::find_pos(uint16_t temp_id)
{
    const auto get_pos = id_map.find(temp_id);
    if (get_pos == id_map.end())
    {
        throw std::runtime_error("invalid input, error:888");
    }
    return get_pos->second;
}

uint16_t binary_file::file_records()
{
    std::fstream binreader(file_name, std::ios::in | std::ios::binary);
    if (!binreader)
    {
        throw std::runtime_error(
            "unable to open file to read and get file size, please try again later, error:333,1");
    }

    binreader.seekg(0, std::ios::end);
    const std::streamoff file_size = binreader.tellg();
    const auto record_count = (file_size - sizeof(file_header)) / sizeof(record);
    binreader.close();
    return static_cast<uint16_t>(record_count);
}

void binary_file::add_records(const std::string& newname)
{
    record newinf{};
    std::fstream user_add(
        file_name,
        std::ios::out | std::ios::binary | std::ios::in | std::ios::app);
    if (!user_add.is_open())
    {
        throw std::runtime_error(
            "unable to open file to add the user, please try again, error:555,1");
    }
    if (newname.size() > newinf.payload.size() - 1 || newname.empty())
    {
        throw std::out_of_range("payload is out of range,error:555,2");
    }

    std::memcpy(newinf.payload.data(), newname.c_str(), newname.size());
    newinf.payload[newname.size()] = '\0';
    newinf.id = allocate_id();
    newinf.flags = 1;
    user_add.seekp(0, std::ios::end);
    user_add.write(reinterpret_cast<const char*>(&newinf), sizeof(newinf));
    if (!user_add)
    {
        throw std::runtime_error("unable to write to the file, error:555,3");
    }

    user_add.seekg(0, std::ios::end);
    const std::streamoff size_of_file = user_add.tellg();
    const auto number_of_records = (size_of_file - sizeof(file_header)) / sizeof(record);
    const std::streampos new_offset =
        (number_of_records - 1) * sizeof(record) + sizeof(file_header);
    id_map[newinf.id] = new_offset;
    user_add.close();
    ++file_record_buffer;
}

void binary_file::binary_read_console_print()
{
    std::fstream binreader(file_name, std::ios::in | std::ios::binary);
    if (!binreader)
    {
        throw std::runtime_error("unable to open file, error:666,1");
    }

    for (uint16_t i{}; i < file_record_buffer; ++i)
    {
        record temp_read{};
        binreader.seekg(i * sizeof(record) + sizeof(file_header), std::ios::beg);
        binreader.read(reinterpret_cast<char*>(&temp_read), sizeof(temp_read));
        const std::string temp_payload(temp_read.payload.data());
        std::cout << "user is active: [" << static_cast<int>(temp_read.flags)
                  << "], id: " << temp_read.id << ", name: " << temp_payload << '\n'
                  << std::endl;
    }
}

void binary_file::binary_read(std::vector<record>& load_records_in_memory)
{
    std::fstream binreader(file_name, std::ios::in | std::ios::binary);
    if (!binreader)
    {
        throw std::runtime_error("unable to open file, error:666,1");
    }

    for (uint16_t i{}; i < file_record_buffer; ++i)
    {
        record temp_read{};
        binreader.seekg(i * sizeof(record) + sizeof(file_header), std::ios::beg);
        binreader.read(reinterpret_cast<char*>(&temp_read), sizeof(temp_read));
        load_records_in_memory.push_back(temp_read);
    }
}

void binary_file::modify_records(uint16_t temp_id, const std::string& new_name)
{
    record temp{};
    if (new_name.empty() || new_name.size() >= temp.payload.size() - 1)
    {
        throw std::out_of_range("payload is out of range, error::777,0");
    }

    std::fstream modify(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!modify)
    {
        throw std::runtime_error("unable to open file, error:777,1");
    }

    const std::streampos offset = find_pos(temp_id);
    modify.seekg(offset, std::ios::beg);
    modify.read(reinterpret_cast<char*>(&temp), sizeof(temp));
    if (!modify)
    {
        throw std::runtime_error("unable to read file, error:777,2");
    }

    std::memset(temp.payload.data(), 0, sizeof(temp.payload));
    std::memcpy(temp.payload.data(), new_name.c_str(), new_name.size());
    temp.payload[new_name.size()] = '\0';
    modify.seekp(offset, std::ios::beg);
    modify.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
    if (!modify)
    {
        throw std::runtime_error("unable to write to file, error:777,3");
    }
}

void binary_file::set_inactive(uint16_t temp_id)
{
    record temp{};
    allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!allocator)
    {
        throw std::runtime_error(
            "fatal error:file is corrupted or don't exist, error:999,1");
    }

    const std::streampos offset = find_pos(temp_id);
    allocator.seekg(offset, std::ios::beg);
    allocator.read(reinterpret_cast<char*>(&temp), sizeof(temp));
    allocator.seekp(offset, std::ios::beg);
    temp.flags = 0;
    allocator.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
    id_map.erase(temp_id);
    allocator.close();
}

void binary_file::clear_inactive_records()
{
    uint16_t write_index = 0;
    file_header temp{};

    allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!allocator)
    {
        throw std::runtime_error("unable to open file,error: 12,0");
    }

    allocator.seekg(0, std::ios::beg);
    allocator.read(reinterpret_cast<char*>(&temp), sizeof(temp));

    const std::string temporary_file_name = file_name + ".tmp";
    std::fstream only_active_file(
        temporary_file_name,
        std::ios::binary | std::ios::out | std::ios::trunc);
    if (!only_active_file)
    {
        throw std::runtime_error("can't open file, error:12,1");
    }
    only_active_file.write(reinterpret_cast<const char*>(&temp), sizeof(temp));

    for (uint16_t i{}; i < file_record_buffer; ++i)
    {
        record temp_read{};
        allocator.seekg(i * sizeof(record) + sizeof(file_header), std::ios::beg);
        allocator.read(reinterpret_cast<char*>(&temp_read), sizeof(temp_read));
        if (temp_read.flags == 1)
        {
            only_active_file.seekp(
                write_index * sizeof(record) + sizeof(file_header),
                std::ios::beg);
            only_active_file.write(reinterpret_cast<const char*>(&temp_read), sizeof(temp_read));
            ++write_index;
        }
    }

    allocator.close();
    only_active_file.close();
    if (std::remove(file_name.c_str()) != 0 ||
        std::rename(temporary_file_name.c_str(), file_name.c_str()) != 0)
    {
        throw std::runtime_error("unable to replace the cleared file, error:12,2");
    }

    allocator.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!allocator)
    {
        throw std::runtime_error("unable to open the cleared file, error:12,3");
    }

    file_record_buffer = write_index;
    id_map.clear();
    for (uint16_t i{}; i < file_record_buffer; ++i)
    {
        record active_record{};
        const std::streampos offset = i * sizeof(record) + sizeof(file_header);
        allocator.seekg(offset, std::ios::beg);
        allocator.read(reinterpret_cast<char*>(&active_record), sizeof(active_record));
        id_map[active_record.id] = offset;
    }
    allocator.close();
}
