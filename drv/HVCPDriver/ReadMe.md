# 说明

本源代码由样例修改而来，通常只支持WinDDK 7600.16385.1。

# WDK

链接: https://pan.baidu.com/s/183NtBGmwnaOH_HCAjerD_A?pwd=8888 提取码: 8888 

iso可直接刻录成光盘安装，也可以采用[7-Zip](http://www.7-zip.org)解压后运行解压后的目录中的安装程序。

# 编译

- 从GRMWDK_EN_7600_1.iso安装好构建环境。
- 从开始菜单进入构建环境。
- 进入当前源代码目录。
- 执行`build`构建。
- 编译完成后可得到dll文件与inf文件，直接替换[prebuilt](../prebuilt) 中相应架构的对应文件即可使用。

