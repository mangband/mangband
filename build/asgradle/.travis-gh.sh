#!/bin/sh

CLIENT_D=mangclient-debug.apk
CLIENT_R=mangclient-release-unsigned.apk

cat <<"HTML"
<html>
<title>MAngband Android builds</title>
<head><meta charset="utf-8"/>
<!--<script src="https://cdnjs.cloudflare.com/ajax/libs/qrcode-generator/1.4.3/qrcode.min.js"></script>-->
<script src="https://cdn.jsdelivr.net/npm/davidshimjs-qrcodejs@0.0.2/qrcode.min.js"></script>
<style>div.qr:nth-of-type(even) { text-align: right; } img { display: inline !important; }</style>
</head>
<body onload="makecodes()">
HTML
echo "<div class='qr'><a href='${CLIENT_D}'>${CLIENT_D}</a><div></div></div>"
echo "<div class='qr'><a href='${CLIENT_R}'>${CLIENT_R}</a><div></div></div>"
echo Built on $(date)
cat <<"HTML"
<script>
function makecodes() {
var os = document.querySelectorAll('div.qr');
var ls = document.querySelectorAll('div.qr a');
var ps = document.querySelectorAll('div.qr div');
var i = 0;
for (i = 0; i < os.length; i++) {
    var d = os[i];
    var a = ls[i];
    var p = ps[i];
    var href = a.href;
    var qr = new QRCode(p, {
    text: href,
    width: 128,
    height: 128,
    colorDark : "#000000",
    colorLight : "#ffffff",
    correctLevel : QRCode.CorrectLevel.H
    });
}
}
</script>
</body>
</html>
HTML