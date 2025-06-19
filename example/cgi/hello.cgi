#!/bin/bash

# CGI ヘッダー出力
echo "Content-Type: text/html"
echo ""

# HTML コンテンツ出力
echo "<html>"
echo "<head><title>Hello CGI</title></head>"
echo "<body>"
echo "<h1>Hello from CGI!</h1>"
echo "<p>このページはCGIスクリプトによって生成されました。</p>"
echo "<p>現在時刻: $(date)</p>"
echo "<p>環境変数:</p>"
echo "<ul>"
echo "<li>REQUEST_METHOD: $REQUEST_METHOD</li>"
echo "<li>REQUEST_URI: $REQUEST_URI</li>"
echo "<li>QUERY_STRING: $QUERY_STRING</li>"
echo "<li>SERVER_NAME: $SERVER_NAME</li>"
echo "<li>SERVER_PORT: $SERVER_PORT</li>"
echo "</ul>"
echo "</body>"
echo "</html>"