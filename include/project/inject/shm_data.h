#ifndef _SHM_DATA_H
#define _SHM_DATA_H


// 2023/08/25 to do : 增加一个封装类，封装shared memory data
struct SharedDataHeader {
    int width, height;
    int channel;
};

#endif
