;  CHK=0x1E37
;+----------------------------------------------------------
; select(S)
;
;#include <stdio.h>
;#include <sys/select.h>
;#include <fcntl.h>
;
;main(argc,argv,envp)
;int argc;
;char **argv;
;char **envp;
;{
;struct timeval t;
;int readfds;
;int fd = open("/dev/null",O_RDONLY,0);
;
;	readfds = 1<<fd | 1<<0;
;
;	t.tv_sec = 5;
;	t.tv_usec = 0;
;
;	printf("%d\n",select(32,&readfds,0,0,&t));
;	printf("%08x\n",readfds);
;	exit(0);
;}	/* end of main */
;
;-----------------------------------------------------------

	title	select

	.386

SYSNUM	equ 	2428h

extrn	_errno:dword

public  _select

_TEXT	segment  dword use32 public 'CODE'
	assume   cs: _TEXT
_select	proc near
	mov	eax, SYSNUM		; Get system call number.

	;
	; I don't even pretend to understand masm syntax.  I tried
	; the following line (and variations) without any success.
	;

;	call    far 7:0			; Switch to kernel and call SYSNUM.

	;
	; Don't laugh, it works.
	;

	db 9ah
	dw 0,0
	dw 7

	jb	short _cerror		; below == error.

;	xor	eax, eax		; zero return value (no error).
	ret				; done.

_cerror:
	mov	_errno, eax		; Save error code in _errno.
	mov	eax, -1			; Return -1 (as error).
	ret				; done.

_select	endp

_TEXT	ends

	end
; vi: set tabstop=8 :
