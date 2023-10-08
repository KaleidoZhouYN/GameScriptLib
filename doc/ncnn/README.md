对于ncnn模型，采用json file来控制模型的输入layer，输出layer，预处理pre_process，后处理post_process

json file的大致形式如下所示：

{<br>
    &emsp;"name": "hero_avatar_feature",<br>
    &emsp;"param_file": xx,<br>
    &emsp;"bin_file": xx,<br>
    &emsp;"input": "image",<br>
    &emsp;"out_layers":<br>
    &emsp;[<br>
        &emsp;&emsp;{"name":xx,},<br>
        &emsp;&emsp;{"name}:xx,},<br>
    &emsp;],<br>
    &emsp;"pre_process": <br>
    &emsp;{<br>
        &emsp;&emsp;"classname": "CusPreProcess", <br>
        &emsp;&emsp;"config":{} <br>
    &emsp;}<br>
    &emsp;"post_process": <br>
    &emsp;{<br>
        &emsp;&emsp;"classname": "CusPostProcess",<br>
        &emsp;&emsp;"config":{} <br>
    &emsp;}<br>
}

其中，input和out_layers中的name，需要将param_file放入[netron.app](https://netron.app/)查看才能得到.

对于pre_process和post_process，你需要自己实现一个Process类，将opencv::Mat转成ncnn::Mat, 示例如下：

```cpp
baseprocess.h
class BaseProcess
{
public:
    BaseProcess() {}
    virtual ~BaseProcess() {}
    virtual void LoadConfig() = 0;
    virtual void doProcess(LPARAM src, LPARAM dst) = 0;
}

template<typename T> BaseProcess* createProcess() { return new T(); };

cus_process.h
class CusPreProcess:public BaseProcess {
public:
    CusPreProcess():BaseProcess() {}
    ~CusPreProcess() {}
    void LoadConfig() override;
    void doProcess(LPARAM src, LPARAM dst) override;
private:
    float mean_vals[3], norm_vals[3];
    int input_w, input_h;
}

void CusPreProcess::LoadConfg() {
    input_w = json["input_w"];
    input_h = json["input_h"];
    int index = 0; 
    for (auto val : json["mean_vals"])
        mean_vals[index++] = val;
    index = 0; 
    for (auto val : json["norm_vals"])
        norm_vals[index++] = val;
}

void CusPreProcess::doProcess(LPARAM src, LPARAM dst) {
    cv::Mat& bgr = *(reinterpret_cast<cv::Mat*>(src));
    int img_w = bgr.cols, img_h = bgr.rows;

    //input ncnn mat
    ncnn::Mat& in = *(reinterpret_cast<ncnn::Mat*>(dst));
    in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, img_w,img_h, input_w, input_h);
    in.substract_mean_normalize(mean_vals, norm_vals);
}

NCNN_INFER::map_type NCNN_INFER::PreProcess_Register_Map["CusPreProcess"] = &createProcess<CusPreProcess>;

```

