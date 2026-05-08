# 3D ASCII Zombie FPS (C Language)

C言語と `ncurses` ライブラリのみを使用して開発した、ターミナルで動作する一人称視点シューティングゲーム（FPS）です。

## 🕹️ 特徴
- **レイキャスティング・エンジン**: 三角関数を用いて、2Dマップからリアルタイムに3D空間をレンダリング。
- **CoDスタイル武器システム**: 右下に配置されたアスキーアートの銃身と、反動を意識した射撃メカニクス。
- **ゾンビAI**: プレイヤーを感知し、壁を避けながら追跡してくる敵キャラクター。
- **ダイナミック・ミニマップ**: プレイヤーの座標、視界方向、敵の位置をリアルタイムに同期表示。
- **カラーレンダリング**: 距離に応じた壁の陰影（ディザリング）と、視認性を高めるカラー表示。

## 🛠️ 技術スタック
- **Language**: C
- **Library**: `ncurses`, `math.h`
- **Technique**: Raycasting, Vector Math, Sprite Rendering

## 🚀 実行方法

### 必要な環境
- GCC (Cコンパイラ)
- ncurses ライブラリ (`sudo apt-get install libncurses5-dev` など)

### コンパイルと実行
```bash
gcc work62_3d.c -o zombie_fps -lncurses -lm
./zombie_fps
