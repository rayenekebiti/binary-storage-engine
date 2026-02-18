#include <iostream>
#include <stdint.h>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>



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
struct size: 37 bytes
offset  size   type      field
5        2     uint16_t   id
7        32    char[32]   name(null-terminate if shorter than 32)
39       1     uint8_t    is_active(1 for true|0 for false)
40       2     uint16_t   balance

each user start at 37*i bytes
*/
#pragma pack(push, 1)
struct info
{
    uint16_t id;
    char name[32];
    uint8_t is_active;
    uint16_t balance;
};
#pragma pack(pop)
class binary_file
{
public:
    std::unordered_map<uint16_t, std::streampos> id_map;
    binary_file() // 111
    {
        file_header head_check;
        file_header temp{0xa055, 1, 1000};
        std::fstream check_file("testbin.bin", std::ios::in | std::ios::out | std::ios::binary);
        if (!check_file.is_open())
        {
            std::cerr << "failed to open fail to check user, error:111,1" << std::endl;
            std::ofstream creatfile("testbin.bin", std::ios::out | std::ios::binary);
            creatfile.write(reinterpret_cast<char *>(&temp), sizeof(temp));
            if (!creatfile)
            {
                std::cerr << "fatal error:cannot create files, error:111,2";
                std::terminate();
            }
            std::cout << "file recreated succesfully" << std::endl;
            return;
        }
        
        check_file.read(reinterpret_cast<char *>(&head_check), sizeof(head_check));
        if (temp.header != head_check.header || temp.version != head_check.version)
        {
            check_file.close();
            std::cerr << "error, wrong file read, error:111,3"<<std::endl;
            std::ofstream rewrite_file("testbin.bin", std::ios::trunc);
            rewrite_file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
            rewrite_file.close();
            if (!rewrite_file)
            {
                std::cerr << "fatal error, can't rewrite in the file, error:111,4";
                std::terminate();
            }
        }
        for (uint16_t i{}; i < file_records(); i++)
        {
            info temp{};
            std::streampos offset = i * sizeof(info) + sizeof(file_header);
            check_file.seekg(offset, std::ios::beg);
            check_file.read(reinterpret_cast<char *>(&temp), sizeof(temp));
            if(temp.is_active==1)
            {
                id_map[temp.id] = offset;
            }
        }
        check_file.close();
    }
    uint16_t allocate_id()//13
    {
        uint16_t id;
        file_header head;
        std::fstream allocator("testbin.bin",std::ios::binary|std::ios::in|std::ios::out);
        if(!allocator)
        {
            throw std::runtime_error("can't open file, error:13,1");
        }
        allocator.seekg(0,std::ios::beg);
        allocator.read(reinterpret_cast<char*>(&head),sizeof(head));
        id=head.next_id;
        head.next_id++;
        allocator.seekp(0,std::ios::beg);
        allocator.write(reinterpret_cast<char*>(&head),sizeof(head));
        allocator.close();
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
        std::fstream binreader("testbin.bin", std::ios::in | std::ios::binary);
        if (!binreader)
        {
            std::cerr << "unable to open file to read and get file size, please try again later, error:333,1" << std::endl;
            return 0;
        }
        binreader.seekg(0, std::ios::end);
        std::streampos size_of_file = binreader.tellg();
        file_records = (size_of_file - sizeof(file_header)) / sizeof(info);
        binreader.close();
        return file_records;
    }

    void add_users() // 555
    {
        info newinf{};
        info next_id_check{};
        std::string newname;
        std::fstream user_add("testbin.bin", std::ios::out | std::ios::binary |std::ios::app);
        if (!user_add.is_open())
        {
            std::cerr << "unable to open file to add the user, please try again, error:555,1";
            return;
        }
        std::cout<<">>";
        std::getline(std::cin, newname);
        std::strncpy(newinf.name, newname.c_str(), sizeof(newinf.name) - 1);
        newinf.id = allocate_id();
        newinf.is_active = 1;
        newinf.balance = 0;
        user_add.seekp(0, std::ios::end);
        user_add.write(reinterpret_cast<char *>(&newinf), sizeof(newinf));
        user_add.close();
    }

    void binary_read() // 666
    {
        std::fstream binreader("testbin.bin", std::ios::in | std::ios::binary);
        if (!binreader)
        {
            std::cerr << "unable to open file to read, error:666,1" << std::endl;
            return;
        }
        for (uint16_t i{}; i < file_records(); i++)
        {
            info temp_read{};
            binreader.seekg(i * sizeof(info) + sizeof(file_header), std::ios::beg);
            binreader.read(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
            std::cout << "user is active: [" <<std::boolalpha<< static_cast<int>(temp_read.is_active) << "], id: " << temp_read.id << ", name: " << temp_read.name << ", balance: " << temp_read.balance << "\n"
                      << std::endl;
        }
        binreader.close();
    }

    void modify_user() // 777
    {
        uint16_t temp_id;
        std::string new_name;
        info temp{};
        std::cout << ">>";
        std::cin >> temp_id;
        std::fstream modify("testbin.bin", std::ios::in | std::ios::out | std::ios::binary);
        if (!modify)
        {
            std::cerr << "fatal error:file is corrupted or don't exist, error:777,1" << std::endl;
            std::terminate();
        }
        std::streampos offset = find_pos(temp_id);
        modify.seekg(offset, std::ios::beg);
        modify.read(reinterpret_cast<char *>(&temp), sizeof(temp));
        std::memset(temp.name, 0, sizeof(temp.name));
        modify.seekp(offset, std::ios::beg);
        std::cout << "\n >>";
        std::getline(std::cin >> std::ws, new_name);
        std::strncpy(temp.name, new_name.c_str(), sizeof(temp.name) - 1);
        modify.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        modify.close();
    }
    void set_inactive() // 999
    {
        uint16_t temp_id;
        info temp{};
        std::cout << ">>";
        std::cin >> temp_id;
        std::fstream inactive_user("testbin.bin", std::ios::binary | std::ios::in | std::ios::out);
        if (!inactive_user)
        {
            std::cerr << "fatal error:file is corrupted or don't exist, error:999,1" << std::endl;
            std::terminate();
        }
        std::streampos offset = find_pos(temp_id);
        inactive_user.seekg(offset, std::ios::beg);
        inactive_user.read(reinterpret_cast<char *>(&temp), sizeof(temp));
        inactive_user.seekp(offset, std::ios::beg);
        temp.is_active = 0;
        inactive_user.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        inactive_user.close();
    }
    void clear_inactive_users()//12
    {
        uint16_t write_index=0;
        uint16_t records = file_records();
        file_header temp{};
       
        std::fstream clear_inactive("testbin.bin", std::ios::binary | std::ios::in);
          if(!clear_inactive)
        {
            std::cerr<<"can't open file, error:12,0"<<std::endl;
            return;
        }

         clear_inactive.seekg(0,std::ios::beg);
         clear_inactive.read(reinterpret_cast<char*>(&temp),sizeof(temp));

        std::fstream only_active_file("temp.bin", std::ios::binary | std::ios::out);
        only_active_file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
        if(!only_active_file)
        {
            std::cerr<<"can't open file, error:12,1"<<std::endl;
            return;
        }

        for (uint16_t i{}; i < records; i++)
        {
            info temp_read{};
            clear_inactive.seekg(i * sizeof(info) + sizeof(file_header), std::ios::beg);
            clear_inactive.read(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
            if (temp_read.is_active == 1)
            {
                only_active_file.seekp(write_index * sizeof(info) + sizeof(file_header), std::ios::beg);
                only_active_file.write(reinterpret_cast<char *>(&temp_read), sizeof(temp_read));
                write_index++;
            }
        }
        std::remove("testbin.bin");
        rename("temp.bin", "testbin.bin");
        std::fstream reset_records("testbin.bin", std::ios::binary | std::ios::in);
        reset_records.seekg(0, std::ios::end);
            std::streampos size_of_file = reset_records.tellg();
            uint16_t new_record_size = (size_of_file - sizeof(file_header)) / sizeof(info);
            id_map.clear();
        for (uint16_t i{}; i < new_record_size; i++)
        {
            info temp{};
            std::streampos offset = i * sizeof(info) + sizeof(file_header);
            reset_records.seekg(offset, std::ios::beg);
            reset_records.read(reinterpret_cast<char *>(&temp), sizeof(temp));
            if(temp.is_active==1)
            {
                id_map[temp.id] = offset;
            }
        }
        clear_inactive.close();
        only_active_file.close();
        reset_records.close();
    }
};
int main()
{
    binary_file bin;
    bin.binary_read();
    bin.add_users();
    bin.binary_read();
    
}
