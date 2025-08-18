#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from http.cookies import SimpleCookie


def _to_int(v, default=0):
    try:
        return int(v)
    except Exception:
        return default


# 受信クッキーを読み取り
in_cookie = SimpleCookie()
raw_cookie = os.environ.get("HTTP_COOKIE", "")
if raw_cookie:
    in_cookie.load(raw_cookie)

count = _to_int(in_cookie["count"].value, 0) if "count" in in_cookie else 0
count += 1

# 送信クッキー（セッション cookie。永続化したい場合は Max-Age を設定）
out_cookie = SimpleCookie()
out_cookie["count"] = str(count)
out_cookie["count"]["path"] = "/"
# 例：1年保持したい場合は次行を有効化
# out_cookie["count"]["max-age"] = "31536000"

# ==== ヘッダー（LF 区切り） ====
print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8")
for morsel in out_cookie.values():
    # Morsel.OutputString() は "key=value; Path=/; ..." を返す（改行は含まない）
    print("Set-Cookie: " + morsel.OutputString())
print()  # 空行でヘッダー終了（LF のみ）

# ==== 本文 ====
print(f"""<!doctype html>
<html lang="ja">
<head>
  <meta charset="utf-8">
  <title>アクセスカウンタ</title>
</head>
<body>
  <p>このブラウザでのアクセスは <strong>{count}</strong> 回目です。</p>
</body>
</html>""")
