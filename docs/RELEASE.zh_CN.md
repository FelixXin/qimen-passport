# 0.5.0 更新内容（相对 0.4.1）
- 双击确定进入真正省电休眠：关闭屏幕、无线及外设，停止应用运行，仅保留内部走时，不定时自动唤醒。
- 右侧任意键唤醒，恢复原页面、原盘面及经过休眠时间后的时钟，避免唤醒按键误操作。
- 双击上键继续查看封面；原双击确定“当前起盘”改从设置菜单第一项进入。
- 无线退出完成后才入睡，增加按键释放检查、退出失败提示和外设关闭失败后的恢复。
- 关闭未使用音频部分的空闲耗电；休眠恢复状态不反复写入存储，保留现有资料及网络设置。
- 已在原板验收休眠、唤醒、时间连续和封面；实际待机电流及续航尚未测量。

完整合并镜像从 `0x0` 安装；保留数据的应用更新仅在分区核对一致后写入 `0x10000`。首次完整安装可能替换原程序和设置，请先备份。

Full image: 2837392 bytes; SHA-256 `1d2e6ed02b0f95c32d3914cafd79388024a30fd789f3a556f9368bb6d6538c67`.

Application: 2771856 bytes; SHA-256 `68c03ace91dc679ad778e8cf6fe433094549e38d68aca22e273db799f81dbc88`.

Source: https://github.com/FelixXin/qimen-passport/tree/v0.5.0 .

Community update: prepared for original project 1758; submission is not yet confirmed.
