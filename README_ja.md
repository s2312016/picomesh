# PicoMesh

PicoMeshは、Raspberry Pi PicoやArduinoなどの小規模マイコンで分散システムを構築するための、軽量で通信方式に依存しないフレームワークです。

非公開の研究プロジェクトAETERNAで開発した基盤のうち、他の開発者が再利用できる一般部分を切り出して育てます。推論モデル、データセット、投票アルゴリズム、未発表の故障管理手法、専用基板の製造データなど、研究固有の部分はこのリポジトリには含めません。

## 現在実装済み

- ヒープ確保を使わない固定容量メッセージフレーム
- プロトコルバージョンと8ビットチェックサム
- ハートビートとタイムアウトによるノード生存管理
- シーケンス番号の周回、重複、古いパケットの判定
- ACK、再送タイムアウト、最大試行回数
- UARTなどの連続バイト列を復元するストリームデコーダ
- Arduino Wireのコントローラ／周辺機器アダプタ
- Arduino用ハートビート送受信サンプル
- Pico SDKのI2Cコントローラ／UARTアダプタ
- PC上で実行できるテストとシミュレータ

## PCでのビルド

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/picomesh_host_simulator
```

## Arduinoでの利用

このリポジトリをArduinoライブラリとして追加し、次のサンプルを開きます。

- `ArduinoHeartbeatNode`
- `ArduinoHeartbeatController`

サンプルではI2Cアドレス`0x42`、100 kHzを使用します。PicoのI2Cは3.3 V系なので、5 Vプルアップが存在する場合は双方向レベル変換回路を使用してください。

## Pico SDKでの利用

`pico_sdk_init()`後に、ルートのCMakeと`ports/pico-sdk`を`add_subdirectory`で追加します。詳細は[`ports/pico-sdk/README.md`](ports/pico-sdk/README.md)を参照してください。

## AETERNAとの境界

PicoMeshへ移すものは、通信、ノード管理、シーケンス番号、ハートビート、再送などの汎用機能です。AETERNA側には、研究独自の推論、評価、合議、重み付け、実験データ、故障シナリオ、専用基板構成を残します。

詳細は[英語README](README.md)、[アーキテクチャ](docs/architecture.md)、[通信仕様](docs/protocol.md)、[抽出方針](docs/aeterna_extraction.md)を参照してください。
