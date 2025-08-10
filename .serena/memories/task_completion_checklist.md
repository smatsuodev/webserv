# タスク完了時のチェックリスト

タスクを完了する前に、以下の項目を確認してください：

## 1. コードフォーマット
```bash
make format
# または
clang-format -i $(git ls-files '*.cpp' '*.hpp')
```

## 2. ビルドの確認
```bash
# クリーンビルド
cd build
make clean
make
```

## 3. テストの実行
```bash
# 全テストを実行
cd build
ctest

# 失敗したテストがある場合は詳細を確認
ctest -V --rerun-failed
```

## 4. 静的解析（オプション）
```bash
# clang-tidy でコードをチェック
clang-tidy src/**/*.cpp -p build/
```

## 5. 実行確認
```bash
# サーバーが正常に起動するか確認
./bin/webserv example/conf/webserv.toml
```

## 6. Git の状態確認
```bash
# 変更内容の確認
git status
git diff

# 不要なファイルが含まれていないか確認
```

## 重要な注意事項
- **コミットメッセージにクレジットを書かない**（CLAUDE.md の指示通り）
- テストが全て通ることを確認
- 新しい機能を追加した場合は、対応するテストも追加
- エラーハンドリングが適切に実装されているか確認
- メモリリークがないか確認（特にデバッグビルドで AddressSanitizer を使用）