; Not all exceptions push an error code onto the stack
; see Section 6.13, Vol 3A Part 1 
%macro vector 1
vector%+%1:
    push %1     ; trapno
    jmp all_handler_asm_entry
%endmacro

%macro no_hwerr_vector 1
vector%+%1:
    push 0x0    ; errno
    push %1     ; trapno
    jmp all_handler_asm_entry
%endmacro

extern exception_handler
extern iret_from_trapframe

section .text
vector 0
vector 1
vector 2
vector 3
vector 4
vector 5
vector 6
vector 7
vector 8
vector 9
vector 10
vector 11
vector 12
vector 13
vector 14
vector 15
vector 16
vector 17
vector 18
vector 19
vector 20
vector 21
vector 22
vector 23
vector 24
vector 25
vector 26
vector 27
vector 28
vector 29
vector 30
vector 31
vector 32
vector 33
vector 34
vector 35
vector 36
vector 37
vector 38
vector 39
vector 40
vector 41
vector 42
vector 43
vector 44
vector 45
vector 46
vector 47
vector 48
vector 49
vector 50
vector 51
vector 52
vector 53
vector 54
vector 55
vector 56
vector 57
vector 58
vector 59
vector 60
vector 61
vector 62
vector 63
no_hwerr_vector 64 ; timer interrupt will not push an error code
vector 65
vector 66
vector 67
vector 68
vector 69
vector 70
vector 71
vector 72
vector 73
vector 74
vector 75
vector 76
vector 77
vector 78
vector 79
vector 80
vector 81
vector 82
vector 83
vector 84
vector 85
vector 86
vector 87
vector 88
vector 89
vector 90
vector 91
vector 92
vector 93
vector 94
vector 95
vector 96
vector 97
vector 98
vector 99
vector 100
vector 101
vector 102
vector 103
vector 104
vector 105
vector 106
vector 107
vector 108
vector 109
vector 110
vector 111
vector 112
vector 113
vector 114
vector 115
vector 116
vector 117
vector 118
vector 119
vector 120
vector 121
vector 122
vector 123
vector 124
vector 125
vector 126
vector 127
no_hwerr_vector 128 ; int 0x80 will not push an error code
vector 129
vector 130
vector 131
vector 132
vector 133
vector 134
vector 135
vector 136
vector 137
vector 138
vector 139
vector 140
vector 141
vector 142
vector 143
vector 144
vector 145
vector 146
vector 147
vector 148
vector 149
vector 150
vector 151
vector 152
vector 153
vector 154
vector 155
vector 156
vector 157
vector 158
vector 159
vector 160
vector 161
vector 162
vector 163
vector 164
vector 165
vector 166
vector 167
vector 168
vector 169
vector 170
vector 171
vector 172
vector 173
vector 174
vector 175
vector 176
vector 177
vector 178
vector 179
vector 180
vector 181
vector 182
vector 183
vector 184
vector 185
vector 186
vector 187
vector 188
vector 189
vector 190
vector 191
vector 192
vector 193
vector 194
vector 195
no_hwerr_vector 196 ; UART COM1 will not push an error code
vector 197
vector 198
vector 199
vector 200
vector 201
vector 202
vector 203
vector 204
vector 205
vector 206
vector 207
vector 208
vector 209
vector 210
vector 211
vector 212
vector 213
vector 214
vector 215
vector 216
vector 217
vector 218
vector 219
vector 220
vector 221
vector 222
vector 223
vector 224
vector 225
vector 226
vector 227
vector 228
vector 229
vector 230
vector 231
vector 232
vector 233
vector 234
vector 235
vector 236
vector 237
vector 238
vector 239
vector 240
vector 241
vector 242
vector 243
vector 244
vector 245
vector 246
vector 247
vector 248
vector 249
vector 250
vector 251
vector 252
vector 253
vector 254
vector 255

all_handler_asm_entry:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdi
    push rdx
    push rcx
    push rbx
    push rax

    ; using kdata segment
    mov rax, (2 << 3)
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov ss, rax

    mov rdi,rsp
    call exception_handler

    jmp iret_from_trapframe

; this array holds entry address of each vector
global allvectors
section .data
allvectors:
%assign i 0 
%rep    256 
    dq vector%+i 
%assign i i+1 
%endrep