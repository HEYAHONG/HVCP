# 说明

此库主要用于辅助编写使用HVCP的应用。现具有以下功能:

- 枚举已安装的HVCP驱动的串口号。
- 写HVCP串口。
- 读HVCP串口。
- 删除HVCP串口(需要管理员权限)。
- 添加HVCP串口(需要管理员权限，若未安装驱动需要在设备管理器中给未知设备添加驱动)。

注意:尽可能使用新的编译器/IDE，如使用MSVC时推荐使用VS 2019 16.8及更新版本。

## 运行环境

此库可在以下环境运行:

- Windows Win32
- [MSYS2](https://msys2.org)
- [Cygwin](https://cygwin.org)