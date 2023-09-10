#ifndef _SHM_DATA_H
#define _SHM_DATA_H

struct SharedDataHeader {
    int width, height;
    int channel;
};

// 2023/08/25 to do : ����һ����װ�࣬��װshared memory data
// done 2023/08/30

// ���frameinfo�ڲ����ṩnew buffer,��ôҲ����Ҫɾ��
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

    void write(BYTE* address)
    {
        memcpy(address, &header, sizeof(header));
        buffer_size = get_buffer_size();
        if (!buffer)
            return;
        memcpy(address + sizeof(header), buffer, buffer_size);
    }
};

#endif
