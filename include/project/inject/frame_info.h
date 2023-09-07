#ifndef _SHM_DATA_H
#define _SHM_DATA_H

struct SharedDataHeader {
    int width, height;
    int channel;
};

// 2023/08/25 to do : 增加一个封装类，封装shared memory data
// done 2023/08/30

// 2023/08/30 to do : 想一个buffer不频繁new的方法
struct FrameInfo
{
public:
    bool usable = false;
    SharedDataHeader header;
    BYTE* buffer = nullptr;

    FrameInfo() = default;
    ~FrameInfo()
    {
        if (buffer)
        {
            delete[] buffer;
            buffer = nullptr; 
        }
    }
    int get_buffer_size()
    {
        return header.width * header.height * header.channel;
    }

    void read(BYTE* address)
    {
        memcpy(&header, address, sizeof(header));
        int buffer_size = get_buffer_size();
        if (buffer)
        {
            delete[] buffer;
            buffer = nullptr;
        }
       
        if (buffer_size > 0)
        {
            buffer = new BYTE[buffer_size];
            memcpy(buffer, address + sizeof(header), buffer_size);
            usable = true;
        }
        else {
            usable = false;
        }
    }
};

#endif
