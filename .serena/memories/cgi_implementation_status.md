# CGI実装ステータス

## 完了した実装 (2025-08-10)

### 1. CGIハンドラーの基本実装
- `src/lib/http/handler/cgi_handler.cpp`のTODO項目を実装
  - `isCgiRequest()`: /cgi/で始まり.cgiで終わるパスを判定
  - `getScriptName()`: スクリプト名を抽出（PATH_INFO除去）
  - `getPathInfo()`: PATH_INFO部分を抽出
  - server port取得: `getLocalAddress().getPort()`使用

### 2. CGIアクション実装
- `RunCgiAction`: socketpairでIPCソケット作成、fork実行
  - CGIメタ変数を環境変数として設定
  - クライアントFDとPIDの管理追加

### 3. CGIハンドラー実装
- `WriteCgiRequestBodyHandler`: リクエストボディをCGIプロセスへ送信
  - 部分書き込み対応（EAGAIN/EWOULDBLOCK）
- `ReadCgiResponseHandler`: CGIレスポンス読み取り
  - 子プロセス終了検出とwaitpid()によるクリーンアップ

## 既知の制限事項とTODO

### 設計上の制限
1. **設定アクセス不可**
   - CgiHandlerがLocationContextにアクセスできない
   - `/cgi/`パスと`.cgi`拡張子がハードコード
   - TODO: configファイルから読み取る機能追加

2. **クライアントへのレスポンス送信未完**
   - CGIレスポンス受信後、元のクライアントへの送信機構が不完全
   - クライアントConnectionオブジェクトへのアクセス方法が必要

3. **CGIプログラムパス解決**
   - 現在: `/cgi/`を`example/cgi/`にハードコードマッピング
   - TODO: 設定ファイルのドキュメントルートとの統合

### 技術的債務
- CGIヘッダーパーサーが簡易実装（Content-Type固定）
- タイムアウト処理未実装
- QUERY_STRING環境変数の処理未実装

## 関連ファイル
- `src/lib/core/action/run_cgi_action.cpp`
- `src/lib/core/handler/read_cgi_response_handler.cpp`
- `src/lib/core/handler/write_cgi_request_handler.cpp`
- `src/lib/http/handler/cgi_handler.cpp`