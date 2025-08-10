# 開発コマンド

## ビルド関連
```bash
# ビルドディレクトリを作成してビルド
mkdir -p build
cd build
cmake ..
make

# デバッグビルド（AddressSanitizerとUndefinedBehaviorSanitizer有効）
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# リリースビルド
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## 実行
```bash
# サーバーを起動
./bin/webserv example/conf/webserv.toml
```

## テスト
```bash
# ビルドディレクトリから全テストを実行
cd build
ctest

# 詳細な出力でテストを実行
ctest -V

# 特定のテストのみ実行
ctest -R <test_name>
```

## コードフォーマット
```bash
# Gitで管理されている全ての .cpp と .hpp ファイルをフォーマット
make format

# または直接 clang-format を使用
clang-format -i $(git ls-files '*.cpp' '*.hpp')
```

## 静的解析
```bash
# clang-tidy を実行（compile_commands.json が必要）
clang-tidy src/**/*.cpp -p build/
```

## Git コマンド
```bash
# 現在のブランチ: run-cgi-2
# メインブランチ: main

# 状態確認
git status

# 差分確認
git diff

# コミット履歴
git log --oneline -10
```

## システムユーティリティ (Darwin/macOS)
```bash
# ファイル一覧
ls -la

# ディレクトリ移動
cd <directory>

# ファイル検索
find . -name "*.cpp"

# テキスト検索（ripgrep推奨）
rg <pattern>

# プロセス確認
ps aux | grep webserv

# ポート使用状況確認
lsof -i :8080
```