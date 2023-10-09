#ifndef _SHM_DATA_H
#define _SHM_DATA_H

enum RENDER_TYPE
{
    NORMAL = 0,
    GDI = 1,
    DX = 2,
    OPENGL = 3
};

enum IBF
{
    B8G8R8 = 0,
    R8G8B8 = 1,
    B8G8R8A8 = 2,
    R8G8B8A8 = 3,
};


struct SharedDataHeader {
    int width, height;
    int channel;
};

// 如果frameinfo内部不提供new buffer,那么也不需要删除
struct FrameInfo
{
public:
    int buffer_size = 0; 
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

    void new_buffer()
    {
        buffer_size = get_buffer_size();
        buffer = new BYTE[buffer_size];
    }

    void delete_buffer()
    {
        if (buffer)
        {
            delete[] buffer; 
            buffer = nullptr; 
        }
    }

    // read head and buffer from address
    void read(BYTE* address)
    {
        memcpy(&header, address, sizeof(header));
        buffer_size = get_buffer_size();
        if (buffer)
        {
            delete[] buffer;
            buffer = nullptr;
        }
       
        if (buffer_size > 0)
        {
            buffer = new BYTE[buffer_size];
            memcpy(buffer, address + sizeof(header), buffer_size);
        }
        else {
            buffer = nullptr; 
        }
    }

    // write head and buffer into address
    void write(BYTE* address)
    {
        if (!buffer)
            return;
        buffer_size = get_buffer_size();
        memcpy(address, &header, sizeof(header));
        memcpy(address + sizeof(header), buffer, buffer_size);
    }
};

#endif
