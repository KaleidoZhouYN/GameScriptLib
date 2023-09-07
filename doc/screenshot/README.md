# 更新 2023/09/07
更新内容：多进程间读写模型(IPCRW)

特征：
1. 同时只能有一个进程进行写入
2. 同时可以有多个进程进行读取
3. 读取和写入互斥

实现：
Mutex + SharedMemory + read_count

# 更新 2023/08/25
更新内容：多线程雷电模拟器截屏示例

https://github.com/KaleidoZhouYN/GameScriptLib/assets/31400191/a295c1fe-49af-4d69-b619-f0751f4ebda6

# 更新 2023/08/30
更新内容：多线程获取同一窗口截屏示例

* 主线程进行dll注入
* 子线程中进行capture buffer操作
* 需要实现进程间的一写多读的信号量

https://github.com/KaleidoZhouYN/GameScriptLib/assets/31400191/79cc07a2-9bf5-4d05-8c46-b7d1b9b35bb1