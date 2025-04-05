psisimux - メディアファイルとpsisiarcの書庫などを結合してMPEG-TSとして出力する

使用法:

psisimux [-s seek][-m msec_seek][-r range][-u duration][-b broadcast_id][-t time][-p][-8][-x caption_ext][-y data_ext][-c caption_src][-d data_src][-e] media_src dest

-s seek (bytes), default=0
  出力ファイルの初期シーク量。0未満のときはファイル末尾から-(seek+1)だけ前方にシークする。
  "-m"オプションとは併用できない。

-m msec_seek, default=0
  出力ファイルのミリ秒単位の初期シーク量。0未満のときはファイル末尾から-(msec_seek+1)だけ前方にシークする。
  "-s"オプションとは併用できない。

-r range (bytes), default=-1
  非負のとき出力サイズを制限する。負のとき無制限。

-u duration (msec), default=-1
  非負のとき出力のミリ秒単位の長さを制限する。負のとき無制限。

-b broadcast_id, default=1/2/3
  NetworkID/TransportStreamID/ServiceID を'/'区切りで指定。
  "-y"または"-d"オプションで書庫を読み込まなかった場合に使われる。

-t time, default=946717200
  UNIX時間で表したTOT(Time Offset Table)の開始時刻を指定。
  既定値は2000-01-01T09:00:00に相当する値。
  "-y"または"-d"オプションで書庫を読み込まなかった場合に使われる。

-p
  出力の先頭に情報を格納したNULL-TSパケットをつける。
  ペイロードには以下の情報がリトルエンディアンで格納される。
  8-15byte: 出力ファイルサイズ(bytes)
  16-23   : 出力開始位置(bytes)
  24-27   : 出力ファイルの長さ(ミリ秒)
  28-31   : 出力開始位置(ミリ秒)
  ファイルオープンに成功したがシーク結果がファイルの範囲外だった場合はこのパケットのみ出力した上で失敗扱いになる。
  このとき出力開始位置にはファイルサイズや長さと同じ値が格納される。

-8
  文字コードがUTF-8の字幕をARIB8単位符号に変換する。
  再生ソフトがUTF-8のARIB字幕に未対応で文字化けする場合に使う。

-x caption_ext
  入力メディアファイル名から拡張子を取り除いてcaption_extを追加した文字列を字幕ファイル名とする。
 `psisimux -x .vtt foo.mp4 bar.m2t` は `psisimux -c foo.vtt foo.mp4 bar.m2t` と等価。

-y data_ext
  入力メディアファイル名から拡張子を取り除いてdata_extを追加した文字列を書庫ファイル名とする。
 `psisimux -y .psc foo.mp4 bar.m2t` は `psisimux -d foo.psc foo.mp4 bar.m2t` と等価。

-c caption_src
  字幕ファイル名。
  https://github.com/xtne6f/b24tovtt でWebVTTに整形されたARIB字幕データを指定する。
  ファイルが存在しなかったり内容が不正な場合は無視される。

-d data_src
  書庫ファイル名。
  https://github.com/xtne6f/psisiarc で出力された書庫を指定する。
  ファイルが存在しない場合は("-e"オプションでなければ)無視される。

-e
  映像と音声の結合を省略する。
  書庫の展開と(存在すれば)字幕の結合のみ行う。
  書庫ファイルが存在しなければ失敗する。
  入力メディアファイル名は必須だがファイルは存在しなくてもよい("-x"や"-y"オプションの解釈に使われるだけ)。

media_src
  入力メディアファイル名。
  1映像(H.264/H.265)+1～2音声(AAC)のMP4ファイルを指定する。
  CodecIDが"hev1"のH.265には未対応。

dest
  出力ファイル名、または"-"で標準出力。

説明:

たとえば
> psisimux -x .vtt -y .psc foo.mp4 bar.m2t
とすると、字幕データ(foo.vtt)やデータ放送など(foo.psc)をfoo.mp4と結合してTSファイルを出力できる。

TSファイルの形態でないとメタデータを活用できない再生ソフトなどの都合に合わせて一時的に変換する用途を想定している。

その他:

ライセンスはMITとする。
