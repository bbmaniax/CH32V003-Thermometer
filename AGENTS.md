# CH32V003-Thermometer 開発

## 概要

CH32V003-Thermometerは、CH32V003マイコンを使用した温度計のプロジェクトです。
このドキュメントには AI Coding Agent を使用して効率的にアプリケーションの保守を行うために必要な情報を記載します。

## 保守対象の仕様

### 機能仕様

- DS18B20 から温度を取得し、0.1℃単位の `int16_t` で扱う
- OLED (SSD1306, 128x32, I2C) に温度を表示する
- 表示モードは以下の2種類
	- チャート表示（温度推移 + 現在温度）
	- テキスト表示（現在温度の大文字表示）
- ボタン操作
	- 短押し: 表示モード切り替え
	- 長押し: 画面上下反転

### ハードウェア設定

- ボタン: `PD0`
- DS18B20: `PC5`
- 温度オフセット: `-9` (0.1℃単位)
- 表示解像度: `128x32`
- 測定間隔: `3000ms`
- グラフ横ステップ: `3px`

### 温度値の扱い

- 単位は 0.1℃（例: 253 は 25.3℃）
- 無効値は `INT16_MIN`
	- `INVALID_SENSOR_VALUE`
	- `INVALID_TEMPERATURE_VALUE`

## ソフトウェア構造

### ファイル構成と責務

- `CH32V003-Thermometer.ino`
	- 依存オブジェクトの生成
	- `setup()/loop()` の制御
	- ボタンイベント/測定完了イベントに応じた再描画制御
- `SensorManager.h/.cpp`
	- DS18B20 の非同期読み取り状態機械
	- 状態: `IDLE` / `REQUESTING` / `READING`
- `SensorDataHistory.h/.cpp`
	- 固定長履歴バッファ（先頭追加）
	- 最小/最大値計算
- `Model.h/.cpp`
	- 現在温度および履歴へのアクセス抽象
- `View.h/.cpp`
	- テキスト表示/チャート表示
	- 画面反転・モード切替
- `DS18B20.h/.cpp`
	- DS18B20 プロトコル実装
	- `requestTemperature()` / `readTemperature()` API
- `OneWire.h/.cpp`
	- 1-Wire 低レイヤ実装
	- AVR と CH32V003 (`__riscv && CH32V003`) を分岐
- `SSD1306.h/.cpp`
	- フレームバッファ描画・I2C転送・簡易テキスト描画
- `Font5x7.h`
	- 5x7フォント定義（度記号 `0x01` を含む）

### レイヤ構造

- Application: `CH32V003-Thermometer.ino`
- Domain/UI: `Model`, `View`, `SensorDataHistory`
- Device service: `SensorManager`
- Device driver: `DS18B20`, `OneWire`, `SSD1306`

依存方向は上位から下位のみを維持し、下位層から上位層を参照しないこと。

## 実行フロー

### setup

1. ボタン・1-Wire・センサーマネージャを初期化
2. `model.begin()` / `view.begin()` を呼び出し

### loop

1. `button.update()`
2. `sensorManager.update()`
3. 長押しなら `view.flip()`
4. クリックなら `view.switchToNextViewMode()`
5. 測定完了なら `model.update(data)`
6. 変更があれば `view.render()`
7. `delay(10)`

## 各モジュールの保守ポイント

### SensorManager

- 測定間隔は最小 `750ms` を下回らない
- `isReady()` は結果取得フラグを消費する（1回だけ true）
- 温度取得失敗時は `INVALID_TEMPERATURE_VALUE` を保存

### SensorDataHistory

- `prepend()` は先頭挿入のため、既存値を後方にシフト
- `getMinMaxValue()` は有効値のみ評価

### View

- チャート表示は履歴の最小値/最大値で縦方向に正規化
- レンジ0の場合は中央ラインで表示
- 温度表示の `C` は `\001C` で描画（度記号 + C）

### DS18B20 / OneWire

- DS18B20の公開APIは `requestTemperature()` / `readTemperature()` を使用する
- CH32V003分岐では `ch32v00x.h` を使ったGPIO直接制御を維持
- タイミング依存処理（`delay_us` 等）の変更は影響が大きいため最小限にする

## ビルド・書き込み・検証

### 主要コマンド

- 依存導入: `make install`
- ビルド: `make build/ch32v003`
- 書き込み: `make deploy/ch32v003`

### Makefile上の重要設定

- `PROJECT=CH32V003-Thermometer`
- `LIBS=DigitalButton`
- `BOARDS=ch32v003`
- `CORES=ch32-riscv-arduino:ch32riscv`
- `build/ch32v003` では `-flto` を付与
- `deploy/ch32v003` は `openocd` + `wch-riscv.cfg` で `.elf` を書き込み

### 検証手順

1. `make build/ch32v003` を実行してコンパイル確認
2. 実機接続済みなら `make deploy/ch32v003` を実行
3. 実機確認
	 - 温度表示が更新される
	 - 短押しで表示モード切替
	 - 長押しで上下反転

## 変更ルール（AI Coding Agent向け）

- 既存公開API・ファイル構成・定数名を維持する
- 新規機能追加より、既存機能の安定保守を優先する
- 変更は最小差分で行う
- ピン配置・表示サイズ・測定間隔を変更する場合は必ず理由を明記する
- 変更後は必ず `make build/ch32v003` を実行して確認する
