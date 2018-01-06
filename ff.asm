
_start:
        LOAD IP, cold_start     ; set the IP to a reference to QUIT
        JMP next


; ------------------
; DOCOL - execute a high-level word
;    IP - points to the word after the one we're about to execute
;    CA - points to the codeword of the word we're about to execute
;
;   %esi -> IP
;   %eax -> CA - true?
; ------------------
DOCOL:  RPUSH IP                ; We're nesting down, so save the IP for when we're done
        INC CA                  ; Move CA to point to the...
        INC CA                  ; ...first data word
        LOAD IP, CA             ; Put data word in IP
        JMP next


; ------------------
; NEXT - move to the next instruction of the word
;       lodsl       - read memory at %esi into %eax
;                   - increments %esi by 4
;       jmp *(%eax) - jumps to address at memory address pointed to by eax
; ------------------
next:   LOAD CA, (IP)
        INC IP
        INC IP                  ; TODO - replace with ADD?
        JMP (CA)


; ------------------
; Words
; ------------------

cold_start:                     ; colon-word w/o a header or codeword
        .word QUIT


; --- STOP - headerless hack word to halt - TODO - remove this
STOP:   .word STOP_code
STOP_code:
        HLT


; --- QUIT
        .dict "QUIT"
QUIT:   .word DOCOL             ; codeword - the interpreter
        ; TODO - need real definition of QUIT here! This is just test code, for now.
        .word LIT
        .word _dad
        .word FETCH
        .word TDUP
        .word LIT
        .word $F00
        .word LIT
        .word _dad
        .word STORE
        .word STOP


; --- DUP
        .dict "DUP"
DUP:    .word DUP_code
DUP_code:
        DPOP X                  ; get data item currently on top of stack
        DPUSH X                 ; and push it...
        DPUSH X                 ; ...twice
        JMP next


; --- 2DUP - testing - TODO - this is NOT the correct, standard definition!
        .dict "2DUP"
TDUP:   .word DOCOL             ; codeword - the interpreter
        .word DUP
        .word DUP
        .word EXIT


; --- EXIT - tacked onto end of all high-level words
        .dict "EXIT"
EXIT:   .word EXIT_code
EXIT_code:
        RPOP IP
        JMP next


; --- LIT

; TODO - should this be headerless? I don't see it in the standard!
        .dict "LIT"
LIT:    .word LIT_code
LIT_code:
        LOAD X, (IP)
        INC IP
        INC IP
        DPUSH X
        JMP next


; -- STORE
        .dict "!"
STORE:  .word STORE_code
STORE_code:
        DPOP X                  ; address to store
        DPOP Y                  ; value to store
        STORE Y, (X)            ; do it - store value of Y at address pointed to by X
        JMP next


; --- FETCH
        .dict "@"
FETCH:  .word FETCH_code
FETCH_code:
        DPOP X                  ; address to fetch
        LOAD Y, (X)             ; fetch it
        DPUSH Y                 ; and put it on the stack
        JMP next


; --- Built-in Variables

; TODO - STATE
; TODO - HERE
; TODO - BASE

        .dict "LATEST"
LATEST: .word LATEST_code       ; codeword
LATEST_code:
        LOAD X, var_LATEST
        DPUSH X                 ; TODO - DPUSH needs addressing modes!
        JMP next

; ------------------
; Data, buffers, etc.
; ------------------

_dad:   .word $DAD              ; TODO - remove this

var_LATEST:
        .lastdict
        ; .word LATEST_head       ; must be the most recent dictionary entry!



