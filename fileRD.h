#ifndef FILE_RD_H
#define FILE_RD_H

#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

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
    std::string file_name;
    std::fstream allocator;
    uint16_t file_record_buffer;

public:
    std::unordered_map<uint16_t, std::streampos> id_map;

    binary_file(const std::string& file_namer, const file_header& header_cord);
    ~binary_file();

    uint16_t allocate_id();
    std::streampos find_pos(uint16_t temp_id);
    uint16_t file_records();

    void add_records(const std::string& newname);
    void binary_read_console_print();
    void binary_read(std::vector<record>& load_records_in_memory);
    void modify_records(uint16_t temp_id, const std::string& new_name);
    void set_inactive(uint16_t temp_id);
    void clear_inactive_records();
};

#endif // FILE_RD_H