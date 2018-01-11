
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
        ADD CA, $2              ; Move CA to point to the first data word
        LOAD IP, CA             ; Put data word in IP
        JMP next


; ------------------
; NEXT - move to the next instruction of the word
;       lodsl       - read memory at %esi into %eax
;                   - increments %esi by 4
;       jmp *(%eax) - jumps to address at memory address pointed to by eax
; ------------------
next:   LOAD CA, (IP)
        ADD IP, $2
        JMP (CA)


; ------------------
; Words
; ------------------

cold_start:                     ; colon-word w/o a header or codeword
        .word QUIT


; --- QUIT
        .dict "QUIT"
QUIT:   .word DOCOL             ; codeword - the interpreter
        ; TODO - clear the return stack
        ; TODO - clear the data stack?
        .word INTERPRET
        .word BRANCH
        .word $-4


; --- INTERPRET

INTERPRET:
        .word INTERPRET_code
INTERPRET_code:
        CALL _WORD              ; returns X=addr, Y=len

        ; Is it in the dictionary?
        ; TODO

        JMP next


; --- BRANCH
        .dict "BRANCH"          ; TODO - should this be headerless?
BRANCH: .word BRANCH_code
BRANCH_code:
        ADD IP, (IP)
        JMP next


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
        ADD IP, $2
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


; --- KEY
        .dict "KEY"
KEY:   .word KEY_code
KEY_code:
        CALL _KEY               ; get a character...
        DPUSH X                 ; ...and push it on the stack
        JMP next

_KEY:   GETC X                  ; read character from stdin
        ; TODO - handle input buffers, etc. Take care to only return a byte!
        RET

; --- EMIT
        .dict "EMIT"
EMIT:   .word EMIT_code
EMIT_code:
        DPOP X                  ; get character to print...
        PUTC X                  ; ...and print it.
        JMP next


; --- WORD
        .dict "WORD"
WORD:   .word WORD_code
WORD_code:
        CALL _WORD
        DPUSH X                 ; push base address
        DPUSH Y                 ; push length
        JMP next

_WORD:  
        ; Search for first non-blank character, skipping \ comments
        ; Returns length in Y and address in X
_WORD_1:
        CALL _KEY               ; get next key, returned in X
        CMP X, $'\'             ; is it the start of a comment?
        JEQ _WORD_3
        CMP X, $20              ; whitespace?
        JLE _WORD_1

        LOAD Z, word_buffer
_WORD_2:
        STOS X, Z               ; store character in X at Z, inc Z
        CALL _KEY
        CMP X, $20              ; space?
        JGT _WORD_2             ; nope, keep going

        ; Return the word (pointer to buffer) and length
        LOAD Y, Z
        SUB Y, word_buffer
        LOAD X, word_buffer
        RET

_WORD_3:        ; skip \ comments to end of current line
        CALL _KEY
        CMP X, $A               ; end of line?
        JNE _WORD_3
        JMP _WORD_1

        ; TODO - .data
        ; A static buffer to hold value returned by WORD.
word_buffer:
        .space $20

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



