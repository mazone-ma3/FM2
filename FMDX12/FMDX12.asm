;===========================================
;	FM(YM2151) MUSIC PLAYER
;	(X1turbo VERSION) 2024/8/26- m@3
;	VERSION 2.00
;
;	FM3��+PSG3���o�͂̑g�ݍ��ݗpBGM�h���C�o
;	�e���|����C���{�t�F�[�h�A�E�g�t��
;
;	���t�f�[�^�͐�p�̃R���p�C����
;	�l�l�k���琶�����܂�
;
;	�R�����g�͌ォ��t�������̂ł�
;===========================================
;-------------------------------------------
; ���f�[�^�^����
; 0CAFFH�Ԓn����0�Ԓn�̕����ɓ����Ă���
; �e���t�R�}���h�͐��l�Ŏw��(�ϒ�)
; 
; ���R�}���h�ꗗ��
; 0-96       ��(����n�An��60/n�b��)�A0�͋x��
; 128-224,n  ��(�����w�薳��)�A128�͋x��
; 225,m,n    SOUND m,n
; 226,n      VOLUME=n�An=16�Ȃ�G���x���[�u
; 227,n      LENGTH=n�A0-96�̃R�}���h�̉���
; 254        �S�Ẵp�[�g�̓��������
; 255        �_�E�J�[�|
;-------------------------------------------
; �ꕔ�Q�l NRTDRV�A�����ɏo��X1
; �Q�l PC-9801�}�V����T�E���h�v���O���~���O 

;	ASEG

;WRTPSG	EQU	0093H		; �o�r�f�ɏo�͂���BIOS���[�`��

PSGDATA	EQU	(0DB3EH-01100H)		; �o�r�f���K�o�͒l�i�[�Ԓn
MBOOT	EQU	(0DB00H-01100H) 		; �ȉ��t�f�[�^�擪�Ԓn�̊i�[�A�h���X

key_table equ	(0DB3EH-01000H)

;MDATA	EQU	0DBFFH		; ���t�f�[�^�i0�Ԓn�Ɍ������Ċi�[����j
;HOOK	EQU	0FD9FH		; �V�X�e����1/60�b���荞�݃t�b�N
;INT3	EQU	0F308H
PARTSUU	EQU	6		; ���t����ő�p�[�g��
FEEDTIME EQU	12		; �t�F�[�h�A�E�g���x��

;	ORG	0DC00H		; �v���O�����̊J�n�Ԓn(BASIC����USR�֐��Ŏ��s)
;	ORG	0B000H
	ORG	0CC00H
	JP	INIT
	JP	STOP
	JP	FEEDO
PARA:
	DB	0,0
PARA2:
	DB	0
PARA3:
	DB	00111000B
PARA4:
	DB	200	; CTC Ch.0
PARA5:
	DB	21	; CTC Ch.3
PARA6:
	DB	2
PARA7:
	DB	PARTSUU

;-------------------------------------------
;	�����ݒ胋�[�`��
;-------------------------------------------
INIT:
	LD	A,(PARA2)
	LD	E,A
	LD	A,6
	CALL	WRTPSG
	LD	A,(PARA3)
	LD	E,A
	LD	A,7
	CALL	WRTPSG2

;	LD	HL,(0F7F8H)	; BASIC����̈��������[�h�i�����̂݁j
	LD	HL,(PARA)

	XOR	A		; �`���W�X�^���O�ɂ���
	LD	(FEEDVOL),A	; �t�F�[�h�A�E�g���ʂ��N���A����
	LD	(STOPPARTS),A	; ���t��~�p�[�g�����N���A����
	LD	(ENDFRG),A	; ���t�I���t���O���N���A����

	LD	A,L		; �����̉���8bit�͋Ȕԍ��f�[�^
	INC	A
	LD	(NSAVE),A	; �Ȕԍ������[�N�G���A�Ɋi�[����

;	JP	Z,STOP		; �Ȕԍ���255(-1)�Ȃ牉�t��~
;	INC	A		; CP 254�̑���
;	JP	Z,FEEDO		; 254(-2)�Ȃ�t�F�[�h�A�E�g

	DI
	PUSH	HL
	LD	A,H		; �����̏��8bit�͉��t�񐔏��
	LD	(LOOPTIME),A	; ���t�񐔂��Z�b�g(0�`255�A0�Ŗ������[�v)
	CALL	PSGOFF		; PSG���I�t(BIOS)

;	LD	DE,HOOKWRK	; �V�X�e���̊��荞�݂��`�F�[������A�h���X
;	LD	A,(DE)
;	OR	A		; 0���ǂ������ׂ�
;	JR	NZ,NOSAVE	; ���ɏ풓���Ă���Ȃ�ď풓���Ȃ�
;	LD	BC,5		; LDIR���߂̓]���o�C�g��
;	LD	HL,HOOK		; �V�X�e����1/60�b���荞�݃x�N�^�i�]�����j
;	LDIR			; �x�N�^�̓��e���Z�[�u����

;NOSAVE:
	POP	HL

	SLA	L
	SLA	L
	LD	A,L
	SLA	L		; 8�{����
	ADD	A,L		; 12�{����
	SLA	A		; 24�{����
	LD	C,A

;	SLA	L		; 16�{����
;	SLA	L		; 32�{����
;	LD	C,L

	LD	B,0		; BC�ɋȔԍ��̃I�t�Z�b�g�l���쐬

	LD	HL,MBOOT	; �ȉ��t���̐擪�A�h���X
	ADD	HL,BC		; �Ȕԍ��̃A�h���X���Z�b�g����

	LD	A,(PARA7)
	LD	B,A	; ���t�p�[�g�����Z�b�g

INILP:	XOR	A		; ���[�v�J�n
	PUSH	BC
	LD	C,B
	DEC	C
	LD	B,A		; BC�Ƀp�[�g���̃I�t�Z�b�g�l���쐬

	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; �ȃf�[�^�J�n�A�h���X���擾
	INC	HL
	PUSH	HL

	CP	D

	JR	NZ,PASS1	; �J�n�A�h���X��0�ȊO�Ȃ番�򂷂�

	CALL	SYNCFGON	; 
	DEC	A		; A���Q�T�T�i�|�P�j�ɂ���i�񉉑t�p�[�g�j

	JR	PASS2

PASS1:
;--
	LD	HL,0F000H	;-01000H
	ADD	HL,DE

	PUSH	HL
	POP	DE

;--
PASS2:

	LD	HL,MAXVOL	; �ő�{�����[���̃��[�N�A�h���X
	ADD	HL,BC		; �i�[�Ԓn�m��
	LD	(HL),A		; �ő�{�����[����255�ɂ���

	LD	HL,COUNT	; ���t�J�E���g�̃��[�N�A�h���X
	ADD	HL,BC		; �i�[�Ԓn�m��
	LD	(HL),1		; ���t�J�E���g��1�ɂ���

	LD	HL,STOPFRG	; ���t��~�t���O�̃��[�N�A�h���X
	ADD	HL,BC		; �i�[�Ԓn�m��
	LD	(HL),A		; ���t��~�t���O���Q�T�T�i�|�P�j�ɂ���

	LD	HL,STARTADR	; ���t�J�n����A�h���X�̊i�[�Ԓn
	ADD	HL,BC		; �Ȕԍ��ɑΉ������i�[�Ԓn���쐬
	ADD	HL,BC		; (�I�t�Z�b�g�l��BC�̂Q�{)

	LD	(HL),E		; ���t�J�n�A�h���X�����[�N�G���A�ɃR�s�[
	INC	HL
	LD	(HL),D



	LD	HL,OFFSET	; ���t���A�h���X�̊i�[�Ԓn
	ADD	HL,BC		; �i�[�Ԓn���쐬
	ADD	HL,BC		; (�I�t�Z�b�g�Q�{)

	LD	(HL),E		; ���l�ɃR�s�[����
	INC	HL
	LD	(HL),D

	POP	HL		; (�ȉ��t���̐擪�A�h���X�𕜋A)
	POP	BC		; (���[�v�J�E���^�𕜋A)
;	DJNZ	INILP		; �p�[�g���Ԃ�J��Ԃ�

;	ld	a,(PARA7)
;	ld b,a
;tonelp:
	push	bc
	push	hl
;	push	de

	ld	a,b
	cp	4

	jr	c,tonelp2		; PSG(b = 1-3)

	ld	a,(PARA6)
	ld	d,a;	TONE_NO

	ld	a,-1-3
	add	a,b
	ld	e,a	;CH_NO
	call	set_tone

tonelp2:
;	pop	de
	pop	hl
	pop	bc

;	DJNZ	tonelp		; �p�[�g���Ԃ�J��Ԃ�
	DJNZ	INILP		; �p�[�g���Ԃ�J��Ԃ�


;	LD	A,I			; �V�X�e���̊��荞�݃x�N�^(EE00H)
;	LD	A,EE
;	LD	H,A
;	LD	L,0C0H+8

;	LD	H,0EEH
	LD	A,(2)
	LD	H,A			; �V�X�e�����[�N�̏�ʃA�h���X

	LD	L,0C0H+6	; CTC Ch.3 ���荞�݃x�N�^

	LD	DE,PLAYLOOP
	LD	(HL),E		; �J�n�A�h���X���ʂ��x�N�^�ɃZ�b�g����
	INC	HL
	LD	(HL),D		; �J�n�A�h���X��ʃx�N�^�ɃZ�b�g����

	PUSH	BC

	LD	A,3
	LD	L,08CH
;	LD	BC,(0EE8CH)	; 01FA0H	;	CTC Work
	LD	C,(HL)					;	CTC Work
	INC	HL
	LD	B,(HL)					;	CTC Work
	DEC	BC
	DEC	BC

	OUT	(C),A	;	CTC Ch.0 Reset
	INC	BC
	INC	BC
	INC	BC
;	LD	BC,01FA3H
	OUT	(C),A	;	CTC Ch.3 Reset

	DEC	BC
	DEC	BC
	DEC	BC
;	LD	BC,01FA0H	;	CTC Ch.0 Set
	LD	A,0111B
	OUT	(C),A
	LD	A,(PARA4)	; 200
	OUT	(C),A

;	LD	A,58H
;	OUT	(C),A

	INC	BC
	INC	BC
	INC	BC
;	LD	BC,01FA3H	;	CTC Ch.3 Set
	LD	A,011000111B
	OUT	(C),A
	LD	A,(PARA5)	; 22
	OUT	(C),A

	POP	BC

;	HALT

END:
	EI
	RET			; �I�����ăV�X�e���ɖ߂�

;-------------------------------------------
;	�t�F�[�h�A�E�g���[�`��
;-------------------------------------------

FEEDO:	LD	A,1		; 
	LD	(FEEDVOL),A	; �t�F�[�h�A�E�g�{�����[�����P�ɂ���
	LD	(FCOUNT),A	; �t�F�[�h�A�E�g�J�E���^���P�ɂ���

	LD	A,(PARA7)
	LD	B,A	; ���[�v�J�E���^���p�[�g���ɃZ�b�g����

FEEDLOOP:
	PUSH	BC

	LD	HL,MAXVOL	; �{�����[���i�[�A�h���X�̐擪�Ԓn
	LD	C,B		; 
	DEC	C		; �p�[�g�����I�t�Z�b�g�ɂ���
	LD	B,0		; 
	ADD	HL,BC		; �A�h���X�𐶐�����

	LD	A,(HL)		; �t�F�[�h�A�E�g�{�����[�����擾
	INC	A
	JR	Z,NOFELFO	; �{�����[�����O�Ȃ牽�����Ȃ�

	CP	16+1
	JR	C,NOFELFO	; �{�����[����15�ȉ��Ȃ牽�����Ȃ�

	LD	A,15
	LD	(HL),A		; �{�����[����15�i�ő�l�j�ɂ���

NOFELFO:POP	BC
	DJNZ	FEEDLOOP

	RET

;-------------------------------------------
;	���t��~���[�`��
;-------------------------------------------

STOP:
	LD	A,(2)
	LD	H,A			; �V�X�e�����[�N�̏�ʃA�h���X

	LD	L,08CH
	LD	C,(HL)
	INC	HL
	LD	B,(HL)
;	LD	BC,(0EE8CH)	; 01FA0H	;	CTC Work

	INC	BC
;	LD	BC,01FA3H
	LD	A,3
	OUT	(C),A	;	CTC Ch.3 Reset

;	LD	HL,HOOKWRK	; �t�b�N�̑ޔ��A�h���X
;	XOR	A
;	CP	(HL)		; �܂��풓���Ă��Ȃ���ΏI������
;	RET	Z
;	LD	DE,HOOK		; 1/60�b���荞�݂̃x�N�^�A�h���X
;	LD	BC,5
;	PUSH	HL

;	EI

;	POP	HL
;	LD	(HL),A		; �O���i�[����

PSGOFF:	LD	HL,MAXVOL	; �{�����[���l�̊i�[�A�h���X
	LD	A,(PARA7)
	LD	B,A	; ���[�v�J�E���^�Ƀp�[�g����ݒ�

OFFLP:	LD	A,(HL)
	INC	A
	JR	Z,offlp1; OFLPED	; �{�����[�����Q�T�T�i�|�P�j�Ȃ烋�[�v�I��

	ld	a,b
	cp	4
	jr	nc,offlp1	; FM(b = 4-6)

;	LD	A,11		; �o�r�f�̃{�����[�����W�X�^�̒l
;	SUB	B		; �p�[�g�ɑΉ�����o�̓��W�X�^��ݒ�

	ld	a,7
	add	b
	LD	E,0		; ���ʂ��O�ɂ���
	CALL	WRTPSG		; �o�r�f�ɉ��ʂO���o�͂���

offlp1:
	INC	HL		; ���̃p�[�g�̃{�����[���l�i�[�A�h���X��

OFLPED:
	ld	a,b
	cp	4
	jr	c,oflped1	; PSG(b = 1-3)

	ld	d,08h	;reg
	ld	e,b
;	dec	e
	ld	a,-3-1
	add	a,e
	ld	e,a

	push	bc
	call	set_fm	;	key off
	pop	bc

oflped1:

	DJNZ	OFFLP		; ���[�v����

	RET

;-------------------------------------------
;	�t�F�[�h�A�E�g�T�u���[�`��
;-------------------------------------------

FEEDSUB:LD	HL,FCOUNT	; �t�F�[�h�A�E�g�J�E���^���Z�b�g
	DEC	(HL)
	RET	NZ		; �������Ԉ���
	LD	(HL),FEEDTIME	; �J�E���^��������

	LD	HL,FEEDVOL	; �t�F�[�h�A�E�g���ʂ̊i�[�Ԓn
	INC	(HL)		; �P���x���グ��

	LD	HL,MAXVOL
	ADD	HL,BC
	LD	A,(HL)
	INC	A
	RET	Z		; ���ɂQ�T�T�i�|�P�j�Ȃ�I��

	CP	16
	CALL	C,PUTVOL	; 15�ȉ��Ȃ�{�����[�����o�͂���
	RET

;-------------------------------------------
;	���t���[�`���A���荞�݂ŌĂ΂��
;-------------------------------------------

;DO_RETI:
;	RETI

PLAYLOOP:
;	CALL	DO_RETI
;	DI
	PUSH	AF
	PUSH	HL
	PUSH	DE
	PUSH	BC
;	PUSH	IX
;	PUSH	IY
;	EX	AF,AF'
;	PUSH	AF

PLAYLOOP1:
PLAYLOOP2:
	LD	A,(FEEDVOL)	; �t�F�[�h�A�E�g���x���𒲂ׂ�
	CP	15
;	JR	Z,STOP		; �t�F�[�h�A�E�g�I���Ȃ牉�t���~����
	JR	Z,HOOKWRK2		; �t�F�[�h�A�E�g�I���Ȃ牉�t���~����
	OR	A
	CALL	NZ,FEEDSUB	; �t�F�[�h�A�E�g����

	LD	A,(PARA7)
	LD	B,A	; ���[�v�J�E���^���p�[�g���ɂ���

COUNTER:
	PUSH	BC

	LD	C,B
	DEC	C
	LD	B,0		; �I�t�Z�b�g�����[�v�J�E���^-1�Ƃ���

	LD	HL,STOPFRG	; ���t��ԃX�e�[�^�X�̃A�h���X���Z�b�g
	ADD	HL,BC
	LD	A,(HL)		; ���̃p�[�g�̉��t��Ԃ𒲂ׂ�
	CP	254		; �����҂����܂��͉��t�I�������f����
	JR	NC,LOOPEND	; ���t�������X�L�b�v����

	LD	HL,COUNT	; �����J�E���^�̐擪�A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	DEC	(HL)		; �����J�E���^����1������
	CALL	Z,PING		; �J�E���^��0�Ȃ牉�t����������

LOOPEND:
	POP	BC
	DJNZ	COUNTER		; ���[�v����

	LD	A,(PARA7)
	LD	B,A	; ���t���Ă���p�[�g��
	LD	A,(STOPPARTS)	; ���t���~�����p�[�g��
	CP	B		; �������ǂ�����r����
	JR	NZ,PSTOP	; ���t���I������

	LD	C,B
	LD	HL,STOPFRG	; ���t��ԃX�e�[�^�X�̃A�h���X
	LD	DE,COUNT	; �����J�E���^�̐擪�A�h���X

LPE:
	LD	A,(HL)		; ���t�X�e�[�^�X�𒲂ׂ�
	INC	A
	JR	Z,LPNX		; 255(-1)�Ȃ牉�t��~�Ȃ̂ŏ������Ȃ�

	XOR	A
	LD	(HL),A		; ���t�X�e�[�^�X��0�i���t���j�Ƃ���
	INC	A
	LD	(DE),A		; �����J�E���^��1�Ƃ���

	DEC	C		; ���t��~�p�[�g���̌v�Z������

LPNX:
	INC	HL		; ���̃p�[�g�̉��t��ԃX�e�[�^�X�̃A�h���X
	INC	DE
	DJNZ	LPE		; ���̃p�[�g������

	LD	A,C
	LD	(STOPPARTS),A	; ���t��~�p�[�g�����Z�b�g����

	JR	PLAYLOOP2	; ���[�v����

;-------------------------------------------
;	���荞�݃��[�`�����I������
;-------------------------------------------
PSTOP:	XOR	A
	LD	HL,ENDFRG	; �I���J�E���^�̃A�h���X
	DEC	(HL)		; �I���J�E���^����1������
	LD	(HL),A		; �I���t���O��0���Z�b�g
	JR	NZ,HOOKWRK	; �I���J�E���^��0�Ȃ犄�荞�ݏI��

	LD	HL,LOOPTIME	; ���[�v����ׂ��񐔂𒲂ׂ�
	OR	(HL)		; 0���ǂ������ׂ�

	JR	Z,HOOKWRK	; 0�Ȃ犄�荞�ݏI��

	DEC	(HL)		; ���[�v�񐔂���1������
	JR	NZ,PLAYLOOP2	; 0�Ȃ烋�[�v����

HOOKWRK2:
	CALL	STOP		; ���t���Ȓ�~

HOOKWRK:
	DI
;	LD	A,5
;	OUT	(0E4H),A	;CUT INT 5-7
	
;	POP	AF
;	EX	AF,AF'
;	POP	IY
;	POP	IX
	POP	BC
	POP	DE
	POP	HL
	POP	AF
	
	EI

	RETI
	;DB	0,0C9H,0C9H,0C9H,0C9H	; ���荞�݂̃`�F�[��

;-------------------------------------------
;	���t�������[�`��
;-------------------------------------------

PING:	LD	HL,OFFSET	; ���t���f�[�^�̃A�h���X���i�[����A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X���Z�b�g
	ADD	HL,BC

	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; ���t���̃f�[�^�̌��݂̔Ԓn�𐶐�

PINGPONG:LD	A,(DE)		; ���s����R�}���h�𒲂ׂ�

	CP	225
	JP	C,PLAY		; 224�ȉ��Ȃ牉�t�R�}���h

	PUSH	HL

	DEC	DE;*		; ���t�Ԓn���ɂ����߂�
	CALL	COMAND		; �R�}���h��̓��[�`�����Ă�

	POP	HL

	DEC	DE;*		; ���t�Ԓn���ɂ����߂�
	JR	PINGPONG	; ���[�v����

;-------------------------------------------
;	���t�f�[�^���R�}���h��̓��[�`��
;-------------------------------------------

COMAND:	CP	226
	JR	Z,CHNVOL	; �R�}���h226�A���ʕύX(MML��V�R�}���h)

	CP	227
	JR	Z,LEN		; �R�}���h227�A�W�������ύX(MML��T�R�}���h)

	CP	228
	JR	Z,CHNTONE

	CP	225
	JR	Z,YCOM		; �R�}���h225�A���ڏo�́iMML��Y�R�}���h�j

	INC	A
	JR	Z,D_C		; �R�}���h255�A���[�v

	INC	A
	RET	NZ		; �R�}���h254�łȂ���΃R�}���h��͏I��

	CALL	SYNCON		; �����R�}���h
	PUSH	DE
	CALL	NOPUT		; ���̏o�͂��~����
	POP	DE

SUTETA:	POP	HL		; �X�^�b�N���猳�̃��^�[���A�h���X��j������
	LD	A,1
	JP	LENG		; ���̂܂܃��C�����[�v�ɖ߂�

;-------------------------------------------
;	�e���t�p�[�g�̓�������
;-------------------------------------------

SYNCON:	LD	HL,STOPFRG	; ���t��ԃX�e�[�^�X�ւ̃A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	(HL),254	; ���t��Ԃ�254(-2)�Ƃ���
	INC	DE;*		; ���t�A�h���X��1�𑫂��Ă���

SYNCFGON:LD	HL,STOPPARTS	; ���t��~�p�[�g���̊i�[�A�h���X
	INC	(HL)		; ���t��~�p�[�g����1�𑫂�
	RET

;-------------------------------------------
;	�{�����[���ύX����
;-------------------------------------------

CHNVOL:	LD	A,(DE)		; ���ʂ��擾����
	LD	HL,MAXVOL	; ���ʂ̊i�[�A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	(HL),A		; ���ʂ��i�[
	RET

;-------------------------------------------
;	���F�ύX����
;-------------------------------------------

CHNTONE:
	ld	a,c
	sub	3
	ret	c	; PSG(0-2)

	push	bc
	push	de

	ld	c,a
	LD	A,(DE)		; ���F���擾����
	ld	d,a	;tone_no
	ld	e,c	;ch

	call	set_tone
	pop	de
	pop	bc
	RET

;-------------------------------------------
;	�W�������ݒ菈��
;-------------------------------------------

LEN:	LD	A,(DE)		; �W���������擾����
	LD	HL,LENGTH	; �W�������̊i�[�A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	(HL),A		; �W���������i�[
	RET

;-------------------------------------------
;	�o�r�f�̃��W�X�^�ɒ��ڒl����������
;-------------------------------------------

YCOM:	LD	A,(DE)		; �o�͂���f�[�^�𓾂�
	LD	H,A		; ��U�ʂ̃��W�X�^�ɑޔ�����
	DEC	DE;*		; ���t�Ԓn���ɂ����߂�
	LD	A,(DE)		; �o�͂���f�[�^�𓾂�
	PUSH	DE
	LD	E,A
	LD	A,H
	CALL	WRTPSG		; PSG�o��(BIOS)���Ăяo��
	POP	DE

	RET

;-------------------------------------------
;	�_�E�J�[�|����
;-------------------------------------------

D_C:
	LD	HL,STARTADR	; ���t�J�n�A�h���X�̊i�[�A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	ADD	HL,BC
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; ���t�J�n�A�h���X���擾����

	INC	DE;*		; �A�h���X��1���Z���Ă���

	LD	A,1
	LD	(ENDFRG),A	; �I���X�e�[�^�X��1�ɂ���

	RET

;-------------------------------------------
;	PSG&FM�ɉ����o�͂���
;-------------------------------------------

PLAY:	PUSH	HL
	push	de
	push	bc

	AND	01111111B	; ���������폜

	push	af
;	CALL	PSGPUT		; PSG�ɉ����o�͂���
;	PUSH	DE
	PUSH	HL
	LD	HL,STOPFRG	; ���t�I����Ԃ̃A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	(HL),A
;*
	POP	HL
;	POP	DE

	ld	h,a
	ld	a,c
	cp	3
	ld	a,h

	jr	c,PSGPUT		; PSG�ɉ����o�͂���(c = 0-2)

	ld	a,-3
	add	a,c
	ld	c,a


	ld	d,08h	;reg

	ld	e,c
	push	bc

	call	set_fm	;	key off

	pop	bc

	pop	af
	push	af

	or	a
	jr	z,play1

	dec	a
	ld	d,a

	ld	e,c

	push	de
	push	bc

	call	set_key

	pop	bc
	pop	de

	ld	d,08h	;reg

;	ld	e,c

	ld	a,0f0h
	add	a,c
	ld	e,a

	push	bc
	call	set_fm		; key on
	pop	bc

play1:
play2:
	pop	af
	pop	bc
	pop	de


	LD	A,(DE)
	AND	10000000B	; �������ݒ肳��Ă��邩���ׂ�
	LD	HL,LENGTH
	ADD	HL,BC		; �iZ�t���O�͉e�����󂯂Ȃ��j
	LD	A,(HL)		; �W���̉������擾����
	JR	NZ,LENG		; �������ݒ肳��ĂȂ��Ȃ番��

	DEC	DE;*		; ���t�A�h���X�������߂�
	LD	A,(DE)		; �������擾����

LENG:	LD	HL,COUNT	; �����J�E���^�̃A�h���X
	ADD	HL,BC		; �p�[�g���ɑΉ������A�h���X�𐶐�
	LD	(HL),A		; �������Z�b�g����

	POP	HL

STCOUNT:DEC	DE;*		; ���t�A�h���X�������߂�

	LD	(HL),D
	DEC	HL
	LD	(HL),E		; ���t�A�h���X���L�^����

	RET

;-------------------------------------------
;	�o�r�f�ɉ����o�͂��郋�[�`��
;-------------------------------------------

PSGPUT:
;	PUSH	DE
;	PUSH	HL
;	LD	HL,STOPFRG	; ���t�I����Ԃ̃A�h���X
;	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
;	LD	(HL),A
;*
;	POP	HL
;	POP	DE
;	ret
;*
	SLA	A		; 2�{����
	LD	E,A
	LD	D,0		; �I�t�Z�b�g�A�h���X�̐���

	LD	HL,PSGDATA	; �g�[���̐擪�i�[�Ԓn
	ADD	HL,DE		; �g�[���̊i�[�Ԓn�𐶐�

	LD	A,C
	SLA	A		; �p�[�g��2�{����(PSG�̏o�̓��W�X�^)
	LD	E,(HL)		; ���ʂ̃g�[�����Z�b�g
	CALL	WRTPSG		; PSG�ɉ��ʃg�[�����o��(BIOS)

	INC	HL		; ��ʂ̃g�[���̊i�[�Ԓn
	INC	A		; �o�͐惌�W�X�^��1�����߂�
	LD	E,(HL)		; ��ʂ̃g�[�����Z�b�g
	CALL	WRTPSG		; PSG�ɏ�ʃg�[�����o��(BIOS)

	CALL	PUTMVOL		; �{�����[���̐ݒ�

;	POP	HL
;	POP	DE
;	RET
	jr	play2

;-------------------------------------------
;	�{�����[����ݒ肷�郋�[�`��
;-------------------------------------------

PUTMVOL:LD	HL,STOPFRG	; ���t��ԃX�e�[�^�X�̃A�h���X
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	A,(HL)		; ���t��Ԃ��擾
	OR	A		; 0���ǂ������ׂ�
	JR	Z,PUTVOL	; ���t��~���Ȃ�{�����[����0�Ƃ���

	LD	HL,MAXVOL	; �ݒ肵���{�����[���𓾂�
	ADD	HL,BC		; �p�[�g���̃A�h���X�ɂ���
	LD	A,(HL)		; �{�����[����ݒ�

PUTVOL:	CP	16		; �G���x���[�u���ǂ������ׂ�
	CALL	NC,PUTENV	; �����𕪊򂷂�

	LD	E,A
	LD	A,(FEEDVOL)	; �t�F�[�h�A�E�g���x���𓾂�
	LD	D,A
	LD	A,E
	SUB	D		; ���ʂ���t�F�[�h�A�E�g���x��������
	JR	NC,YESPUT	; ���ʂ�0�ȏ�Ȃ番��

NOPUT:	XOR	A		; ���ʂ�0�Ƃ���

YESPUT:	LD	E,A		; ���ʂ�ݒ肷��

	LD	A,C		; 
	cp	3
	ret	nc		; FM(c = 3-5)

	LD	A,8		; �擪�̉��ʃ��W�X�^
	ADD	A,C		; �p�[�g���̃��W�X�^�ɂ���
	JP	WRTPSG		; ��ɉ����o��(BEEP���A���ւ̑΍�)

;-------------------------------------------
;	�o�r�f�̃n�[�h�G���x���[�u�ݒ�
;-------------------------------------------

PUTENV:	ADD	A,-16		; �G���x���[�u�̃p�^�[���ԍ��𓾂�
	LD	E,A
	LD	A,13		; �G���x���[�u�̏o�̓��W�X�^
	CALL	WRTPSG		; PSG�ɏo��(BIOS)
	LD	A,16		; ���ʂ��G���x���[�u�o�͂Ƃ���
	RET

;-------------------------------------------
;	���W�X�^�Z�b�g
;-------------------------------------------

WRTPSG:
	cp	7
	jr	z, wrtpsg3
WRTPSG2:
	PUSH	DE
	LD	D,A
;WRTPSGL:
;	IN	A,(44H)
;	RLCA
;	JR	C,WRTPSGL

	PUSH	BC
	LD	BC,01C00H

	LD	A,D
	OUT	(C),A		; register
:	LD	A,(IX+0)

	DEC	B
	LD	A,E
	OUT	(C),A		; data

	POP	BC

	LD	A,D
	POP	DE
	RET

wrtpsg3:
	ret

;============================================
; FM���W�X�^�ݒ� 
;============================================

set_fm:	;d = no, e = data
;	while(inp(0x701) & 0x80);
;	outp(0x700,reg);
	ld	bc,701h
set_fm1:
	in	a,(c)
	and	80h
	jr	nz,set_fm1

	dec	c

	ld	a,d
	out	(c),a		; reg

	ld	a,(ix+0)

;	outp(0x701,data);
	ld	a,e
	inc	c
	out	(c),a		; data

	ret
;}
;============================================
; ���F�ݒ�
;============================================

set_tone:	; d = no, e = ch
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

;	j = 0x40 + ch;
	ld	a,40h
	add	a,e
	ld	d,a		; reg

	ld	b,24
;	for(i = 0; i < 28; ++i){

set_tone1:
	push	bc
;	push	de

;		set_fm(j, tone_table[no][i]);

	ld	a,(hl)
	ld	e,a		; data

	call	set_fm

;	pop	de

;		j+=8;

	ld	a,8
	add	a,d
	ld	d,a

	inc	hl


	pop	bc

;	inc	b
;	ld	a,b
;	cp	24
;	jr	nz,set_tone1
	djnz	set_tone1

;	}

;	j += 0x20;

	ld	a,20h
	add	a,d
	ld	d,a

;	set_fm(j, tone_table[no][i+1]);

	inc	hl
	inc	hl
	inc	hl
	inc	hl
	ld	a,(Hl)
	or	0c0h
	ld	e,a

	call	set_fm
	ret
;}

;============================================
; ���K�ݒ�
;============================================
;
set_key: ; d = no, e = ch{
;	set_fm(0x28 + ch, key_table[no][0]);

	push	de

	ld	hl,key_table

	ld	a,d
	ld	c,a
	ld	b,0
	add	hl,bc
;	add	hl,bc

	ld	a,028h
	add	a,e
	ld	d,a		; reg

	ld	a,(hl)	; key_table[no][0]
	ld	e,a		; data

	call	set_fm

;	set_fm(0xa0 + ch, key_table[no][1]);

;	ld	a,-4
;	add	a,d
;	ld	d,a		; reg

;	inc	hl
;	ld	a,(hl)	; key_table[no][1]
;	ld	e,a		; data

;	call	set_fm


;	set_fm(0x30 + ch, 5);

	pop	de

	ld	d,e

	ld	a,030h
	add	a,d
	ld	d,a
	ld	e,5h
	call	set_fm

;	set_fm(0x20 + ch, 0xc0);
;	ld	a,-010h
;	add	a,d
;	ld	d,a
;	ld	e,0c0h
;	call	set_fm

	ret
;}

include "tonex1.inc"

;-------------------------------------------
;	���[�N�G���A
;-------------------------------------------
;	ORG	0DE1EH

COUNT:	DB	1,1,1, 1,1,1,1, 1,1,1,1		; �����J�E���^
STOPFRG:DB	0,0,0, 0,0,0,0, 0,0,0,0		; WAIT&SYNC&OFF
MAXVOL:	DB	15,15,15, 15,15,15,15, 15,15,15,15	; �{�����[��
LENGTH:	DB	5,5,5, 5,5,5,5, 5,5,5,5		; ��{����

OFFSET:	DB	0,0,0,0,0,0, 0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0	; ���t�f�[�^���s���A�h���X
STARTADR:DB	0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0	; ���t�f�[�^�J�n�A�h���X
FEEDVOL:DB	0		; �t�F�[�h�A�E�g���x��
FCOUNT:DB	1		; �t�F�[�h�A�E�g�J�E���^
LOOPTIME:DB	0		; ���t�񐔁i�O�͖������[�v�j
STOPPARTS:DB	0		; 
ENDFRG:	DB	0		; 
NSAVE:DB	0		; 

;	END
