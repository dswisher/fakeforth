
_start:
        LOAD X, $DAD    ; HACK! Random stuff on the stack to test DUP
        DPUSH X         ; ...more HACK...

        ; "real" code starts here
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
        ; TODO - implement indirect JMP
        ; JMP (CA)
        GO CA   ; TODO - get rid of GO!


; ------------------
; Words
; ------------------

cold_start:                     ; colon-word w/o a header or codeword
        .word QUIT


; --- QUIT
QUIT_head:
        .word $0                ; link to previous word (none, in this case)
        .byte $4                ; length of word - TODO - flags?
        .ascii "QUIT"
QUIT:   .word DOCOL             ; codeword - the interpreter
        ; TODO - need real definition of QUIT here!
        .word TDUP
        .word STOP


; --- DUP
DUP_head:
        .word QUIT_head         ; point back to QUIT
        .byte $3
        .ascii "DUP"
DUP:    .word DUP_code
DUP_code:
        DPOP X                  ; get data item currently on top of stack
        DPUSH X                 ; and push it...
        DPUSH X                 ; ...twice
        JMP next


; --- 2DUP - testing - TODO - this is NOT the correct, standard definition!
TDUP_head:
        .word DUP_head          ; link to previous word
        .byte $4                ; length of word - TODO - flags?
        .ascii "2DUP"
TDUP:   .word DOCOL             ; codeword - the interpreter
        .word DUP
        .word DUP
        .word EXIT


; --- EXIT - tacked onto end of all high-level words
EXIT_head:
        .word TDUP_head         ; link to previous word
        .byte $4                ; length of word - TODO - flags?
        .ascii "EXIT"
EXIT:   .word EXIT_code         ; codeword
EXIT_code:
        RPOP IP
        JMP next


; --- STOP - headerless hack word to halt - TODO - remove this
STOP:   .word STOP_code
STOP_code:
        HLT

; --- Built-in Variables

; TODO - STATE
; TODO - HERE
; TODO - BASE

LATEST_head:
        .word EXIT_head         ; link to previous word
        .byte $6                ; length of word - TODO - flags?
        .ascii "LATEST"
LATEST: .word LATEST_code       ; codeword
LATEST_code:
        LOAD X, var_LATEST
        DPUSH X                 ; TODO - DPUSH needs addressing modes!
        JMP next
var_LATEST:
        .word LATEST_head       ; must be the most recent dictionary entry!



