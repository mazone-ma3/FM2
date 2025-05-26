;===========================================
;	FM(YM2413) MUSIC PLAYER MSX-DOS Ver.
;	(MSX VERSION) 2024/8/29- m@3
;	VERSION 2.00
;
;	FM3音+PSG3音出力の組み込み用BGMドライバ
;	テンポずれ修正＋フェードアウト付き
;
;	演奏データは専用のコンパイラで
;	ＭＭＬから生成します
;
;	コメントは後から付けたものです
;===========================================
;-------------------------------------------
; ＜データ型式＞
; 0BAFFH番地から0番地の方向に入っていく
; 各演奏コマンドは数値で指示(可変長)
; 
; ＜コマンド一覧＞
; 0-96       音(音長n、nは60/n秒間)、0は休符
; 128-224,n  音(音長指定無し)、128は休符
; 225,m,n    SOUND m,n
; 226,n      VOLUME=n、n=16ならエンベローブ
; 227,n      LENGTH=n、0-96のコマンドの音長
; 254        全てのパートの同期を取る
; 255        ダ・カーポ
;-------------------------------------------
; 参考 PC-9801マシン語サウンドプログラミング / MSX-DataPack

;	ASEG

;WRTPSG	EQU	0093H		; ＰＳＧに出力するBIOSルーチン
PSGDATA	EQU	(0DB3EH-0400H)		; ＰＳＧ音階出力値格納番地
MBOOT	EQU	(0DB00H-0400H) 		; 曲演奏データ先頭番地の格納アドレス

key_table equ (0DB3EH-0300H)

;MDATA	EQU	0DBFFH		; 演奏データ（0番地に向かって格納する）
HOOK	EQU	0FD9FH		; システムの1/60秒割り込みフック
PARTSUU	EQU	6		; 演奏する最大パート数
FEEDTIME EQU	12		; フェードアウトレベル

;	ORG	0DC00H		; プログラムの開始番地(BASICからUSR関数で実行)

;	ORG	0BB00H
	ORG	0D900H
	JP	INIT
	JP	STOP
	JP	FEEDO
PARA:
	DB	0,0
PARA2:
	DB	0
PARA3:
	DB	10111000B
PARA4:
	DB	1
PARA5:
	DB	PARTSUU
PARA6:
	DB 0FFH

;-------------------------------------------
;	初期設定ルーチン
;-------------------------------------------

INIT:
;	LD	A,(PARA6)
;	CP	0FFH
;	RET	Z
	LD	A,(PARA2)
	LD	E,A
	LD	A,6
	CALL	WRTPSG
	LD	A,(PARA3)
	LD	E,A
	LD	A,7
	CALL	WRTPSG
	ld	d,0x0e
	ld	e,0
	call	set_fm

	LD	HL,(0F7F8H)	; BASICからの引数をロード（整数のみ）
;	LD	HL,(PARA)

	XOR	A		; Ａレジスタを０にする
	LD	(FEEDVOL),A	; フェードアウト音量をクリアする
	LD	(STOPPARTS),A	; 演奏停止パート情報をクリアする
	LD	(ENDFRG),A	; 演奏終了フラグをクリアする

	LD	A,L		; 引数の下位8bitは曲番号データ
	INC	A
	LD	(NSAVE),A	; 曲番号をワークエリアに格納する
	JP	Z,STOP		; 曲番号が255(-1)なら演奏停止
	INC	A		; CP 254の代わり
	JP	Z,FEEDO		; 254(-2)ならフェードアウト

	DI
	PUSH	HL
	LD	A,H		; 引数の上位8bitは演奏回数情報
	LD	(LOOPTIME),A	; 演奏回数をセット(0～255、0で無限ループ)
	CALL	PSGOFF		; PSGをオフ(BIOS)
	LD	DE,HOOKWRK	; システムの割り込みをチェーンするアドレス
	LD	A,(DE)
	OR	A		; 0かどうか調べる
	JR	NZ,NOSAVE	; 既に常駐しているなら再常駐しない
	LD	BC,5		; LDIR命令の転送バイト数
	LD	HL,HOOK		; システムの1/60秒割り込みベクタ（転送元）
	LDIR			; ベクタの内容をセーブする

NOSAVE:	
	POP	HL

	SLA	L
	SLA	L
	LD	A,L
	SLA	L		; 8倍する
	ADD	A,L		; 12倍する
	SLA	A		; 24倍する
	LD	C,A

;	SLA	L		; 16倍する
;	SLA	L		; 32倍する
;	LD	C,L
;	LD	B,0		; BCに曲番号のオフセット値を作成

	LD	HL,MBOOT	; 曲演奏情報の先頭アドレス
	ADD	HL,BC		; 曲番号のアドレスをセットする

	LD	A,(PARA5)
	LD	B,A	; 演奏パート数をセット

INILP:	XOR	A		; ループ開始
	PUSH	BC
	LD	C,B
	DEC	C
	LD	B,A		; BCにパート毎のオフセット値を作成

	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; 曲データ開始アドレスを取得
	INC	HL
	PUSH	HL

	CP	D

	JR	NZ,PASS1	; 開始アドレスが0以外なら分岐する

	CALL	SYNCFGON	; 
	DEC	A		; Aを２５５（－１）にする（非演奏パート）
	JR	PASS2

PASS1:
	LD	HL,-300H;	-2100H	; 
	ADD	HL,DE

	LD	D,H
	LD	E,L

PASS2:
	LD	HL,MAXVOL	; 最大ボリュームのワークアドレス
	ADD	HL,BC		; 格納番地確定
	LD	(HL),A		; 最大ボリュームを255にする

	LD	HL,COUNT	; 演奏カウントのワークアドレス
	ADD	HL,BC		; 格納番地確定
	LD	(HL),1		; 演奏カウントを1にする

	LD	HL,STOPFRG	; 演奏停止フラグのワークアドレス
	ADD	HL,BC		; 格納番地確定
	LD	(HL),A		; 演奏停止フラグを２５５（－１）にする

	LD	HL,STARTADR	; 演奏開始するアドレスの格納番地
	ADD	HL,BC		; 曲番号に対応した格納番地を作成
	ADD	HL,BC		; (オフセット値はBCの２倍)

	LD	(HL),E		; 演奏開始アドレスをワークエリアにコピー
	INC	HL
	LD	(HL),D



	LD	HL,OFFSET	; 演奏中アドレスの格納番地
	ADD	HL,BC		; 格納番地を作成
	ADD	HL,BC		; (オフセット２倍)

	LD	(HL),E		; 同様にコピーする
	INC	HL
	LD	(HL),D

	POP	HL		; (曲演奏情報の先頭アドレスを復帰)
	POP	BC		; (ループカウンタを復帰)
;	DJNZ	INILP		; パート数ぶん繰り返す

;	LD	a,(PARA5)
;	ld b,a
;tonelp:
	push	bc
	push	hl

	ld	a,b
	cp	4

	jr	c,tonelp2		; PSG(b = 1-3)

	ld	a,(PARA4);	TONE_NO
;	ld	hl,TONENO-1
	ld	c,b
	ld	b,0
;	add	hl,bc
	call	CHNTONE2
	ld	(HL),a

tonelp2:
	pop	hl
	pop	bc

;	DJNZ	tonelp		; パート数ぶん繰り返す
	DJNZ	INILP		; パート数ぶん繰り返す


	LD	HL,HOOK		; システムの1/60秒割り込みベクタ+2
	LD	BC,PLAYLOOP	; 常駐演奏ルーチンの実行開始アドレス

	LD	(HL),0F7H	; RST命令（自己書き換え）
	INC	HL
	INC	HL
	LD	(HL),C		; 開始アドレス下位をベクタにセットする
	INC	HL
	LD	(HL),B		; 開始アドレス上位ベクタにセットする
	INC	HL
	LD	(HL),0C9H	; 'RET'命令をベクタにセットする

	ld	A,B
	ld	BC,0F341H	;slot0
	CP	40H
	JR	C,slotset
	INC	BC			;slot1
	CP	80H
	JR	C,slotset
	INC	BC			;slot2
	CP	0C0H
	JR	C,slotset
	INC	BC			;slot3
slotset:
	LD	A,(BC)
	LD	(HOOK+1),A

	EI
	RET			; 終了してシステムに戻る

;-------------------------------------------
;	フェードアウトルーチン
;-------------------------------------------

FEEDO:	LD	A,1		; 
	LD	(FEEDVOL),A	; フェードアウトボリュームを１にする
	LD	(FCOUNT),A	; フェードアウトカウンタを１にする

	LD	A,(PARA5)
	LD	B,A	; ループカウンタをパート数にセットする

FEEDLOOP:
	PUSH	BC

	LD	HL,MAXVOL	; ボリューム格納アドレスの先頭番地
	LD	C,B		; 
	DEC	C		; パート数をオフセットにする
	LD	B,0		; 
	ADD	HL,BC		; アドレスを生成する

	LD	A,(HL)		; フェードアウトボリュームを取得
	INC	A
	JR	Z,NOFELFO	; ボリュームが０なら何もしない

	CP	16+1
	JR	C,NOFELFO	; ボリュームが15以下なら何もしない

	LD	A,15
	LD	(HL),A		; ボリュームを15（最大値）にする

NOFELFO:POP	BC
	DJNZ	FEEDLOOP

	RET

;-------------------------------------------
;	演奏停止ルーチン
;-------------------------------------------

STOP:	LD	HL,HOOKWRK	; フックの退避アドレス
	XOR	A
	CP	(HL)		; まだ常駐していなければ終了する
	RET	Z
	LD	DE,HOOK		; 1/60秒割り込みのベクタアドレス
	LD	BC,5
	PUSH	HL
	DI
	LDIR			; 割り込みベクタを常駐前に戻し、常駐解除
	EI
	POP	HL
	LD	(HL),A		; ０を格納する

PSGOFF:	LD	HL,MAXVOL	; ボリューム値の格納アドレス
	LD	A,(PARA5)
	LD	B,A	; ループカウンタにパート数を設定

OFFLP:	LD	A,(HL)
	INC	A
	JR	Z,offlp1 ;OFLPED	; ボリュームが２５５（－１）ならループ終了

	ld	a,b
	cp	4
	jr	nc,offlp1	; FM(b = 4-6)

	LD	A,11		; ＰＳＧのボリュームレジスタの値
	SUB	B		; パートに対応する出力レジスタを設定
	LD	E,0		; 音量を０にする
	CALL	WRTPSG		; ＰＳＧに音量０を出力する(BIOS)

offlp1:
	INC	HL		; 次のパートのボリューム値格納アドレスへ

OFLPED:
	ld	a,b
	cp	4
	jr	c,oflped1	; PSG(b = 1-3)

	ld	a,20h-1-3
	ld	d,b
	add	a,d
	ld	d,a
	ld	e,0
	push	bc
	call	set_fm
	pop	bc

oflped1:
	DJNZ	OFFLP		; ループする

	RET

;-------------------------------------------
;	フェードアウトサブルーチン
;-------------------------------------------

FEEDSUB:LD	HL,FCOUNT	; フェードアウトカウンタをセット
	DEC	(HL)
	RET	NZ		; 処理を間引く
	LD	(HL),FEEDTIME	; カウンタを初期化

	LD	HL,FEEDVOL	; フェードアウト音量の格納番地
	INC	(HL)		; １レベル上げる

	LD	HL,MAXVOL
	ADD	HL,BC
	LD	A,(HL)
	INC	A
	RET	Z		; 既に２５５（－１）なら終了

	CP	16
	CALL	C,PUTVOL	; 15以下ならボリュームを出力する
	RET

;-------------------------------------------
;	演奏ルーチン、割り込みで呼ばれる
;-------------------------------------------

PLAYLOOP:
	PUSH	AF
PLAYLOOP2:
	LD	A,(FEEDVOL)	; フェードアウトレベルを調べる
	CP	15
	JR	NZ,PLAYLOOP3
	POP	AF
	JR	STOP		; フェードアウト終了なら演奏を停止する

PLAYLOOP3:
	OR	A
	CALL	NZ,FEEDSUB	; フェードアウトする

	LD	A,(PARA5)
	LD	B,A	; ループカウンタをパート数にする

COUNTER:PUSH	BC

	LD	C,B
	DEC	C
	LD	B,0		; オフセットをループカウンタ-1とする

	LD	HL,STOPFRG	; 演奏状態ステータスのアドレスをセット
	ADD	HL,BC
	LD	A,(HL)		; そのパートの演奏状態を調べる
	CP	254		; 同期待ち中または演奏終了か判断する
	JR	NC,LOOPEND	; 演奏処理をスキップする

	LD	HL,COUNT	; 音長カウンタの先頭アドレス
	ADD	HL,BC		; パート毎のアドレスにする
	DEC	(HL)		; 音長カウンタから1を引く
	CALL	Z,PING		; カウンタが0なら演奏処理をする

LOOPEND:POP	BC
	DJNZ	COUNTER		; ループする

	LD	A,(PARA5)
	LD	B,A	; 演奏しているパート数
	LD	A,(STOPPARTS)	; 演奏を停止したパート数
	CP	B		; 同じかどうか比較する
	JR	NZ,PSTOP	; 演奏を終了する

	LD	C,B
	LD	HL,STOPFRG	; 演奏状態ステータスのアドレス
	LD	DE,COUNT	; 音長カウンタの先頭アドレス

LPE:	LD	A,(HL)		; 演奏ステータスを調べる
	INC	A
	JR	Z,LPNX		; 255(-1)なら演奏停止なので処理しない

	XOR	A
	LD	(HL),A		; 演奏ステータスを0（演奏中）とする
	INC	A
	LD	(DE),A		; 音長カウンタを1とする

	DEC	C		; 演奏停止パート数の計算をする

LPNX:	INC	HL		; 次のパートの演奏状態ステータスのアドレス
	INC	DE
	DJNZ	LPE		; 次のパートを見る

	LD	A,C
	LD	(STOPPARTS),A	; 演奏停止パート数をセットする
	JR	PLAYLOOP2	; ループする

;-------------------------------------------
;	割り込みルーチンを終了する
;-------------------------------------------
PSTOP:
;	XOR	A
	LD	HL,ENDFRG	; 終了カウンタのアドレス
	DEC	(HL)		; 終了カウンタから1を引く
	LD	(HL),0		; 終了フラグに0をセット
	JR	NZ,HOOKWRK2	; 終了カウンタが0なら割り込み終了

	LD	HL,LOOPTIME	; ループするべき回数を調べる
	OR	(HL)		; 0かどうか調べる
	JR	Z,HOOKWRK2	; 0なら割り込み終了

	DEC	(HL)		; ループ回数から1を引く
	JR	NZ,PLAYLOOP2; 0ならループする

	POP	AF
	JP	STOP		; 演奏自己停止

HOOKWRK2:
	POP	AF
HOOKWRK:
	DB	0,0C9H,0C9H,0C9H,0C9H	; 割り込みのチェーン

;-------------------------------------------
;	演奏処理ルーチン
;-------------------------------------------

PING:	LD	HL,OFFSET	; 演奏中データのアドレスを格納するアドレス
	ADD	HL,BC		; パート毎のアドレスをセット
	ADD	HL,BC

	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; 演奏中のデータの現在の番地を生成

PINGPONG:LD	A,(DE)		; 実行するコマンドを調べる

	CP	225
	JP	C,PLAY		; 224以下なら演奏コマンド

	PUSH	HL

	DEC	DE;*		; 演奏番地を先にすすめる
	CALL	COMAND		; コマンド解析ルーチンを呼ぶ

	POP	HL

	DEC	DE;*		; 演奏番地を先にすすめる
	JR	PINGPONG	; ループする

;-------------------------------------------
;	演奏データ内コマンド解析ルーチン
;-------------------------------------------

COMAND:	CP	226
	JR	Z,CHNVOL	; コマンド226、音量変更(MMLのVコマンド)

	CP	227
	JR	Z,LEN		; コマンド227、標準音長変更(MMLのTコマンド)

	CP	228
	JR	Z,CHNTONE

	CP	225
	JR	Z,YCOM		; コマンド225、直接出力（MMLのYコマンド）

	INC	A
	JR	Z,D_C		; コマンド255、ループ

	INC	A
	RET	NZ		; コマンド254でなければコマンド解析終了

	CALL	SYNCON		; 同期コマンド
	PUSH	DE
	CALL	NOPUT		; 音の出力を停止する
	POP	DE

SUTETA:	POP	HL		; スタックから元のリターンアドレスを破棄する
	LD	A,1
	JP	LENG		; そのままメインループに戻る

;-------------------------------------------
;	各演奏パートの同期処理
;-------------------------------------------

SYNCON:	LD	HL,STOPFRG	; 演奏状態ステータスへのアドレス
	ADD	HL,BC		; パート毎のアドレスにする
	LD	(HL),254	; 演奏状態を254(-2)とする
	INC	DE;*		; 演奏アドレスに1を足しておく

SYNCFGON:LD	HL,STOPPARTS	; 演奏停止パート数の格納アドレス
	INC	(HL)		; 演奏停止パート数に1を足す
	RET

;-------------------------------------------
;	ボリューム変更処理
;-------------------------------------------

CHNVOL:	LD	A,(DE)		; 音量を取得する
	LD	HL,MAXVOL	; 音量の格納アドレス
	ADD	HL,BC		; パート毎のアドレスにする
	LD	(HL),A		; 音量を格納
	RET

;-------------------------------------------
;	音色変更処理
;-------------------------------------------

CHNTONE:
	LD	A,(DE)		; 音色を取得する
CHNTONE2:
	cp	16
	jr	c,CHNTONE3
	add	a,-16
	ld	d,a
;	ld	e,c
	push	bc
	push	de
	call	set_tone
	pop	de
	pop	bc
	xor	a
CHNTONE3:
	ld	HL,TONENO
	ADD	HL,BC
	ld	(HL),a

	RET

;-------------------------------------------
;	標準音長設定処理
;-------------------------------------------

LEN:	LD	A,(DE)		; 標準音長を取得する
	LD	HL,LENGTH	; 標準音長の格納アドレス
	ADD	HL,BC		; パート毎のアドレスにする
	LD	(HL),A		; 標準音長を格納
	RET

;-------------------------------------------
;	ＰＳＧのレジスタに直接値を書く処理
;-------------------------------------------

YCOM:	LD	A,(DE)		; 出力するデータを得る
	LD	H,A		; 一旦別のレジスタに退避する
	DEC	DE;*		; 演奏番地を先にすすめる
	LD	A,(DE)		; 出力するデータを得る
	PUSH	DE
	LD	E,A
	LD	A,H
	CALL	WRTPSG		; PSG出力(BIOS)を呼び出す
	POP	DE

	RET

;-------------------------------------------
;	ダ・カーポ処理
;-------------------------------------------

D_C:	LD	HL,STARTADR	; 演奏開始アドレスの格納アドレス
	ADD	HL,BC		; パート毎のアドレスにする
	ADD	HL,BC
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; 演奏開始アドレスを取得する
	INC	DE;*		; アドレスに1加算しておく

	LD	A,1
	LD	(ENDFRG),A	; 終了ステータスを1にする

	RET

;-------------------------------------------
;	PSG&FMに音を出力する
;-------------------------------------------

PLAY:	PUSH	HL
	push	de
	push	bc

	AND	01111111B	; 音調情報を削除
	push	af
;	CALL	PSGPUT		; PSGに音を出力する
;	PUSH	DE

	PUSH	HL
	LD	HL,STOPFRG	; 演奏終了状態のアドレス
	ADD	HL,BC		; パート毎のアドレスにする
	LD	(HL),A
;*
	POP	HL
;	POP	DE

	ld	h,a
	ld	a,c
	cp	3
	ld	a,h

	jr	c,PSGPUT		; PSGに音を出力する(c = 0-2)

	ld	a,20h-3
	ld	d,c
	add	a,d
	ld	d,a
	ld	e,0
	push	bc

	call	set_fm	;	key off

	pop	bc
	push	bc

	push	de
	push	bc
	call	PUTMVOL
	pop	bc

	ld	a,e
;	xor	a,0fh
	cpl	a
	and a,0fh
	ld	e,a

	ld	HL,TONENO
	ADD	HL,BC
	ld	a,(HL)
	add	a,a
	add	a,a
	add	a,a
	add	a,a
	or	a,e
	ld	e,a

	ld	a,30h-3
	add	a,c
	ld	d,a

	call	set_fm

	pop	de

	pop	bc

	pop	af
	push	af

	or	a
	jr	z,play1

	dec	a

	ld	d,a

	ld	a,-3
	add	a,c
	ld	e,a

	call	set_key

play1:
play2:
	pop	af
	pop	bc
	pop	de


	LD	A,(DE)
	AND	10000000B	; 音長が設定されているか調べる
	LD	HL,LENGTH
	ADD	HL,BC		; （Zフラグは影響を受けない）
	LD	A,(HL)		; 標準の音調を取得する
	JR	NZ,LENG		; 音調が設定されてないなら分岐

	DEC	DE;*		; 演奏アドレスをすすめる
	LD	A,(DE)		; 音長を取得する

LENG:	LD	HL,COUNT	; 音長カウンタのアドレス
	ADD	HL,BC		; パート数に対応したアドレスを生成
	LD	(HL),A		; 音長をセットする

	POP	HL

STCOUNT:DEC	DE;*		; 演奏アドレスをすすめる

	LD	(HL),D
	DEC	HL
	LD	(HL),E		; 演奏アドレスを記録する

	RET

;-------------------------------------------
;	ＰＳＧに音を出力するルーチン
;-------------------------------------------

PSGPUT:
;	PUSH	DE
;	PUSH	HL
;	LD	HL,STOPFRG	; 演奏終了状態のアドレス
;	ADD	HL,BC		; パート毎のアドレスにする
;	LD	(HL),A
;*
;	POP	HL
;	POP	DE
;	ret
;
	SLA	A		; 2倍する
	LD	E,A
	LD	D,0		; オフセットアドレスの生成

	LD	HL,PSGDATA	; トーンの先頭格納番地
	ADD	HL,DE		; トーンの格納番地を生成

	LD	A,C
	SLA	A		; パートを2倍する(PSGの出力レジスタ)
	LD	E,(HL)		; 下位のトーンをセット
	CALL	WRTPSG		; PSGに下位トーンを出力(BIOS)

	INC	HL		; 上位のトーンの格納番地
	INC	A		; 出力先レジスタを1すすめる
	LD	E,(HL)		; 上位のトーンをセット
	CALL	WRTPSG		; PSGに上位トーンを出力(BIOS)

	CALL	PUTMVOL		; ボリュームの設定

;	POP	HL
;	POP	DE
;	RET
	jr	play2

;-------------------------------------------
;	ボリュームを設定するルーチン
;-------------------------------------------

PUTMVOL:LD	HL,STOPFRG	; 演奏状態ステータスのアドレス
	ADD	HL,BC		; パート毎のアドレスにする
	LD	A,(HL)		; 演奏状態を取得
	OR	A		; 0かどうか調べる
	JR	Z,PUTVOL	; 演奏停止中ならボリュームを0とする

	LD	HL,MAXVOL	; 設定したボリュームを得る
	ADD	HL,BC		; パート毎のアドレスにする
	LD	A,(HL)		; ボリュームを設定

PUTVOL:	CP	16		; エンベローブかどうか調べる
	CALL	NC,PUTENV	; 処理を分岐する

	LD	E,A
	LD	A,(FEEDVOL)	; フェードアウトレベルを得る
	LD	D,A
	LD	A,E
	SUB	D		; 音量からフェードアウトレベルを引く
	JR	NC,YESPUT	; 音量が0以上なら分岐

NOPUT:	XOR	A		; 音量を0とする

YESPUT:	LD	E,A		; 音量を設定する

	LD	A,C		; 
	cp	3
	ret	nc		; FM(c = 3-5)

	LD	A,8		; 先頭の音量レジスタ
	ADD	A,C		; パート毎のレジスタにする
	JP	WRTPSG		; 常に音を出す(BEEP音、等への対策)

;-------------------------------------------
;	ＰＳＧのハードエンベローブ設定
;-------------------------------------------

PUTENV:	ADD	A,-16		; エンベローブのパターン番号を得る
	LD	E,A
	LD	A,13		; エンベローブの出力レジスタ
	CALL	WRTPSG		; PSGに出力(BIOS)
	LD	A,16		; 音量をエンベローブ出力とする
	RET

;-------------------------------------------
;	ＰＳＧ出力
;-------------------------------------------

WRTPSG:
	PUSH	IX
	PUSH	HL
	PUSH	BC

	ld	h,a

	ld	a,(0FCC1H)	; EXPTBL
	ld	b,a
	ld	c,0
	push	bc
	pop	iy

	ld	a,h
;	ld	e,l

	LD IX,0093H	; WRTPSG(MAINROM)
	call	001CH	; CALSLT

	POP	BC
	POP	HL
	POP	IX

	RET


;============================================
; FMレジスタ設定 
;============================================

set_fm: ;_direct:	;d = no, e = data
;;	outp(0x7c,reg);
	ld	c,7ch

	ld	a,d
	out	(c),a		; reg

	ld	a,(ix+0)

;;	outp(0x7d,data);
	ld	a,e
	inc	c
	out	(c),a		; data

	ret
;}

set_fm_bios:
	push	iy
	push	ix
	push	de
	ld	a,(PARA6)
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	pop	de
	ld	a,d
	ld ix,04110h	; WRTOPL

	call	001ch	; CALSLT
	pop	ix
	pop	iy

	ret

;============================================
; オリジナル音色設定
;============================================
;void set_tone(int no)
;{
set_tone:	; d = no
;	int i, j;

	ld	hl,tone_table
	ld	a,d
	add	a,a
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	a,(hl)
	ld	c,a
	inc	hl
	ld	a,(hl)
	ld	b,a
	push	bc
	pop	hl		; tone_table[no]

;	j = 0x0;
	ld	a,0h
;	add	a,e
	ld	d,a		; reg

	ld	b,8
;	for(i = 0; i < 8; ++i){

set_tone1:
	push	bc
;	push	de

;		set_fm(j, tone_table[no][i]);

	ld	a,(hl)
	ld	e,a		; data

	call	set_fm

;	pop	de

;		j+=1;

	ld	a,1
	add	a,d
	ld	d,a

	inc	hl


	pop	bc

;	inc	b
;	ld	a,b
;	cp	28
;	jr	nz,set_tone1
	djnz	set_tone1

;	}

	ret
;}

;============================================
; 音程設定 
;============================================
;
set_key: ; d = no, e = ch{
;	set_fm(0x10 + ch, key_table[no][1]);
	ld	hl,key_table

	ld	a,d
	ld	c,a
	ld	b,0
	add	hl,bc
	add	hl,bc
	inc	hl

	ld	a,010h

	add	a,e
	ld	d,a		; reg

	ld	a,(hl)	; key_table[no][1]
	ld	e,a		; data

	call	set_fm

;	set_fm(0x20 + ch,  0x10 | key_table[no][0]);

	ld	a,10h
	add	a,d
	ld	d,a		; reg

	dec	hl
	ld	a,(hl)	; key_table[no][0]

	ld	e, 10h

	or	e
	ld	e,a		; data

	call	set_fm

	ret
;}

include "tonemsx.inc"

;-------------------------------------------
;	ワークエリア
;-------------------------------------------

COUNT:	DB	1,1,1, 1,1,1, 1,1,1, 1,1,1		; 音長カウンタ
STOPFRG:DB	0,0,0, 0,0,0, 0,0,0, 0,0,0		; WAIT&SYNC&OFF
MAXVOL:	DB	15,15,15, 15,15,15, 15,15,15	; ボリューム
LENGTH:	DB	5,5,5, 5,5,5, 5,5,5, 5,5,5		; 基本音長
TONENO:	DB	0,0,0, 0,0,0, 0,0,0, 0,0,0		; 音色

OFFSET:	DB	0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0	; 演奏データ実行中アドレス
STARTADR:DB	0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0	; 演奏データ開始アドレス

FEEDVOL:DB	0		; フェードアウトレベル
FCOUNT:DB	1		; フェードアウトカウンタ
LOOPTIME:DB	0		; 演奏回数（０は無限ループ）
STOPPARTS:DB	0		; 
ENDFRG:	DB	0		; 
NSAVE:DB	0		; 

;	END
