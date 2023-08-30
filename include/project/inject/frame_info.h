#ifndef _SHM_DATA_H
#define _SHM_DATA_H

struct SharedDataHeader {
    int width, height;
    int channel;
};

// 2023/08/25 to do : ����һ����װ�࣬��װshared memory data
// done 2023/08/30

// 2023/08/30 to do : ��һ��buffer��Ƶ��new�ķ���
struct FrameInfo
{
public:
    bool usable = false;
    SharedDataHeader header;
    BYTE* buffer = nullptr;

    FrameInfo() = default;
    ~FrameInfo()
    {
        if (!buffer)
        {
            delete buffer;
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
            delete buffer;
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
