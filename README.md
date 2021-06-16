# 流媒体使用教程
 
## ZLMediaKit  
```shell
#install ssl ffmpeg
sudo apt-get install libssl-dev
sudo apt-get install libsdl-dev
sudo apt-get install libavcodec-dev
sudo apt-get install libavutil-dev
sudo apt-get install ffmpeg

#install srtp
git clone https://github.com/cisco/libsrtp.git
cd libsrtp
./configure --enable-openssl
make 
sudo make install

#install ZLMediaKit
git clone https://github.com/ZLMediaKit/ZLMediaKit.git
cd ZLMediaKit
mkdir build & cd build
cmake ..
make -j2
sudo make install

#run
sudo MediaServer -c /home/li/ZLMediaKit/conf/config.ini
```

## wvp-gb28181
```shell
#install openjdk
apt-get install openjdk-11-jre redis-server

#config database
spring:
    datasource:
        name: eiot
        url: jdbc:sqlite:/home/li/other_ws/wvp.sqlite
        username:
        password:
        type: com.alibaba.druid.pool.DruidDataSource
        driver-class-name:  org.sqlite.JDBC
        max-active: 1
        min-idle: 1

#run 以下路径使用绝对路径
java -jar /xxx/wvp-2.3.5.RELEASE.jar --spring.config.location=/xxx/application.yml
```
## 功能
- 截图
api: /index/api/getSnap  
参数:    
| 参数                  | 是否必选    |  释意                                   |
| :--------              | -----:    | :----:                                 |
| secret                | Y         |   default                                 |
| url                   | Y         |   需要截图的url                            |
| timeout_sec           | Y         |   截图失败超时时间，防止FFmpeg一直等待截图    |
| expire_sec            | Y         |   截图的过期时间，该时间内产生的截图都会作为缓存返|

- 数据持久化
再application.yml中使用sqlite

- webrtc
/ZLMediaKit/www/webrtc/index.html

