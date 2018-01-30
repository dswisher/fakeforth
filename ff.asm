
; TODO - set initial origin to something other than zero

        .set F_IMMED, $80
        .set F_HIDDEN, $20
        .set F_LENMASK, $1F     ; mask to get just the length (low order bits)
        .set F_LENHIDMASK, $3F  ; F_LENMASK | F_HIDDEN

_start:
        LDW IP, cold_start      ; set the IP to a reference to QUIT
        LDW A, HIMEM            ; get end of used memory...
        STW A, (var_HERE)       ; ...and save it as HERE
        JMP next


; ------------------
; DOCOL - execute a high-level word
;    IP - points to the word after the one we're about to execute
;    CA - points to the codeword of the word we're about to execute
; ------------------
DOCOL:  RPUSH IP                ; We're nesting down, so save the IP for when we're done
        ADD CA, $2              ; Move CA to point to the first data word
        LDW IP, CA              ; Put data word in IP
        JMP next


; ------------------
; NEXT - move to the next instruction of the high level word
; ------------------
next:   LDW CA, (IP)
        ADD IP, $2
        JMP (CA)


cold_start:                     ; colon-word w/o a header or codeword
        .word QUIT



; -------------------------------------------------------------------
; Easy Forth Primitives
; -------------------------------------------------------------------


; --- DROP - drop top of stack
        .dict "DROP"
DROP:   .word DROP_code
DROP_code:
        DPOP A                  ; throw away
        JMP next


; --- SWAP - swap top two elements of stack
        .dict "SWAP"
SWAP:   .word SWAP_code
SWAP_code:
        DPOP A
        DPOP B
        DPUSH A
        DPUSH B
        JMP next


; --- DUP
        .dict "DUP"
DUP:    .word DUP_code
DUP_code:
        DPOP A
        DPUSH A
        DPUSH A
        JMP next


; --- OVER
        .dict "OVER"
OVER:   .word OVER_code
OVER_code:
        DPOP A
        DPOP B
        DPUSH B
        DPUSH A
        DPUSH B
        JMP next


; --- ROT
        .dict "ROT"
ROT:    .word ROT_code
ROT_code:
        DPOP A
        DPOP B
        DPOP C
        DPUSH B
        DPUSH A
        DPUSH C
        JMP next


; --- 2DROP
        .dict "2DROP"
TDROP:  .word TDROP_code
TDROP_code:
        DPOP A
        DPOP A
        JMP next


; --- 2DUP
        .dict "2DUP"
TDUP:   .word TDUP_code
TDUP_code:
        DPOP A
        DPOP B
        DPUSH B
        DPUSH A
        DPUSH B
        DPUSH A
        JMP next


; --- 2SWAP
        .dict "2SWAP"
TSWAP:  .word TSWAP_code
TSWAP_code:
        DPOP A
        DPOP B
        DPOP C
        DPOP D
        DPUSH B
        DPUSH A
        DPUSH D
        DPUSH C
        JMP next


; --- ?DUP
        .dict "?DUP"
QDUP:   .word QDUP_code
QDUP_code:
        DPOP A          ; get the top of stack
        DPUSH A
        CMP A, $0
        JEQ _QDUP
        DPUSH A
_QDUP:  JMP next


; --- 1+
        .dict "1+"
INC:    .word INC_code
INC_code:
        DPOP A
        INC A
        DPUSH A
        JMP next

; --- 1-
        .dict "1-"
DEC:    .word DEC_code
DEC_code:
        DPOP A
        DEC A
        DPUSH A
        JMP next



; --- ADD (+)
        .dict "+"
ADD:    .word ADD_code
ADD_code:
        DPOP A
        DPOP B
        ADD A, B
        DPUSH A
        JMP next


; TODO - -
; TODO - *
; TODO - /MOD



; -------------------------------------------------------------------
; Comparison operations
; -------------------------------------------------------------------


; --- TRUE
        .dict "TRUE"
TRUE:   .word TRUE_code
TRUE_code:
        LDW A, $FF
        DPUSH A
        JMP next


; --- FALSE
        .dict "FALSE"
FALSE:  .word FALSE_code
FALSE_code:
        LDW A, $0
        DPUSH A
        JMP next


; --- EQUAL (=)
        .dict "="
EQUAL:  .word EQUAL_code
EQUAL_code:
        DPOP A
        DPOP B
        CMP A, B
        JEQ TRUE_code
        JMP FALSE_code


; --- NOT EQUAL (<>)
        .dict "<>"
NEQUAL:  .word NEQUAL_code
NEQUAL_code:
        DPOP A
        DPOP B
        CMP A, B
        JEQ FALSE_code
        JMP TRUE_code


; TODO - <
; TODO - >
; TODO - <=
; TODO - >=
; TODO - 0=
; TODO - 0<>
; TODO - 0<
; TODO - 0>
; TODO - 0<=
; TODO - AND
; TODO - OR
; TODO - XOR


; --- INVERT (bitwise negation)
        .dict "INVERT"
INVERT: .word INVERT_code
INVERT_code:
        DPOP A
        NOT A
        DPUSH A
        JMP next



; -------------------------------------------------------------------
; Returning from Forth words
; -------------------------------------------------------------------

; --- EXIT - tacked onto end of all high-level words
        .dict "EXIT"
EXIT:   .word EXIT_code
EXIT_code:
        RPOP IP
        JMP next


; -------------------------------------------------------------------
; Literals
; -------------------------------------------------------------------

; --- LIT

; TODO - should this be headerless? I don't see it in the standard! Does it have another name?
        .dict "LIT"
LIT:    .word LIT_code
LIT_code:
        LDW X, (IP)
        ADD IP, $2
        DPUSH X
        JMP next



; -------------------------------------------------------------------
; Memory
; -------------------------------------------------------------------

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


; TODO - +!
; TODO - -!
; TODO - C!
; TODO - C@
; TODO - C@C!
; TODO - CMOVE


; -------------------------------------------------------------------
; Built-in Variables
; -------------------------------------------------------------------


; --- LATEST
        .dict "LATEST"
LATEST: .word LATEST_code
LATEST_code:
        LDW A, var_LATEST
        DPUSH A
        JMP next


; --- BASE
        .dict "BASE"
BASE:   .word BASE_code
BASE_code:
        LDW A, var_BASE
        DPUSH A
        JMP next


; --- HERE
        .dict "HERE"
HERE:   .word HERE_code
HERE_code:
        LDW A, var_HERE
        DPUSH A
        JMP next


; --- STATE
        .dict "STATE"
STATE:  .word STATE_code
STATE_code:
        LDW A, var_STATE
        DPUSH A
        JMP next


; -------------------------------------------------------------------
; Built-in Constants
; -------------------------------------------------------------------

; TODO - VERSION constant
; TODO - R0 constant
; TODO - DOCOL constant
; TODO - F_IMMED constant


; --- F_HIDDEN constant
        .dict "F_HIDDEN"
        .word F_HIDDEN_code
F_HIDDEN_code:
        LDW A, F_HIDDEN
        DPUSH A
        JMP next


; TODO - F_LENMASK constant




; -------------------------------------------------------------------
; Return Stack
; -------------------------------------------------------------------

; --- >R - push top data stack item onto return stack
        .dict ">R"
TOR:    .word TOR_code
TOR_code:
        DPOP A
        RPUSH A
        JMP next


; --- R> - pop return stack and push on data stack
        .dict "R>"
FROMR:  .word FROMR_code
FROMR_code:
        RPOP A
        DPUSH A
        JMP next


; TODO - RSP@ - need return stack in memory to implement these two
; TODO - RSP!


; --- RDROP - drop top return stack item
        .dict "RDROP"
RDROP:  .word RDROP_code
RDROP_code:
        RPOP A
        JMP next


; --- R@ - copy from return stack to data stack - ( -- x ) ( R: x -- x )
        .dict "R@"
RCOPY:  .word RCOPY_code
RCOPY_code:
        RPOP A
        RPUSH A
        DPUSH A
        JMP next



; -------------------------------------------------------------------
; Data Stack
; -------------------------------------------------------------------

; TODO - DSP@ - need data (parameter) stack in memory to implement these two
; TODO - DSP!



; -------------------------------------------------------------------
; Input and Output
; -------------------------------------------------------------------

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
        JNE _NUMBER_1           ; read the digits
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
        NEG X

_NUMBER_5:
        RET


; -------------------------------------------------------------------
; Dictionary Lookups
; -------------------------------------------------------------------

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
        AND B, F_LENHIDMASK     ; ...and apply it
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
_FIND_y:
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
        AND A, F_LENMASK        ; mask off the flags
        INC Z                   ; skip len+flags
        ADD Z, A                ; skip the name
        RET


; -------------------------------------------------------------------
; Compiling
; -------------------------------------------------------------------

; --- CREATE
        .dict "CREATE"
CREATE: .word CREATE_code
CREATE_code:
        DPOP C                  ; get word length
        DPOP B                  ; get word address
        LDW I, (var_HERE)       ; get the address where we'll be writing things
        LDW J, I                ; save current point
        LDW A, (var_LATEST)     ; get pointer to prev word
        STW A, (I)              ; add link
        ADD I, $2               ; bump address
        STB C, (I)              ; save the length
        INC I                   ; bump address

        ; Copy the word itself
_CREATE_1:
        LDB D, (B)
        STB D, (I)
        INC I
        INC B
        DEC C
        CMP C, $0
        JNE _CREATE_1

        ; update LATEST and HERE
_CREATE_2:
        STW J, (var_LATEST)
        STW I, (var_HERE)

        JMP next


; --- COMMA (,)
        .dict ","
COMMA:  .word COMMA_code
COMMA_code:
        DPOP Z                  ; data to store
        CALL _COMMA
        JMP next
_COMMA: LDW I, (var_HERE)       ; add word in Z to dict entry
        STW Z, (I)
        ADD I, $2
        STW I, (var_HERE)
        RET


; --- LBRAC ([)
        .dict "[", IMMED
LBRAC:  .word LBRAC_code
LBRAC_code:
        LDW A, $0
        STW A, (var_STATE)      ; set STATE to zero (immediate mode)
        JMP next


; --- RBRAC (])
        .dict "]"
RBRAC:  .word RBRAC_code
RBRAC_code:
        LDW A, $1
        STW A, (var_STATE)      ; set STATE to one (compile mode)
        JMP next


; --- COLON (:)
        .dict ":"
COLON:  .word DOCOL
        .word WORD
        .word CREATE
        .word LIT
        .word DOCOL
        .word COMMA
        .word LATEST
        .word FETCH
        .word HIDDEN            ; make the word hidden until the definition is finished
        .word RBRAC             ; go into compile mode
        .word EXIT


; --- SEMICOLON (;)
        .dict ";", IMMED
SEMICOLON:
        .word DOCOL
        .word LIT
        .word EXIT
        .word COMMA             ; append EXIT (so the word will return)
        .word LATEST
        .word FETCH
        .word HIDDEN            ; unhide the word
        .word LBRAC             ; go back into immediate mode
        .word EXIT


; -------------------------------------------------------------------
; Extending the compiler
; -------------------------------------------------------------------


; --- IMMEDIATE
        .dict "IMMEDIATE"
IMMEDIATE:
        .word IMMEDIATE_code
IMMEDIATE_code:
        DPOP A                  ; get the address of the dict entry
        ADD A, $2               ; skip link pointer
        LDB B, (A)              ; get the length/flags byte
        XOR B, F_IMMED          ; flip the bit
        STB B, (A)              ; and save it back
        JMP next


; --- HIDDEN
        .dict "HIDDEN"
HIDDEN: .word HIDDEN_code
HIDDEN_code:
        DPOP A                  ; get the address of the dict entry
        ADD A, $2               ; skip link pointer
        LDB B, (A)              ; get the length/flags byte
        XOR B, F_HIDDEN         ; flip the bit
        STB B, (A)              ; and save it back
        JMP next


; --- ' (tick)
        .dict "'"
TICK:   .word TICK_code
TICK_code:
        CALL _WORD              ; parse the next word
        CALL _FIND              ; look up the word
        CMP Z, $0               ; did we find a word?
        JEQ _TICK_1             ; no, push the zero
        CALL _TCFA              ; convert dict entry to code address and push that
_TICK_1:
        DPUSH Z
        JMP next



; -------------------------------------------------------------------
; Branching
; -------------------------------------------------------------------

; --- BRANCH
        .dict "BRANCH"          ; TODO - what is the standard word for this? Should it be headerless/hidden?
BRANCH: .word BRANCH_code
BRANCH_code:
        ADD IP, (IP)
        JMP next

; TODO - 0BRANCH



; -------------------------------------------------------------------
; Literal Strings
; -------------------------------------------------------------------

; TODO - LITSTRING
; TODO - TELL



; -------------------------------------------------------------------
; Quit and Interpret
; -------------------------------------------------------------------

; --- QUIT
        .dict "QUIT"
QUIT:   .word DOCOL             ; codeword - the interpreter
        ; .word CSTACK
        .word INTERPRET
        .word BRANCH
        .word $-4


; --- INTERPRET

        .dict "INTERPRET"
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

        LDW M, Z                ; is it an immediate word?
        ADD M, $2
_FRED:  LDB M, (M)
        AND M, F_IMMED

        CALL _TCFA              ; takes dict pointer in Z and returns codeword address in Z

        LDW CA, Z

        CMP M, $0
        JEQ _INTERP_2           ; not immediate
        JMP _INTERP_4           ; immediate, go execute it!


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
        LDW A, (var_STATE)      ; check STATE
        CMP A, $0
        JEQ _INTERP_4           ; executing

        CALL _COMMA             ; append the word
        LDW A, (interpret_is_lit)
        CMP A, $0               ; is it a literal?
        JEQ _INTERP_3           ; nope
        LDW Z, X                ; yep - literal - store it
        CALL _COMMA

_INTERP_3:
        JMP next

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
        ; TODO - include some context in the message
        LDW A, error_message
        PUTS A
        LDW A, $A               ; newline
        PUTC A
        JMP next


; -------------------------------------------------------------------
; Odds and Ends
; -------------------------------------------------------------------

; TODO - CHAR


; --- EXECUTE
        .dict "EXECUTE"
EXEC:   .word EXEC_code
EXEC_code:
        DPOP A                  ; get the code word...
        JMP (A)                 ; ...and go there.


; -------------------------------------------------------------------
; My Extensions (pulled in from .f)
; -------------------------------------------------------------------

; --- HEX
        .dict "HEX"
HEX:    .word HEX_code
HEX_code:
        LDW A, $10
        STW A, (var_BASE)
        JMP next


; --- DECIMAL
        .dict "DECIMAL"
DECIMAL:.word DECIMAL_code
DECIMAL_code:
        LDW A, $A
        STW A, (var_BASE)
        JMP next


; -------------------------------------------------------------------
; Temporary Debugging HACKS - TODO - remove these!?
; -------------------------------------------------------------------


; --- Clear stack hack

CSTACK: .word CSTACK_code
CSTACK_code:
        ; DCLR                    ; clear data stack?
        RCLR                    ; clear return stack
        JMP next


; --- DOT-S (hack version for debugging) - TODO - replace this with a real version
        .dict ".S"
        .word DOTS_code
DOTS_code:
        LDW A, (var_BASE)
        PSTACK A
        JMP next


; --- DOT-R (hack version for debugging) - TODO - remove this?
        .dict ".R"
        .word DOTR_code
DOTR_code:
        LDW A, (var_BASE)
        PRSTACK A
        JMP next


; --- DOT (hack version for debugging) - TODO - replace this with a real version
        .dict "."
        .word DOT_code
DOT_code:
        LDW A, B
        LDW A, (var_BASE)
        DPOP B
        PUTN B, A
        LDW A, $A               ; CR
        PUTC A
        JMP next


; -- BREAK (hack for debugging) - break into debugger
        .dict "BREAK"
BREAK:  .word BREAK_code
BREAK_code:
        BRK
        JMP next



; -------------------------------------------------------------------
; Data
; -------------------------------------------------------------------

        ; TODO - .data
        ; A static buffer to hold value returned by WORD.
word_buffer:
        .space $20

interpret_is_lit:
        .word $0                ; flag used to record if reading a literal

error_message:
        .asciz "PARSE ERROR"

var_STATE:
        .word $0
var_BASE:
        .word $A
var_HERE:
        .word $0
var_LATEST:
        .lastdict               ; most recent entry in dictionary; must be AFTER all .dict entries!
HIMEM:

