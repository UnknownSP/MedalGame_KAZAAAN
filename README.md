# MedalGame_KAZAAAN

SEGA様のKAZAAAN!!のハードウェアを基にして、回路及びハーネス、プログラムを一から作成し動作させたものです。

リモートコントロール用の[サーバ](https://github.com/UnknownSP/MedalGame_RemoteControlServer)と[クライアント](https://github.com/UnknownSP/MedalGame_RemoteControlClient)に接続することでスマートフォンから操作をすることも可能になります。

また、赤外線リモコン操作用の[プログラム及び回路](https://github.com/UnknownSP/RemoteControl)と組み合わせることで部屋の電気も連動させることが可能です。

## 動作例

https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/6ec2da77-b9f0-4431-896d-a4ff326c2f17

部屋の電気連動

https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/43c4e1c7-e956-428e-9d36-ee12938be73d

## 回路及びハーネス

![KAZAAAN_circuit1](https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/c38948ab-6c1b-4da4-b5f3-5463e894f175)

### 各ステーション用基板(KAZAAAN_446_sub が動作)

![KAZAAAN_circuit_sub3](https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/74c2a7c1-2d2c-4903-a83a-f44f8ab52cf2)
![KAZAAAN_circuit_sub1](https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/d3750096-4acd-47d4-a442-4fee4e7ca133)
![KAZAAAN_circuit_sub2](https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/9c61c477-406e-46ec-9435-45fc632ceece)

### メイン基板(KAZAAAN_446_main が動作)

![KAZAAAN_circuit_main](https://github.com/UnknownSP/MedalGame_KAZAAAN/assets/39638661/d1478839-02ca-458e-9c85-dd3fdfd96123)

## 仕様

KAZAAN_446_mainはメイン処理、KAZAAN_446_subは各ステーションの処理を行います。今のところ1ステーション分のみ製作しています。
細かい仕様は以下に示します。

- KAZAAN_446_mainの行う処理
  - subとのCAN通信 
  - 1st,2nd,3rd,JPCのエンコーダ値の管理
  - 1st&2nd,3rd,JPCのBLDCモータ制御
  - センターユニットのLED及び上昇下降制御
  - JPC抽選の制御
  - 3rdホールドポケットの管理
- KAZAAN_446_subの行う処理
  - mainとのCAN通信
  - 1st,2nd,3rdのLED制御
  - 1st,2nd,3rdの抽選制御
  - 発射メカの制御
  - ホールドボール回収制御
  - 7セグ制御(未実装)

※本プロジェクトで使用している音源は全て実機の録音源を使用しております。

## 問題点及び要修正点

- sub基板のGNDベタ忘れ
- I2CからCANへの変更に伴うCANトランシーバ用の追加回路
  - 外付けしているものをsub基板内に入れ込む
- コネクタのピン数が同じものが同一基板に複数ある
  - XADに変更してコネクタをまとめる
