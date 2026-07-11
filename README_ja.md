# PicoMesh

PicoMeshは、Raspberry Pi PicoやArduinoなどの小規模マイコンで分散システムを構築するための、軽量で通信方式に依存しないフレームワークです。

非公開の研究プロジェクトAETERNAで開発した基盤のうち、他の開発者が再利用できる一般部分を切り出して育てます。推論モデル、データセット、投票アルゴリズム、未発表の故障管理手法など、研究固有の部分はこのリポジトリには含めません。

## 現在実装済み

- バージョン付きの小型メッセージフレーム
- 8ビットチェックサム
- ハートビートとタイムアウトによるノード生存管理
- I2C、UART、CAN、USB、PCシミュレータに接続できる通信抽象化
- PC上で実行できるテストとシミュレータ

## ビルド

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/picomesh_host_simulator
```

## AETERNAとの境界

PicoMeshへ移すものは、通信、ノード管理、シーケンス番号、ハートビート、再送などの汎用機能です。AETERNA側には、研究独自の推論、評価、合議、重み付け、実験データ、専用基板構成を残します。

詳細は[英語README](README.md)、[アーキテクチャ](docs/architecture.md)、[通信仕様](docs/protocol.md)を参照してください。
