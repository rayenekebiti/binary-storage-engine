#include <iostream>
#include <stdint.h>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <expected>

/*
struct size: 5 bytes
offset  size   type       field
  0      2    uint16_t    header
  2      1    uint8_t     version
  3      2    uint16_t    next_id
  there exists only one header at the beginning of the file

*/
#pragma pack(push, 1)
struct file_header
{
    uint16_t header;
    uint8_t version;
    uint16_t next_id;
    std::string file_name;
};
#pragma pack(pop)

/*
File format: Bin
struct size: 35 bytes
offset  size   type      field
5        2     uint16_t   id
7        32    char[32]   name(null-terminate if shorter than 32)
39       1     uint8_t    flags(1 for true|0 for false)
each user start at 35*i bytes
*/
#pragma pack(push, 1)
struct record
{
    uint16_t id;
    std::array<char, 32> payload;
    uint8_t flags;
};
#pragma pack(pop)
class binary_file
{
public:
    file_header filename;
    std::fstream allocator;
    std::unordered_map<uint16_t, std::streampos> id_map;
    binary_file() // 111
    {
        file_header head_check;
        file_header temp{0xa055, 3, 999};
        allocator.open(filename.file_name, std::ios::binary | std::ios::in | std::ios::out);
        
        if (!allocator.is_open())
        {
            std::ofstream creatfile(filename.file_name, std::ios::out | std::ios::binary);
            creatfile.write(reinterpret_cast<char *>(&temp), sizeof(temp));
            if (!creatfile)
            {
                throw std::runtime_error("FATAL ERROR: unable to create the file");
            }
            creatfile.close(); 
            allocator.open(filename.file_name, std::ios::binary | std::ios::in | std::ios::out);
        }
        allocator.read(reinterpret_cast<char *>(&head_check), sizeof(head_check));
        if (temp.header != head_check.header || temp.version != head_check.version)
        {
            allocator.close();
            std::ofstream rewrite_file(filename.file_name, std::ios::trunc);
            rewrite_file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
            if (!rewrite_file)
            {
                throw std::runtime_error("fatal error, can't rewrite in the file, error:111,4");
            }
            allocator.open(filename.file_name, std::ios::binary | std::ios::in | std::ios::out);
            rewrite_file.close();
        }
        for (uint16_t i{}; i < file_records(); i++)
        {
            record temp{};
            std::streampos offset = i * sizeof(record) + sizeof(file_header);
            allocator.seekg(offset, std::ios::beg);
            allocator.read(reinterpret_cast<char *>(&temp), sizeof(temp));
            if (temp.flags == 1)
            {
                id_map[temp.id] = offset;
            }
        }
    }
    ~binary_file()
    {
     if(allocator.is_open())
     allocator.close();
    }
    uint16_t allocate_id() // 13
    {
        uint16_t id;
        file_header head;
        
        if (!allocator)
        {
            throw std::runtime_error("can't open file, error:13,1");
        }
        allocator.seekg(0, std::ios::beg);
        allocator.read(reinterpret_cast<char *>(&head), sizeof(head));
        id = head.next_id;
        head.next_id++;
        allocator.seekp(0, std::ios::beg);
        allocator.write(reinterpret_cast<char *>(&head), sizeof(head));
        return id;
    }
    std::streampos find_pos(uint16_t temp_id) // 888
    {
        auto get_pos = id_map.find(temp_id);
        if (get_pos == id_map.end())
        {
            throw std::runtime_error("invalid input, error:888");
        };
        std::streampos offset = get_pos->second;
        return offset;
    }
    uint16_t file_records() // 333
    {
        uint16_t file_records;
        std::fstream binreader(filename.file_name, std::ios::in | std::ios::binary);
        if (!binreader)
        {
            throw std::runtime_error("unable to open file to read and get file size, please try again later, error:333,1");
        }
        binreader.seekg(0, std::ios::end);
        std::streampos size_of_file = binreader.tellg();
        file_records = (size_of_file - sizeof(file_header)) / sizeof(record);
        binreader.close();
        return file_records;
    }

    void add_records(const std::string& newname) // 555
    {
        record newinf{};
        std::fstream user_add(filename.file_name, std::ios::out | std::ios::binary | std::ios::in |std::ios::app);
        if (!user_add.is_open())
        {
            throw std::runtime_error("unable to open file to add the user, please try again, error:555,1");
        }
        if(newname.size()>newinf.payload.size()-1||newname.empty())
        {
         throw std::out_of_range("payload is out of range,error:555,2");
        }
        std::memcpy(newinf.payload.data(),newname.c_str(),newname.size());
        newinf.payload[newname.size()]='\0';
        newinf.id = allocate_id();
        newinf.flags = 1;
        user_add.seekp(0,std::ios::end);
        user_add.write(reinterpret_cast<char *>(&newinf), sizeof(newinf));
        if(!user_add)
        {
            throw std::runtime_error("unable to write to the file, error:555,3");
        }
        user_add.seekg(0, std::ios::end);
        std::streampos size_of_file = user_add.tellg();
        uint16_t new_record = (size_of_file - sizeof(file_header)) / sizeof(record);
        std::streampos new_offset=(new_record-1)*sizeof(record)+sizeof(file_header);
        id_map[newinf.id]=new_offset;
    }

    // void binary_read() // 666
    //{
    //     
    //     std::fstream binreader(filename.file_name, std::ios::in | std::ios::binary);
    //     if (!binreader)
    //     {
    //         std::cerr << "unable to open file to read, error:666,1" << std::endl;
    //         return;
    //     }
    //
    //     for (uint16_t i{}; i < file_records(); i++)
    //     {
    //         record temp_read{};
    //         binreader.seekg(i * sizeof(record) + sizeof(file_header), std::ios::beg);
    //         binreader.read(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
    //         std::string temp_payload(temp_read.payload.data());
    //         std::cout << "user is active: [" << static_cast<int>(temp_read.flags) << "], id: " << temp_read.id << ", name: "
    //          <<temp_payload<< "\n"<< std::endl;
    //     }
    //     binreader.close();
    // }

    void modify_records(uint16_t temp_id,const std::string& new_name) // 777
    {
        record temp{};
        if(new_name.empty()||new_name.size()>=temp.payload.size()-1)
        {
            throw std::out_of_range("payload is out of range, error::777,0");
        }
        std::fstream modify(filename.file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!modify)
        {
            throw std::runtime_error("unable to open file, error:777,1");
        }
        std::streampos offset = find_pos(temp_id);
        modify.seekg(offset, std::ios::beg);
        modify.read(reinterpret_cast<char *>(&temp), sizeof(temp));
        if(!modify)
        {
            throw std::runtime_error("unable to read file, error:777,2");
        }
        std::memset(temp.payload.data(), 0, sizeof(temp.payload));
        modify.seekp(offset, std::ios::beg);
        std::memcpy(temp.payload.data(), new_name.c_str(), new_name.size());
        temp.payload[new_name.size()]='\0';
        modify.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        if(!modify)
        {
            throw std::runtime_error("unable to write to file, error:777,3");
        }
    }
    void set_inactive(uint16_t temp_id) // 999
    {
        record temp{};
        std::fstream inactive_user(filename.file_name, std::ios::binary | std::ios::in | std::ios::out);
        if (!inactive_user)
        {
            throw std::runtime_error("fatal error:file is corrupted or don't exist, error:999,1");
        }
        std::streampos offset = find_pos(temp_id);
        inactive_user.seekg(offset, std::ios::beg);
        inactive_user.read(reinterpret_cast<char *>(&temp), sizeof(temp));
        inactive_user.seekp(offset, std::ios::beg);
        temp.flags = 0;
        inactive_user.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        //call clear_inactive_records() to clear inactive records from memory and disk
    }
    void clear_inactive_records() // 12
    {
        uint16_t write_index = 0;
        uint16_t records = file_records();
        file_header temp{};

        std::fstream clear_inactive(filename.file_name, std::ios::binary | std::ios::in);
        if (!clear_inactive)
        {
            throw std::runtime_error("unable to open file,error: 12,0");
        }

        clear_inactive.seekg(0, std::ios::beg);
        clear_inactive.read(reinterpret_cast<char *>(&temp), sizeof(temp));

        std::fstream only_active_file("temp.bin", std::ios::binary | std::ios::out);
        only_active_file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        if (!only_active_file)
        {
            throw std::runtime_error("can't open file, error:12,1");
        }

        for (uint16_t i{}; i < records; i++)
        {
            record temp_read{};
            clear_inactive.seekg(i * sizeof(record) + sizeof(file_header), std::ios::beg);
            clear_inactive.read(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
            if (temp_read.flags == 1)
            {
                only_active_file.seekp(write_index * sizeof(record) + sizeof(file_header), std::ios::beg);
                only_active_file.write(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
                write_index++;
            }
        }
        std::remove(filename.file_name.c_str());
        rename("temp.bin", filename.file_name.c_str());
        std::fstream reset_records(filename.file_name, std::ios::binary | std::ios::in);
        reset_records.seekg(0, std::ios::end);
        std::streampos size_of_file = reset_records.tellg();
        uint16_t new_record_size = (size_of_file - sizeof(file_header)) / sizeof(record);
        id_map.clear();
        for (uint16_t i{}; i < new_record_size; i++)
        {
            record temp{};
            std::streampos offset = i * sizeof(record) + sizeof(file_header);
            reset_records.seekg(offset, std::ios::beg);
            reset_records.read(reinterpret_cast<char *>(&temp), sizeof(temp));
            if (temp.flags == 1)
            {
                id_map[temp.id] = offset;
            }
        }
    }
};
