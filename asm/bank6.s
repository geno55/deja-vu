; Deja Vu (USA), NES: PRG bank 6 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK6"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
test_hotspot           := $C26F
click_picture           := $C2A8
print_message_5 := $C4AE
print_message_4 := $C4EF
print_message_2_or_5:= $C501
item_code_for_object           := $C520
split_item_code           := $C533
show_special_scene           := $C74B
wait_180_frames           := $C7E4
wait_60_frames           := $C7EE
run_command           := $CB4B
script_resume   := $CB5C
op_63           := $CD21
script_condition_true:= $CFDF
script_condition_false:= $CFE6
test_story_flag           := $D10B
test_item_obtained           := $D179
test_object_open           := $D1C0
test_door_flag           := $D24E
set_door_flag           := $D25D
test_event_flag           := $D29B
set_event_flag           := $D2A5
clear_event_flag           := $D2BC
op_1B           := $D33F
test_flag_045C           := $D42C
test_flag_047A           := $D46A
load_container_view           := $D4DF
select_item_list           := $D4ED
op_27           := $D5E4
add_coins_and_show           := $D61A
op_45           := $D7B6
next_random           := $E7B1
set_chr_banks_inline:= $E916
apply_chr_banks := $E97D
draw_rle_screen_2000:= $E9D5
read_joypads    := $EB41
wait_vblank     := $EB9C
wait_nmi_work_done           := $EBB2
wait_vblank_and_hide_sprites:= $EBB9
clear_and_upload_sprites           := $EBDE
request_oam_dma           := $EBE1
jump_table      := $EBE6
clear_nametables           := $EE65
request_vram_queue           := $EEE0
queue_vram_block           := $EEE5
queue_vram_fill           := $EEF9
flush_vram_queue_down           := $F054
queue_vram_column           := $F060
set_attr_block           := $F131
fill_all_attrs           := $F20E
request_attrs_2000           := $F260
request_attr_upload           := $F26A
request_palette_upload           := $F299
fade_in_palette           := $F29E
fade_out_palette           := $F2DB
far_call_8020   := $F3C5
far_call_8040   := $F3D5
far_call_8400   := $F3E5
play_sound      := $F409
draw_layout_number           := $FBED
draw_metasprite           := $FE5D
; ----------------------------------------------------------------------------
        .addr   load_scene_palette
        .addr   load_scene_chr_banks
        .addr   load_palette
        .addr   far_8000_63
        .addr   far_8000_64
        .addr   far_8000_65
        .addr   far_8000_66
        .addr   far_8000_67
        .addr   reset_cursor
        .addr   handle_input
        .addr   draw_cursor
        .addr   dissolve_row_mask
        .addr   far_8000_6C
        .addr   far_8000_6D
        .addr   far_8000_6E
        .addr   far_8000_6F
        .addr   add_object_sprites
        .addr   select_room_scene
        .addr   draw_room_tile_patches
        .addr   script_slot_for_command
        .addr   far_8020_64
        .addr   draw_object_sprites
        .addr   store_in_inventory_slot
        .addr   far_8020_67
        .addr   draw_inventory_entry
        .addr   far_8020_69
        .addr   draw_inventory_page
        .addr   delete_inventory_entry
        .addr   far_8020_6C
        .addr   far_8020_6D
        .addr   far_8020_6E
        .addr   play_slot_machine
        .addr   opening_logos
        .addr   ending_sequence
        .addr   item_number_for_object
        .addr   draw_address_book
        .addr   far_8040_64
        .addr   highlight_entry
; ----------------------------------------------------------------------------
far_8000_6D:
        lda     #$08
        sta     $20
        lda     #$98
        sta     $21
        lda     #$6D
        sta     $1E
        lda     #$80
        sta     $1F
        lda     #$03
        sta     $22
        jsr     draw_metasprite
        jsr     request_oam_dma
        lda     system_flags
        ora     #$01
        sta     system_flags
        rts

; ----------------------------------------------------------------------------
        .byte   $11,$00,$23
; ----------------------------------------------------------------------------
load_scene_chr_banks:
        lda     #$00
        sta     $5E
        lda     scene
        sta     $5D
        asl     $5D
        rol     $5E
        lda     $5D
        clc
        adc     #$AC
        sta     $5D
        lda     $5E
        adc     #$80
        sta     $5E
        ldy     #$00
        lda     ($5D),y
        sta     chr_bank_shadows
        iny
        lda     ($5D),y
        sta     chr_bank_shadows+4
        ldy     #$00
        sty     chr_bank_shadows+2
        sty     chr_bank_shadows+6
        iny
        sty     chr_bank_shadows+7
        lda     #$C0
        sta     chr_bank_shadows+5
        jsr     apply_chr_banks
        rts

; ----------------------------------------------------------------------------
        .byte   $02,$C1,$02,$C1,$04,$C2,$06,$C2
        .byte   $08,$00,$08,$00,$08,$00,$08,$00
        .byte   $0A,$C2,$0A,$C2,$0C,$C2,$0C,$C2
        .byte   $0E,$00,$0E,$00,$10,$C2,$10,$C2
        .byte   $10,$C2,$10,$C2,$12,$CA,$12,$CA
        .byte   $14,$00,$14,$00,$14,$00,$14,$00
        .byte   $16,$00,$16,$00,$18,$00,$18,$00
        .byte   $1A,$00,$1A,$00,$1A,$00,$1A,$00
        .byte   $1C,$C1,$1C,$C1,$1C,$C1,$1C,$C1
        .byte   $1E,$00,$1E,$00,$20,$00,$20,$00
        .byte   $22,$00,$22,$00,$22,$00,$22,$00
        .byte   $24,$00,$24,$00,$26,$00,$26,$00
        .byte   $28,$00,$2A,$00,$28,$00,$28,$00
        .byte   $28,$00,$2A,$00,$2A,$00,$2A,$00
        .byte   $28,$00,$28,$00,$28,$00,$2A,$00
        .byte   $2A,$00,$2A,$00,$28,$00,$2A,$00
        .byte   $2C,$C3,$2E,$00,$2E,$00,$30,$00
        .byte   $32,$C3,$34,$C3,$36,$C3,$38,$C9
        .byte   $38,$C9,$3A,$C3,$3C,$C6,$3E,$00
        .byte   $40,$C3,$42,$00,$44,$00,$46,$00
        .byte   $48,$00,$4A,$00,$4A,$00,$4C,$CA
        .byte   $4E,$00,$50,$00,$52,$C3,$54,$00
        .byte   $56,$C3,$56,$C3,$56,$C3,$56,$C3
        .byte   $58,$00,$58,$00,$58,$00,$58,$00
        .byte   $5A,$00,$5A,$00,$5C,$00,$5C,$00
        .byte   $5C,$00,$5E,$C9,$60,$00,$60,$00
        .byte   $62,$00,$62,$00,$64,$C3,$66,$00
        .byte   $66,$00,$68,$00,$68,$00,$5C,$00
        .byte   $6A,$00,$6A,$00,$6C,$C6,$6E,$C4
        .byte   $6E,$C4,$70,$C5,$70,$C5,$72,$00
        .byte   $74,$00,$76,$00,$78,$00,$7A,$00
        .byte   $7A,$00,$7C,$C5,$7E,$00,$7E,$00
        .byte   $7E,$00,$7E,$00,$80,$C5,$80,$C5
        .byte   $82,$C5,$82,$C5,$84,$00,$86,$00
        .byte   $86,$00,$84,$00,$88,$00,$8A,$00
        .byte   $8A,$00,$8C,$00,$8E,$00,$90,$C7
        .byte   $92,$00,$94,$00,$96,$00,$98,$00
        .byte   $9A,$00,$9C,$00,$9E,$00,$A0,$00
        .byte   $90,$C8
; ----------------------------------------------------------------------------
load_scene_palette:
        ldy     scene
        lda     scene_palette_ids,y
        sta     scene_palette_id
load_palette:
        jsr     palette_pointer
        ldy     #$00
L81EC:  lda     ($1E),y
        sta     palette_buffer,y
        iny
        cpy     #$20
        bcc     L81EC
        rts

; ----------------------------------------------------------------------------
far_8000_63:
        jsr     palette_pointer
        jsr     fade_in_palette
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_81FE:
        jsr     palette_pointer
        jsr     fade_in_palette
        rts

; ----------------------------------------------------------------------------
far_8000_65:
        jsr     palette_pointer
        jsr     fade_out_palette
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_820C:
        jsr     palette_pointer
        jsr     fade_out_palette
        rts

; ----------------------------------------------------------------------------
palette_pointer:  lda     scene_palette_id
        asl     a
        tay
        lda     palette_table,y
        sta     $1E
        lda     L82BD,y
        sta     $1F
        rts

; ----------------------------------------------------------------------------
scene_palette_ids:  .byte   $00,$00,$01,$01,$02,$02,$02,$02
        .byte   $03,$03,$03,$03,$04,$04,$05,$05
        .byte   $05,$05,$06,$06,$07,$07,$07,$07
        .byte   $08,$08,$09,$09,$0A,$0A,$0A,$0A
        .byte   $0B,$0B,$0B,$0B,$0F,$0F,$10,$10
        .byte   $0C,$0C,$0C,$0C,$0D,$0D,$0E,$0E
        .byte   $11,$11,$11,$11,$11,$11,$11,$11
        .byte   $11,$11,$11,$11,$11,$11,$11,$11
        .byte   $12,$2B,$2B,$29,$2A,$13,$14,$15
        .byte   $15,$16,$2D,$17,$18,$19,$1B,$1B
        .byte   $1C,$1A,$1A,$26,$25,$27,$28,$45
        .byte   $2E,$2E,$2F,$2F,$1D,$1D,$1E,$1E
        .byte   $30,$30,$31,$31,$31,$32,$1F,$1F
        .byte   $20,$20,$33,$21,$21,$22,$22,$34
        .byte   $35,$35,$36,$37,$37,$38,$38,$23
        .byte   $23,$39,$3A,$3A,$3A,$3C,$3D,$3D
        .byte   $3D,$3D,$3E,$3E,$3F,$3F,$40,$41
        .byte   $42,$40,$43,$44,$44,$2C,$24,$3B
        .byte   $46,$47,$48,$49,$4A,$4B,$4C,$4D
        .byte   $4E
palette_table:  .byte   $5A
L82BD:  .byte   $83,$7A,$83,$9A,$83,$BA,$83,$DA
        .byte   $83,$FA,$83,$1A,$84,$3A,$84,$5A
        .byte   $84,$7A,$84,$9A,$84,$BA,$84,$DA
        .byte   $84,$FA,$84,$1A,$85,$3A,$85,$5A
        .byte   $85,$7A,$85,$9A,$85,$BA,$85,$DA
        .byte   $85,$FA,$85,$1A,$86,$3A,$86,$5A
        .byte   $86,$7A,$86,$9A,$86,$BA,$86,$DA
        .byte   $86,$FA,$86,$1A,$87,$3A,$87,$5A
        .byte   $87,$7A,$87,$9A,$87,$BA,$87,$BA
        .byte   $8B,$DA,$87,$FA,$87,$1A,$88,$3A
        .byte   $88,$5A,$88,$7A,$88,$9A,$88,$BA
        .byte   $88,$DA,$88,$FA,$88,$1A,$89,$3A
        .byte   $89,$5A,$89,$7A,$89,$9A,$89,$BA
        .byte   $89,$DA,$89,$FA,$89,$1A,$8A,$3A
        .byte   $8A,$5A,$8A,$7A,$8A,$9A,$8A,$BA
        .byte   $8A,$DA,$8A,$FA,$8A,$1A,$8B,$3A
        .byte   $8B,$5A,$8B,$7A,$8B,$9A,$8B,$DA
        .byte   $8B,$FA,$8B,$1A,$8C,$3A,$8C,$5A
        .byte   $8C,$7A,$8C,$9A,$8C,$BA,$8C,$DA
        .byte   $8C,$FA,$8C,$1A,$8D,$0F,$07,$17
        .byte   $30,$0F,$31,$17,$30,$0F,$28,$38
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $37,$0F,$0F,$26,$10,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$02,$17
        .byte   $30,$0F,$07,$17,$30,$0F,$21,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$0F,$0F,$17,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$28
        .byte   $30,$0F,$17,$21,$30,$0F,$28,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$0F
        .byte   $0F,$0F,$0F,$0F,$0F,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$02,$18
        .byte   $30,$0F,$08,$18,$30,$0F,$22,$18
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$31
        .byte   $12,$0F,$27,$31,$30,$0F,$17,$31
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$07,$37
        .byte   $17,$0F,$17,$12,$30,$0F,$17,$37
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$00
        .byte   $30,$0F,$0F,$2A,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$07,$17
        .byte   $37,$0F,$07,$17,$12,$0F,$07,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$16
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$11,$17
        .byte   $37,$0F,$11,$17,$07,$0F,$11,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$16,$10
        .byte   $30,$0F,$16,$28,$2A,$0F,$28,$2A
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$27
        .byte   $37,$0F,$17,$27,$30,$0F,$21,$27
        .byte   $37,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$21
        .byte   $30,$0F,$12,$17,$30,$0F,$27,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$0B,$22
        .byte   $32,$0F,$17,$12,$30,$0F,$21,$12
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$26
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$15
        .byte   $30,$0F,$27,$21,$30,$0F,$15,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$27
        .byte   $30,$0F,$17,$27,$21,$0F,$17,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $21,$0F,$27,$17,$30,$0F,$18,$39
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$11,$10
        .byte   $30,$0F,$18,$10,$30,$0F,$18,$31
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$11,$10
        .byte   $30,$0F,$18,$10,$30,$0F,$18,$31
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$17
        .byte   $37,$0F,$21,$17,$37,$0F,$00,$08
        .byte   $37,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$07,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$17
        .byte   $37,$0F,$2C,$17,$36,$0F,$10,$17
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$00
        .byte   $30,$0F,$17,$27,$30,$0F,$01,$00
        .byte   $12,$0F,$11,$0F,$38,$0F,$0F,$30
        .byte   $1A,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$00
        .byte   $30,$0F,$17,$27,$36,$0F,$17,$00
        .byte   $01,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$27
        .byte   $30,$0F,$2A,$14,$30,$0F,$01,$07
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$00
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$27
        .byte   $36,$0F,$2A,$14,$30,$0F,$10,$07
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$12
        .byte   $30,$0F,$21,$12,$30,$0F,$19,$12
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$17
        .byte   $36,$0F,$00,$12,$30,$0F,$17,$12
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$02,$27
        .byte   $30,$0F,$00,$27,$30,$0F,$02,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$08,$18
        .byte   $00,$0F,$17,$30,$00,$0F,$17,$29
        .byte   $31,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$18,$12
        .byte   $30,$0F,$08,$18,$30,$0F,$17,$10
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$06,$16
        .byte   $00,$0F,$17,$27,$00,$0F,$00,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$11,$21
        .byte   $30,$0F,$00,$21,$30,$0F,$00,$27
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$11,$27
        .byte   $30,$0F,$00,$27,$30,$0F,$00,$27
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$14
        .byte   $36,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$2A,$21
        .byte   $30,$0F,$00,$1C,$30,$0F,$2A,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$2A,$21
        .byte   $30,$0F,$00,$27,$30,$0F,$2A,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$21,$17
        .byte   $30,$0F,$00,$12,$30,$0F,$12,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$21,$17
        .byte   $30,$0F,$00,$27,$30,$0F,$12,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$17
        .byte   $30,$0F,$12,$2A,$00,$0F,$2A,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$26
        .byte   $36,$0F,$17,$29,$36,$0F,$07,$17
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$07,$17
        .byte   $36,$0F,$17,$26,$36,$0F,$17,$12
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$01,$06
        .byte   $36,$0F,$17,$06,$16,$0F,$16,$06
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$16,$06
        .byte   $36,$0F,$10,$00,$30,$0F,$01,$21
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$00
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$12
        .byte   $30,$0F,$16,$12,$30,$0F,$0B,$12
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$18
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$12
        .byte   $30,$0F,$16,$12,$30,$0F,$15,$36
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$01
        .byte   $30,$0F,$00,$17,$37,$0F,$0C,$00
        .byte   $10,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$08
        .byte   $18,$0F,$12,$08,$37,$0F,$12,$08
        .byte   $21,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$00
        .byte   $21,$0F,$17,$27,$36,$0F,$17,$21
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $10,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$1C,$2C
        .byte   $30,$0F,$01,$00,$30,$0F,$01,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$21
        .byte   $36,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$2C
        .byte   $30,$0F,$27,$00,$30,$0F,$27,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0C,$21
        .byte   $36,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$12
        .byte   $30,$0F,$00,$27,$30,$0F,$27,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0C,$21
        .byte   $36,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$21
        .byte   $30,$0F,$17,$10,$30,$0F,$0C,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0C,$21
        .byte   $36,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$21
        .byte   $30,$0F,$1A,$21,$30,$0F,$0C,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$27,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $37,$0F,$27,$17,$30,$0F,$21,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$25,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$21
        .byte   $30,$0F,$17,$10,$30,$0F,$0C,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$25,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$02
        .byte   $30,$0F,$17,$27,$30,$0F,$38,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$25,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $30,$0F,$27,$17,$36,$0F,$21,$00
        .byte   $30,$0F,$11,$0F,$38,$0F,$21,$27
        .byte   $30,$0F,$0F,$00,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$21
        .byte   $30,$0F,$17,$27,$30,$0F,$0F,$0F
        .byte   $0F,$0F,$11,$0F,$38,$0F,$17,$27
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$21
        .byte   $30,$0F,$17,$27,$30,$0F,$17,$21
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$17,$27
        .byte   $37,$0F,$07,$17,$37,$0D,$17,$10
        .byte   $37,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$17
        .byte   $30,$0F,$07,$17,$21,$0F,$12,$17
        .byte   $21,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$23,$33
        .byte   $30,$0F,$13,$23,$30,$0F,$37,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $37,$0F,$0F,$16,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $36,$0F,$27,$30,$36,$0F,$2C,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$37,$12
        .byte   $30,$0F,$27,$17,$30,$0F,$27,$1C
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $36,$0F,$27,$17,$2B,$0F,$27,$2B
        .byte   $36,$0F,$11,$0F,$38,$0F,$0F,$00
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$26,$32
        .byte   $30,$0F,$27,$32,$30,$0F,$27,$38
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $31,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$00
        .byte   $10,$0F,$27,$17,$30,$0F,$27,$1C
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$00
        .byte   $37,$0F,$12,$17,$30,$0F,$1C,$17
        .byte   $0F,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$00
        .byte   $30,$0F,$07,$11,$30,$0F,$17,$1C
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$01,$00
        .byte   $37,$0F,$01,$17,$30,$0F,$01,$00
        .byte   $12,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$08
        .byte   $18,$0F,$12,$08,$16,$0F,$12,$08
        .byte   $21,$0F,$11,$0F,$38,$0F,$0F,$27
        .byte   $30,$0F,$0F,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$0F,$00
        .byte   $10,$0F,$0C,$00,$30,$0F,$0C,$17
        .byte   $31,$0F,$11,$0F,$38,$0F,$0F,$25
        .byte   $30,$0F,$0F,$21,$30,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$07,$17
        .byte   $01,$0F,$26,$36,$16,$0F,$26,$36
        .byte   $01,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$27,$17
        .byte   $30,$0F,$27,$00,$30,$0F,$27,$12
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$21
        .byte   $38,$0F,$12,$27,$38,$0F,$12,$25
        .byte   $38,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$36,$10
        .byte   $17,$0F,$36,$00,$17,$0F,$0F,$0F
        .byte   $0F,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$2A,$1A
        .byte   $00,$0F,$00,$10,$0B,$0F,$01,$12
        .byte   $10,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$12,$21
        .byte   $30,$0F,$27,$16,$30,$0F,$12,$16
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$16,$26
        .byte   $30,$0F,$26,$1A,$30,$0F,$18,$25
        .byte   $16,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$16,$27
        .byte   $30,$0F,$12,$27,$30,$0F,$18,$25
        .byte   $16,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$00,$10
        .byte   $30,$0F,$2A,$16,$30,$0F,$26,$14
        .byte   $30,$0F,$11,$0F,$38,$0F,$0F,$17
        .byte   $30,$00,$00,$00,$00,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36,$0F,$23,$33
        .byte   $30,$0F,$13,$23,$30,$0F,$37,$17
        .byte   $30,$0F,$11,$0F,$38,$0F,$17,$27
        .byte   $36,$0F,$06,$15,$36,$0F,$0F,$10
        .byte   $30,$0F,$0F,$26,$36
; ----------------------------------------------------------------------------
click_at_cursor:  jsr     find_screen_area
        lda     screen_area
        cmp     #$03
        bcc     L8D44
        rts

; ----------------------------------------------------------------------------
L8D44:  jsr     jump_table
        .addr   click_picture
        .addr   click_command_area
        .addr   click_inventory_area
; ----------------------------------------------------------------------------
find_screen_area:  ldy     #$00
        lda     #$79
        sta     hotspot_ptr
        lda     #$8D
        sta     hotspot_ptr+1
L8D57:  jsr     test_hotspot
        lda     script_flags
        and     #$01
        bne     L8D68
        jsr     next_screen_rect
        iny
        cpy     #$03
        bcc     L8D57
L8D68:  sty     screen_area
        rts

; ----------------------------------------------------------------------------
next_screen_rect:  lda     hotspot_ptr
        clc
        adc     #$04
        sta     hotspot_ptr
        lda     hotspot_ptr+1
        adc     #$00
        sta     hotspot_ptr+1
        rts

; ----------------------------------------------------------------------------
        .byte   $10,$7F,$20,$8F,$10,$EF,$A0,$E4
        .byte   $90,$EF,$10,$94
; ----------------------------------------------------------------------------
click_command_area:  ldy     #$00
        lda     #$2D
        sta     hotspot_ptr
        lda     #$8E
        sta     hotspot_ptr+1
L8D8F:  jsr     test_hotspot
        lda     script_flags
        and     #$01
        bne     L8DA1
        jsr     next_screen_rect
        iny
        cpy     #$0E
        bcc     L8D8F
        rts

; ----------------------------------------------------------------------------
L8DA1:  lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L8DBD
        cpy     #$08
        beq     L8DBD
        cpy     #$0B
        beq     L8DBD
        cpy     #$0C
        beq     L8DBD
        cpy     #$0D
        beq     L8DBD
        rts

; ----------------------------------------------------------------------------
L8DBD:  cpy     #$08
        beq     click_self
        cpy     #$0D
        bne     L8DCF
        lda     command
        cmp     #$FF
        bne     L8DCC
        rts

; ----------------------------------------------------------------------------
L8DCC:  jmp     special_command_by_room

; ----------------------------------------------------------------------------
L8DCF:  lda     command
        sta     $B1
        cmp     #$FF
        beq     L8DDE
        cmp     #$0D
        bcs     L8DDE
        jsr     unhighlight_command
L8DDE:  sty     command
        cpy     #$0D
        bcs     L8DE7
        jsr     highlight_command
L8DE7:  cpy     #$08
        beq     click_self
        cpy     #$09
        beq     L8E27
        cpy     #$0B
        beq     L8DF8
        cpy     #$0C
        beq     L8DF8
        rts

; ----------------------------------------------------------------------------
L8DF8:  jsr     clear_selection_unless_waiting
        jsr     unhighlight_command
        lda     $B1
        sta     command
        cmp     #$FF
        beq     L8E09
        jsr     highlight_command
L8E09:  rts

; ----------------------------------------------------------------------------
click_self:  lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     L8E1E
        lda     #$00
        sta     script_table
        lda     #$73
        sta     object_id
        rts

; ----------------------------------------------------------------------------
L8E1E:  lda     #$00
        sta     switch_key
        lda     #$73
        sta     switch_key+1
        rts

; ----------------------------------------------------------------------------
L8E27:  lda     #$24
        jsr     far_call_8400
        rts

; ----------------------------------------------------------------------------
        .byte   $10,$4E,$A0,$B3,$10,$4E,$B4,$C3
        .byte   $10,$4E,$C4,$D3,$4E,$70,$A0,$B3
        .byte   $10,$4E,$D4,$E5,$4E,$70,$B4,$C3
        .byte   $4E,$70,$C4,$D3,$4E,$70,$D4,$E5
        .byte   $BB,$F0,$C4,$D3,$BB,$F0,$D4,$E5
        .byte   $86,$BB,$A0,$B3,$BB,$D3,$A0,$C3
        .byte   $D4,$F0,$A0,$C3,$86,$BB,$B4,$E5
; ----------------------------------------------------------------------------
click_inventory_area:  lda     command
        cmp     #$FF
        bne     L8E6C
        rts

; ----------------------------------------------------------------------------
L8E6C:  lda     inventory_page
        bne     L8E72
        rts

; ----------------------------------------------------------------------------
L8E72:  ldy     #$00
        lda     #$C4
        sta     hotspot_ptr
        lda     #$8E
        sta     hotspot_ptr+1
L8E7C:  jsr     test_hotspot
        lda     script_flags
        and     #$01
        bne     L8E8E
        jsr     next_screen_rect
        iny
        cpy     #$07
        bcc     L8E7C
        rts

; ----------------------------------------------------------------------------
L8E8E:  sty     inventory_entry
        jsr     check_inventory_slot_empty
        lda     script_flags
        and     #$10
        bne     L8EC3
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L8EB4
        lda     inventory_page
        sta     $04B2
        lda     inventory_entry
        sta     $04B3
        jmp     L8EC0

; ----------------------------------------------------------------------------
L8EB4:  lda     inventory_page
        sta     $04B0
        lda     inventory_entry
        sta     $04B1
L8EC0:  jsr     highlight_entry
L8EC3:  rts

; ----------------------------------------------------------------------------
        .byte   $90,$EF,$20,$2F,$90,$EF,$30,$3F
        .byte   $90,$EF,$40,$4F,$90,$EF,$50,$5F
        .byte   $90,$EF,$60,$6F,$90,$EF,$70,$7F
        .byte   $90,$EF,$80,$8F
; ----------------------------------------------------------------------------
special_command_room_1E:  lda     #$06
        sta     special_scene
        jsr     show_special_scene
        lda     #$01
        sta     text_keep_open
        lda     #$E4
        jsr     print_message_4
        jsr     op_63
        lda     #$E5
        jsr     print_message_4
        lda     #$E7
        jsr     print_message_4
        rts

; ----------------------------------------------------------------------------
special_command_by_room:  lda     room
        cmp     #$1E
        beq     special_command_room_1E
        cmp     #$20
        beq     special_command_room_20
        cmp     #$24
        beq     special_command_room_24
        cmp     #$4A
        beq     special_command_rooms_4A
        cmp     #$4B
        beq     special_command_rooms_4A
        cmp     #$4C
        beq     special_command_rooms_4A
        cmp     #$4D
        beq     special_command_rooms_4A
        jmp     special_command_other_rooms

; ----------------------------------------------------------------------------
special_command_room_20:  lda     #$07
        sta     special_scene
        jsr     show_special_scene
        lda     #$16
        sta     special_scene
        jsr     show_special_scene
        lda     #$E6
        jsr     print_message_2_or_5
        lda     #$15
        sta     special_scene
        jsr     show_special_scene
        lda     #$62
        jsr     print_message_2_or_5
        lda     script_flags
        ora     #$80
        sta     script_flags
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
special_command_room_24:  lda     #$37
        jsr     print_message_4
        lda     #$15
        sta     special_scene
        jsr     show_special_scene
        lda     #$62
        jsr     print_message_2_or_5
        lda     script_flags
        ora     #$80
        sta     script_flags
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
special_command_rooms_4A:  lda     #$07
        sta     special_scene
        jsr     show_special_scene
        lda     #$F0
        jsr     print_message_4
        lda     #$15
        sta     special_scene
        jsr     show_special_scene
        lda     #$62
        jsr     print_message_2_or_5
        lda     script_flags
        ora     #$80
        sta     script_flags
        lda     #$07
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L8F93
        lda     #$95
        jsr     set_event_flag
L8F93:  jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
special_command_other_rooms:  lda     #$1C
        sta     hotspot_ptr
        lda     #$90
        sta     hotspot_ptr+1
        jsr     test_hotspot
        lda     script_flags
        and     #$01
        bne     L8FA9
        rts

; ----------------------------------------------------------------------------
L8FA9:  jsr     room_scene_list
        lda     cursor_y
        sec
        sbc     #$B8
        lsr     a
        lsr     a
        lsr     a
        sta     $60
        lda     cursor_x
        sec
        sbc     #$88
        asl     a
        and     #$F0
        ora     $60
        sta     $60
L8FC2:  ldy     #$00
        lda     ($5E),y
        cmp     #$FF
        beq     L8FEF
        jsr     tile_patch_hidden_alt
        lda     script_flags
        and     #$20
        bne     L8FFC
        lda     ($5E),y
        cmp     $60
        bne     L8FFC
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     L8FF0
        iny
        lda     ($5E),y
        and     #$7F
        sta     object_id
        lda     #$05
        sta     script_table
L8FEF:  rts

; ----------------------------------------------------------------------------
L8FF0:  iny
        lda     ($5E),y
        and     #$7F
        sta     switch_key+1
        lda     #$05
        sta     switch_key
        rts

; ----------------------------------------------------------------------------
L8FFC:  lda     $5E
        clc
        adc     #$02
        sta     $5E
        lda     $5F
        adc     #$00
        sta     $5F
        jmp     L8FC2

; ----------------------------------------------------------------------------
room_scene_list:  lda     room
        asl     a
        tay
        lda     L9023,y
        sta     $5E
        lda     L9024,y
        sta     $5F
        rts

; ----------------------------------------------------------------------------
        .byte   $88,$B0,$B8,$DF,$01,$01,$6D
L9023:  .byte   $BF
L9024:  .byte   $90,$C2,$90,$C7,$90,$CE,$90,$D3
        .byte   $90,$D8,$90,$E1,$90,$E6,$90,$ED
        .byte   $90,$F2,$90,$F7,$90,$FA,$90,$FF
        .byte   $90,$06,$91,$0B,$91,$10,$91,$15
        .byte   $91,$1C,$91,$2D,$91,$32,$91,$39
        .byte   $91,$40,$91,$45,$91,$4C,$91,$53
        .byte   $91,$54,$91,$5D,$91,$66,$91,$6F
        .byte   $91,$78,$91,$7D,$91,$80,$91,$85
        .byte   $91,$8A,$91,$8F,$91,$94,$91,$99
        .byte   $91,$9E,$91,$A1,$91,$A4,$91,$A7
        .byte   $91,$AC,$91,$AF,$91,$B2,$91,$B5
        .byte   $91,$BC,$91,$C1,$91,$C4,$91,$C9
        .byte   $91,$CE,$91,$D3,$91,$DC,$91,$DD
        .byte   $91,$E0,$91,$E7,$91,$EC,$91,$F1
        .byte   $91,$F4,$91,$FD,$91,$04,$92,$09
        .byte   $92,$0C,$92,$11,$92,$12,$92,$15
        .byte   $92,$18,$92,$1B,$92,$1E,$92,$21
        .byte   $92,$24,$92,$27,$92,$2A,$92,$2D
        .byte   $92,$30,$92,$33,$92,$44,$92,$49
        .byte   $92,$50,$92,$21,$00,$FF,$31,$01
        .byte   $24,$00,$FF,$02,$02,$40,$03,$24
        .byte   $01,$FF,$41,$02,$03,$04,$FF,$88
        .byte   $4B,$24,$04,$FF,$10,$05,$40,$06
        .byte   $00,$07,$24,$03,$FF,$00,$88,$24
        .byte   $05,$FF,$42,$08,$02,$09,$24,$0A
        .byte   $FF,$40,$89,$24,$0B,$FF,$11,$0C
        .byte   $24,$0B,$FF,$10,$8C,$FF,$00,$0F
        .byte   $41,$10,$FF,$20,$12,$88,$4A,$24
        .byte   $14,$FF,$02,$14,$24,$15,$FF,$20
        .byte   $15,$24,$07,$FF,$40,$10,$24,$11
        .byte   $FF,$42,$12,$20,$11,$24,$13,$FF
        .byte   $10,$06,$24,$18,$88,$47,$32,$1B
        .byte   $00,$16,$03,$17,$43,$19,$12,$1A
        .byte   $FF,$44,$17,$04,$1C,$FF,$31,$1E
        .byte   $44,$1C,$04,$1D,$FF,$22,$23,$44
        .byte   $1D,$04,$1F,$FF,$22,$28,$44,$1F
        .byte   $FF,$33,$22,$30,$16,$00,$13,$FF
        .byte   $20,$20,$04,$19,$44,$21,$FF,$FF
        .byte   $88,$0D,$11,$2D,$04,$24,$44,$29
        .byte   $FF,$88,$0D,$00,$31,$04,$25,$44
        .byte   $2A,$FF,$88,$0D,$31,$33,$13,$26
        .byte   $44,$2B,$FF,$88,$0D,$20,$39,$04
        .byte   $27,$44,$2C,$FF,$20,$18,$24,$41
        .byte   $FF,$24,$16,$FF,$44,$1C,$04,$1D
        .byte   $FF,$04,$17,$44,$19,$FF,$04,$17
        .byte   $44,$19,$FF,$88,$47,$24,$1A,$FF
        .byte   $88,$48,$42,$1B,$FF,$88,$4D,$22
        .byte   $43,$FF,$24,$1E,$FF,$43,$23,$FF
        .byte   $43,$28,$FF,$11,$2F,$24,$2D,$FF
        .byte   $12,$2F,$FF,$24,$30,$FF,$24,$31
        .byte   $FF,$11,$35,$41,$37,$24,$33,$FF
        .byte   $20,$36,$24,$37,$FF,$24,$36,$FF
        .byte   $20,$38,$24,$35,$FF,$88,$3A,$24
        .byte   $38,$FF,$24,$39,$88,$49,$FF,$88
        .byte   $3B,$31,$3C,$11,$3D,$24,$49,$FF
        .byte   $FF,$24,$3C,$FF,$30,$3E,$01,$3F
        .byte   $24,$3D,$FF,$01,$40,$24,$3E,$FF
        .byte   $41,$40,$24,$3F,$FF,$22,$45,$FF
        .byte   $20,$42,$02,$43,$42,$44,$24,$46
        .byte   $FF,$20,$0A,$42,$44,$02,$45,$FF
        .byte   $00,$41,$22,$43,$FF,$20,$46,$FF
        .byte   $20,$22,$24,$42,$FF,$FF,$10,$0E
        .byte   $FF,$10,$0F,$FF,$12,$30,$FF,$43
        .byte   $24,$FF,$43,$29,$FF,$43,$25,$FF
        .byte   $43,$2A,$FF,$43,$26,$FF,$43,$2B
        .byte   $FF,$43,$27,$FF,$43,$2C,$FF,$10
        .byte   $06,$24,$18,$88,$47,$32,$1B,$00
        .byte   $16,$03,$17,$43,$19,$12,$1A,$FF
        .byte   $44,$17,$04,$1C,$FF,$31,$1E,$44
        .byte   $1C,$04,$1D,$FF,$22,$23,$44,$1D
        .byte   $04,$1F,$FF
; ----------------------------------------------------------------------------
clear_selection_unless_waiting:  tya
        pha
        jsr     cursor_in_rects_933D
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     L926D
        lda     #$FF
        sta     script_table
        sta     object_id
L926D:  lda     #$FF
        sta     switch_key
        sta     switch_key+1
        pla
        tay
        rts

; ----------------------------------------------------------------------------
cursor_in_rects_933D:  ldy     #$00
        lda     #$3D
        sta     hotspot_ptr
        lda     #$93
        sta     hotspot_ptr+1
L9280:  jsr     test_hotspot
        jsr     next_screen_rect
        lda     script_flags
        and     #$01
        bne     L9292
        iny
        cpy     #$02
        bcc     L9280
        rts

; ----------------------------------------------------------------------------
L9292:  tya
        jsr     jump_table
        .addr   far_8040_64
        .addr   L929A
; ----------------------------------------------------------------------------
L929A:  lda     inventory_page
        cmp     #$0B
        bcs     L92BA
        cmp     #$02
        bcs     L92AF
        lda     open_containers
        cmp     #$FF
        bne     L92DD
        inc     inventory_page
L92AF:  jsr     far_8020_67
on_page_04B0:  lda     $04B0
        cmp     inventory_page
        beq     L92BB
L92BA:  rts

; ----------------------------------------------------------------------------
L92BB:  lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     L92C7
        rts

; ----------------------------------------------------------------------------
L92C7:  lda     $04B1
        sta     inventory_entry
        lda     #$65
        jsr     far_call_8040
        rts

; ----------------------------------------------------------------------------
L92D3:  dec     current_container
        jsr     far_8020_67
        jsr     on_page_04B0
        rts

; ----------------------------------------------------------------------------
L92DD:  inc     current_container
        lda     open_containers
        cmp     current_container
        bcc     L92D3
        jsr     select_item_list
        jsr     load_container_view
        lda     #$00
        sta     inventory_page
        lda     #$67
        jsr     far_call_8020
        jsr     on_page_04B0
        rts

; ----------------------------------------------------------------------------
far_8040_64:
        lda     inventory_page
        beq     L92BA
        cmp     #$03
        bcs     L9316
        lda     open_containers
        cmp     #$FF
        bne     L931D
        dec     inventory_page
L930F:  jsr     far_8000_64
        jsr     on_page_04B0
        rts

; ----------------------------------------------------------------------------
L9316:  jsr     far_8020_6D
        jsr     on_page_04B0
        rts

; ----------------------------------------------------------------------------
L931D:  lda     inventory_page
        cmp     #$02
        beq     L9329
        dec     current_container
        bmi     L930F
L9329:  jsr     select_item_list
        jsr     load_container_view
        lda     #$02
        sta     inventory_page
        lda     #$6D
        jsr     far_call_8020
        jsr     on_page_04B0
        rts

; ----------------------------------------------------------------------------
        .byte   $BB,$D3,$A0,$C3,$D4,$F0,$A0,$C3
; ----------------------------------------------------------------------------
select_room_scene:
        txa
        pha
        tya
        pha
        jsr     pick_room_variant
        lda     room
        asl     a
        tay
        lda     L939B,y
        sta     $5E
        lda     L939C,y
        sta     $5F
        ldy     $66
        lda     ($5E),y
        sta     scene
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
pick_room_variant:  jsr     room_scene_list
        ldy     #$00
        sty     $66
        ldx     room
        lda     L9522,x
        sta     $67
        beq     L939A
L9378:  lda     ($5E),y
        iny
        lda     ($5E),y
        and     #$7F
        jsr     test_door_flag
        dey
        lda     script_flags
        and     #$04
        beq     L9394
        tya
        lsr     a
        tax
        lda     $66
        clc
        adc     $C45A,x
        sta     $66
L9394:  iny
        iny
        cpy     $67
        bcc     L9378
L939A:  rts

; ----------------------------------------------------------------------------
L939B:  .byte   $37
L939C:  .byte   $94,$3A,$94,$3D,$94,$42,$94,$47
        .byte   $94,$4A,$94,$4F,$94,$52,$94,$57
        .byte   $94,$5A,$94,$5D,$94,$60,$94,$65
        .byte   $94,$6A,$94,$6D,$94,$70,$94,$73
        .byte   $94,$76,$94,$87,$94,$89,$94,$8C
        .byte   $94,$8E,$94,$90,$94,$93,$94,$96
        .byte   $94,$98,$94,$9D,$94,$A2,$94,$A7
        .byte   $94,$AA,$94,$AD,$94,$AF,$94,$B1
        .byte   $94,$B3,$94,$B5,$94,$B8,$94,$BB
        .byte   $94,$BE,$94,$C0,$94,$C2,$94,$C4
        .byte   $94,$C7,$94,$CA,$94,$CC,$94,$CE
        .byte   $94,$D0,$94,$D3,$94,$D5,$94,$D8
        .byte   $94,$DB,$94,$DD,$94,$E2,$94,$E5
        .byte   $94,$E7,$94,$EC,$94,$EF,$94,$F2
        .byte   $94,$F4,$94,$F6,$94,$F8,$94,$FA
        .byte   $94,$FC,$94,$FF,$94,$01,$95,$04
        .byte   $95,$07,$95,$0A,$95,$0C,$95,$0E
        .byte   $95,$10,$95,$12,$95,$14,$95,$16
        .byte   $95,$18,$95,$1A,$95,$1C,$95,$1E
        .byte   $95,$20,$95,$00,$01,$FF,$02,$03
        .byte   $FF,$04,$06,$05,$07,$FF,$08,$0A
        .byte   $09,$0B,$FF,$0C,$0D,$FF,$0E,$0F
        .byte   $10,$11,$FF,$12,$13,$FF,$16,$14
        .byte   $17,$15,$FF,$19,$18,$FF,$1A,$1B
        .byte   $FF,$1D,$1C,$FF,$20,$21,$22,$23
        .byte   $FF,$28,$29,$2A,$2B,$FF,$2C,$2D
        .byte   $FF,$2E,$2F,$FF,$24,$25,$FF,$26
        .byte   $27,$FF,$30,$33,$34,$3A,$32,$38
        .byte   $39,$3E,$31,$36,$37,$3D,$35,$3B
        .byte   $3C,$3F,$FF,$45,$FF,$47,$48,$FF
        .byte   $4B,$FF,$4D,$FF,$51,$52,$FF,$4E
        .byte   $4F,$FF,$50,$FF,$5C,$5E,$5D,$5F
        .byte   $FF,$66,$68,$67,$69,$FF,$6B,$6D
        .byte   $6C,$6E,$FF,$77,$78,$FF,$8B,$8C
        .byte   $FF,$54,$FF,$53,$FF,$55,$FF,$56
        .byte   $FF,$43,$44,$FF,$41,$42,$FF,$8D
        .byte   $8E,$FF,$4A,$FF,$58,$FF,$5A,$FF
        .byte   $60,$61,$FF,$62,$63,$FF,$65,$FF
        .byte   $6A,$FF,$6F,$FF,$70,$71,$FF,$72
        .byte   $FF,$73,$74,$FF,$75,$76,$FF,$79
        .byte   $FF,$7B,$7A,$7C,$7B,$FF,$8F,$98
        .byte   $FF,$7D,$FF,$7E,$7F,$80,$81,$FF
        .byte   $82,$83,$FF,$84,$85,$FF,$86,$FF
        .byte   $87,$FF,$88,$FF,$89,$FF,$8A,$FF
        .byte   $8B,$8C,$FF,$57,$FF,$1D,$1F,$FF
        .byte   $1D,$1E,$FF,$62,$64,$FF,$58,$FF
        .byte   $5A,$FF,$58,$FF,$5A,$FF,$58,$FF
        .byte   $5A,$FF,$58,$FF,$5A,$FF,$40,$FF
        .byte   $46,$FF,$49,$FF,$4C,$FF
L9522:  .byte   $02,$02,$04,$04,$02,$04,$02,$04
        .byte   $02,$02,$02,$04,$04,$02,$02,$02
        .byte   $02,$08,$00,$02,$00,$00,$02,$02
        .byte   $00,$04,$04,$04,$02,$02,$00,$00
        .byte   $00,$00,$02,$02,$02,$00,$00,$00
        .byte   $02,$02,$00,$00,$00,$02,$00,$02
        .byte   $02,$00,$04,$00,$00,$04,$02,$02
        .byte   $00,$00,$00,$00,$00,$02,$00,$02
        .byte   $02,$02,$00,$00,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$00,$00
; ----------------------------------------------------------------------------
reset_cursor:
        lda     #$1A
        sta     cursor_x
        lda     #$AA
        sta     cursor_y
        lda     #$01
        sta     cursor_mode
        lda     #$07
        sta     command_cursor
        lda     #$00
        sta     key_repeat
        rts

; ----------------------------------------------------------------------------
handle_input:
        tya
        pha
        jsr     get_input
        pla
        tay
        rts

; ----------------------------------------------------------------------------
get_input:  jsr     read_input
        lda     data_ptr
        beq     get_input
        and     #$F0
        beq     move_cursor
        jmp     handle_buttons

; ----------------------------------------------------------------------------
move_cursor:  lda     cursor_mode
        and     #$0F
        jsr     jump_table
        .addr   move_cursor_in_scene
        .addr   move_cursor_in_commands
        .addr   move_cursor_in_inventory
; ----------------------------------------------------------------------------
read_input:  lda     #$00
        sta     data_ptr
        jsr     read_joypads
        lda     cursor_mode
        and     #$0F
        jsr     jump_table
        .addr   read_input_free
        .addr   read_input_with_repeat
        .addr   read_input_free
; ----------------------------------------------------------------------------
read_input_free:  lda     joy_held
        and     #$0F
        sta     data_ptr
        lda     joy_pressed
        and     #$F0
        ora     data_ptr
        sta     data_ptr
        jsr     dpad_to_delta
        jsr     reset_key_repeat
        rts

; ----------------------------------------------------------------------------
read_input_with_repeat:  lda     #$00
        sta     data_ptr+1
        sta     $57
        lda     joy_pressed
        and     #$F0
        sta     data_ptr
        lda     joy_held
        and     #$0F
        asl     a
        asl     a
        asl     a
        asl     a
        sta     $64
        beq     reset_key_repeat
        lda     cursor_mode
        and     #$F0
        beq     L95FC
        cmp     $64
        bne     L95FC
        inc     key_repeat
        lda     key_repeat
        and     #$7F
        beq     L95FC
        rts

; ----------------------------------------------------------------------------
L95FC:  lda     joy_held
        and     #$0F
        ora     data_ptr
        sta     data_ptr
        lda     cursor_mode
        and     #$0F
        ora     $64
        sta     cursor_mode
        jmp     dpad_to_delta

; ----------------------------------------------------------------------------
reset_key_repeat:  lda     #$00
        sta     key_repeat
        lda     cursor_mode
        and     #$0F
        sta     cursor_mode
        rts

; ----------------------------------------------------------------------------
dpad_to_delta:  lda     data_ptr
        and     #$0F
        tay
        lda     dpad_dx,y
        sta     data_ptr+1
        lda     dpad_dy,y
        sta     $57
        rts

; ----------------------------------------------------------------------------
dpad_dx:  .byte   $00,$01,$FF,$00,$00,$01,$FF,$00
        .byte   $00,$01,$FF,$00,$00,$00,$00,$00
dpad_dy:  .byte   $00,$00,$00,$00,$01,$01,$01,$01
        .byte   $FF,$FF,$FF,$FF,$00,$00,$00,$00
; ----------------------------------------------------------------------------
move_cursor_in_scene:  jsr     move_cursor_x
        lda     cursor_x
        bmi     L9663
        cmp     #$10
        bcc     L967D
L9655:  jsr     move_cursor_y
        lda     cursor_y
        cmp     #$90
        bcs     L966F
        cmp     #$20
        bcc     L9684
        rts

; ----------------------------------------------------------------------------
L9663:  lda     cursor_y
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sec
        sbc     #$02
        jmp     L96D3

; ----------------------------------------------------------------------------
L966F:  lda     cursor_x
        cmp     #$50
        bcs     L9679
        lda     #$07
        bne     L96D3
L9679:  lda     #$0B
        bne     L96D3
L967D:  lda     #$10
        sta     cursor_x
        jmp     L9655

; ----------------------------------------------------------------------------
L9684:  lda     cursor_x
        cmp     #$50
        bcs     L968E
        lda     #$0A
        bne     L96D3
L968E:  lda     #$0E
        bne     L96D3
move_cursor_in_inventory:  jsr     move_cursor_x
        lda     cursor_x
        cmp     #$80
        bcc     L96AD
        cmp     #$B8
        bcs     L96BB
        jsr     move_cursor_y
        lda     cursor_y
        cmp     #$B6
        bcc     L96CB
        cmp     #$E8
        bcs     L96CF
        rts

; ----------------------------------------------------------------------------
L96AD:  lda     cursor_y
        sec
        sbc     #$B0
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        clc
        adc     #$0C
        bne     L96D3
L96BB:  lda     cursor_y
        sec
        sbc     #$B0
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        clc
        adc     #$10
        sta     command_cursor
        bne     L96D3
L96CB:  lda     #$0F
        bne     L96D3
L96CF:  lda     #$00
        bne     L96D3
L96D3:  sta     command_cursor
        lda     #$01
        sta     cursor_mode
        lda     joy_held
        asl     a
        asl     a
        asl     a
        asl     a
        ora     cursor_mode
        sta     cursor_mode
        lda     #$00
        sta     key_repeat
snap_cursor_to_command:  ldy     command_cursor
        lda     L96F4,y
        sta     cursor_x
        lda     L9708,y
        sta     cursor_y
        rts

; ----------------------------------------------------------------------------
L96F4:  .byte   $9A,$9A,$9A,$9A,$9A,$9A,$9A,$1A
        .byte   $1A,$1A,$1A,$52,$52,$52,$52,$8A
        .byte   $C2,$C2,$C2,$E2
L9708:  .byte   $2A,$3A,$4A,$5A,$6A,$7A,$8A,$AA
        .byte   $BA,$CA,$DA,$AA,$BA,$CA,$DA,$AA
        .byte   $AA,$CA,$DA,$AA
; ----------------------------------------------------------------------------
move_cursor_x:  lda     cursor_x
        clc
        adc     data_ptr+1
        sta     cursor_x
        rts

; ----------------------------------------------------------------------------
move_cursor_y:  lda     cursor_y
        clc
        adc     $57
        sta     cursor_y
        rts

; ----------------------------------------------------------------------------
move_cursor_in_commands:  jsr     command_grid_position
        jsr     move_in_command_grid
        jsr     command_at_grid
        lda     $58
        bmi     L9743
        cmp     #$20
        beq     L9747
        cmp     #$30
        beq     L9778
        sta     command_cursor
L9743:  jsr     snap_cursor_to_command
        rts

; ----------------------------------------------------------------------------
L9747:  lda     cursor_mode
        and     #$F0
        sta     cursor_mode
        ldy     command_cursor
        lda     command_cursor_x,y
        sta     cursor_x
        lda     command_cursor_y,y
        sta     cursor_y
        rts

; ----------------------------------------------------------------------------
command_cursor_x:  .byte   $78,$78,$78,$78,$78,$78,$78,$48
        .byte   $00,$00,$48,$48,$00,$00,$48
command_cursor_y:  .byte   $58,$58,$58,$58,$58,$58,$58,$88
        .byte   $00,$00,$28,$88,$00,$00,$28
; ----------------------------------------------------------------------------
L9778:  lda     cursor_mode
        and     #$F0
        ora     #$02
        sta     cursor_mode
        lda     #$98
        sta     cursor_x
        lda     #$C8
        sta     cursor_y
        rts

; ----------------------------------------------------------------------------
command_grid_position:  ldy     command_cursor
        lda     L9796,y
        sta     $59
        lda     L97AA,y
        sta     $5A
        rts

; ----------------------------------------------------------------------------
L9796:  .byte   $03,$03,$03,$03,$03,$03,$03,$01
        .byte   $01,$01,$01,$02,$02,$02,$02,$03
        .byte   $04,$04,$04,$05
L97AA:  .byte   $01,$02,$03,$04,$05,$06,$07,$08
        .byte   $09,$0A,$0B,$08,$09,$0A,$0B,$08
        .byte   $08,$09,$0A,$08
; ----------------------------------------------------------------------------
move_in_command_grid:  lda     $59
        clc
        adc     data_ptr+1
        sta     $59
        lda     $5A
        clc
        adc     $57
        sta     $5A
        rts

; ----------------------------------------------------------------------------
command_at_grid:  lda     $5A
        asl     a
        sta     $5B
        asl     a
        clc
        adc     $5A
        clc
        adc     $5B
        clc
        adc     $59
        tay
        lda     L97E3,y
        sta     $58
        rts

; ----------------------------------------------------------------------------
L97E3:  .byte   $FF,$FF,$FF,$30,$FF,$FF,$FF,$FF
        .byte   $FF,$20,$00,$FF,$FF,$FF,$FF,$FF
        .byte   $20,$01,$FF,$FF,$FF,$FF,$FF,$20
        .byte   $02,$FF,$FF,$FF,$FF,$FF,$20,$03
        .byte   $FF,$FF,$FF,$FF,$FF,$20,$04,$FF
        .byte   $FF,$FF,$FF,$FF,$20,$05,$FF,$FF
        .byte   $FF,$FF,$20,$20,$06,$06,$06,$FF
        .byte   $FF,$07,$0B,$0F,$10,$13,$FF,$FF
        .byte   $08,$0C,$30,$11,$11,$FF,$FF,$09
        .byte   $0D,$30,$12,$FF,$FF,$FF,$0A,$0E
        .byte   $30,$00,$FF,$FF,$FF,$20,$20,$FF
        .byte   $FF,$FF,$FF
; ----------------------------------------------------------------------------
handle_buttons:  lda     data_ptr
        asl     a
        bcs     press_a
        asl     a
        bcs     press_b
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_9847:
        asl     a
        bcs     L9891
        jsr     print_04B4_message
        inc     $04B4
        beq     L985F
        lda     $04B4
        bne     L985E
        lda     $04B5
        cmp     #$80
        beq     L985F
L985E:  rts

; ----------------------------------------------------------------------------
L985F:  inc     $04B5
        lda     $04B5
        cmp     #$03
        bcs     L986A
        rts

; ----------------------------------------------------------------------------
L986A:  lda     #$00
        sta     $04B5
        rts

; ----------------------------------------------------------------------------
print_04B4_message:  lda     $04B5
        jsr     jump_table
        .addr   L987C
        .addr   L9883
        .addr   L988A
; ----------------------------------------------------------------------------
L987C:  lda     $04B4
        jsr     print_message_5
        rts

; ----------------------------------------------------------------------------
L9883:  lda     $04B4
        jsr     print_message_4
        rts

; ----------------------------------------------------------------------------
L988A:  lda     $04B4
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
L9891:  jsr     prompt_message_2B
        rts

; ----------------------------------------------------------------------------
press_b:  lda     #$80
        jsr     clear_event_flag
        lda     command
        cmp     #$0D
        bcs     L98A3
        jsr     unhighlight_command
L98A3:  lda     #$FF
        sta     command
L98A7:  jsr     clear_entry_highlight
L98AA:  lda     #$FF
        sta     script_table
        sta     object_id
L98B0:  lda     #$FF
        sta     switch_key
        sta     switch_key+1
        rts

; ----------------------------------------------------------------------------
press_a:  lda     #$19
        jsr     play_sound
        jsr     next_random
        jsr     click_at_cursor
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L98EB
        lda     switch_key
        cmp     #$FF
        beq     L9921
        jsr     script_resume
        lda     $04B1
        sta     inventory_entry
        jsr     clear_entry_highlight
        lda     $04B3
        sta     inventory_entry
        jsr     clear_entry_highlight
        jmp     L98AA

; ----------------------------------------------------------------------------
L98EB:  lda     command
        cmp     #$FF
        beq     L9921
        lda     object_id
        cmp     #$FF
        beq     L9921
        jsr     run_command
        lda     #$BF
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L990E
        lda     #$FF
        sta     command
        lda     #$BF
        jsr     clear_event_flag
L990E:  lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     L98A7
        lda     inventory_entry
        sta     $B2
        jmp     L98B0

; ----------------------------------------------------------------------------
L9921:  rts

; ----------------------------------------------------------------------------
draw_cursor:
        lda     cursor_x
        sec
        sbc     #$05
        sta     $20
        lda     cursor_y
        sta     $21
        lda     #$03
        sta     $22
        lda     #$3D
        sta     $1E
        lda     #$99
        sta     $1F
        jsr     draw_metasprite
        rts

; ----------------------------------------------------------------------------
        .byte   $22,$7A,$01,$7B,$01,$7C,$01,$7D
        .byte   $01,$FF
; ----------------------------------------------------------------------------
far_8000_66:
        tya
        pha
        lda     address_book
        cmp     #$FF
        beq     L9974
        lda     #$00
        sta     command_cursor
L9954:  lda     #$00
        sta     data_ptr
        jsr     read_joypads
        jsr     read_input_with_repeat
        lda     data_ptr
        sta     $83
        jsr     step_address_choice
        jsr     redraw_cursor_and_sprites
        jsr     wait_vblank_and_hide_sprites
        lda     $83
        bmi     L997D
        asl     a
        bmi     L9979
        bpl     L9954
L9974:  lda     #$69
        jsr     print_message_2_or_5
L9979:  lda     #$FF
        bmi     L998C
L997D:  lda     #$19
        jsr     play_sound
        lda     command_cursor
        sta     inventory_entry
        jsr     highlight_entry
        lda     command_cursor
L998C:  sta     $84
        pla
        tay
        jsr     reset_cursor
        rts

; ----------------------------------------------------------------------------
step_address_choice:  lda     command_cursor
        clc
        adc     $57
        sta     command_cursor
        bmi     L99AA
        cmp     #$07
        beq     L99B1
        tay
        lda     address_book,y
        cmp     #$FF
        beq     step_address_choice
        rts

; ----------------------------------------------------------------------------
L99AA:  lda     #$07
        sta     command_cursor
        jmp     step_address_choice

; ----------------------------------------------------------------------------
L99B1:  lda     #$FF
        sta     command_cursor
        jmp     step_address_choice

; ----------------------------------------------------------------------------
redraw_cursor_and_sprites:  jsr     snap_cursor_to_command
        jsr     draw_cursor
        jsr     draw_object_sprites
        rts

; ----------------------------------------------------------------------------
prompt_message_2B:  txa
        pha
        tya
        pha
        lda     #$01
        sta     text_keep_open
        lda     #$2B
        jsr     print_message_2_or_5
        jsr     wait_choice_input
L99D3:  jsr     read_joypads
        lda     joy_held
        bne     L99D3
        jsr     op_45
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
wait_choice_input:  jsr     draw_choice_marker
L99E5:  jsr     read_joypads
        lda     joy_pressed
        lsr     a
        lsr     a
        lsr     a
        bcs     L9A05
        lsr     a
        bcs     L9A13
        lsr     a
        bcs     L9A01
        lsr     a
        bcs     L9A01
        lsr     a
        bcs     L9A01
        lsr     a
        bcs     L9A01
        jmp     L99E5

; ----------------------------------------------------------------------------
L9A01:  jsr     play_click
        rts

; ----------------------------------------------------------------------------
L9A05:  lda     text_delay
        cmp     #$03
        beq     L9A10
        inc     text_delay
        jsr     play_click
L9A10:  jmp     wait_choice_input

; ----------------------------------------------------------------------------
L9A13:  lda     text_delay
        cmp     #$01
        beq     L9A1E
        dec     text_delay
        jsr     play_click
L9A1E:  jmp     wait_choice_input

; ----------------------------------------------------------------------------
play_click:  lda     #$19
        jsr     play_sound
        rts

; ----------------------------------------------------------------------------
draw_choice_marker:  txa
        pha
        lda     #$28
        sta     $20
        ldx     text_delay
        dex
        lda     L9A4D,x
        sta     $21
        lda     #$03
        sta     $22
        lda     #$3D
        sta     $1E
        lda     #$99
        sta     $1F
        jsr     draw_metasprite
        jsr     draw_object_sprites
        jsr     wait_vblank_and_hide_sprites
        pla
        tax
        rts

; ----------------------------------------------------------------------------
L9A4D:  .byte   $BC,$CC,$DC
; ----------------------------------------------------------------------------
dissolve_row_mask:
        lda     dissolve_pattern
        asl     a
        tay
        lda     L9A94,y
        sta     $57
        lda     L9A95,y
        sta     $58
        lda     $78
        asl     a
        asl     a
        sta     $59
        asl     a
        sta     $5A
        asl     a
        clc
        adc     $59
        clc
        adc     $5A
        clc
        adc     $57
        sta     $57
        lda     #$00
        adc     $58
        sta     $58
        lda     $77
        asl     a
        clc
        adc     $57
        sta     $57
        lda     #$00
        adc     $58
        sta     $58
        ldy     #$00
        lda     ($57),y
        sta     $83
        iny
        lda     ($57),y
        sta     $84
        rts

; ----------------------------------------------------------------------------
L9A94:  .byte   $9A
L9A95:  .byte   $9A,$42,$9B,$EA,$9B,$08,$04,$80
        .byte   $08,$22,$20,$20,$84,$02,$10,$12
        .byte   $01,$00,$00,$00,$11,$12,$04,$80
        .byte   $20,$40,$02,$41,$08,$20,$04,$81
        .byte   $40,$41,$50,$04,$20,$08,$05,$41
        .byte   $00,$48,$02,$00,$20,$10,$89,$44
        .byte   $00,$00,$40,$14,$04,$02,$20,$88
        .byte   $00,$80,$00,$08,$08,$80,$20,$10
        .byte   $81,$00,$00,$0C,$20,$20,$80,$20
        .byte   $08,$42,$20,$10,$82,$80,$10,$00
        .byte   $00,$10,$10,$00,$A0,$44,$40,$00
        .byte   $00,$20,$80,$40,$40,$40,$10,$00
        .byte   $11,$00,$04,$44,$46,$00,$44,$00
        .byte   $40,$08,$01,$48,$89,$80,$40,$24
        .byte   $02,$10,$18,$20,$22,$10,$09,$01
        .byte   $06,$90,$48,$00,$0A,$94,$20,$01
        .byte   $00,$24,$00,$81,$04,$21,$08,$01
        .byte   $00,$05,$8D,$00,$10,$03,$A2,$42
        .byte   $91,$06,$02,$2A,$10,$05,$82,$92
        .byte   $40,$01,$49,$88,$90,$89,$12,$2A
        .byte   $28,$44,$A2,$22,$52,$28,$00,$12
        .byte   $45,$08,$01,$14,$04,$82,$20,$41
        .byte   $10,$20,$08,$10,$04,$08,$02,$04
        .byte   $01,$82,$20,$41,$10,$20,$08,$10
        .byte   $04,$08,$02,$04,$01,$82,$20,$41
        .byte   $10,$41,$10,$20,$08,$10,$04,$08
        .byte   $02,$04,$01,$82,$20,$41,$10,$20
        .byte   $08,$10,$04,$08,$02,$04,$01,$82
        .byte   $20,$41,$10,$20,$08,$20,$08,$10
        .byte   $04,$08,$02,$04,$01,$82,$20,$41
        .byte   $10,$20,$08,$10,$04,$08,$02,$04
        .byte   $01,$82,$20,$41,$10,$20,$08,$10
        .byte   $04,$10,$04,$08,$02,$04,$01,$82
        .byte   $20,$41,$10,$20,$08,$10,$04,$08
        .byte   $02,$04,$01,$82,$20,$41,$10,$20
        .byte   $08,$10,$04,$08,$02,$08,$02,$04
        .byte   $01,$82,$20,$41,$10,$20,$08,$10
        .byte   $04,$08,$02,$04,$01,$82,$20,$41
        .byte   $10,$20,$08,$10,$04,$08,$02,$04
        .byte   $01,$04,$01,$82,$20,$41,$10,$20
        .byte   $08,$10,$04,$08,$02,$04,$01,$82
        .byte   $20,$41,$10,$20,$08,$10,$04,$08
        .byte   $02,$04,$01,$82,$20,$80,$00,$80
        .byte   $00,$80,$00,$80,$00,$80,$00,$80
        .byte   $00,$80,$00,$FF,$FF,$80,$00,$80
        .byte   $00,$80,$00,$80,$00,$80,$00,$80
        .byte   $00,$00,$03,$00,$03,$00,$03,$00
        .byte   $03,$00,$01,$0F,$01,$7F,$01,$00
        .byte   $00,$40,$FF,$40,$F0,$40,$00,$60
        .byte   $00,$60,$00,$60,$00,$00,$3C,$00
        .byte   $1C,$01,$0C,$03,$04,$0F,$06,$10
        .byte   $02,$00,$00,$00,$00,$00,$00,$20
        .byte   $0C,$30,$F0,$18,$C0,$1C,$80,$1E
        .byte   $00,$01,$C0,$03,$60,$06,$30,$0C
        .byte   $08,$10,$00,$20,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$02,$08,$0C,$04
        .byte   $38,$02,$70,$01,$E0,$0E,$00,$0C
        .byte   $80,$18,$C0,$10,$F0,$20,$78,$40
        .byte   $0C,$00,$02,$00,$00,$20,$00,$18
        .byte   $01,$06,$02,$03,$04,$01,$0C,$00
        .byte   $18,$70,$00,$70,$00,$60,$00,$60
        .byte   $00,$40,$80,$00,$F0,$00,$FC,$00
        .byte   $00,$1F,$00,$07,$00,$01,$01,$00
        .byte   $03,$00,$03,$00,$07
; ----------------------------------------------------------------------------
far_8000_6F:
        lda     #$A6
        sta     $1E
        lda     #$9C
        sta     $1F
        lda     #$61
        sta     $1C
        lda     #$20
        sta     $1D
        jsr     queue_vram_fill
        rts

; ----------------------------------------------------------------------------
        .byte   $10,$10,$A6
; ----------------------------------------------------------------------------
far_8000_6C:
        lda     #$EF
        sta     $1E
        lda     #$9D
        sta     $1F
        lda     #$82
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_block
        rts

; ----------------------------------------------------------------------------
far_8000_67:
        lda     #$D1
        sta     $1E
        lda     #$9C
        sta     $1F
        lda     #$F1
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_fill
        rts

; ----------------------------------------------------------------------------
        .byte   $05,$05,$A6
; ----------------------------------------------------------------------------
far_8000_6E:
        lda     #$00
        sta     $79
        sta     $0532
        lda     #$51
        sta     $77
        lda     #$20
        sta     $78
        jsr     draw_label_string
        jsr     far_8020_6E
        rts

; ----------------------------------------------------------------------------
far_8020_6D:
        lda     inventory_page
        bne     L9CF0
        rts

; ----------------------------------------------------------------------------
L9CF0:  jsr     draw_object_sprites
        jsr     wait_vblank_and_hide_sprites
        jsr     previous_page
        jsr     far_8020_6E
        jsr     draw_inventory_page
        rts

; ----------------------------------------------------------------------------
far_8000_64:
        lda     inventory_page
        bne     L9D06
        rts

; ----------------------------------------------------------------------------
L9D06:  jsr     previous_page
        jsr     far_8020_6E
        jsr     draw_address_book
        rts

; ----------------------------------------------------------------------------
previous_page:  dec     inventory_page
        lda     #$38
        sta     $5C
        lda     #$9D
        sta     $5D
        lda     #$00
        sta     $79
draw_page_strings:  ldy     #$00
L9D21:  lda     #$20
        sta     $78
        lda     ($5C),y
        sta     $77
        iny
        lda     ($5C),y
        sta     $0532
        iny
        jsr     draw_label_string
        cpy     #$1C
        bcc     L9D21
        rts

; ----------------------------------------------------------------------------
        .byte   $5D,$09,$52,$01,$52,$02,$53,$03
        .byte   $54,$04,$55,$04,$56,$04,$57,$04
        .byte   $58,$04,$59,$04,$5A,$04,$5B,$04
        .byte   $5C,$05,$5D,$06
; ----------------------------------------------------------------------------
far_8020_67:
        lda     inventory_page
        cmp     store_page
        bcc     L9D5D
        rts

; ----------------------------------------------------------------------------
L9D5D:  jsr     draw_object_sprites
        jsr     wait_vblank_and_hide_sprites
        jsr     next_page
        jsr     far_8020_6E
        jsr     draw_inventory_page
        rts

; ----------------------------------------------------------------------------
next_page:  inc     inventory_page
        lda     #$7E
        sta     $5C
        lda     #$9D
        sta     $5D
        sta     $79
        jsr     draw_page_strings
        rts

; ----------------------------------------------------------------------------
        .byte   $5D,$01,$5D,$07,$5D,$08,$5C,$08
        .byte   $5B,$08,$5A,$08,$59,$08,$58,$08
        .byte   $57,$08,$56,$08,$55,$08,$54,$08
        .byte   $52,$09,$5D,$06
; ----------------------------------------------------------------------------
draw_label_string:  tya
        pha
        lda     $0532
        asl     a
        tay
        lda     L9EED,y
        sta     $5A
        lda     L9EEE,y
        sta     $5B
        ldy     #$00
L9DAD:  lda     ($5A),y
        cmp     #$FF
        beq     L9DEC
        asl     a
        sta     $1E
        asl     a
        asl     a
        asl     a
        clc
        adc     $1E
        sta     $1E
        lda     #$2D
        clc
        adc     $1E
        sta     $1E
        lda     #$9F
        adc     #$00
        sta     $1F
        cpy     #$00
        beq     L9DDA
        lda     $79
        beq     L9DD8
        dec     $77
        jmp     L9DDA

; ----------------------------------------------------------------------------
L9DD8:  inc     $77
L9DDA:  lda     $77
        sta     $1C
        lda     $78
        sta     $1D
        jsr     queue_vram_column
        jsr     flush_vram_queue_down
        iny
        jmp     L9DAD

; ----------------------------------------------------------------------------
L9DEC:  pla
        tay
        rts

; ----------------------------------------------------------------------------
        .byte   $1C,$09,$54,$DE,$DE,$DE,$DE,$DE
        .byte   $DE,$5F,$DE,$DE,$DE,$DE,$DE,$DE
        .byte   $5F,$DE,$DE,$DE,$DE,$DE,$55,$54
        .byte   $DE,$DE,$DE,$DE,$DE,$55,$56,$69
        .byte   $B5,$C8,$B1,$BD,$A1,$56,$69,$BF
        .byte   $C0,$B5,$BE,$7E,$56,$69,$BD,$BF
        .byte   $C6,$B5,$57,$56,$69,$50,$51,$52
        .byte   $69,$57,$59,$DE,$DE,$DE,$DE,$DE
        .byte   $DE,$5D,$DE,$DE,$DE,$DE,$DE,$DE
        .byte   $5D,$DE,$DE,$DE,$DE,$DE,$6A,$56
        .byte   $6F,$60,$61,$62,$6E,$57,$56,$69
        .byte   $C3,$C0,$B5,$B1,$BB,$56,$69,$C4
        .byte   $B1,$BB,$B5,$7E,$56,$A6,$A6,$A6
        .byte   $A6,$A6,$57,$56,$7E,$BE,$BF,$C4
        .byte   $B5,$57,$59,$DE,$DE,$DE,$DE,$DE
        .byte   $DE,$5D,$DE,$DE,$DE,$DE,$DE,$DE
        .byte   $5C,$A6,$A6,$A6,$A6,$A6,$57,$59
        .byte   $DE,$DE,$DE,$DE,$DE,$6A,$56,$69
        .byte   $B8,$B9,$C4,$7E,$7E,$56,$69,$BC
        .byte   $B5,$B1,$C6,$B5,$56,$A6,$A6,$A6
        .byte   $A6,$A6,$57,$56,$69,$C3,$B5,$BC
        .byte   $B6,$57,$59,$DE,$DE,$DE,$DE,$DE
        .byte   $DE,$5D,$DE,$DE,$DE,$DE,$DE,$DE
        .byte   $5C,$A6,$A6,$A6,$A6,$A6,$57,$59
        .byte   $DE,$DE,$DE,$DE,$DE,$6A,$56,$69
        .byte   $B3,$BC,$BF,$C3,$B5,$56,$69,$C5
        .byte   $C3,$B5,$7E,$7E,$56,$A6,$A6,$A6
        .byte   $A6,$A6,$57,$56,$69,$C3,$B1,$C6
        .byte   $B5,$57,$64,$66,$66,$66,$66,$66
        .byte   $66,$5E,$66,$66,$66,$66,$66,$66
        .byte   $5E,$66,$66,$66,$66,$66,$65,$64
        .byte   $66,$66,$66,$66,$66,$65
L9EED:  .byte   $01
L9EEE:  .byte   $9F,$11,$9F,$13,$9F,$17,$9F,$1B
        .byte   $9F,$1F,$9F,$22,$9F,$24,$9F,$27
        .byte   $9F,$2B,$9F,$00,$01,$02,$03,$03
        .byte   $03,$03,$03,$03,$03,$03,$03,$04
        .byte   $05,$06,$FF,$07,$FF,$01,$07,$02
        .byte   $FF,$02,$07,$02,$FF,$03,$07,$02
        .byte   $FF,$03,$07,$FF,$04,$FF,$02,$07
        .byte   $FF,$03,$02,$07,$FF,$01,$FF,$11
        .byte   $A2,$A3,$A3,$A3,$A3,$A3,$A3,$A3
        .byte   $A3,$A3,$A3,$A3,$A3,$A3,$A3,$A3
        .byte   $A3,$11,$7E,$7E,$7E,$7E,$7E,$7E
        .byte   $7E,$7E,$7E,$7E,$7E,$7E,$7E,$7E
        .byte   $7E,$7E,$7E,$11,$DE,$7E,$DE,$A6
        .byte   $DE,$A6,$DE,$A6,$DE,$A6,$DE,$A6
        .byte   $DE,$A6,$DE,$A6,$DE,$11,$DE,$7E
        .byte   $DE,$7E,$DE,$7E,$DE,$7E,$DE,$7E
        .byte   $DE,$7E,$DE,$7E,$DE,$7E,$DE,$11
        .byte   $7E,$7E,$DF,$7E,$DF,$7E,$DF,$7E
        .byte   $7E,$7E,$DF,$7E,$DF,$7E,$DF,$7E
        .byte   $7E,$11,$7F,$7F,$FE,$7F,$FE,$7F
        .byte   $FE,$7F,$7F,$7F,$FE,$7F,$FE,$7F
        .byte   $FE,$7F,$7F,$11,$7E,$7E,$7E,$7E
        .byte   $7E,$7E,$7E,$7E,$7E,$7E,$7E,$7E
        .byte   $7E,$7E,$7E,$7E,$7E,$11,$7F,$7F
        .byte   $7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F
        .byte   $7F,$7F,$7F,$7F,$7F,$7F,$7F
; ----------------------------------------------------------------------------
draw_room_tile_patches:
        jsr     room_scene_list
L9FC0:  lda     #$00
        sta     $1D
        ldy     #$00
        lda     ($5E),y
        cmp     #$FF
        bne     L9FCD
        rts

; ----------------------------------------------------------------------------
L9FCD:  bmi     L9FEA
        jsr     tile_patch_hidden
        lda     script_flags
        and     #$20
        bne     L9FEA
        ldy     #$00
        lda     ($5E),y
        jsr     tile_patch_vram_address
        lda     #$95
        sta     $1E
        lda     #$A0
        sta     $1F
        jsr     queue_vram_block
L9FEA:  lda     $5E
        clc
        adc     #$02
        sta     $5E
        lda     $5F
        adc     #$00
        sta     $5F
        jmp     L9FC0

; ----------------------------------------------------------------------------
tile_patch_hidden:  lda     script_flags
        and     #$DF
        sta     script_flags
        ldy     #$01
        lda     ($5E),y
        bmi     LA03D
        jsr     tile_patch_hidden_alt
        rts

; ----------------------------------------------------------------------------
tile_patch_hidden_alt:  tya
        pha
        jsr     tile_patch_room_rules
        pla
        tay
        rts

; ----------------------------------------------------------------------------
tile_patch_room_rules:  lda     script_flags
        and     #$DF
        sta     script_flags
        ldy     #$01
        lda     ($5E),y
        cmp     #$24
        beq     LA049
        cmp     #$25
        beq     LA049
        cmp     #$26
        beq     LA049
        cmp     #$27
        beq     LA049
        cmp     #$29
        beq     LA055
        cmp     #$2A
        beq     LA055
        cmp     #$2B
        beq     LA055
        cmp     #$2C
        beq     LA055
        rts

; ----------------------------------------------------------------------------
LA03D:  and     #$7F
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        beq     LA061
        rts

; ----------------------------------------------------------------------------
LA049:  lda     #$0D
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        bne     LA061
        rts

; ----------------------------------------------------------------------------
LA055:  lda     #$0D
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        beq     LA061
        rts

; ----------------------------------------------------------------------------
LA061:  lda     script_flags
        ora     #$20
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
tile_patch_vram_address:  pha
        and     #$0F
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        rol     $1D
        sta     $1C
        pla
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        clc
        adc     $1C
        sta     $1C
        lda     $1D
        adc     #$00
        sta     $1D
        lda     #$F1
        clc
        adc     $1C
        sta     $1C
        lda     #$22
        adc     $1D
        sta     $1D
        rts

; ----------------------------------------------------------------------------
        .byte   $05,$05,$A6,$01,$01,$6C
; ----------------------------------------------------------------------------
draw_object_sprite:  cmp     #$FF
        beq     LA0D2
        sta     data_ptr
        txa
        pha
        tya
        pha
        lda     data_ptr
        bpl     LA0B5
        and     #$7F
        asl     a
        tax
        lda     LA1D4,x
        sta     $22
        lda     LA1D3,x
        jmp     LA0BF

; ----------------------------------------------------------------------------
LA0B5:  asl     a
        tax
        lda     LA0D4,x
        sta     $22
        lda     LA0D3,x
LA0BF:  asl     a
        tax
        lda     LA171,x
        sta     $1E
        lda     LA172,x
        sta     $1F
        jsr     draw_metasprite
        pla
        tay
        pla
        tax
LA0D2:  rts

; ----------------------------------------------------------------------------
LA0D3:  .byte   $00
LA0D4:  .byte   $00,$01,$00,$02,$00,$03,$00,$04
        .byte   $00,$05,$00,$06,$00,$07,$01,$08
        .byte   $00,$09,$00,$0A,$00,$0B,$00,$0C
        .byte   $00,$0D,$01,$0E,$01,$0F,$00,$10
        .byte   $01,$11,$00,$12,$00,$13,$00,$14
        .byte   $00,$15,$00,$16,$01,$17,$00,$18
        .byte   $01,$19,$00,$1A,$01,$1B,$00,$1B
        .byte   $00,$1B,$00,$1C,$00,$1D,$00,$1E
        .byte   $01,$1E,$01,$1E,$01,$1E,$01,$1E
        .byte   $01,$1E,$01,$1F,$01,$1F,$01,$1F
        .byte   $01,$1F,$01,$1F,$01,$1F,$01,$20
        .byte   $00,$21,$00,$22,$01,$23,$00,$24
        .byte   $00,$25,$00,$26,$00,$27,$00,$28
        .byte   $00,$29,$00,$2A,$00,$2B,$00,$2C
        .byte   $00,$2D,$00,$2E,$00,$2F,$00,$30
        .byte   $00,$31,$00,$32,$00,$33,$01,$34
        .byte   $01,$35,$00,$36,$01,$37,$00,$38
        .byte   $00,$39,$00,$3A,$01,$3B,$00,$3C
        .byte   $00,$3D,$01,$3E,$01,$3F,$01,$40
        .byte   $01,$41,$00,$42,$00
LA171:  .byte   $F7
LA172:  .byte   $A1,$1C,$A2,$41,$A2,$5A,$A2,$73
        .byte   $A2,$8C,$A2,$95,$A2,$9E,$A2,$A7
        .byte   $A2,$F9,$A2,$18,$A3,$79,$A3,$92
        .byte   $A3,$B3,$A3,$CC,$A3,$ED,$A3,$06
        .byte   $A4,$27,$A4,$38,$A4,$51,$A4,$7A
        .byte   $A4,$99,$A4,$C2,$A4,$D0,$A2,$23
        .byte   $A5,$30,$A5,$3D,$A5,$40,$A5,$43
        .byte   $A5,$50,$A5,$5D,$A5,$60,$A5,$63
        .byte   $A5,$68,$A5,$6F,$A5,$72,$A5,$75
        .byte   $A5,$78,$A5,$7D,$A5,$82,$A5,$87
        .byte   $A5,$8C,$A5,$91,$A5,$96,$A5,$9B
        .byte   $A5,$A0,$A5,$AA,$A5,$A5,$A5,$AF
        .byte   $A5
LA1D3:  .byte   $B4
LA1D4:  .byte   $A5,$B7,$A5,$C4,$A5,$D1,$A5,$DE
        .byte   $A5,$27,$A6,$46,$A6,$53,$A6,$64
        .byte   $A6,$69,$A6,$70,$A6,$79,$A6,$86
        .byte   $A6,$93,$A6,$A0,$A6,$A9,$A6,$B2
        .byte   $A6,$BB,$A6,$36,$39,$00,$01,$00
        .byte   $02,$00,$10,$00,$11,$00,$12,$00
        .byte   $20,$00,$21,$00,$22,$00,$30,$00
        .byte   $31,$00,$32,$00,$03,$00,$04,$00
        .byte   $05,$00,$13,$00,$14,$00,$15,$00
        .byte   $36,$06,$00,$07,$00,$08,$00,$16
        .byte   $00,$17,$00,$18,$00,$26,$00,$27
        .byte   $00,$28,$00,$36,$00,$37,$00,$38
        .byte   $00,$23,$00,$24,$00,$25,$00,$33
        .byte   $00,$34,$00,$35,$00,$43,$09,$00
        .byte   $0A,$00,$0B,$00,$0C,$00,$19,$00
        .byte   $1A,$00,$1B,$00,$1C,$00,$29,$00
        .byte   $2A,$00,$2B,$00,$2C,$00,$26,$0D
        .byte   $00,$0E,$00,$1D,$00,$1E,$00,$3D
        .byte   $00,$3D,$00,$3D,$00,$3D,$00,$3D
        .byte   $00,$3D,$00,$3D,$00,$3D,$00,$26
        .byte   $2D,$00,$2E,$00,$0F,$00,$1E,$00
        .byte   $1F,$00,$3D,$00,$2F,$00,$3D,$00
        .byte   $3F,$00,$3D,$00,$3E,$00,$3D,$00
        .byte   $22,$10,$00,$05,$00,$14,$00,$15
        .byte   $00,$22,$06,$00,$07,$00,$16,$00
        .byte   $17,$00,$22,$08,$01,$09,$01,$18
        .byte   $01,$10,$01,$45,$10,$00,$10,$00
        .byte   $10,$00,$10,$00,$10,$00,$10,$00
        .byte   $10,$00,$10,$00,$10,$00,$10,$00
        .byte   $10,$00,$10,$00,$0C,$00,$0D,$00
        .byte   $0E,$00,$0F,$00,$10,$00,$10,$00
        .byte   $10,$00,$10,$00,$45,$1F,$00,$10
        .byte   $00,$10,$00,$10,$00,$1E,$00,$10
        .byte   $00,$10,$00,$10,$00,$1D,$00,$10
        .byte   $00,$10,$00,$10,$00,$1C,$00,$10
        .byte   $00,$10,$00,$10,$00,$10,$00,$10
        .byte   $00,$10,$00,$10,$00,$35,$01,$00
        .byte   $02,$00,$03,$00,$11,$00,$12,$00
        .byte   $13,$00,$21,$00,$22,$00,$23,$00
        .byte   $31,$00,$32,$00,$33,$00,$04,$00
        .byte   $05,$00,$06,$00,$68,$3C,$00,$01
        .byte   $00,$02,$00,$03,$00,$04,$00,$3C
        .byte   $00,$10,$00,$11,$00,$12,$00,$13
        .byte   $00,$14,$00,$15,$00,$3C,$00,$3C
        .byte   $00,$22,$00,$23,$00,$24,$00,$3C
        .byte   $00,$3C,$00,$31,$00,$32,$00,$33
        .byte   $00,$34,$00,$35,$00,$3C,$00,$07
        .byte   $00,$08,$00,$09,$00,$0A,$00,$0B
        .byte   $00,$16,$00,$17,$00,$18,$00,$19
        .byte   $00,$1A,$00,$1B,$00,$26,$00,$27
        .byte   $00,$28,$00,$29,$00,$2A,$00,$2B
        .byte   $00,$36,$00,$37,$00,$38,$00,$38
        .byte   $00,$38,$00,$3B,$00,$34,$01,$00
        .byte   $02,$00,$03,$00,$11,$00,$12,$00
        .byte   $13,$00,$21,$00,$22,$00,$23,$00
        .byte   $31,$00,$32,$00,$33,$00,$44,$04
        .byte   $00,$05,$00,$06,$00,$07,$00,$14
        .byte   $00,$15,$00,$16,$00,$17,$00,$24
        .byte   $00,$25,$00,$26,$00,$27,$00,$34
        .byte   $00,$35,$00,$36,$00,$37,$00,$34
        .byte   $08,$01,$09,$01,$0A,$01,$18,$01
        .byte   $19,$01,$1A,$01,$28,$01,$29,$01
        .byte   $2A,$01,$38,$01,$39,$01,$3A,$01
        .byte   $44,$0B,$01,$0C,$01,$0D,$01,$30
        .byte   $01,$1B,$01,$1C,$01,$1D,$01,$30
        .byte   $01,$2B,$01,$2C,$01,$2D,$01,$30
        .byte   $01,$3B,$01,$3C,$01,$3D,$01,$3E
        .byte   $01,$34,$01,$00,$02,$00,$03,$00
        .byte   $11,$00,$12,$00,$13,$00,$21,$00
        .byte   $22,$00,$23,$00,$31,$00,$32,$00
        .byte   $33,$00,$44,$04,$01,$05,$01,$06
        .byte   $01,$07,$01,$14,$01,$15,$01,$16
        .byte   $01,$17,$01,$24,$01,$25,$01,$26
        .byte   $01,$27,$01,$34,$01,$35,$01,$36
        .byte   $01,$37,$01,$24,$0E,$00,$0F,$00
        .byte   $1E,$00,$1F,$00,$2E,$00,$2F,$00
        .byte   $10,$00,$2F,$00,$34,$0A,$00,$0B
        .byte   $00,$0C,$00,$1A,$00,$1B,$00,$1C
        .byte   $00,$2A,$00,$2B,$00,$2C,$00,$3A
        .byte   $00,$3B,$00,$3C,$00,$45,$10,$00
        .byte   $10,$00,$10,$00,$10,$00,$10,$00
        .byte   $20,$00,$21,$00,$10,$00,$10,$00
        .byte   $30,$00,$31,$00,$10,$00,$22,$00
        .byte   $23,$00,$24,$00,$10,$00,$32,$00
        .byte   $33,$00,$34,$00,$10,$00,$35,$30
        .byte   $00,$31,$00,$32,$00,$33,$00,$34
        .byte   $00,$35,$00,$36,$00,$37,$00,$38
        .byte   $00,$39,$00,$3A,$00,$3B,$00,$3C
        .byte   $00,$3D,$00,$3E,$00,$45,$0A,$00
        .byte   $0B,$00,$0C,$00,$0D,$00,$1A,$00
        .byte   $1B,$00,$1C,$00,$1D,$00,$2A,$00
        .byte   $2B,$00,$2C,$00,$2D,$00,$3A,$00
        .byte   $3B,$00,$3C,$00,$3D,$00,$36,$00
        .byte   $37,$00,$38,$00,$39,$00,$68,$3C
        .byte   $01,$01,$01,$02,$01,$03,$01,$04
        .byte   $01,$3C,$01,$0C,$01,$0D,$01,$0E
        .byte   $01,$0F,$01,$14,$01,$15,$01,$1C
        .byte   $01,$1D,$01,$1E,$01,$1F,$01,$20
        .byte   $01,$21,$01,$2C,$01,$2D,$01,$2E
        .byte   $01,$2F,$01,$25,$01,$3C,$01,$3C
        .byte   $01,$3D,$01,$3E,$01,$3F,$01,$3C
        .byte   $01,$3C,$01,$16,$01,$17,$01,$18
        .byte   $01,$19,$01,$39,$01,$3A,$01,$3C
        .byte   $01,$27,$01,$28,$01,$29,$01,$3C
        .byte   $01,$3C,$01,$3C,$01,$3C,$01,$3C
        .byte   $01,$3C,$01,$3C,$01,$3C,$01,$32
        .byte   $04,$01,$05,$01,$06,$01,$14,$01
        .byte   $15,$01,$16,$01,$32,$10,$00,$11
        .byte   $00,$12,$00,$20,$00,$21,$00,$22
        .byte   $00,$11,$3A,$01,$11,$0D,$00,$23
        .byte   $0A,$00,$0B,$00,$1A,$00,$1B,$00
        .byte   $10,$00,$19,$00,$32,$01,$00,$02
        .byte   $00,$03,$00,$11,$00,$12,$00,$13
        .byte   $00,$11,$02,$01,$11,$03,$01,$21
        .byte   $28,$00,$29,$00,$31,$25,$00,$26
        .byte   $00,$27,$00,$11,$35,$01,$11,$37
        .byte   $00,$11,$38,$00,$12,$07,$00,$17
        .byte   $00,$12,$08,$00,$18,$00,$12,$09
        .byte   $00,$19,$00,$12,$0A,$00,$1A,$00
        .byte   $12,$0B,$00,$1B,$00,$12,$0C,$00
        .byte   $1C,$00,$12,$0D,$00,$1D,$00,$12
        .byte   $0E,$00,$1E,$00,$12,$26,$00,$1F
        .byte   $00,$12,$24,$00,$34,$00,$12,$25
        .byte   $00,$35,$00,$12,$26,$00,$36,$00
        .byte   $11,$01,$00,$23,$03,$00,$04,$00
        .byte   $13,$00,$14,$00,$02,$00,$24,$00
        .byte   $23,$05,$01,$06,$01,$15,$01,$16
        .byte   $01,$25,$01,$26,$01,$23,$07,$01
        .byte   $08,$01,$17,$01,$18,$01,$27,$01
        .byte   $28,$01,$49,$1B,$00,$01,$00,$02
        .byte   $00,$03,$00,$10,$00,$11,$00,$12
        .byte   $00,$13,$00,$20,$00,$21,$00,$22
        .byte   $00,$23,$00,$30,$00,$31,$00,$32
        .byte   $00,$33,$00,$3B,$00,$3C,$00,$3D
        .byte   $00,$3E,$00,$04,$00,$05,$00,$06
        .byte   $00,$07,$00,$14,$00,$15,$00,$16
        .byte   $00,$17,$00,$24,$00,$25,$00,$26
        .byte   $00,$27,$00,$34,$00,$35,$00,$36
        .byte   $00,$37,$00,$35,$08,$01,$09,$01
        .byte   $0A,$01,$18,$01,$19,$01,$1A,$01
        .byte   $28,$01,$29,$01,$2A,$01,$38,$01
        .byte   $39,$01,$3A,$01,$0B,$01,$0C,$01
        .byte   $3F,$01,$23,$2A,$00,$2B,$00,$3A
        .byte   $00,$3B,$00,$2C,$00,$2D,$00,$24
        .byte   $10,$00,$01,$00,$11,$00,$01,$00
        .byte   $20,$00,$21,$00,$30,$00,$31,$00
        .byte   $12,$09,$00,$19,$00,$13,$0A,$01
        .byte   $1A,$01,$2A,$01,$22,$1B,$00,$1C
        .byte   $00,$2B,$00,$2C,$00,$32,$0D,$00
        .byte   $0E,$00,$0F,$00,$1D,$00,$1E,$00
        .byte   $1F,$00,$23,$0F,$01,$0F,$01,$2D
        .byte   $01,$2E,$01,$3D,$01,$3E,$01,$23
        .byte   $0D,$01,$0E,$01,$1D,$01,$1E,$01
        .byte   $0F,$01,$0F,$01,$22,$2F,$01,$2F
        .byte   $01,$2B,$01,$2C,$01,$22,$2D,$01
        .byte   $2E,$01,$2F,$01,$2F,$01,$22,$0E
        .byte   $00,$0F,$00,$1E,$00,$1F,$00,$42
        .byte   $01,$00,$02,$00,$03,$00,$04,$00
        .byte   $11,$00,$12,$00,$13,$00,$14,$00
; ----------------------------------------------------------------------------
add_object_sprites:
        lda     $BD
        asl     a
        tay
        lda     object_sprite_tables,y
        sta     $63
        lda     LA6FE,y
        sta     $64
        ldy     #$00
LA6DC:  lda     ($63),y
        cmp     #$FF
        beq     LA6FC
        cmp     $BE
        bne     LA6E9
        jsr     object_sprite_visible
LA6E9:  jsr     next_object_sprite_record
        jmp     LA6DC

; ----------------------------------------------------------------------------
next_object_sprite_record:  lda     $63
        clc
        adc     #$04
        sta     $63
        lda     $64
        adc     #$00
        sta     $64
LA6FC:  rts

; ----------------------------------------------------------------------------
object_sprite_tables:  .byte   $25
LA6FE:  .byte   $A8,$86,$A8,$8F,$A8,$B0,$A8,$4D
        .byte   $A9
; ----------------------------------------------------------------------------
object_sprite_visible:  tya
        pha
        jsr     object_sprite_condition
        pla
        tay
        rts

; ----------------------------------------------------------------------------
object_sprite_condition:  lda     $BD
        jsr     jump_table
        .addr   LA71E
        .addr   LA774
        .addr   LA7F7
        .addr   LA783
        .addr   LA7BB
; ----------------------------------------------------------------------------
LA71E:  iny
        iny
        iny
        lda     ($63),y
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LA75A
        jsr     sprite_value_is_0A_or_16
        lda     script_flags
        and     #$04
        bne     LA75A
        dey
        dey
        dey
append_object_sprite:  iny
        lda     ($63),y
        dey
        sta     ($67),y
        iny
        iny
        lda     ($63),y
        dey
        sta     ($67),y
        iny
        iny
        lda     ($63),y
        dey
        sta     ($67),y
        iny
        lda     $67
        clc
        adc     #$03
        sta     $67
        lda     $68
        adc     #$00
        sta     $68
LA75A:  rts

; ----------------------------------------------------------------------------
sprite_value_is_0A_or_16:  jsr     script_condition_false
        lda     ($63),y
        cmp     #$0A
        beq     LA769
        cmp     #$16
        beq     LA769
        rts

; ----------------------------------------------------------------------------
LA769:  lda     scene
        cmp     #$73
        beq     LA773
        jsr     script_condition_true
LA773:  rts

; ----------------------------------------------------------------------------
LA774:  lda     ($63),y
        jsr     test_object_open
        lda     script_flags
        and     #$04
        beq     LA782
        jsr     append_object_sprite
LA782:  rts

; ----------------------------------------------------------------------------
LA783:  lda     ($63),y
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        bne     LA7A5
        lda     ($63),y
        jsr     test_flag_045C
        lda     script_flags
        and     #$04
        bne     LA7A5
        jsr     sprite_value_is_2C
        lda     script_flags
        and     #$04
        bne     LA7A5
        jsr     append_object_sprite
LA7A5:  rts

; ----------------------------------------------------------------------------
sprite_value_is_2C:  jsr     script_condition_false
        lda     ($63),y
        cmp     #$2C
        beq     LA7B0
        rts

; ----------------------------------------------------------------------------
LA7B0:  lda     scene
        cmp     #$44
        beq     LA7BA
        jsr     script_condition_true
LA7BA:  rts

; ----------------------------------------------------------------------------
LA7BB:  lda     ($63),y
        jsr     test_item_obtained
        lda     script_flags
        and     #$04
        bne     LA7DD
        lda     ($63),y
        jsr     test_flag_047A
        lda     script_flags
        and     #$04
        bne     LA7DD
        jsr     sprite_value_is_0_or_2
        lda     script_flags
        and     #$04
        bne     LA7DD
        jsr     append_object_sprite
LA7DD:  rts

; ----------------------------------------------------------------------------
sprite_value_is_0_or_2:  jsr     script_condition_false
        lda     ($63),y
        cmp     #$00
        beq     LA7EC
        cmp     #$02
        beq     LA7EC
        rts

; ----------------------------------------------------------------------------
LA7EC:  lda     scene
        cmp     #$00
        beq     LA7F6
        jsr     script_condition_true
LA7F6:  rts

; ----------------------------------------------------------------------------
LA7F7:  lda     ($63),y
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LA805
        jsr     append_object_sprite
LA805:  pla
        txa
        rts

; ----------------------------------------------------------------------------
draw_object_sprites:
        ldy     #$00
LA80A:  lda     object_sprite_list,y
        cmp     #$FF
        beq     LA824
        sta     $20
        lda     object_sprite_list+1,y
        sta     $21
        lda     object_sprite_list+2,y
        jsr     draw_object_sprite
        iny
        iny
        iny
        jmp     LA80A

; ----------------------------------------------------------------------------
LA824:  rts

; ----------------------------------------------------------------------------
        .byte   $03,$20,$40,$00,$07,$20,$40,$01
        .byte   $0D,$40,$38,$02,$12,$60,$60,$03
        .byte   $12,$60,$60,$04,$75,$50,$40,$05
        .byte   $76,$40,$40,$06,$77,$48,$50,$07
        .byte   $6D,$58,$60,$08,$54,$58,$20,$09
        .byte   $6B,$30,$30,$0A,$6C,$18,$60,$0B
        .byte   $6C,$14,$60,$0C,$6C,$18,$60,$0D
        .byte   $6C,$14,$60,$0E,$6C,$18,$60,$0F
        .byte   $6C,$14,$60,$10,$6C,$1C,$60,$11
        .byte   $6C,$18,$60,$12,$6D,$58,$60,$13
        .byte   $65,$28,$20,$14,$2C,$38,$58,$15
        .byte   $6B,$30,$30,$16,$6D,$58,$60,$17
        .byte   $FF,$06,$50,$60,$18,$08,$18,$50
        .byte   $19,$FF,$9A,$20,$80,$3F,$20,$20
        .byte   $80,$40,$9B,$40,$80,$3D,$21,$40
        .byte   $80,$3E,$9C,$60,$78,$42,$00,$60
        .byte   $78,$42,$00,$10,$80,$41,$23,$10
        .byte   $80,$41,$FF,$27,$70,$70,$1A,$28
        .byte   $68,$80,$1B,$29,$70,$88,$1C,$2A
        .byte   $78,$88,$1D,$2B,$60,$68,$1E,$2C
        .byte   $38,$40,$1F,$3D,$68,$68,$20,$3E
        .byte   $70,$68,$21,$3F,$68,$70,$22,$40
        .byte   $70,$70,$23,$41,$68,$78,$24,$42
        .byte   $70,$78,$25,$4A,$28,$68,$2B,$4F
        .byte   $30,$68,$26,$50,$28,$70,$27,$51
        .byte   $30,$70,$28,$52,$28,$78,$29,$53
        .byte   $30,$78,$2A,$2E,$38,$70,$2C,$2F
        .byte   $58,$48,$2D,$30,$10,$5A,$2E,$31
        .byte   $18,$5A,$2F,$32,$1C,$5A,$30,$33
        .byte   $20,$30,$31,$58,$28,$30,$32,$55
        .byte   $30,$30,$33,$34,$38,$30,$34,$35
        .byte   $20,$40,$35,$36,$28,$40,$36,$5A
        .byte   $30,$40,$37,$59,$38,$40,$38,$5B
        .byte   $20,$50,$39,$56,$28,$4F,$3A,$57
        .byte   $30,$50,$3B,$5C,$38,$4F,$3C,$37
        .byte   $50,$58,$3D,$38,$70,$38,$3E,$39
        .byte   $40,$20,$3F,$3A,$18,$38,$40,$FF
        .byte   $00,$40,$48,$41,$02,$48,$50,$42
        .byte   $06,$28,$78,$43,$11,$48,$68,$44
        .byte   $0B,$28,$40,$45,$0C,$30,$38,$46
        .byte   $10,$38,$50,$47,$0F,$30,$48,$48
        .byte   $FF
; ----------------------------------------------------------------------------
script_slot_for_command:
        lda     script_table
        and     #$7F
        asl     a
        sta     script_slot
        asl     a
        asl     a
        clc
        adc     script_table
        adc     script_slot
        adc     command
        tay
        lda     LA985,y
        sta     script_slot
        rts

; ----------------------------------------------------------------------------
LA985:  .byte   $00,$01,$02,$FF,$FF,$FF,$FF,$03
        .byte   $FF,$FF,$FF,$00,$FF,$FF,$01,$02
        .byte   $FF,$FF,$03,$FF,$FF,$FF,$00,$FF
        .byte   $FF,$01,$02,$FF,$FF,$03,$FF,$FF
        .byte   $FF,$00,$FF,$FF,$FF,$FF,$01,$02
        .byte   $03,$FF,$FF,$FF,$00,$FF,$FF,$01
        .byte   $02,$03,$04,$05,$FF,$FF,$FF,$00
        .byte   $FF,$FF,$01,$02,$FF,$FF,$FF,$FF
        .byte   $FF,$03
; ----------------------------------------------------------------------------
script_table_is_0:  jsr     script_condition_false
        lda     script_table
        cmp     #$00
        beq     LA9D1
        rts

; ----------------------------------------------------------------------------
LA9D1:  lda     object_id
        cmp     #$12
        beq     LA9D8
        rts

; ----------------------------------------------------------------------------
LA9D8:  lda     command
        cmp     #$03
        beq     LA9E3
        cmp     #$04
        beq     LAA12
        rts

; ----------------------------------------------------------------------------
LA9E3:  lda     #$04
        jsr     test_event_flag
        lda     #$6A
        sta     text_item
        lda     script_flags
        and     #$04
        beq     LA9FC
        lda     #$01
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LA9FC:  lda     #$00
        jsr     print_message_2_or_5
        lda     #$03
        jsr     clear_event_flag
        lda     #$04
        jsr     set_event_flag
        jsr     op_27
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LAA12:  lda     #$03
        jsr     test_event_flag
        lda     #$6A
        sta     text_item
        lda     script_flags
        and     #$04
        beq     LAA2B
        lda     #$03
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LAA2B:  lda     #$02
        jsr     print_message_2_or_5
        lda     #$04
        jsr     clear_event_flag
        lda     #$03
        jsr     set_event_flag
        jsr     op_27
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
far_8020_64:
        jsr     special_room_response
        lda     script_flags
        and     #$04
        beq     LAA4B
        rts

; ----------------------------------------------------------------------------
LAA4B:  jsr     script_table_is_0
        lda     script_flags
        and     #$04
        beq     LAA55
        rts

; ----------------------------------------------------------------------------
LAA55:  jsr     script_table_is_0_b
        lda     script_flags
        and     #$04
        beq     LAA5F
        rts

; ----------------------------------------------------------------------------
LAA5F:  lda     script_table
        asl     a
        asl     a
        sta     $5E
        asl     a
        clc
        adc     $5E
        sec
        sbc     script_table
        asl     a
        clc
        adc     command
        adc     command
        tay
        lda     LAAFC,y
        cmp     #$FF
        beq     LAA89
        jsr     jump_table
        .addr   LAA83
        .addr   LAA8A
        .addr   LAA91
; ----------------------------------------------------------------------------
LAA83:  lda     LAAFB,y
        jsr     print_message_5
LAA89:  rts

; ----------------------------------------------------------------------------
LAA8A:  lda     LAAFB,y
        jsr     print_message_4
        rts

; ----------------------------------------------------------------------------
LAA91:  lda     LAAFB,y
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
special_room_response:  lda     room
        cmp     #$1E
        beq     LAABB
        cmp     #$20
        beq     LAABE
        cmp     #$24
        beq     LAAC1
        cmp     #$4A
        beq     LAAC4
        cmp     #$4B
        beq     LAAC4
        cmp     #$4C
        beq     LAAC4
        cmp     #$4D
        beq     LAAC4
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LAABB:  jmp     special_command_room_1E

; ----------------------------------------------------------------------------
LAABE:  jmp     special_command_room_20

; ----------------------------------------------------------------------------
LAAC1:  jmp     special_command_room_24

; ----------------------------------------------------------------------------
LAAC4:  jmp     special_command_rooms_4A

; ----------------------------------------------------------------------------
script_table_is_0_b:  jsr     script_condition_false
        lda     script_table
        cmp     #$00
        beq     LAAD1
        rts

; ----------------------------------------------------------------------------
LAAD1:  lda     object_id
        cmp     #$3F
        beq     LAAD8
        rts

; ----------------------------------------------------------------------------
LAAD8:  lda     scene
        cmp     #$44
        beq     LAAE0
        rts

; ----------------------------------------------------------------------------
LAAE0:  lda     command
        cmp     #$04
        beq     LAAE7
        rts

; ----------------------------------------------------------------------------
LAAE7:  lda     #$05
        sta     script_table
        lda     #$47
        sta     object_id
        jsr     op_1B
        lda     #$5D
        jsr     clear_event_flag
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LAAFB:  .byte   $FF
LAAFC:  .byte   $FF,$FF,$FF,$FF,$FF,$13,$00,$37
        .byte   $02,$16,$00,$13,$00,$FF,$FF,$13
        .byte   $00,$FF,$FF,$14,$00,$FF,$FF,$1B
        .byte   $00,$7D,$00,$FF,$FF,$FF,$FF,$16
        .byte   $00,$13,$00,$FF,$FF,$13,$00,$FF
        .byte   $FF,$13,$00,$FF,$FF,$1B,$00,$7D
        .byte   $00,$FF,$FF,$FF,$FF,$16,$00,$13
        .byte   $00,$FF,$FF,$13,$00,$FF,$FF,$13
        .byte   $00,$FF,$FF,$13,$00,$1A,$00,$14
        .byte   $00,$13,$00,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$14,$00,$FF,$FF,$13,$00,$FF
        .byte   $FF,$1B,$00,$1A,$00,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$13
        .byte   $00,$FF,$FF,$13,$00,$FF,$FF,$13
        .byte   $00,$1A,$00,$FF,$FF,$FF,$FF,$16
        .byte   $00,$13,$00,$FF,$FF,$13,$00,$FF
        .byte   $FF,$FF,$FF
; ----------------------------------------------------------------------------
highlight_command:  tya
        pha
        jsr     command_vram_address
        jsr     draw_command_highlight
        pla
        tay
        rts

; ----------------------------------------------------------------------------
draw_command_highlight:  lda     #$F8
        sta     $1E
        lda     #$AB
        sta     $1F
        jsr     queue_vram_block
        jsr     request_vram_queue
        jsr     wait_nmi_work_done
        rts

; ----------------------------------------------------------------------------
unhighlight_command:  tya
        pha
        jsr     command_vram_address
        lda     #$FB
        sta     $1E
        lda     #$AB
        sta     $1F
        jsr     queue_vram_block
        jsr     request_vram_queue
        jsr     wait_nmi_work_done
        pla
        tay
        rts

; ----------------------------------------------------------------------------
command_vram_address:  lda     command
        asl     a
        tay
        lda     LAC01,y
        sta     $1C
        lda     LAC02,y
        sta     $1D
        rts

; ----------------------------------------------------------------------------
clear_entry_highlight:  tya
        pha
        jsr     inventory_entry_vram_address
        lda     #$FE
        sta     $1E
        lda     #$AB
        sta     $1F
        jsr     queue_vram_block
        jsr     request_vram_queue
        jsr     wait_nmi_work_done
        pla
        tay
        rts

; ----------------------------------------------------------------------------
highlight_entry:
        tya
        pha
        jsr     inventory_entry_vram_address
        jsr     draw_command_highlight
        pla
        tay
        rts

; ----------------------------------------------------------------------------
inventory_entry_vram_address:  lda     inventory_entry
        asl     a
        tay
        lda     LAC1B,y
        sta     $1C
        lda     LAC1C,y
        sta     $1D
        rts

; ----------------------------------------------------------------------------
        .byte   $01,$01,$A9,$01,$01,$69,$01,$01
        .byte   $A6
LAC01:  .byte   $A3
LAC02:  .byte   $22,$E3,$22,$23,$23,$AA,$22,$63
        .byte   $23,$EA,$22,$2A,$23,$6A,$23,$38
        .byte   $23,$78,$23,$B1,$22,$B8,$22,$BC
        .byte   $22
LAC1B:  .byte   $B3
LAC1C:  .byte   $20,$F3,$20,$33,$21,$73,$21,$B3
        .byte   $21,$F3,$21,$33,$22
; ----------------------------------------------------------------------------
store_in_inventory_slot:
        tya
        pha
        jsr     item_code_for_object
        jsr     item_code_in_list_AC53
        lda     store_page
        asl     a
        asl     a
        asl     a
        sec
        sbc     store_page
        clc
        adc     store_entry
        tay
        lda     $C1
        sta     address_book,y
        lda     script_table
        sta     $BD
        lda     object_id
        sta     $BE
        jsr     item_number_for_object
        pla
        tay
        rts

; ----------------------------------------------------------------------------
item_code_in_list_AC53:  lda     $C1
        cmp     #$3D
        beq     LAC8E
        cmp     #$3E
        beq     LAC8E
        cmp     #$3F
        beq     LAC8E
        cmp     #$40
        beq     LAC8E
        cmp     #$41
        beq     LAC8E
        cmp     #$42
        beq     LAC8E
        cmp     #$43
        beq     LAC8E
        cmp     #$49
        beq     LAC8E
        cmp     #$4A
        beq     LAC95
        cmp     #$4F
        beq     LAC95
        cmp     #$50
        beq     LAC95
        cmp     #$51
        beq     LAC95
        cmp     #$52
        beq     LAC95
        cmp     #$53
        beq     LAC95
        rts

; ----------------------------------------------------------------------------
LAC8E:  lda     #$08
        sta     object_id
        sta     $C1
        rts

; ----------------------------------------------------------------------------
LAC95:  lda     #$2D
        sta     object_id
        sta     $C1
        rts

; ----------------------------------------------------------------------------
item_number_for_object:
        tya
        pha
        ldy     $BD
        lda     LAD70,y
        clc
        adc     $BE
        sta     $C0
        jsr     kind_is_3
        pla
        tay
        rts

; ----------------------------------------------------------------------------
kind_is_3:  lda     $BD
        cmp     #$03
        beq     LACB5
        rts

; ----------------------------------------------------------------------------
LACB5:  ldy     #$00
        lda     $C0
        cmp     #$3D
        beq     LAD1C
        cmp     #$3E
        beq     LAD1C
        cmp     #$3F
        beq     LAD1C
        cmp     #$40
        beq     LAD1C
        cmp     #$41
        beq     LAD1C
        cmp     #$42
        beq     LAD1C
        cmp     #$43
        beq     LAD1C
        cmp     #$49
        beq     LAD1C
        iny
        cmp     #$4A
        beq     LAD1C
        cmp     #$4F
        beq     LAD1C
        cmp     #$50
        beq     LAD1C
        cmp     #$51
        beq     LAD1C
        cmp     #$52
        beq     LAD1C
        cmp     #$53
        beq     LAD1C
        iny
        cmp     #$54
        beq     LAD1C
        iny
        cmp     #$55
        beq     LAD1C
        cmp     #$56
        beq     LAD1C
        cmp     #$57
        beq     LAD1C
        iny
        cmp     #$58
        beq     LAD1C
        cmp     #$59
        beq     LAD1C
        iny
        cmp     #$5A
        beq     LAD1C
        iny
        cmp     #$5B
        beq     LAD1C
        cmp     #$5C
        beq     LAD1C
        rts

; ----------------------------------------------------------------------------
LAD1C:  lda     LAD22,y
        sta     $C0
        rts

; ----------------------------------------------------------------------------
LAD22:  .byte   $08,$2D,$07,$28,$29,$33,$35
; ----------------------------------------------------------------------------
check_inventory_slot_empty:  lda     script_flags
        and     #$EF
        sta     script_flags
        jsr     inventory_slot_index
        lda     address_book,y
        cmp     #$FF
        bne     LAD40
        lda     script_flags
        ora     #$10
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
LAD40:  jsr     split_item_code
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LAD57
        lda     $BD
        sta     script_table
        lda     $BE
        sta     object_id
        rts

; ----------------------------------------------------------------------------
LAD57:  lda     $BD
        sta     switch_key
        lda     $BE
        sta     switch_key+1
        rts

; ----------------------------------------------------------------------------
inventory_slot_index:  lda     inventory_page
        asl     a
        asl     a
        asl     a
        sec
        sbc     inventory_page
        clc
        adc     inventory_entry
        tay
        rts

; ----------------------------------------------------------------------------
LAD70:  .byte   $60,$D0,$E0,$00,$40,$00
; ----------------------------------------------------------------------------
draw_inventory_page:
        lda     script_table
        pha
        lda     object_id
        pha
        lda     #$00
        sta     inventory_entry
LAD81:  jsr     check_inventory_slot_empty
        jsr     item_number_for_object
        lda     script_flags
        and     #$10
        bne     LAD9A
        jsr     draw_inventory_entry
        inc     inventory_entry
        lda     inventory_entry
        cmp     #$07
        bcc     LAD81
LAD9A:  pla
        sta     object_id
        pla
        sta     script_table
        rts

; ----------------------------------------------------------------------------
draw_address_book:
        lda     script_table
        pha
        lda     object_id
        pha
        lda     #$00
        sta     inventory_entry
LADAC:  ldy     inventory_entry
        lda     address_book,y
        cmp     #$FF
        beq     LADC5
        sta     $C0
        jsr     draw_inventory_entry
        inc     inventory_entry
        lda     inventory_entry
        cmp     #$07
        bcc     LADAC
LADC5:  pla
        sta     object_id
        pla
        sta     script_table
        rts

; ----------------------------------------------------------------------------
draw_inventory_entry:
        tya
        pha
        lda     #$14
        sta     $9C
        ldy     inventory_entry
        lda     LAE86,y
        sta     $9D
        lda     #$00
        ldy     $C0
        cpy     #$00
        beq     LADF3
        cpy     #$08
        beq     LAE01
        cpy     #$2D
        beq     LAE1D
LADEA:  jsr     draw_layout_number
        jsr     request_vram_queue
        pla
        tay
        rts

; ----------------------------------------------------------------------------
LADF3:  lda     #$01
        ldy     inventory_page
        cpy     #$01
        bne     LADEA
        lda     #$02
        jmp     LADEA

; ----------------------------------------------------------------------------
LAE01:  lda     #$03
        ldy     inventory_page
        cpy     #$01
        bne     LADEA
        lda     #$04
        ldy     $B0
        cpy     #$02
        beq     LADEA
        lda     #$08
        ldy     #$07
        beq     LADEA
        lda     #$09
        jmp     LADEA

; ----------------------------------------------------------------------------
LAE1D:  lda     #$05
        ldy     inventory_page
        cpy     #$01
        bne     LADEA
        lda     #$0A
        jmp     LADEA

; ----------------------------------------------------------------------------
far_8020_6E:
        tya
        pha
        lda     current_container
        asl     a
        tay
        lda     container_stack,y
        sta     $AF
        lda     container_stack+1,y
        sta     $B0
        lda     #$13
        sta     $9C
        lda     #$03
        sta     $9D
        jsr     inventory_page_header
        lda     #$00
        jsr     draw_layout_number
        jsr     request_vram_queue
        pla
        tay
        rts

; ----------------------------------------------------------------------------
inventory_page_header:  lda     inventory_page
        cmp     #$02
        bcc     LAE5B
        lda     #$02
LAE5B:  jsr     jump_table
        .addr   LAE64
        .addr   LAE69
        .addr   LAE81
; ----------------------------------------------------------------------------
LAE64:  lda     #$BE
        sta     $C0
        rts

; ----------------------------------------------------------------------------
LAE69:  tya
        pha
        ldy     $AF
        cpy     #$FF
        bne     LAE76
        lda     #$BF
        jmp     LAE7C

; ----------------------------------------------------------------------------
LAE76:  lda     LAD70,y
        clc
        adc     $B0
LAE7C:  sta     $C0
        pla
        tay
        rts

; ----------------------------------------------------------------------------
LAE81:  lda     #$BD
        sta     $C0
        rts

; ----------------------------------------------------------------------------
LAE86:  .byte   $05,$07,$09,$0B,$0D,$0F,$11
; ----------------------------------------------------------------------------
far_8020_69:
        txa
        pha
        ldx     #$00
LAE91:  lda     LAE9F,x
        jsr     set_door_flag
        inx
        cpx     #$23
        bcc     LAE91
        pla
        tax
        rts

; ----------------------------------------------------------------------------
LAE9F:  .byte   $07,$0A,$0B,$11,$13,$16,$17,$19
        .byte   $1A,$1C,$1D,$1F,$21,$23,$24,$25
        .byte   $26,$27,$28,$29,$2A,$2B,$2C,$35
        .byte   $37,$39,$3D,$41,$42,$43,$44,$45
        .byte   $46,$49,$4C
; ----------------------------------------------------------------------------
delete_inventory_entry:
        lda     #$54
        sta     $84
LAEC6:  tya
        pha
        txa
        pha
        jsr     inventory_slot_index
        tya
        tax
        inx
LAED0:  cpx     $84
        bcs     LAEDF
        lda     address_book,x
        sta     address_book,y
        iny
        inx
        jmp     LAED0

; ----------------------------------------------------------------------------
LAEDF:  lda     #$FF
        sta     address_book,y
        pla
        txa
        pla
        tya
        rts

; ----------------------------------------------------------------------------
far_8020_6C:
        lda     #$0E
        sta     $84
        jmp     LAEC6

; ----------------------------------------------------------------------------
; no caller found
unreferenced_AEF0:
        tya
        pha
        ldy     #$00
        lda     #$FF
LAEF6:  sta     coat_slots,y
        iny
        cpy     #$07
        bcc     LAEF6
        sta     $AF
        sta     $B0
        lda     #$01
        jsr     far_8000_6E
        rts

; ----------------------------------------------------------------------------
play_slot_machine:
        lda     #$11
        jsr     play_sound
        jsr     wait_60_frames
        jsr     clear_and_upload_sprites
        jsr     slot_machine_payout
        lda     #$12
        jsr     play_sound
        jsr     spin_all_reels
        jsr     stop_reels
        lda     $84
        beq     LAF4E
        sta     text_number
        sta     script_args
        lda     #$14
        jsr     play_sound
        jsr     wait_180_frames
        jsr     wait_60_frames
        lda     #$15
        jsr     play_sound
        jsr     wait_60_frames
        jsr     wait_60_frames
        lda     #$00
        jsr     play_sound
        jsr     add_coins_and_show
        lda     #$18
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
LAF4E:  lda     #$16
        jsr     play_sound
        lda     #$75
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
slot_machine_payout:  lda     coins
        beq     LAF6D
        cmp     #$46
        bcs     LAF89
        jsr     next_random
        lda     random_value
        and     #$07
        cmp     #$06
        bcc     LAF89
LAF6D:  jsr     next_random
        lda     random_value
        and     #$07
        tay
        lda     LAF81,y
        sta     $84
        sty     data_ptr
        sty     data_ptr+1
        sty     $57
        rts

; ----------------------------------------------------------------------------
LAF81:  .byte   $1E,$16,$14,$12,$10,$0E,$0C,$0A
; ----------------------------------------------------------------------------
LAF89:  lda     #$00
        sta     $84
LAF8D:  ldy     #$00
LAF8F:  jsr     next_random
        lda     random_value
        and     #$07
        sta     data_ptr,y
        iny
        cpy     #$03
        bne     LAF8F
        lda     data_ptr
        cmp     data_ptr+1
        bne     LAFA8
        cmp     $57
        beq     LAF8D
LAFA8:  rts

; ----------------------------------------------------------------------------
spin_all_reels:  lda     #$00
        sta     $64
LAFAD:  jsr     spin_reel_0
        jsr     spin_reel_1
        jsr     spin_reel_2
        jsr     request_vram_queue
        jsr     request_attrs_2000
        jsr     wait_7_frames
        inc     $64
        lda     $64
        cmp     #$1D
        bne     LAFAD
        rts

; ----------------------------------------------------------------------------
stop_reels:  lda     #$13
        jsr     play_sound
        lda     data_ptr
        sta     $58
        lda     #$00
        sta     $64
LAFD5:  jsr     draw_reel_0
        jsr     spin_reel_1
        jsr     spin_reel_2
        jsr     request_vram_queue
        jsr     request_attrs_2000
        jsr     wait_7_frames
        inc     $64
        lda     $64
        cmp     #$13
        bne     LAFD5
        lda     #$13
        jsr     play_sound
        lda     data_ptr+1
        sta     $59
        lda     #$00
        sta     $64
LAFFC:  jsr     draw_reel_0
        jsr     draw_reel_1
        jsr     spin_reel_2
        jsr     request_vram_queue
        jsr     request_attrs_2000
        jsr     wait_7_frames
        inc     $64
        lda     $64
        cmp     #$13
        bne     LAFFC
        lda     #$13
        jsr     play_sound
        lda     $57
        sta     $5A
        jsr     draw_reel_0
        jsr     draw_reel_1
        jsr     draw_reel_2
        jsr     request_vram_queue
        jsr     request_attrs_2000
        jsr     wait_7_frames
        rts

; ----------------------------------------------------------------------------
spin_reel_0:  jsr     next_random
        lda     random_value
        and     #$07
        cmp     $58
        beq     spin_reel_0
        sta     $58
        jsr     draw_reel_0
        rts

; ----------------------------------------------------------------------------
spin_reel_1:  jsr     next_random
        lda     random_value
        and     #$07
        cmp     $59
        beq     spin_reel_1
        sta     $59
        jsr     draw_reel_1
        rts

; ----------------------------------------------------------------------------
spin_reel_2:  jsr     next_random
        lda     random_value
        and     #$07
        cmp     $5A
        beq     spin_reel_2
        sta     $5A
        jsr     draw_reel_2
        rts

; ----------------------------------------------------------------------------
draw_reel_0:  ldx     #$00
        beq     LB06F
draw_reel_1:  ldx     #$01
        bne     LB06F
draw_reel_2:  ldx     #$02
LB06F:  lda     $58,x
        sta     $65
        txa
        asl     a
        tax
        lda     LB0AA,x
        pha
        sta     $1C
        lda     LB0AB,x
        pha
        sta     $1D
        ldx     $65
        lda     LB0B0,x
        sta     $22
        lda     #$02
        sta     $20
        sta     $21
        jsr     set_attr_block
        pla
        sta     $1D
        pla
        sta     $1C
        lda     $65
        asl     a
        tax
        lda     LB0B8,x
        sta     $1E
        lda     LB0B9,x
        sta     $1F
        jsr     queue_vram_block
        rts

; ----------------------------------------------------------------------------
LB0AA:  .byte   $04
LB0AB:  .byte   $21,$08,$21,$0C,$21
LB0B0:  .byte   $01,$00,$01,$02,$02,$01,$02,$02
LB0B8:  .byte   $C8
LB0B9:  .byte   $B0,$CE,$B0,$D4,$B0,$DA,$B0,$E0
        .byte   $B0,$E6,$B0,$EC,$B0,$F2,$B0,$02
        .byte   $02,$33,$34,$43,$44,$02,$02,$35
        .byte   $36,$45,$46,$02,$02,$37,$38,$47
        .byte   $48,$02,$02,$39,$3A,$49,$4A,$02
        .byte   $02,$3B,$3C,$4B,$4C,$02,$02,$3D
        .byte   $3E,$4D,$4E,$02,$02,$53,$54,$63
        .byte   $64,$02,$02,$55,$56,$65,$66
; ----------------------------------------------------------------------------
wait_7_frames:  jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
opening_logos:
        jsr     black_palette_buffer
        lda     #$FF
        jsr     clear_nametables
        lda     #$00
        jsr     fill_all_attrs
        jsr     request_attr_upload
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     draw_logo_screen
        jsr     set_chr_banks_inline
        .byte   $E8,$E9,$EA,$EB,$E8,$E9,$EA,$EB
; ----------------------------------------------------------------------------
        jsr     wait_30_frames_b6
        jsr     logo_palette_cycle_1
        jsr     wait_30_frames_b6
        jsr     wait_30_frames_b6
        jsr     wait_30_frames_b6
        jsr     logo_palette_cycle_2
        jmp     wait_30_frames_b6

; ----------------------------------------------------------------------------
black_palette_buffer:  ldx     #$00
LB149:  lda     #$0F
        sta     palette_buffer,x
        inx
        cpx     #$20
        bne     LB149
        jsr     request_palette_upload
        rts

; ----------------------------------------------------------------------------
wait_30_frames_b6:  ldx     #$00
LB159:  jsr     wait_vblank
        inx
        cpx     #$1E
        bne     LB159
        rts

; ----------------------------------------------------------------------------
logo_palette_cycle_1:  ldx     #$00
LB164:  lda     LB184,x
        sta     palette_buffer+1
        lda     LB185,x
        sta     palette_buffer+2
        lda     LB186,x
        sta     palette_buffer+3
        jsr     request_palette_upload
        jsr     wait_6_frames
        inx
        inx
        inx
        cpx     #$0F
        bne     LB164
        rts

; ----------------------------------------------------------------------------
LB184:  .byte   $0F
LB185:  .byte   $02
LB186:  .byte   $02,$01,$12,$12,$11,$12,$12,$21
        .byte   $12,$12,$31,$12,$12
; ----------------------------------------------------------------------------
logo_palette_cycle_2:  ldx     #$00
LB195:  lda     LB1B5,x
        sta     palette_buffer+1
        lda     LB1B6,x
        sta     palette_buffer+2
        lda     LB1B7,x
        sta     palette_buffer+3
        jsr     request_palette_upload
        jsr     wait_6_frames
        inx
        inx
        inx
        cpx     #$0F
        bne     LB195
        rts

; ----------------------------------------------------------------------------
LB1B5:  .byte   $21
LB1B6:  .byte   $12
LB1B7:  .byte   $12,$11,$12,$12,$01,$12,$02,$0F
        .byte   $02,$0F,$0F,$0F,$0F
; ----------------------------------------------------------------------------
wait_6_frames:  jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
draw_logo_screen:  jsr     draw_rle_screen_2000
        .byte   $FF,$FF,$FF,$FF,$FF,$88,$00,$01
        .byte   $02,$03,$04,$05,$06,$07,$08,$09
        .byte   $0A,$0B,$0C,$0D,$0E,$0F,$30,$FF
        .byte   $FF,$0F,$10,$11,$12,$13,$14,$15
        .byte   $16,$17,$18,$19,$1A,$1B,$1C,$1D
        .byte   $1E,$1F,$40,$FF,$FF,$0F,$20,$21
        .byte   $22,$23,$24,$25,$26,$27,$28,$29
        .byte   $2A,$2B,$2C,$2D,$2E,$2F,$31,$FF
        .byte   $FF,$34,$41,$32,$33,$42,$33,$43
        .byte   $34,$42,$FF,$FF,$F6,$4C,$4D,$4E
        .byte   $4E,$4F,$FF,$FF,$02,$45,$3A,$3E
        .byte   $38,$46,$FF,$FF,$30,$3D,$3C,$38
        .byte   $3A,$3F,$48,$3A,$39,$FF,$FF,$01
        .byte   $37,$4A,$FF,$FF,$01,$3F,$3C,$3F
        .byte   $49,$3A,$3F,$39,$46,$FF,$FF,$0F
        .byte   $46,$3B,$FF,$FF,$01,$36,$3E,$3A
        .byte   $32,$3C,$38,$36,$FF,$FF,$01,$3C
        .byte   $3F,$38,$4B,$FF,$FF,$48,$FF,$00
        .byte   $40,$FF,$FF,$00
; ----------------------------------------------------------------------------
ending_sequence:
        jsr     ending_setup
        lda     #$06
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$07
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$0B
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$0C
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$0D
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$0E
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        lda     #$0F
        jsr     draw_layout_number
        jsr     request_vram_queue
        jsr     ending_wait_button
        jsr     draw_ending_box
        ldx     #$01
        jsr     ending_wait_button
LB2C6:  jmp     LB2C6

; ----------------------------------------------------------------------------
ending_wait_button:  ldy     #$01
LB2CB:  lda     LB312,y
        sta     palette_buffer+15
        jsr     request_palette_upload
        jsr     wait_6_frames_2
        iny
        cpy     #$04
        bne     LB2CB
        cpx     #$01
        beq     LB311
LB2E0:  jsr     read_joypads
        lda     joy_pressed
        bpl     LB2E0
        ldy     #$03
LB2E9:  lda     LB312,y
        sta     palette_buffer+15
        jsr     request_palette_upload
        jsr     wait_6_frames_2
        dey
        bpl     LB2E9
        lda     #$16
        sta     $1E
        lda     #$B3
        sta     $1F
        lda     #$A3
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_fill
        jsr     request_vram_queue
        jsr     wait_vblank
LB311:  rts

; ----------------------------------------------------------------------------
LB312:  .byte   $0F,$10,$20,$30,$1A,$06,$FF
; ----------------------------------------------------------------------------
wait_6_frames_2:  jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
ending_setup:  lda     #$0E
        jsr     play_sound
        jsr     fade_out_palette
        lsr     system_flags
        asl     system_flags
        jsr     clear_and_upload_sprites
        jsr     draw_ending_screen
        jsr     set_chr_banks_inline
        .byte   $EC,$ED,$EE,$EF,$F0,$F1,$F2,$F3
; ----------------------------------------------------------------------------
        jsr     wait_vblank
        jsr     wait_vblank
        ldx     #$00
LB351:  lda     LB371,x
        sta     oam_buffer,x
        inx
        cpx     #$04
        bne     LB351
        jsr     request_oam_dma
        lda     system_flags
        ora     #$01
        sta     system_flags
        lda     #$75
        sta     $1E
        lda     #$B3
        sta     $1F
        jsr     fade_in_palette
        rts

; ----------------------------------------------------------------------------
LB371:  .byte   $A0,$00,$23,$10,$0F,$17,$27,$36
        .byte   $0F,$13,$01,$27,$0F,$17,$01,$07
        .byte   $0F,$0F,$22,$0F,$0F,$17,$27,$36
        .byte   $0F,$13,$01,$27,$0F,$17,$01,$07
        .byte   $0F,$0F,$22,$0F
; ----------------------------------------------------------------------------
draw_ending_box:  lda     #$AC
        sta     $1E
        lda     #$B3
        sta     $1F
        lda     #$EB
        sta     $1C
        lda     #$22
        sta     $1D
        jsr     queue_vram_block
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
        .byte   $0A,$02,$03,$04,$05,$06,$A9,$07
        .byte   $08,$09,$0A,$0B,$13,$14,$15,$16
        .byte   $A9,$17,$18,$19,$1A,$1B
; ----------------------------------------------------------------------------
draw_ending_screen:  jsr     draw_rle_screen_2000
        .byte   $FF,$0F,$20,$C0,$C1,$C2,$C3,$FF
        .byte   $0F,$04,$C0,$C1,$C2,$C3,$FF,$0F
        .byte   $09,$C0,$C1,$C2,$C3,$C0,$C1,$C2
        .byte   $C3,$FF,$0F,$08,$0B,$FF,$0F,$05
        .byte   $C0,$C1,$C2,$C3,$C0,$C1,$C2,$C3
        .byte   $C0,$FF,$0F,$0C,$C0,$C1,$C2,$0F
        .byte   $1A,$1B,$FF,$0F,$11,$C0,$C1,$C0
        .byte   $C1,$FF,$0F,$0C,$C0,$C1,$C2,$C3
        .byte   $0F,$06,$07,$08,$0F,$C0,$C1,$C2
        .byte   $C3,$C0,$C1,$C2,$C3,$FF,$0F,$08
        .byte   $C0,$C1,$C2,$C0,$C1,$C2,$C3,$FF
        .byte   $0F,$05,$16,$17,$18,$19,$FF,$0F
        .byte   $07,$9D,$9E,$9F,$B0,$0F,$C0,$C1
        .byte   $C2,$C3,$FF,$0F,$06,$C0,$C1,$FF
        .byte   $0F,$03,$25,$26,$27,$28,$29,$FF
        .byte   $0F,$06,$80,$81,$82,$83,$09,$FF
        .byte   $0F,$10,$35,$36,$37,$38,$39,$3A
        .byte   $3B,$FF,$0F,$04,$90,$91,$92,$93
        .byte   $0A,$FF,$0F,$10,$45,$46,$47,$48
        .byte   $49,$4A,$4B,$FF,$0F,$04,$A0,$A1
        .byte   $A2,$A3,$0F,$9C,$31,$32,$60,$B0
        .byte   $FF,$0F,$0B,$55,$56,$57,$58,$59
        .byte   $5A,$5B,$FF,$0F,$04,$B0,$B1,$B2
        .byte   $FF,$0F,$03,$51,$52,$53,$C0,$FF
        .byte   $0F,$0A,$64,$65,$66,$67,$68,$69
        .byte   $6A,$6B,$FF,$0F,$06,$6D,$FF,$0F
        .byte   $03,$61,$62,$63,$FF,$0F,$06,$03
        .byte   $0F,$05,$0F,$0F,$AC,$AD,$AE,$AF
        .byte   $BC,$BD,$BE,$BF,$FF,$0F,$06,$6D
        .byte   $FF,$0F,$03,$41,$42,$54,$0F,$0C
        .byte   $0F,$4C,$4D,$5D,$13,$0F,$23,$0F
        .byte   $0F,$84,$85,$86,$87,$88,$89,$8A
        .byte   $8B,$FF,$0F,$06,$6D,$0F,$0F,$B0
        .byte   $B1,$2A,$2B,$0F,$1C,$0E,$1E,$4E
        .byte   $6F,$23,$0E,$23,$0E,$0E,$94,$95
        .byte   $96,$97,$98,$99,$9A,$9B,$30,$3F
        .byte   $0E,$0E,$1D,$0E,$CD,$15,$4E,$0E
        .byte   $0E,$CD,$0E,$0E,$2C,$2D,$2E,$5E
        .byte   $A9,$33,$15,$23,$1F,$2D,$A4,$A5
        .byte   $A6,$A7,$A8,$B3,$AA,$AB,$40,$4F
        .byte   $2C,$1F,$15,$1F,$8D,$15,$5E,$14
        .byte   $1F,$8D,$1F,$1F,$3C,$3D,$3E,$6E
        .byte   $A9,$43,$23,$23,$15,$15,$B4,$B5
        .byte   $B6,$B7,$B8,$B9,$BA,$BB,$50,$5F
        .byte   $2C,$14,$14,$5F,$CE,$14,$6E,$14
        .byte   $14,$6C,$14,$2F,$FF,$A9,$0A,$E2
        .byte   $C5,$C6,$C7,$C8,$C9,$CA,$CB,$FF
        .byte   $A9,$06,$8E,$E2,$A9,$A9,$E2,$CC
        .byte   $FF,$A9,$0C,$E2,$D5,$D6,$D7,$D8
        .byte   $D9,$DA,$DB,$FF,$A9,$06,$6C,$E2
        .byte   $A9,$A9,$E2,$8C,$FF,$A9,$0C,$E2
        .byte   $E5,$E6,$E7,$E8,$E9,$EA,$E2,$FF
        .byte   $A9,$06,$CC,$E2,$A9,$A9,$E2,$E2
        .byte   $FF,$A9,$0C,$E2,$F5,$F6,$F7,$F8
        .byte   $F9,$FA,$E2,$FF,$A9,$06,$8C,$E2
        .byte   $A9,$A9,$E2,$E2,$FF,$A9,$04,$11
        .byte   $FF,$01,$1A,$02,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$10
        .byte   $FF,$A9,$1A,$12,$FF,$A9,$04,$20
        .byte   $FF,$21,$1A,$22,$FF,$A9,$22,$AA
        .byte   $56,$FF,$55,$06,$AA,$66,$95,$8A
        .byte   $A5,$65,$55,$59,$AA,$66,$FF,$AA
        .byte   $03,$A5,$55,$55,$99,$75,$A9,$AA
        .byte   $6A,$5A,$95,$95,$FF,$FF,$02,$BB
        .byte   $AA,$EE,$FF,$FF,$01,$DD,$CD,$FF
        .byte   $FF,$10,$FF,$0F,$08,$FF,$FF,$00
; 1539 bytes of $FF
unused_6_B5FD:
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
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF
; ----------------------------------------------------------------------------
; drawn by 7:FBF0
screen_layouts:
        .addr   layout_00
        .addr   layout_01
        .addr   layout_02
        .addr   layout_03
        .addr   layout_04
        .addr   layout_05
        .addr   layout_06
        .addr   layout_07
        .addr   layout_08
        .addr   layout_09
        .addr   layout_0A
        .addr   layout_0B
        .addr   layout_0C
        .addr   layout_0D
        .addr   layout_0E
        .addr   layout_0F

; screen layouts: 491 bytes from your ROM
layouts_data:
        .incbin "layouts_data.bin"
layout_00       := layouts_data + $0000
layout_01       := layouts_data + $0004
layout_02       := layouts_data + $000D
layout_03       := layouts_data + $0016
layout_04       := layouts_data + $001F
layout_05       := layouts_data + $0028
layout_08       := layouts_data + $0031
layout_09       := layouts_data + $003A
layout_0A       := layouts_data + $0043
layout_06       := layouts_data + $004C
layout_07       := layouts_data + $008E
layout_0B       := layouts_data + $00D4
layout_0C       := layouts_data + $011E
layout_0D       := layouts_data + $0144
layout_0E       := layouts_data + $017E
layout_0F       := layouts_data + $01C8

; ----------------------------------------------------------------------------
; 501 unused bytes of $FF
free_space_6:
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
        .byte   $FF,$FF,$FF,$FF,$FF
