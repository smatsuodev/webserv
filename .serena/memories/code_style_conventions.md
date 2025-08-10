# コードスタイルと規約

## C++ 標準
- 現在は C++11 を使用（将来的に C++98 への移行を予定）
- C++03 標準に基づくフォーマット設定

## フォーマット設定（.clang-format）
- **ベーススタイル**: LLVM
- **インデント幅**: 4スペース
- **タブ幅**: 4
- **行の最大長**: 120文字
- **ブレーススタイル**: カスタム（開き括弧は同じ行）
- **名前空間のインデント**: すべてインデント
- **インクルードのソート**: 無効

## 命名規則（観察された規則）
- **クラス名**: PascalCase（例：`Server`, `EventNotifier`）
- **メソッド名**: camelCase（例：`loadConfigFromFile`, `isNone`）
- **ファイル名**: snake_case（例：`event_notifier.cpp`, `request_parser.hpp`）
- **定数/マクロ**: UPPER_SNAKE_CASE（例：`LOG_ERROR`）

## ディレクトリ構造
- **ヘッダーファイル**: `.hpp` 拡張子
- **実装ファイル**: `.cpp` 拡張子
- **ヘッダーと実装**: 同じディレクトリに配置

## 静的解析（.clang-tidy）
以下のカテゴリのチェックが有効：
- bugprone-* （バグの可能性があるコード）
- cert-* （CERT C++ セキュアコーディング標準）
- performance-* （パフォーマンス問題）
- readability-misleading-indentation
- readability-redundant-declaration

## その他の規約
- **Non-copyable クラス**: `NonCopyable` を継承して実装
- **エラー処理**: `Result<T, E>` と `Option<T>` を使用
- **ログ**: `LOG_ERROR`, `LOG_INFO` などのマクロを使用
- **スマートポインタ**: AutoFd, AutoDeleter などのカスタム RAII クラス
- **名前空間**: 機能ごとに分割（transport, http, event など）