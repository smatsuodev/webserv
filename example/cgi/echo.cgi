#!/bin/bash

# CGI ヘッダー出力
echo "Content-Type: text/html"
echo ""

# HTML コンテンツ出力
echo "<html>"
echo "<head><title>Echo CGI</title></head>"
echo "<body>"
echo "<h1>Echo CGI</h1>"
echo "<p>このスクリプトはPOSTデータをエコーします。</p>"

if [ "$REQUEST_METHOD" = "POST" ]; then
    echo "<h2>受信したPOSTデータ:</h2>"
    echo "<pre>"
    # 標準入力からPOSTデータを読み取り
    cat
    echo "</pre>"
else
    echo "<h2>GET リクエストが送信されました</h2>"
    echo "<p>QUERY_STRING: $QUERY_STRING</p>"
fi

echo "<h2>環境変数:</h2>"
echo "<ul>"
echo "<li>REQUEST_METHOD: $REQUEST_METHOD</li>"
echo "<li>CONTENT_TYPE: $CONTENT_TYPE</li>"
echo "<li>CONTENT_LENGTH: $CONTENT_LENGTH</li>"
echo "<li>SCRIPT_NAME: $SCRIPT_NAME</li>"
echo "<li>PATH_INFO: $PATH_INFO</li>"
echo "</ul>"
echo "</body>"
echo "</html>"