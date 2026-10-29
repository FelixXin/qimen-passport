<p align="right"><a href="ARCHITECTURE.md">English</a> · <strong>简体中文</strong></p>

# 项目结构

`main/reader_qimen*` 为纯排盘、状态表和有界富文本输出，不依赖 ESP-IDF 或 LVGL。`qimen_navigation` 为纯状态机，包含完整日期校验、唤醒手势抑制及七篇象义边界。`qimen_ui` 复用固定 LVGL 对象，调用者持有 BSP 锁。`qimen_app` 持有时钟、盘面和状态，消费按键、USB、BLE、HTTP 队列；传输回调不直接操作界面。

网络和蓝牙工作任务保留混合版的回滚与重试生命周期，移除词书上传。HTTP 使用有界消息箱；BLE 使用三分钟应用 AUTH 会话与代际标识。协议无资料、头像和词书操作，响应标识为 QIMEN，与原厂安装器及旧 CET6 GET 响应不兼容。

只使用 `qimen_app` 设置与 `qimen_net` 网络记录，不挂载文件系统；时间锚点位于内存。板级驱动 `components/bsp` 原样复用，音频驱动源码属于上游基础代码，应用未注册音频功能。

抽离不改动混合版源码、依赖或设备。依赖目录、日志、产物保持忽略。本目录是源码快照而非带上游历史的 clone；发布时应整理到从上游基线创建的 fork 功能分支，不能冒充上游 Git 历史。
