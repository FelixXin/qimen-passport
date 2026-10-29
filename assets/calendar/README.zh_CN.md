<p align="right"><a href="README.md">English</a> · <strong>简体中文</strong></p>

# 离线历法

0.4.0 的节气表覆盖 1899—2100 年，排盘支持公历 1900—2099 年的固定北京时间 UTC+8。数据使用有符号 64 位秒数。

原目录记录的 SHA256 与固定 Git blob 的实际内容不一致。本次分别读取官方 Git blob `355b7bcd6fcc773b79e8bd179f0df3c5fb73a4f1` 和 npm lunar-javascript 1.7.7 发布包，确认 lunar.js 字节完全一致，实际 SHA256 为 `9750324bfe1aa63c146f8c72b1143df924466c11c8a5277d7d9225c541a18aaa`。重新生成的 2023—2100 全部 1,872 个节气时间戳与旧表完全一致；原 516 个四柱样例也一致。

生成依赖和 MIT 许可固定在 tools/vendor/，仅构建工具读取，不编入固件。新增 1,724 组历史样例及逐日范围约束。参考库同源对比不能代替对所有流派的独立天文验证。

```sh
node tools/generate_qimen_terms.js tools/vendor/lunar.js main/reader_qimen_terms.inc
node tools/generate_history_fixtures.js tests/qimen_calendar_fixtures.json tests/qimen_history_fixtures.json
```
