# コードベース構造

## ディレクトリ構成

### `/src` - ソースコード
- **`/cmd`** - メインエントリーポイント
  - `main.cpp` - サーバー起動

- **`/lib`** - ライブラリコード
  - **`/core`** - コアサーバー機能
    - `server.cpp/hpp` - メインサーバークラス
    - `virtual_server.cpp/hpp` - 仮想サーバー管理
    - `server_state.cpp/hpp` - サーバー状態管理
    - `/handler` - イベントハンドラー
    - `/action` - アクション処理
  
  - **`/transport`** - ネットワーク層
    - `listener.cpp/hpp` - ソケットリスナー
    - `connection.cpp/hpp` - 接続管理
    - `address.cpp/hpp` - アドレス処理
  
  - **`/http`** - HTTPプロトコル処理
    - `method.cpp/hpp` - HTTPメソッド
    - `status.cpp/hpp` - HTTPステータスコード
    - `mime.cpp/hpp` - MIMEタイプ
    - `/request` - リクエスト処理
      - `/reader` - リクエスト読み取りステートマシン
    - `/response` - レスポンス生成
    - `/handler` - HTTPハンドラー
      - `/middleware` - ミドルウェア（ログ、エラーページ）
  
  - **`/event`** - イベント駆動システム
    - `event_notifier.cpp/hpp` - epoll/kqueue ラッパー
    - `event_handler.cpp/hpp` - イベントハンドラー基底クラス
  
  - **`/cgi`** - CGI処理
    - `factory.cpp/hpp` - CGIプロセス生成
    - `request.cpp/hpp` - CGIリクエスト
    - `response.cpp/hpp` - CGIレスポンス
    - `meta_variable.cpp/hpp` - CGI環境変数
  
  - **`/config`** - 設定管理
    - `config.cpp/hpp` - 設定クラス
    - `/toml` - TOMLパーサー
  
  - **`/utils`** - ユーティリティ
    - `/types` - カスタム型（Option, Result, Either など）
    - `/io` - I/O ユーティリティ
    - `logger.cpp/hpp` - ログ機能
    - `string.cpp/hpp` - 文字列ユーティリティ
    - `auto_fd.cpp/hpp` - RAII ファイルディスクリプタ

### `/tests` - テストコード
- 各コンポーネントのユニットテスト
- `/utils` - テスト用ユーティリティ

### `/example` - サンプルファイル
- `/conf` - 設定ファイル例
- `/html` - 静的HTMLファイル
- `/cgi` - CGIスクリプト例
- `/uploads` - アップロード用ディレクトリ

### `/docs` - ドキュメント
### `/scripts` - ビルド・開発用スクリプト
### `/bin` - ビルド出力ディレクトリ

## アーキテクチャの特徴
- イベント駆動型（epoll/kqueue使用）
- ステートマシンベースのHTTPリクエスト処理
- RAII パターンによるリソース管理
- Result/Option 型による安全なエラー処理
- ミドルウェアパターンによる拡張可能な処理パイプライン