<p align="right"><a href="PROTOCOL.md">English</a> · <strong>简体中文</strong></p>

# 本地协议

USB 和 BLE 接收 UTF-8、LF 结尾的指令，每行除 LF 外最多 383 字节。BLE 使用 main/reader_ble.c 声明的自有 128 位服务，RX 写入、TX 通知；先订阅，再发送 AUTH，按有界片段写入（20 字节适用于默认 ATT MTU），等完整 LF 响应。连接不等于授权：先在设备上开启窗口，发送 AUTH、制表符和屏幕六位码。这是应用授权，不是蓝牙链路加密；只在可信近距离环境使用，勿把网络秘密交给不可信客户端。

支持 HELLO、GET、TIME＋制表符＋UTC 秒数、CAST、NET、WIFI＋制表符＋SSID UTF-8 十六进制＋制表符＋密码十六进制、AP＋名称及密码十六进制、STATS。不支持 PROFILE、AVATAR_* 或词书上传。RADIO 拒绝执行，Wi-Fi 开关由实体键控制，避免网页请求尚未回复就停止自身服务。

响应以 QIMEN、制表符、OK／ERR 开始。HELLO 附版本；GET 附时间戳，0 表示没有完整日期；TIME／CAST 无附加字段；WIFI 附非零请求号。NET 依次附 enabled、connected、connecting、provisioned、request ID、phase、IP，共 10 字段；需同一请求号＋CONNECTED＋connected/provisioned 才确认成功。STATS 附空闲堆、最大连续堆、空闲 DMA 内存及应用栈余量，共 7 字段，单位字节。状态响应不返回凭据。

HTTP POST /api/command 使用相同单行指令但不带 LF，X-Qimen-Key 请求头填写设备八位管理码。管理码和密码不写入 URL、日志或 localStorage。只在设备热点或可信网络使用；本地 HTTP 为明文，不是远程安全管理服务。

完整时间有范围校验，排盘支持北京时间 2024—2099 年。校时不会替换已存在的有效快照，需要 CAST、切局或本地设时保存后起新盘。手机蓝牙客户端不发送 TIME 时，设备不会自行获得手机时间。
