
_start:
        LDW IP, cold_start      ; set the IP to a reference to QUIT
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
        LDW IP, CA              ; Put data word in IP
        JMP next


; ------------------
; NEXT - move to the next instruction of the word
;       lodsl       - read memory at %esi into %eax
;                   - increments %esi by 4
;       jmp *(%eax) - jumps to address at memory address pointed to by eax
; ------------------
next:   LDW CA, (IP)
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

        ; TODO - no dict entry?
INTERPRET:
        .word INTERPRET_code
INTERPRET_code:
        CALL _WORD              ; returns X=addr, Y=len

        ; Is it in the dictionary?
        LDW A, $0
        STW A, (interpret_is_lit)
        CALL _FIND
        CMP Z, $0               ; found?
        JEQ _INTERP_1           ; nope

        ; In the dictionary. Is it an IMMEDIATE codeword?
        ; TODO - test for immediate by checking flags, and jump if it is
        CALL _TCFA              ; takes dict pointer in Z and returns codeword address in Z

        JMP _INTERP_2           ; not immediate

        ; Not in the dictionary; assume a literal number
_INTERP_1:
        LDW A, $1
        STW A, (interpret_is_lit)
        CALL _NUMBER            ; Returns the parsed number in X, Y > 0 if error
        CMP Y, $0
        JNE _INTERP_6
        LDW Z, LIT

        ; We have a word, are we compiling or executing?
_INTERP_2:
        ; TODO - test STATE to see if we're executing or compiling

        JMP _INTERP_4           ; executing

        ; TODO - _INTERP_3 - compile the word

        ; Executing - run the word
_INTERP_4:
        LDW A, (interpret_is_lit)
        CMP A, $0
        JNE _INTERP_5           ; literal!

        ; Not a literal, execute it now. This never returns, but the word
        ; will eventually JMP next, which will reenter the loop in QUIT.
        JMP (Z)

        ; Executing a literal - push it on the stack
_INTERP_5:
        DPUSH X
        JMP next
        
        ; Parse error (not a known word or valid number). Print error message and perhaps some context.
_INTERP_6:
        ; TODO
        HLT

interpret_is_lit:
        .word $0                ; flag used to record if reading a literal


; --- >NUMBER  (NUMBER in JonesForth)
        .dict ">NUMBER"
NUMBER: .word NUMBER_code
NUMBER_code:
        DPOP Y                  ; length of string
        DPOP X                  ; string address
        CALL _NUMBER
        DPUSH X                 ; parsed number
        DPUSH Y                 ; number of unparsed chars (0 = no error)
        JMP next

        ; Parse number
        ; input: X = string address, Y = length
        ; output: Z = parsed number, Y = number of unparsed chars (0 = success)
_NUMBER:
        CMP Y, $0               ; trying to parse a zero-length string is an error, but will return 0
        JEQ _NUMBER_5

        LDW I, X
        LDW X, $0
        LDW B, (var_BASE)

        ; Check if first char is '-'
        LDB D, (I)
        INC I
        LDW A, $0
        DPUSH A
        CMP D, $'-'
        JNE _NUMBER_2
        DPOP A                  ; throw away
        LDW A, $1
        DPUSH A                 ; push <> 0 to indicate negative
        DEC Y                   ; decrement the length, as we consumed the '-'
        CMP Y, $0
        JNE _NUMBER_2           ; read the digits (note, Jonesforth jumped to _NUMBER_1)
        DPOP A                  ; error, string is only '-'
        LDW Y, $1
        RET

        ; Loop reading digits
_NUMBER_1:
        MUL X, B                ; A *= BASE
        LDB D, (I)              ; get next char
        INC I

        ; Convert 0-9, A-Z to a number 0-35.
_NUMBER_2:
        CMP D, $'0'             ; < '0'?
        JLT _NUMBER_4
        SUB D, $'0'
        CMP D, $A               ; > '0'?
        JLT _NUMBER_3           ; nope, was a digit - go process
        CMP D, $11              ; > '0' but < 'A'?
        JLT _NUMBER_4           ; yes, invalid
        SUB D, $7

_NUMBER_3:
        CMP D, B                ; >= BASE?
        JGE _NUMBER_4

        ADD X, D                ; digit is okay, add it and loop
        DEC Y
        CMP Y, $0               ; at end of string?
        JNE _NUMBER_1           ; not yet

        ; Negate the result if first char was '-' (saved on stack)
_NUMBER_4:
        DPOP A
        CMP A, $0
        JEQ _NUMBER_5
        ; TODO - implement NEG
        ; NEG X

_NUMBER_5:
        RET


; --- >BODY  (>CFA in JonesForth)
        .dict ">BODY"
TCFA:   .word TCFA_code
TCFA_code:
        DPOP Z
        CALL _TCFA
        DPUSH Z
        JMP next

        ; Convert dict pointer in Z to codeword pointer in Z
_TCFA:  ADD Z, $2               ; skip link pointer
        LDB A, (Z)              ; load len+flags into M
        INC Z                   ; skip len+flags
        ; TODO - mask the flags in M to get just the length
        ADD Z, A                ; skip the name
        RET


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
        LDW X, (IP)
        ADD IP, $2
        DPUSH X
        JMP next


; -- STORE
        .dict "!"
STORE:  .word STORE_code
STORE_code:
        DPOP X                  ; address to store
        DPOP Y                  ; value to store
        STW Y, (X)              ; do it - store value of Y at address pointed to by X
        JMP next


; --- FETCH
        .dict "@"
FETCH:  .word FETCH_code
FETCH_code:
        DPOP X                  ; address to fetch
        LDW Y, (X)              ; fetch it
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


; --- FIND

        .dict "FIND"
FIND:   .word FIND_code
FIND_code:
        DPOP Y                   ; word length
        DPOP X                   ; word address
        CALL _FIND
        DPUSH Z
        JMP next

        ; Lookup word in dictionary
        ; input: X = word address, Y = word length
        ; output: Z = address of dict entry if found, or 0 if not found
_FIND:
        LDW I, (var_LATEST)
_FIND_1:
        CMP I, $0               ; is this the null pointer at the end of the linked list?
        JEQ _FIND_4             ; ...yup...

        ; Compare the lengths
        LDW A, $2               ; get length address...
        ADD A, I                ; ...into X
        LDB B, (A)              ; get the length
        CMP Y, B                ; do lengths match?
        JNE _FIND_2             ; nope, try next dict entry

        ; Lengths match, check the string
        DPUSH X                 ; save the address
        DPUSH Y                 ; save the length
        LDW A, $3               ; point to the dict string...
        ADD A, I                ; ...we want to compare
_FIND_x:
        LDB C, (X)              ; get the bytes...
        LDB D, (A)              ; ...to compare
        CMP C, D
        JNE _FIND_3             ; strings do not match

        INC X                   ; inc word address
        INC A                   ; inc dict address
        DEC Y                   ; dec length
        CMP Y, $0
        JNE _FIND_x             ; compare remaining chars

        ; Hey! We have a match!
        DPOP Y                  ; clean up the stack
        DPOP X
        LDW Z, I                ; return dict pointer
        RET

        ; Current word is not a match; try prior link
_FIND_2:
        LDW I, (I)
        JMP _FIND_1

        ; String does not match; restore X and Y and move on to next dict entry
_FIND_3:
        DPOP Y                  ; restore length
        DPOP X                  ; restore address
        JMP _FIND_2

        ; Not found
_FIND_4:
        LDW Z, $0               ; return zero to indicate not found
        RET

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
        ; Uses: I
_WORD_1:
        CALL _KEY               ; get next key, returned in X
        CMP X, $'\'             ; is it the start of a comment?
        JEQ _WORD_3
        CMP X, $20              ; whitespace?
        JLE _WORD_1

        LDW I, word_buffer
_WORD_2:
        STB X, (I)
        INC I
        CALL _KEY
        CMP X, $20              ; space?
        JGT _WORD_2             ; nope, keep going

        ; Return the word (pointer to buffer) and length
        LDW Y, I
        SUB Y, word_buffer
        LDW X, word_buffer
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
        LDW X, var_LATEST
        DPUSH X                 ; TODO - DPUSH needs addressing modes!
        JMP next

; ------------------
; Data, buffers, etc.
; ------------------

_dad:   .word $DAD              ; TODO - remove this

var_BASE:
        .word $A

var_LATEST:
        .lastdict               ; most recent entry in dictionary



