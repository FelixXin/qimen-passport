# 奇门随身盘开发约定

独立 ESP32-C3、8MB、无 PSRAM 应用，派生自 FoloToy AI Passport。

- 保留已有项目和设备数据；不擅自擦除、刷机、提交、上传或发布。
- 驱动在 components/bsp，应用在 main，资源在 assets。
- 编辑前读取 README.zh_CN.md、docs/ARCHITECTURE.zh_CN.md 及相关模块。
- 使用 ESP-IDF 5.5.3，运行 tools/validate.sh；分别报告构建、主机测试、真机测试及未验证项。
- 非显示任务访问 LVGL 先持 BSP 锁；回调只入队，无线阻塞工作放在任务中。
- 新增中文需检查实际字形覆盖与页面显示。
- build/ 与设备日志为本地输出，不加入私人资料和凭据。
- 文档保持英中配对，保留上游与第三方许可。
