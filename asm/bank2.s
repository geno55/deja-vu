; Deja Vu (USA), NES: PRG bank 2 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK2"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
load_game           := $C800
save_game           := $C858
erase_save_file           := $C89F
check_all_save_files           := $C8A9
restore_state_backup           := $CAC7
draw_game_screen           := $E68D
set_chr_banks_inline:= $E916
draw_rle_screen_2000:= $E9D5
read_joypads    := $EB41
wait_vblank     := $EB9C
wait_vblank_and_hide_sprites:= $EBB9
clear_and_upload_sprites           := $EBDE
jump_table      := $EBE6
request_vram_queue           := $EEE0
queue_vram_block           := $EEE5
queue_vram_fill           := $EEF9
fade_in_palette           := $F29E
fade_out_palette           := $F2DB
play_sound      := $F409
draw_metasprite           := $FE5D
; ----------------------------------------------------------------------------
; scenes $98-$9B
scene_pictures_2:
        .addr   scene_98
        .addr   scene_99
        .addr   scene_9A
        .addr   scene_9B

; scene pictures: 848 bytes from your ROM
scenes_2_data:
        .incbin "scenes_2_data.bin"
scene_98        := scenes_2_data + $0000
scene_99        := scenes_2_data + $00D4
scene_9A        := scenes_2_data + $01A8
scene_9B        := scenes_2_data + $027C

; ----------------------------------------------------------------------------
        .byte   $80,$01,$80,$00,$80,$11,$00,$01
        .byte   $07,$10,$80,$10,$01,$10,$80,$10
        .byte   $00,$01,$00,$11,$07,$01,$00,$01
        .byte   $00,$11,$00,$01,$00,$01,$00,$01
        .byte   $00,$01,$00,$11,$80,$11,$80,$21
        .byte   $00,$01,$00,$11,$00,$01,$00,$01
        .byte   $00,$01,$00,$01,$00,$01,$00,$01
        .byte   $00,$11,$07,$01,$00,$01,$00,$01
        .byte   $07,$01,$00,$01,$00,$21,$80,$10
        .byte   $00,$00,$00,$10,$80,$10,$01,$10
        .byte   $80,$01,$80,$10,$00,$10,$07,$00
        .byte   $00,$10,$00,$10,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$00,$12,$00,$01
        .byte   $00,$00,$00,$00,$07,$10,$00,$00
        .byte   $00,$00,$00,$00,$0A,$00,$00,$00
        .byte   $00,$00,$00,$10,$00,$00,$00,$11
        .byte   $00,$00,$00,$00,$00,$00,$00,$02
        .byte   $00,$00,$00,$11,$80,$00,$80,$10
        .byte   $01,$01,$00,$10,$80,$10,$00,$00
        .byte   $00,$01,$00,$10,$00,$10,$00,$00
        .byte   $00,$00,$00,$00,$00,$10,$00,$01
; ----------------------------------------------------------------------------
        .addr   title_screen
        .addr   title_screen
        .addr   file_select_menu
        .addr   quit_dialog
        .addr   save_dialog
        .addr   far_8400_25
; ----------------------------------------------------------------------------
title_screen:
        jsr     draw_title_picture
        jsr     set_chr_banks_inline
        .byte   $E0,$E1,$E2,$E3,$E8,$E9,$EA,$EB
; ----------------------------------------------------------------------------
        jsr     clear_and_upload_sprites
        lda     #$3D
        sta     $1E
        lda     #$84
        sta     $1F
        jsr     fade_in_palette
L8428:  jsr     read_joypads
        lda     joy_pressed
        and     #$D0
        beq     L8428
L8431:  jsr     read_joypads
        lda     joy_held
        and     #$D0
        bne     L8431
        jmp     fade_out_palette

; ----------------------------------------------------------------------------
        .byte   $0F,$18,$28,$33,$0F,$12,$23,$33
        .byte   $0F,$11,$02,$33,$0F,$18,$28,$33
        .byte   $0F,$0F,$0F,$0F,$0F,$0F,$0F,$0F
        .byte   $0F,$0F,$0F,$0F,$0F,$0F,$0F,$0F
; ----------------------------------------------------------------------------
draw_title_picture:  jsr     draw_rle_screen_2000
        .byte   $FF,$FF,$2A,$FF,$BE,$16,$FF,$FF
        .byte   $09,$6D,$5E,$5F,$6E,$6F,$7E,$FF
        .byte   $CA,$11,$FF,$FF,$08,$8C,$FF,$FF
        .byte   $02,$E7,$FF,$FF,$03,$7F,$FF,$E3
        .byte   $10,$FF,$FF,$09,$E8,$82,$83,$84
        .byte   $FF,$FF,$02,$7C,$7D,$FF,$57,$0F
        .byte   $FF,$FF,$06,$58,$FF,$FF,$02,$81
        .byte   $FF,$F0,$03,$A7,$FF,$FF,$02,$8D
        .byte   $8E,$FF,$57,$0E,$FF,$FF,$06,$68
        .byte   $FF,$FF,$01,$90,$F0,$92,$93,$94
        .byte   $B7,$FF,$FF,$03,$9E,$FF,$57,$0E
        .byte   $FF,$FF,$06,$68,$FF,$FF,$01,$A0
        .byte   $A1,$A2,$A3,$A4,$C7,$FF,$FF,$03
        .byte   $9E,$FF,$57,$0E,$FF,$FF,$06,$68
        .byte   $79,$B0,$B1,$FF,$F0,$04,$95,$86
        .byte   $FF,$FF,$01,$8F,$FF,$57,$0B,$76
        .byte   $57,$57,$FF,$FF,$05,$77,$78,$89
        .byte   $C0,$C1,$C2,$FF,$F0,$04,$96,$FF
        .byte   $FF,$01,$8F,$FF,$57,$0B,$CD,$57
        .byte   $CD,$FF,$FF,$05,$87,$88,$99,$D0
        .byte   $D1,$D2,$D3,$FF,$F0,$03,$A6,$9F
        .byte   $FF,$57,$05,$76,$FF,$57,$06,$CE
        .byte   $57,$DD,$FF,$FF,$04,$6A,$97,$98
        .byte   $99,$FF,$FF,$01,$B2,$B3,$FF,$F0
        .byte   $03,$A5,$B6,$AF,$FF,$75,$05,$DD
        .byte   $75,$85,$FF,$75,$04,$CE,$75,$CE
        .byte   $FF,$FF,$04,$7A,$7B,$A8,$A9,$FF
        .byte   $FF,$01,$C3,$C4,$FF,$F0,$04,$BF
        .byte   $80,$CE,$FF,$80,$04,$DD,$80,$CE
        .byte   $FF,$80,$03,$CD,$CD,$80,$CE,$FF
        .byte   $FF,$04,$8A,$8B,$B8,$B9,$FF,$FF
        .byte   $01,$E1,$E2,$D4,$D5,$F0,$9B,$CA
        .byte   $CA,$CD,$CA,$CE,$CD,$CA,$CE,$67
        .byte   $CD,$67,$CA,$CA,$DD,$CD,$CA,$FF
        .byte   $FF,$05,$9A,$FF,$FF,$01,$C8,$C9
        .byte   $FF,$FF,$03,$DE,$DF,$F0,$9B,$CA
        .byte   $67,$FF,$FF,$01,$50,$51,$FF,$FF
        .byte   $01,$52,$53,$FF,$FF,$03,$DD,$CA
        .byte   $DD,$FF,$FF,$07,$AA,$AB,$AC,$D9
        .byte   $FF,$FF,$03,$EE,$EF,$F0,$F0,$67
        .byte   $FF,$FF,$02,$60,$61,$62,$19,$21
        .byte   $63,$65,$66,$FF,$FF,$01,$CA,$FF
        .byte   $FF,$08,$BA,$BB,$BC,$BD,$FF,$FF
        .byte   $03,$FE,$6B,$D6,$D7,$D8,$FF,$FF
        .byte   $02,$3C,$3D,$72,$29,$38,$34,$73
        .byte   $64,$FF,$FF,$0A,$BA,$CB,$CC,$00
        .byte   $01,$02,$03,$FF,$FF,$03,$07,$08
        .byte   $09,$0A,$0B,$0C,$0D,$0E,$70,$34
        .byte   $73,$64,$FF,$FF,$0A,$DA,$57,$DC
        .byte   $10,$11,$12,$13,$14,$15,$16,$17
        .byte   $18,$19,$CA,$1B,$1C,$1D,$1E,$1F
        .byte   $34,$73,$64,$FF,$FF,$0A,$EA,$EB
        .byte   $EC,$20,$21,$22,$31,$24,$25,$26
        .byte   $28,$38,$29,$2A,$2B,$2C,$2D,$2E
        .byte   $2F,$34,$1A,$74,$FF,$FF,$0D,$20
        .byte   $31,$20,$31,$24,$04,$05,$28,$38
        .byte   $0E,$3A,$3B,$3C,$3D,$CA,$3F,$44
        .byte   $39,$3E,$FF,$FF,$0D,$20,$31,$20
        .byte   $31,$24,$25,$06,$27,$38,$49,$4A
        .byte   $4B,$4C,$4D,$4E,$FF,$FF,$11,$20
        .byte   $31,$20,$31,$24,$35,$36,$37,$38
        .byte   $59,$5A,$5B,$5C,$5D,$FF,$FF,$12
        .byte   $20,$31,$23,$31,$24,$45,$46,$47
        .byte   $48,$69,$FF,$FF,$16,$20,$40,$32
        .byte   $33,$54,$55,$56,$FF,$FF,$19,$30
        .byte   $41,$42,$43,$FF,$FF,$17,$F4,$71
        .byte   $9C,$9D,$DB,$FF,$FF,$1C,$71,$9C
        .byte   $9C,$F9,$FF,$FF,$01,$E5,$E4,$F2
        .byte   $F6,$FF,$FF,$01,$F3,$E5,$F6,$F8
        .byte   $F5,$E0,$F7,$E5,$F2,$F1,$F3,$CF
        .byte   $E5,$F1,$E4,$C6,$FF,$FF,$43,$00
        .byte   $00,$98,$FF,$AA,$05,$00,$95,$FF
        .byte   $55,$06,$00,$59,$FF,$55,$06,$00
        .byte   $FF,$55,$03,$D5,$F5,$75,$15,$00
        .byte   $D5,$FF,$FF,$01,$F5,$FF,$FF,$02
        .byte   $33,$00,$00,$CC,$FF,$FF,$03,$0F
        .byte   $03,$00,$C0,$FC,$FF,$FF,$01,$FF
        .byte   $F0,$05,$FF,$00,$08,$FF,$FF,$00
; ----------------------------------------------------------------------------
file_select_menu:
        jsr     file_select_screen
L86D3:  jsr     draw_file_cursor
        jsr     wait_vblank_and_hide_sprites
        jsr     read_joypads
        lda     joy_pressed
        bmi     L8702
        and     #$0C
        beq     L86D3
        cmp     #$04
        beq     L86F4
        dec     save_file
        lda     save_file
        bpl     L86D3
        lda     #$02
        sta     save_file
        bne     L86D3
L86F4:  inc     save_file
        lda     save_file
        cmp     #$03
        bne     L86D3
        lda     #$00
        sta     save_file
        beq     L86D3
L8702:  ldx     save_file
        lda     scene_index
        and     L8741,x
        beq     L874F
        lda     #$00
        sta     $84
        jsr     draw_file_options
L8712:  jsr     draw_option_cursor
        jsr     wait_vblank_and_hide_sprites
        jsr     read_joypads
        lda     joy_pressed
        bmi     L8744
        and     #$0C
        beq     L8712
        and     #$04
        bne     L8733
        dec     $84
        lda     $84
        bpl     L8712
        lda     #$02
        sta     $84
        bne     L8712
L8733:  inc     $84
        lda     $84
        cmp     #$03
        bne     L8712
        lda     #$00
        sta     $84
        beq     L8712
L8741:  .byte   $01,$02,$04
; ----------------------------------------------------------------------------
L8744:  lda     $84
        jsr     jump_table
        .addr   L8753
        .addr   L875E
        .addr   L8764
; ----------------------------------------------------------------------------
L874F:  lda     #$00
        beq     L8758
L8753:  jsr     load_game
        lda     #$FF
L8758:  sta     game_started
        jsr     fade_out_palette
        rts

; ----------------------------------------------------------------------------
L875E:  jsr     draw_file_list
        jmp     L86D3

; ----------------------------------------------------------------------------
L8764:  jsr     show_file_options
        lda     scene_index
        bne     L876E
far_8400_25:
        jsr     erase_save_file
L876E:  jsr     check_all_save_files
        jmp     L875E

; ----------------------------------------------------------------------------
file_select_screen:  jsr     check_all_save_files
        jsr     fade_out_palette
        jsr     clear_and_upload_sprites
        jsr     set_chr_banks_inline
        .byte   $E4,$E5,$E6,$E7,$E4,$E5,$E6,$E7
; ----------------------------------------------------------------------------
        jsr     draw_dialog_screen
        jsr     draw_file_list
        lda     #$00
        sta     save_file
        jsr     draw_file_cursor
        jsr     wait_vblank_and_hide_sprites
        jmp     L879B

; ----------------------------------------------------------------------------
L879B:  lda     #$A6
        sta     $1E
        lda     #$87
        sta     $1F
        jmp     fade_in_palette

; ----------------------------------------------------------------------------
        .byte   $0F,$27,$02,$30,$0F,$07,$00,$30
        .byte   $0F,$07,$28,$17,$0F,$27,$28,$37
        .byte   $0F,$27,$02,$30,$0F,$07,$00,$30
        .byte   $0F,$07,$28,$17,$0F,$27,$28,$37
; ----------------------------------------------------------------------------
draw_file_list:  lda     scene_index
        sta     $76
        jsr     clear_file_area
        ldx     #$00
L87CF:  jsr     draw_file_entry
        jsr     draw_file_status
        inx
        cpx     #$03
        bne     L87CF
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
draw_file_entry:  jsr     file_entry_setup
        lda     L8832,x
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_block
        rts

; ----------------------------------------------------------------------------
file_entry_setup:  ldy     #$00
L87F0:  lda     L882A,y
        sta     $57,y
        iny
        cpy     #$05
        bne     L87F0
        lda     L882F,x
        sta     $5C
        lda     #$06
        sta     data_ptr
        lda     #$01
        sta     data_ptr+1
        lda     #$55
        sta     $1E
        lda     #$00
        sta     $1F
        rts

; ----------------------------------------------------------------------------
draw_file_status:  lsr     $76
        bcs     L8829
        lda     #$38
        sta     $1E
        lda     #$88
        sta     $1F
        lda     #$22
        sta     $1D
        lda     L8835,x
        sta     $1C
        jsr     queue_vram_block
L8829:  rts

; ----------------------------------------------------------------------------
L882A:  .byte   $22,$53,$54,$5B,$FF
L882F:  .byte   $0C,$0D,$12
L8832:  .byte   $72,$B2,$F2
L8835:  .byte   $79,$B9,$F9,$03,$01,$ED,$EE,$EF
; ----------------------------------------------------------------------------
draw_file_options:  jsr     clear_file_area
        ldx     save_file
        jsr     file_entry_setup
        lda     #$70
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_block
        ldx     #$00
L8852:  lda     L8873,x
        sta     $1E
        lda     L8874,x
        sta     $1F
        lda     L88B9,x
        sta     $1C
        lda     L88BA,x
        sta     $1D
        jsr     queue_vram_block
        inx
        inx
        cpx     #$06
        bne     L8852
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
L8873:  .byte   $79
L8874:  .byte   $88,$95,$88,$9F,$88,$0D,$02,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$F0,$F1,$F2,$F3
        .byte   $F4,$F2,$6B,$5B,$FF,$FF,$FF,$FF
        .byte   $FF,$08,$01,$5B,$6C,$F4,$F3,$FF
        .byte   $FF,$FF,$FF,$0C,$02,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$F2,$5B,$6D,$FF,$6E,$6F,$AE
        .byte   $5B,$FF,$FF,$FF,$FF
L88B9:  .byte   $90
L88BA:  .byte   $22,$F0,$22,$10,$23
; ----------------------------------------------------------------------------
clear_file_area:  lda     #$D9
        sta     $1E
        lda     #$88
        sta     $1F
        lda     #$70
        sta     $1C
fill_file_area:  lda     #$22
        sta     $1D
        jsr     queue_vram_fill
        jsr     request_vram_queue
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
        .byte   $0D,$07,$FF
; ----------------------------------------------------------------------------
draw_file_cursor:  ldx     save_file
        lda     L88F7,x
        sta     $21
        lda     #$80
        sta     $20
L88E7:  lda     #$FA
        sta     $1E
        lda     #$88
        sta     $1F
        lda     #$02
        sta     $22
        jsr     draw_metasprite
        rts

; ----------------------------------------------------------------------------
L88F7:  .byte   $98,$A8,$B8,$11,$DF,$02
; ----------------------------------------------------------------------------
draw_option_cursor:  ldx     $84
        lda     unreferenced_890A,x
        sta     $21
        lda     #$70
        sta     $20
        bne     L88E7
; no caller found
unreferenced_890A:
        tay
        clv
        iny
show_file_options:  lda     #$02
        sta     $84
        lda     #$00
        sta     scene_index
        lda     #$4B
        sta     $1E
        lda     #$89
        sta     $1F
        lda     #$90
        sta     $1C
        jsr     fill_file_area
        ldx     #$04
        ldy     #$00
L8928:  lda     L89D8,x
        sta     $1E
        lda     L89D9,x
        sta     $1F
        lda     L894E,y
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_block
        inx
        inx
        iny
        cpy     #$02
        bne     L8928
        jsr     request_vram_queue
        jmp     confirm_dialog_loop

; ----------------------------------------------------------------------------
        .byte   $0D,$04,$FF
L894E:  .byte   $B3,$F3
; ----------------------------------------------------------------------------
quit_dialog:
        lda     #$01
        sta     $84
        jsr     open_confirm_dialog
        jsr     confirm_dialog_loop
        lda     scene_index
        bne     L8961
        jsr     restore_state_backup
L8961:  jsr     fade_out_palette
        rts

; ----------------------------------------------------------------------------
save_dialog:
        lda     repeat_sound
        pha
        lda     #$00
        sta     $84
        jsr     open_confirm_dialog
        jsr     confirm_dialog_loop
        lda     scene_index
        bne     L8979
        jsr     save_game
L8979:  jsr     fade_out_palette
        jsr     draw_game_screen
        pla
        jsr     play_sound
        rts

; ----------------------------------------------------------------------------
open_confirm_dialog:  jsr     fade_out_palette
        lda     #$01
        jsr     play_sound
        lsr     system_flags
        asl     system_flags
        jsr     clear_and_upload_sprites
        jsr     set_chr_banks_inline
        .byte   $F4,$F5,$F6,$F7,$F4,$F5,$F6,$F7
; ----------------------------------------------------------------------------
        jsr     draw_dialog_screen
        jsr     draw_dialog
        jmp     L879B

; ----------------------------------------------------------------------------
draw_dialog:  jsr     draw_dialog_text
        jsr     request_vram_queue
        jsr     reset_dialog_answer
        jsr     wait_vblank_and_hide_sprites
        rts

; ----------------------------------------------------------------------------
draw_dialog_text:  lda     $84
        bne     L89FE
        ldx     #$00
L89BA:  lda     L89D8,x
        sta     $1E
        lda     L89D9,x
        sta     $1F
        lda     L89F6,x
        sta     $1C
        lda     L89F7,x
        sta     $1D
        jsr     queue_vram_block
        inx
        inx
        cpx     #$08
        bne     L89BA
        rts

; ----------------------------------------------------------------------------
L89D8:  .byte   $E0
L89D9:  .byte   $89,$E3,$89,$ED,$89,$F2,$89,$01
        .byte   $01,$FF,$08,$01,$6B,$6C,$6D,$6E
        .byte   $6F,$FF,$FF,$FF,$03,$01,$7A,$5B
        .byte   $8A,$02,$01,$90,$A0
L89F6:  .byte   $74
L89F7:  .byte   $22,$92,$22,$F5,$22,$35,$23
; ----------------------------------------------------------------------------
L89FE:  ldx     #$00
L8A00:  lda     L8A1E,x
        sta     $1E
        lda     L8A1F,x
        sta     $1F
        lda     L8A31,x
        sta     $1C
        lda     L8A32,x
        sta     $1D
        jsr     queue_vram_block
        inx
        inx
        cpx     #$04
        bne     L8A00
        rts

; ----------------------------------------------------------------------------
L8A1E:  .byte   $22
L8A1F:  .byte   $8A,$2C,$8A,$08,$01,$F0,$F1,$F2
        .byte   $F3,$F4,$F2,$CE,$5B,$03,$01,$5B
        .byte   $F2,$DE
L8A31:  .byte   $B3
L8A32:  .byte   $22,$13,$23
; ----------------------------------------------------------------------------
reset_dialog_answer:  lda     #$00
        sta     scene_index
        jsr     draw_dialog_cursor
L8A3C:  rts

; ----------------------------------------------------------------------------
draw_dialog_cursor:  lda     $84
        asl     a
        asl     a
        sta     data_ptr
        lda     scene_index
        asl     a
        clc
        adc     data_ptr
        tay
        lda     L8A57,y
        sta     $20
        lda     L8A58,y
        sta     $21
        jmp     L88E7

; ----------------------------------------------------------------------------
L8A57:  .byte   $98
L8A58:  .byte   $B8,$98,$C8,$88,$A8,$88,$C0,$88
        .byte   $A8,$88,$B8
; ----------------------------------------------------------------------------
confirm_dialog_loop:  jsr     draw_dialog_cursor
        jsr     wait_vblank_and_hide_sprites
        jsr     read_joypads
        lda     joy_pressed
        bmi     L8A3C
        and     #$0C
        beq     confirm_dialog_loop
        cmp     #$04
        beq     L8A7D
        dec     scene_index
        jmp     L8A7F

; ----------------------------------------------------------------------------
L8A7D:  inc     scene_index
L8A7F:  lda     scene_index
        and     #$01
        sta     scene_index
        jmp     confirm_dialog_loop

; ----------------------------------------------------------------------------
draw_dialog_screen:  jsr     draw_rle_screen_2000
        .byte   $FF,$FF,$2D,$0E,$0F,$52,$FF,$FF
        .byte   $1B,$1C,$1D,$1E,$1F,$62,$63,$64
        .byte   $FF,$FF,$19,$2C,$2D,$2E,$2F,$55
        .byte   $56,$57,$FF,$FF,$18,$61,$3C,$3D
        .byte   $3E,$3F,$65,$66,$67,$FF,$FF,$16
        .byte   $39,$3A,$3B,$4C,$4D,$4E,$4F,$58
        .byte   $59,$5A,$FF,$FF,$16,$49,$4A,$4B
        .byte   $5C,$5D,$5E,$5F,$68,$69,$6A,$FF
        .byte   $FF,$13,$50,$05,$06,$07,$08,$09
        .byte   $0A,$0B,$2A,$2B,$FF,$FF,$15,$51
        .byte   $60,$15,$16,$17,$18,$19,$1A,$1B
        .byte   $FF,$FF,$14,$00,$01,$02,$03,$04
        .byte   $25,$26,$27,$28,$29,$FF,$FF,$16
        .byte   $10,$11,$FF,$FF,$01,$13,$14,$35
        .byte   $36,$37,$38,$FF,$FF,$17,$20,$21
        .byte   $FF,$FF,$01,$23,$24,$45,$46,$47
        .byte   $48,$FF,$FF,$04,$70,$71,$72,$73
        .byte   $74,$75,$FF,$FF,$0D,$30,$31,$32
        .byte   $33,$34,$FF,$FF,$06,$98,$98,$80
        .byte   $81,$82,$83,$84,$85,$86,$87,$88
        .byte   $89,$FF,$FF,$09,$40,$41,$42,$43
        .byte   $44,$FF,$FF,$2C,$7B,$7C,$7D,$7E
        .byte   $7F,$91,$92,$93,$94,$FF,$FF,$17
        .byte   $8B,$8C,$8D,$8E,$8F,$A1,$A2,$A3
        .byte   $A4,$FF,$FF,$01,$98,$98,$FF,$FF
        .byte   $6B,$B6,$B7,$DB,$EB,$FF,$FF,$1C
        .byte   $C6,$C7,$FF,$FF,$01,$BC,$FF,$FF
        .byte   $1C,$C0,$D2,$9F,$CC,$FF,$FF,$1C
        .byte   $E1,$E2,$A5,$DC,$FF,$FF,$1A,$A6
        .byte   $A7,$A8,$A9,$AA,$AB,$AC,$AD,$FF
        .byte   $FF,$13,$B1,$B2,$B3,$B4,$B5,$B0
        .byte   $B0,$B8,$B9,$BA,$BB,$B0,$BD,$FF
        .byte   $FF,$13,$C1,$C2,$C3,$C4,$C5,$B0
        .byte   $B0,$C8,$C9,$CA,$CB,$B0,$CD,$FF
        .byte   $FF,$12,$D0,$D1,$B0,$D3,$D4,$D5
        .byte   $D6,$D7,$D8,$D9,$DA,$B0,$B0,$DD
        .byte   $FF,$FF,$12,$E0,$B0,$B0,$E3,$E4
        .byte   $E5,$E6,$E7,$E8,$E9,$EA,$B0,$EC
        .byte   $FF,$FF,$13,$FF,$B0,$05,$F5,$F6
        .byte   $F7,$F8,$F9,$FA,$FB,$FC,$FF,$FF
        .byte   $13,$FF,$B0,$06,$99,$9A,$9B,$9C
        .byte   $9D,$9E,$FF,$FF,$13,$FF,$00,$1B
        .byte   $0F,$CF,$FF,$FF,$01,$F0,$FF,$00
        .byte   $03,$10,$30,$FF,$F0,$04,$55,$55
        .byte   $AA,$62,$FF,$FF,$04,$55,$55,$AA
        .byte   $66,$FF,$FF,$04,$FF,$05,$03,$01
        .byte   $FF,$0F,$04,$FF,$FF,$00
; 255 bytes of $FF
unused_2_8C01:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF

; item names: 2048 bytes from your ROM
item_names_data:
        .incbin "item_names_data.bin"
item_names      := item_names_data + $0000


; word names: 1792 bytes from your ROM
word_names_data:
        .incbin "word_names_data.bin"
word_names      := word_names_data + $0000

; ----------------------------------------------------------------------------
; per room ($0514): 6-byte records, $FF ends
room_hotspots:
        .addr   hotspots_00
        .addr   hotspots_01
        .addr   hotspots_02
        .addr   hotspots_03
        .addr   hotspots_04
        .addr   hotspots_05
        .addr   hotspots_06
        .addr   hotspots_07
        .addr   hotspots_08
        .addr   hotspots_09
        .addr   hotspots_0A
        .addr   hotspots_0B
        .addr   hotspots_0C
        .addr   hotspots_0D
        .addr   hotspots_0E
        .addr   hotspots_0F
        .addr   hotspots_10
        .addr   hotspots_11
        .addr   hotspots_12
        .addr   hotspots_13
        .addr   hotspots_14
        .addr   hotspots_15
        .addr   hotspots_16
        .addr   hotspots_17
        .addr   hotspots_18
        .addr   hotspots_19
        .addr   hotspots_1A
        .addr   hotspots_1B
        .addr   hotspots_1C
        .addr   hotspots_1D
        .addr   hotspots_1E
        .addr   hotspots_1F
        .addr   hotspots_20
        .addr   hotspots_21
        .addr   hotspots_22
        .addr   hotspots_23
        .addr   hotspots_24
        .addr   hotspots_25
        .addr   hotspots_26
        .addr   hotspots_27
        .addr   hotspots_28
        .addr   hotspots_29
        .addr   hotspots_2A
        .addr   hotspots_2B
        .addr   hotspots_2C
        .addr   hotspots_2D
        .addr   hotspots_2E
        .addr   hotspots_2F
        .addr   hotspots_30
        .addr   hotspots_31
        .addr   hotspots_32
        .addr   hotspots_33
        .addr   hotspots_34
        .addr   hotspots_35
        .addr   hotspots_36
        .addr   hotspots_37
        .addr   hotspots_38
        .addr   hotspots_39
        .addr   hotspots_3A
        .addr   hotspots_3B
        .addr   hotspots_3C
        .addr   hotspots_3D
        .addr   hotspots_3E
        .addr   hotspots_3F
        .addr   hotspots_40
        .addr   hotspots_41
        .addr   hotspots_42
        .addr   hotspots_43
        .addr   hotspots_44
        .addr   hotspots_45
        .addr   hotspots_46
        .addr   hotspots_47
        .addr   hotspots_48
        .addr   hotspots_49
        .addr   hotspots_4A
        .addr   hotspots_4B
        .addr   hotspots_4C
        .addr   hotspots_4D

; room hotspot lists: 1806 bytes from your ROM
hotspots_data:
        .incbin "hotspots_data.bin"
hotspots_00     := hotspots_data + $0000
hotspots_01     := hotspots_data + $001F
hotspots_02     := hotspots_data + $003E
hotspots_03     := hotspots_data + $005D
hotspots_04     := hotspots_data + $0082
hotspots_05     := hotspots_data + $009B
hotspots_06     := hotspots_data + $00C0
hotspots_07     := hotspots_data + $00E5
hotspots_08     := hotspots_data + $00FE
hotspots_09     := hotspots_data + $0117
hotspots_0A     := hotspots_data + $0124
hotspots_0B     := hotspots_data + $0143
hotspots_0C     := hotspots_data + $0174
hotspots_0D     := hotspots_data + $0199
hotspots_0E     := hotspots_data + $01C4
hotspots_0F     := hotspots_data + $01DD
hotspots_10     := hotspots_data + $01EA
hotspots_11     := hotspots_data + $01FD
hotspots_12     := hotspots_data + $022E
hotspots_13     := hotspots_data + $0247
hotspots_14     := hotspots_data + $025A
hotspots_15     := hotspots_data + $0261
hotspots_16     := hotspots_data + $0268
hotspots_17     := hotspots_data + $027B
hotspots_18     := hotspots_data + $028E
hotspots_19     := hotspots_data + $028F
hotspots_1A     := hotspots_data + $02A8
hotspots_1B     := hotspots_data + $02C1
hotspots_1C     := hotspots_data + $02DA
hotspots_1D     := hotspots_data + $02F3
hotspots_1E     := hotspots_data + $02FA
hotspots_1F     := hotspots_data + $0301
hotspots_20     := hotspots_data + $030E
hotspots_21     := hotspots_data + $0315
hotspots_22     := hotspots_data + $0322
hotspots_23     := hotspots_data + $033B
hotspots_24     := hotspots_data + $0366
hotspots_25     := hotspots_data + $036D
hotspots_26     := hotspots_data + $03C8
hotspots_27     := hotspots_data + $03DB
hotspots_28     := hotspots_data + $03EE
hotspots_29     := hotspots_data + $03FB
hotspots_2A     := hotspots_data + $0408
hotspots_2B     := hotspots_data + $0445
hotspots_2C     := hotspots_data + $046A
hotspots_2D     := hotspots_data + $0477
hotspots_2E     := hotspots_data + $047E
hotspots_2F     := hotspots_data + $04D9
hotspots_30     := hotspots_data + $04F8
hotspots_31     := hotspots_data + $0523
hotspots_32     := hotspots_data + $0530
hotspots_33     := hotspots_data + $0549
hotspots_34     := hotspots_data + $054A
hotspots_35     := hotspots_data + $057B
hotspots_36     := hotspots_data + $0588
hotspots_37     := hotspots_data + $05AD
hotspots_38     := hotspots_data + $05D2
hotspots_39     := hotspots_data + $05D9
hotspots_3A     := hotspots_data + $05EC
hotspots_3B     := hotspots_data + $05F3
hotspots_3C     := hotspots_data + $0600
hotspots_3D     := hotspots_data + $060D
hotspots_3E     := hotspots_data + $061A
hotspots_3F     := hotspots_data + $061B
hotspots_40     := hotspots_data + $063A
hotspots_41     := hotspots_data + $0659
hotspots_42     := hotspots_data + $0666
hotspots_43     := hotspots_data + $0673
hotspots_44     := hotspots_data + $0680
hotspots_45     := hotspots_data + $068D
hotspots_46     := hotspots_data + $069A
hotspots_47     := hotspots_data + $06A7
hotspots_48     := hotspots_data + $06B4
hotspots_49     := hotspots_data + $06C1
hotspots_4A     := hotspots_data + $06CE
hotspots_4B     := hotspots_data + $06D5
hotspots_4C     := hotspots_data + $06E8
hotspots_4D     := hotspots_data + $0701

; ----------------------------------------------------------------------------
; 86 bytes of $FF
unused_2_A3AA:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF
; ----------------------------------------------------------------------------
; read by 7:D4FD
item_lists_A400:
        .addr   item_lists_A453
        .addr   item_lists_A404
; read by 7:D4FD
item_lists_A404:
        .addr   LA428
        .addr   LA42F
        .addr   LA433
        .addr   LA435
        .addr   LA437
        .addr   LA439
        .addr   LA43B
        .addr   LA43F
        .addr   LA441
        .addr   LA443
        .addr   LA445
        .addr   LA447
        .addr   LA449
        .addr   LA44B
        .addr   LA44D
        .addr   LA44F
        .addr   LA450
        .addr   LA451
; ----------------------------------------------------------------------------
LA428:  .byte   $00,$01,$02,$03,$04,$61,$FF
LA42F:  .byte   $05,$06,$07,$FF
LA433:  .byte   $08,$FF
LA435:  .byte   $0E,$FF
LA437:  .byte   $0F,$FF
LA439:  .byte   $10,$FF
LA43B:  .byte   $14,$54,$67,$FF
LA43F:  .byte   $43,$FF
LA441:  .byte   $16,$FF
LA443:  .byte   $49,$FF
LA445:  .byte   $1E,$FF
LA447:  .byte   $1F,$FF
LA449:  .byte   $20,$FF
LA44B:  .byte   $22,$FF
LA44D:  .byte   $23,$FF
LA44F:  .byte   $FF
LA450:  .byte   $FF
LA451:  .byte   $2D,$FF
; ----------------------------------------------------------------------------
; read by 7:D4FD
item_lists_A453:
        .addr   LA46D
        .addr   LA46F
        .addr   LA472
        .addr   LA475
        .addr   LA477
        .addr   LA47B
        .addr   LA47E
        .addr   LA483
        .addr   LA485
        .addr   LA489
        .addr   LA48B
        .addr   LA48F
        .addr   LA493
; ----------------------------------------------------------------------------
LA46D:  .byte   $0B,$FF
LA46F:  .byte   $0C,$0D,$FF
LA472:  .byte   $63,$64,$FF
LA475:  .byte   $65,$FF
LA477:  .byte   $11,$12,$13,$FF
LA47B:  .byte   $15,$68,$FF
LA47E:  .byte   $17,$18,$19,$1A,$FF
LA483:  .byte   $69,$FF
LA485:  .byte   $1B,$1C,$1D,$FF
LA489:  .byte   $6A,$FF
LA48B:  .byte   $21,$6E,$6D,$FF
LA48F:  .byte   $24,$25,$26,$FF
LA493:  .byte   $FF
; 108 bytes of $FF
unused_2_A494:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF
; ----------------------------------------------------------------------------
; 128 message pointers (tools/text.py)
messages_2:
        .addr   msg2_00
        .addr   msg2_01
        .addr   msg2_02
        .addr   msg2_03
        .addr   msg2_04
        .addr   msg2_05
        .addr   msg2_06
        .addr   msg2_07
        .addr   msg2_08
        .addr   msg2_09
        .addr   msg2_0A
        .addr   msg2_0B
        .addr   msg2_0C
        .addr   msg2_0D
        .addr   msg2_0E
        .addr   msg2_0F
        .addr   msg2_10
        .addr   msg2_11
        .addr   msg2_12
        .addr   msg2_13
        .addr   msg2_14
        .addr   msg2_15
        .addr   msg2_16
        .addr   msg2_17
        .addr   msg2_18
        .addr   msg2_19
        .addr   msg2_1A
        .addr   msg2_1B
        .addr   msg2_1C
        .addr   msg2_1D
        .addr   msg2_1E
        .addr   msg2_1F
        .addr   msg2_20
        .addr   msg2_21
        .addr   msg2_22
        .addr   msg2_23
        .addr   msg2_24
        .addr   msg2_25
        .addr   msg2_26
        .addr   msg2_27
        .addr   msg2_28
        .addr   msg2_29
        .addr   msg2_2A
        .addr   msg2_2B
        .addr   msg2_2C
        .addr   msg2_2D
        .addr   msg2_2E
        .addr   msg2_2F
        .addr   msg2_30
        .addr   msg2_31
        .addr   msg2_32
        .addr   msg2_33
        .addr   msg2_34
        .addr   msg2_35
        .addr   msg2_36
        .addr   msg2_37
        .addr   msg2_38
        .addr   msg2_39
        .addr   msg2_3A
        .addr   msg2_3B
        .addr   msg2_3C
        .addr   msg2_3D
        .addr   msg2_3E
        .addr   msg2_3F
        .addr   msg2_40
        .addr   msg2_41
        .addr   msg2_42
        .addr   msg2_43
        .addr   msg2_44
        .addr   msg2_45
        .addr   msg2_46
        .addr   msg2_47
        .addr   msg2_48
        .addr   msg2_49
        .addr   msg2_4A
        .addr   msg2_4B
        .addr   msg2_4C
        .addr   msg2_4D
        .addr   msg2_4E
        .addr   msg2_4F
        .addr   msg2_50
        .addr   msg2_51
        .addr   msg2_52
        .addr   msg2_53
        .addr   msg2_54
        .addr   msg2_55
        .addr   msg2_56
        .addr   msg2_57
        .addr   msg2_58
        .addr   msg2_59
        .addr   msg2_5A
        .addr   msg2_5B
        .addr   msg2_5C
        .addr   msg2_5D
        .addr   msg2_5E
        .addr   msg2_5F
        .addr   msg2_60
        .addr   msg2_61
        .addr   msg2_62
        .addr   msg2_63
        .addr   msg2_64
        .addr   msg2_65
        .addr   msg2_66
        .addr   msg2_67
        .addr   msg2_68
        .addr   msg2_69
        .addr   msg2_6A
        .addr   msg2_6B
        .addr   msg2_6C
        .addr   msg2_6D
        .addr   msg2_6E
        .addr   msg2_6F
        .addr   msg2_70
        .addr   msg2_71
        .addr   msg2_72
        .addr   msg2_73
        .addr   msg2_74
        .addr   msg2_75
        .addr   msg2_76
        .addr   msg2_77
        .addr   msg2_78
        .addr   msg2_79
        .addr   msg2_7A
        .addr   msg2_7B
        .addr   msg2_7C
        .addr   msg2_7D
        .addr   msg2_7E
        .addr   msg2_7F

; compressed messages: 5158 bytes from your ROM
messages_2_data:
        .incbin "messages_2_data.bin"
msg2_00         := messages_2_data + $0000
msg2_01         := messages_2_data + $0019
msg2_02         := messages_2_data + $006D
msg2_03         := messages_2_data + $0080
msg2_04         := messages_2_data + $009E
msg2_05         := messages_2_data + $00D4
msg2_06         := messages_2_data + $00ED
msg2_07         := messages_2_data + $0113
msg2_08         := messages_2_data + $0133
msg2_09         := messages_2_data + $0143
msg2_0A         := messages_2_data + $0154
msg2_0B         := messages_2_data + $0162
msg2_0C         := messages_2_data + $016F
msg2_0D         := messages_2_data + $017A
msg2_0E         := messages_2_data + $019E
msg2_0F         := messages_2_data + $01C3
msg2_10         := messages_2_data + $01EA
msg2_11         := messages_2_data + $0215
msg2_12         := messages_2_data + $0224
msg2_13         := messages_2_data + $0236
msg2_14         := messages_2_data + $0250
msg2_15         := messages_2_data + $0262
msg2_16         := messages_2_data + $0275
msg2_17         := messages_2_data + $029F
msg2_18         := messages_2_data + $02D9
msg2_19         := messages_2_data + $02F2
msg2_1A         := messages_2_data + $0320
msg2_1B         := messages_2_data + $032F
msg2_1C         := messages_2_data + $0354
msg2_1D         := messages_2_data + $035F
msg2_1E         := messages_2_data + $037B
msg2_1F         := messages_2_data + $03FC
msg2_20         := messages_2_data + $0414
msg2_21         := messages_2_data + $0427
msg2_22         := messages_2_data + $0430
msg2_23         := messages_2_data + $043A
msg2_24         := messages_2_data + $046C
msg2_25         := messages_2_data + $048C
msg2_26         := messages_2_data + $04A2
msg2_27         := messages_2_data + $04C9
msg2_28         := messages_2_data + $051A
msg2_29         := messages_2_data + $057A
msg2_2A         := messages_2_data + $0598
msg2_2B         := messages_2_data + $05AD
msg2_2C         := messages_2_data + $05E6
msg2_2D         := messages_2_data + $05FA
msg2_2E         := messages_2_data + $060D
msg2_2F         := messages_2_data + $0634
msg2_30         := messages_2_data + $0664
msg2_31         := messages_2_data + $066D
msg2_32         := messages_2_data + $0694
msg2_33         := messages_2_data + $06A8
msg2_34         := messages_2_data + $06BC
msg2_35         := messages_2_data + $06D1
msg2_36         := messages_2_data + $06E6
msg2_37         := messages_2_data + $071F
msg2_38         := messages_2_data + $075C
msg2_39         := messages_2_data + $0769
msg2_3A         := messages_2_data + $0786
msg2_3B         := messages_2_data + $079A
msg2_3C         := messages_2_data + $07AE
msg2_3D         := messages_2_data + $07D8
msg2_3E         := messages_2_data + $081C
msg2_3F         := messages_2_data + $0854
msg2_40         := messages_2_data + $0875
msg2_41         := messages_2_data + $0895
msg2_42         := messages_2_data + $08AF
msg2_43         := messages_2_data + $08CA
msg2_44         := messages_2_data + $0907
msg2_45         := messages_2_data + $0922
msg2_46         := messages_2_data + $09D0
msg2_47         := messages_2_data + $0A36
msg2_48         := messages_2_data + $0A68
msg2_49         := messages_2_data + $0A97
msg2_4A         := messages_2_data + $0AAB
msg2_4B         := messages_2_data + $0AD5
msg2_4C         := messages_2_data + $0AFD
msg2_4D         := messages_2_data + $0B11
msg2_4E         := messages_2_data + $0B23
msg2_4F         := messages_2_data + $0B33
msg2_50         := messages_2_data + $0B4E
msg2_51         := messages_2_data + $0B58
msg2_52         := messages_2_data + $0B67
msg2_53         := messages_2_data + $0B99
msg2_54         := messages_2_data + $0BBE
msg2_55         := messages_2_data + $0BD1
msg2_56         := messages_2_data + $0BED
msg2_57         := messages_2_data + $0C07
msg2_58         := messages_2_data + $0C39
msg2_59         := messages_2_data + $0C50
msg2_5A         := messages_2_data + $0C70
msg2_5B         := messages_2_data + $0CA8
msg2_5C         := messages_2_data + $0D22
msg2_5D         := messages_2_data + $0D58
msg2_5E         := messages_2_data + $0DDF
msg2_5F         := messages_2_data + $0DF5
msg2_60         := messages_2_data + $0E56
msg2_61         := messages_2_data + $0E8A
msg2_62         := messages_2_data + $0EA7
msg2_63         := messages_2_data + $0ED4
msg2_64         := messages_2_data + $0F2B
msg2_65         := messages_2_data + $0F8E
msg2_66         := messages_2_data + $1036
msg2_67         := messages_2_data + $1067
msg2_68         := messages_2_data + $108D
msg2_69         := messages_2_data + $10C5
msg2_6A         := messages_2_data + $10E6
msg2_6B         := messages_2_data + $10F4
msg2_6C         := messages_2_data + $1121
msg2_6D         := messages_2_data + $112D
msg2_6E         := messages_2_data + $113D
msg2_6F         := messages_2_data + $1157
msg2_70         := messages_2_data + $1189
msg2_71         := messages_2_data + $1197
msg2_72         := messages_2_data + $11BD
msg2_73         := messages_2_data + $11D2
msg2_74         := messages_2_data + $11E5
msg2_75         := messages_2_data + $1230
msg2_76         := messages_2_data + $1243
msg2_77         := messages_2_data + $125E
msg2_78         := messages_2_data + $1272
msg2_79         := messages_2_data + $12E8
msg2_7A         := messages_2_data + $1318
msg2_7B         := messages_2_data + $133D
msg2_7C         := messages_2_data + $135D
msg2_7D         := messages_2_data + $1372
msg2_7E         := messages_2_data + $13BC
msg2_7F         := messages_2_data + $13C0

; ----------------------------------------------------------------------------
; 1498 unused bytes of $FF
free_space_2:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF
