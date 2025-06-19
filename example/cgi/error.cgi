#!/bin/bash

# エラーステータスを返すCGIスクリプト
echo "Status: 500 Internal Server Error"
echo "Content-Type: text/html"
echo ""

echo "<html>"
echo "<head><title>CGI Error</title></head>"
echo "<body>"
echo "<h1>500 Internal Server Error</h1>"
echo "<p>これはCGIエラーのテストページです。</p>"
echo "</body>"
echo "</html>"

# 標準エラーにもメッセージを出力
echo "CGI script encountered an error" >&2

exit 1