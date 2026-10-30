<p align="right"><a href="README.md">English</a> · <strong>简体中文</strong></p>

# 奇门随身盘 · Qimen Passport

FoloToy AI Passport 的独立离线奇门学习盘。当前版本 **0.6.1**。

**[打开使用说明书](docs/GETTING_STARTED.zh_CN.md)**：首次校时、按键操作、指定时间起盘、七类象义、手机配网、换封面、休眠唤醒。

## 功能

- 当前起盘或指定 1900—2099 年的公历时间，前后两小时切局。
- 四柱、九宫、天地暗干、门星神、空亡及驿马；详情显示十二长生、月日时旺衰及墓刑迫状态。
- 宫、天盘干、地盘干、暗干、门、星、神七类象义，逐行滚动、顺序翻篇。
- 四种内置封面、自定义图片及名称，手机网页设置，断电保存。
- 本地、手机网页、蓝牙协议及网络校时；Wi-Fi 详情显示连接人数、外部网络状态、地址与信号。
- 双击确定黑屏省电休眠，右侧键唤醒恢复盘面和走时；双击上键查看封面。

## 快速开始

1. 长按确定 → 日期与时间，设置完整日期；返回 → 当前起盘。
2. 上下选宫，确定看详情，按下看符号象义。
3. 手机设置：Wi-Fi 网络 → 开启，连接设备显示的热点，打开 http://192.168.4.1 ，输入板上管理码。默认热点 Qimen-Passport，默认密码 12345678；已保存的自定义设置优先。

![七分类实际渲染预览](assets/images/seven-categories.png)

[四种封面渲染预览](assets/images/covers-0.4.1.png) · [社区封面插画](assets/images/qimen-passport-cover.png)。插画不是实拍，界面图为程序渲染。

## 构建

安装并激活 **ESP-IDF 5.5.3** 与 ESP32-C3 工具链，依赖固定在 `dependencies.lock`。主机测试需要 Python 3、C 编译器，网页测试需要 Node.js。

```sh
./tools/validate.sh --static
./tools/validate.sh --firmware
# 或一次运行两项：
./tools/validate.sh
```

可选的实际 LVGL 桌面渲染（先安装 Managed Components，另需 CMake 和 Ninja）：

```sh
cmake -S tests/native -B build/native -G Ninja
cmake --build build/native
mkdir -p build/previews
build/native/preview build/previews
```

此项使用真实 240×320 渲染代码与 48KiB LVGL 池，输出 PPM 预览。

固件门禁使用已登记默认配置在隔离目录构建，检查分区和镜像，生成 `build/FoloToy-AI-Passport-full.bin` 及 `build/firmware/` 内匹配的 ELF、MAP、校验值。产物文件名沿用上游工具约定；独立应用从 0.2.0 计版本，与之前混合版分开。

新设备使用已校验的**完整合并镜像，从 0x0 写入**；这可能清除原有设置。应用单体镜像不能写到 0x0。独立版含 NVS、PHY、单应用和双槽封面分区，无词书文件系统依赖。应用位于 0x10000—0x380000；封面位于 0x380000—0x400000，不触及旧混合版从 0x400000 开始的词书。分区变化后不能只刷应用；已有设备需核对并更新分区表，先保存可恢复备份。现有混合版升级需另外审核分段写入与资料保留方案，本项目不会自动修改或刷写设备。

## 验证与边界

0.6.1 已在原板验收网络页面、网络校时、联网休眠和唤醒。实际待机电流、续航及长期走时误差尚未测量。排盘使用固定北京时间 UTC+8，不模拟历史夏令时或真太阳时；当前时钟可校准 2024—2099 年，历史起盘可选 1900—2099 年。

排盘口径参照已记录的 OpenFate 默认四柱＋时家结果；测试不代表所有流派或所有日期完全一致。月日时旺衰以对应柱支作五行参照，不计算能量百分比。象义用于学习，不生成自动断语。

本版不包含背单词或私人资料。板级基础源自 FoloToy MIT 项目；适用硬件为 ESP32-C3、240×320、8MB Flash、无 PSRAM。

[使用说明](docs/GETTING_STARTED.zh_CN.md) · [协议](docs/PROTOCOL.zh_CN.md) · [结构](docs/ARCHITECTURE.zh_CN.md) · [来源与许可](docs/SOURCES.zh_CN.md) · [发布说明与校验值](docs/RELEASE.zh_CN.md)
