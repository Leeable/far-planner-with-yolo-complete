# far-planner-with-yolo
# FAR Planner version with yolo to detect object<br>
![image](https://github.com/Leeable/far-planner-with-yolo/blob/Noetic/2022-06-19%20183632.gif)
## Tested in ubuntu 20.04, ubuntu 18.04, Manjaro  (you need to use AUR to build ROS1 noetic(recommend) or melodic)
## Note that the opencv version should higher than 3.3 opencv的版本要求要大于3.3<br>
### you need to build newer opencv from source 我们需要去opencv的官网进行源码编译，连接如下<br>
### download source from https://opencv.org/ <br>
> cd opencv-4.5.4<br>
> mkdir build & cd build<br>
> cd opencv-4.5.4<br>
> cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_TBB=ON -DBUILD_NEW_PYTHON_SUPPORT=ON -DWITH_V4L=ON -DINSTALL_C_EXAMPLES=ON -DINSTALL_PYTHON_EXAMPLES=ON -DBUILD_EXAMPLES=ON -DWITH_QT=ON -DWITH_GTK=ON -DWITH_OPENGL=ON ..<br>
> make<br>
> sudo make install<br>

### The yolo file path should be placed at src/yolo/config/default.yaml 请把yolo相关的文件放在src/yolo/config/default.yaml中<br>

and then, plesae use catkin_make 执行catkin_make<br>
> cd /far-planner-with-yolo<br>
> catkin_make<br>
> sudo chmod +x run.sh<br>
> ./run.sh<br>

### yolo cfg, weight and coconame should be downloaded and put at /src/yolo/data yolo相关的cfg、weight和coconame要放在/src/yolo/data这个目录下<br>

cfg weight coconames can be download from https://github.com/AlexeyAB/darknet 这个是对应的cfg、weight和coconames的下载连接，请选择合适的进行使用<br>


The original far_planner is here, please check the url below <br>
https://github.com/MichaelFYang/far_planner
