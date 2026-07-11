# PicoMesh

[English](README.md)

PicoMeshは、Raspberry Pi PicoやArduinoなどの小規模マイコンで分散システムを構築するための、軽量で通信方式に依存しないフレームワークです。

通信フレーム、ノード生存確認、シーケンス番号、ACK、再送などを、プロトコル上重要な経路で動的メモリを使わず提供します。同じC++17コアをPC上のシミュレータ、Arduino、Raspberry Pi Pico SDKから利用できます。

> **現在の状態:** 1.0未満の開発版です。PC・Arduino・Pico/Pico 2の自動ビルドは整備済みで、初回リリースに向けた実機相互接続試験を進めています。

## 主な機能

- 最大32バイトの固定容量・バージョン付きフレーム
- フレーム処理、ストリーム復元、ノード管理、再送キューでヒープ確保を不使用
- 最大32ノードのハートビートとタイムアウト管理
- シーケンス番号の周回、重複、古いパケットの判定
- ACK、再送タイムアウト、最大試行回数
- I2C、UART、CAN、USB CDC、シミュレータ向け通信抽象化
- Arduino Wireのコントローラ／周辺機器アダプタ
- Pico/Pico 2 SDKのI2Cコントローラ、I2C周辺機器、UARTアダプタ
- PC用テスト、損失リンクシミュレータ、サニタイザ、CodeQL
- CMakeインストールパッケージ、Arduino／PlatformIO用メタデータ

固定上限は[`docs/resource_limits.md`](docs/resource_limits.md)にまとめています。

## PCでのビルド

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/picomesh_host_simulator
```

## Arduinoでの利用

このリポジトリをArduinoライブラリとして追加し、次のサンプルを開きます。

- `ArduinoHeartbeatNode`
- `ArduinoHeartbeatController`
- `ArduinoMultiNodeController`

基準サンプルはI2Cアドレス`0x42`、100 kHzを使用します。Pico側は3.3 V系なので、接続前に[`docs/hardware_wiring.md`](docs/hardware_wiring.md)を確認してください。

## Pico SDKでの利用

`pico_sdk_init()`後に、ルートと`ports/pico-sdk`を`add_subdirectory`で追加します。詳細は[`ports/pico-sdk/README.md`](ports/pico-sdk/README.md)を参照してください。

## 主要ドキュメント

- [アーキテクチャ](docs/architecture.md)
- [通信仕様](docs/protocol.md)
- [互換性方針](docs/compatibility.md)
- [固定リソース上限](docs/resource_limits.md)
- [用途例](docs/use_cases.md)
- [実機検証手順](docs/hardware_validation.md)
- [公開範囲](docs/project_boundary.md)

## 保守と貢献

貢献方法は[`CONTRIBUTING.md`](CONTRIBUTING.md)、意思決定は[`GOVERNANCE.md`](GOVERNANCE.md)、保守担当は[`MAINTAINERS.md`](MAINTAINERS.md)に記載しています。脆弱性は公開Issueではなく[`SECURITY.md`](SECURITY.md)の方法で報告してください。

## ライセンス

Apache License 2.0です。詳細は[`LICENSE`](LICENSE)を参照してください。
