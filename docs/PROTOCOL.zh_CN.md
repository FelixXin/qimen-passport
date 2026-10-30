<p align="right"><a href="PROTOCOL.md">English</a> · <strong>简体中文</strong></p>

# 本地协议

USB 和 BLE 接收 UTF-8、LF 结尾的指令，每行除 LF 外最多 383 字节。BLE 使用 main/reader_ble.c 声明的自有 128 位服务，RX 写入、TX 通知；先订阅，再发送 AUTH，按有界片段写入（20 字节适用于默认 ATT MTU），等完整 LF 响应。连接不等于授权：先在设备上开启窗口，发送 AUTH、制表符和屏幕六位码。这是应用授权，不是蓝牙链路加密；只在可信近距离环境使用，勿把网络秘密交给不可信客户端。

支持 HELLO、GET、TIME＋制表符＋UTC 秒数、CAST、NET、WIFI＋制表符＋SSID UTF-8 十六进制＋制表符＋密码十六进制、AP＋名称及密码十六进制、STATS。不支持 PROFILE、AVATAR_* 或词书上传。RADIO 拒绝执行，Wi-Fi 开关由实体键控制，避免网页请求尚未回复就停止自身服务。

响应以 QIMEN、制表符、OK／ERR 开始。HELLO 附版本；GET 附时间戳，0 表示没有完整日期；TIME／CAST 无附加字段；WIFI 附非零请求号。NET 依次附 enabled、connected、connecting、provisioned、request ID、phase、IP，共 10 字段；需同一请求号＋CONNECTED＋connected/provisioned 才确认成功。STATS 附空闲堆、最大连续堆、空闲 DMA 内存及应用栈余量，共 7 字段，单位字节。状态响应不返回凭据。

HTTP POST /api/command 使用相同单行指令但不带 LF，X-Qimen-Key 请求头填写设备八位管理码。管理码和密码不写入 URL、日志或 localStorage。只在设备热点或可信网络使用；本地 HTTP 为明文，不是远程安全管理服务。

完整时间有范围校验，排盘支持北京时间 1900—2099 年，当前时钟校准仍限 2024—2099 年。校时不会替换已存在的有效快照，需要 CAST、切局或本地设时保存后起新盘。手机蓝牙客户端不发送 TIME 时，设备不会自行获得手机时间。

## 封面扩展（0.3.0）

`NAME\t<UTF-8 hex>` 保存最多八个字的显示名称，空值恢复应用名称。`COVERINFO` 返回 `QIMEN / OK / COVERINFO / name_hex / custom / generation` 六个制表符分隔字段。`COVERRESET` 保留名称，恢复太极图像。名称需通过 UTF-8、长度和设备字库检查。

HTTP `POST /api/cover` 接收严格 153600 字节的 240×320 小端 RGB565。请求头需 `X-Qimen-Key` 和八位十六进制 `X-Cover-CRC32`；服务端使用固定缓冲写入非活动槽，完整校验后提交。失败不替换当前图。`GET /api/cover` 使用同一鉴权，返回当前 RGB565 或无自定义图片时的 204。无文件名、目录或任意 Flash 地址输入。

## 0.4.0 扩展

`CASTAT\t<signed UTC seconds>`：起指定时刻的盘并显示九宫，范围为 -2209017600 至 4102415999；不修改当前时钟。`TIME` 仍只校准 2024—2099 年。`STYLE\t<0..4>` 保存封面样式，依次为墨色太极、浅色太极、大字时钟、简洁名片、自定义图片；无图片时拒绝 4。`STYLEINFO` 返回五字段 `QIMEN / OK / STYLEINFO / style / image_exists`。切换样式不删除上传图片；网页上传后发送 STYLE 4 选择新图。上述指令沿用 USB、经 AUTH 的 BLE 和经管理码鉴权的 HTTP。

## 0.5.1 热点密码

`AP\t<ssid_hex>\t<password_hex>` 保存设备热点名称和密码。SSID 留空表示保留当前热点名，例如 `AP\t\t3132333435363738` 保存密码 12345678。空 SSID 的 WIFI 命令仍被拒绝。热点设置不影响外部网络凭据；配置成功只表示已保存，须通过板上关闭再开启 Wi-Fi 才应用，HTTP 请求不会主动关闭自己的服务。已有自定义热点配置优先于默认值。

## 0.6.0 网络状态

`NETINFO` 返回 16 个 Tab 分隔字段：QIMEN、OK、NETINFO、enabled、portal、热点客户端数（-1 未知）、当前 SSID hex、已存 SSID hex、状态、IP、RSSI（-127 未知）、断开原因编号、重试数、设备热点名 hex、热点变更待应用、错误分类。状态编号依次为 0 关闭、1 未配置、2 连接中、3 重连中、4 路由器已连接、5 失败、6 等待。错误分类 0 无、1 断开、2 丢失地址、3 超时、4 保存失败、5 队列忙、6 启动失败、7 无已存网络、8 蓝牙忙；断开原因按 SDK 提供的原始编号，不猜测具体密码错误。回复不含密码。旧 NET 的 10 字段保持兼容。

`RECONNECT` 请求重连已保存网络，成功接收返回 QIMEN/OK/RECONNECT/请求编号；接收不等于连接成功。继续通过 NET 与 NETINFO 观察。未保存网络返回 ERR/NO_SAVED。AP 现在允许名称或密码任一留空，保留对应原值；两者同时留空被拒绝。权限沿用管理码 HTTP、AUTH 后蓝牙及 USB 规则。
