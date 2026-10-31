; Deja Vu (USA), NES: PRG bank 7 ($C000-$FFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK7"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
L0010           := $0010
L0051           := $0051
; ----------------------------------------------------------------------------
change_scene:  txa
        pha
        tya
        pha
        jsr     load_dissolve_pattern
        jsr     clear_and_upload_sprites
        lda     #$01
        sta     scene_blank
        jsr     draw_scene_dissolve
        jsr     select_scene_bank
        lda     #$61
        jsr     far_call_8000
        jsr     load_scene_attrs
        jsr     request_attr_upload
        jsr     wait_vblank
        jsr     wait_vblank
        lda     #$60
        jsr     far_call_8000
        jsr     request_palette_upload
        lda     #$00
        sta     scene_blank
        jsr     draw_scene_dissolve
        jsr     apply_room_settings
        jsr     pop_bank
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
change_scene_instant:  txa
        pha
        tya
        pha
        jsr     clear_and_upload_sprites
        jsr     blacken_palette_colors
        jsr     select_scene_bank
        jsr     wait_vblank
        lda     #$61
        jsr     far_call_8000
        jsr     load_scene_attrs
        jsr     request_attr_upload
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     apply_room_settings
        jsr     draw_scene
        lda     #$60
        jsr     far_call_8000
        jsr     request_palette_upload
        jsr     pop_bank
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; draw the picture for $0515
load_scene:
        txa
        pha
        tya
        pha
        jsr     clear_and_upload_sprites
        jsr     select_scene_bank
        lda     #$61
        jsr     far_call_8000
        jsr     load_scene_attrs
        jsr     request_attr_upload
        jsr     wait_vblank
        jsr     apply_room_settings
        jsr     wait_vblank
        jsr     draw_scene
        jsr     pop_bank
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; switch to the bank holding $0515; $75 = index in it
select_scene_bank:
        lda     scene
        sta     scene_index
        cmp     #$4C
        bcs     LC0AE
        lda     #$00
        jsr     push_and_switch_bank
        rts

; ----------------------------------------------------------------------------
LC0AE:  cmp     #$98
        bcs     LC0C0
        lda     #$01
        jsr     push_and_switch_bank
        lda     scene
        sec
        sbc     #$4C
        sta     scene_index
        rts

; ----------------------------------------------------------------------------
LC0C0:  lda     #$02
        jsr     push_and_switch_bank
        lda     scene
        sec
        sbc     #$98
        sta     scene_index
        rts

; ----------------------------------------------------------------------------
; ($55) = picture $75 of the current bank
scene_pointer:
        lda     scene_index
        asl     a
        tay
        lda     $8000,y
        sta     data_ptr
        lda     $8001,y
        sta     data_ptr+1
        rts

; ----------------------------------------------------------------------------
; tile by tile; all $FF when $7D is set
draw_scene_dissolve:
        lda     #$27
        jsr     play_sound
        jsr     scene_pointer
        lda     #$00
        sta     $78
LC0E9:  lda     #$00
        sta     $77
LC0ED:  lda     #$6B
        jsr     far_call_8000
        ldx     #$00
LC0F4:  lsr     $84
        ror     $83
        bcc     LC0FD
        jsr     dissolve_draw_tile
LC0FD:  inx
        cpx     #$0E
        bne     LC0F4
        inc     $77
        lda     $77
        cmp     #$0E
        bne     LC0ED
        jsr     request_vram_queue
        jsr     wait_vblank
        inc     $78
        lda     $78
        cmp     #$06
        bne     LC0E9
        rts

; ----------------------------------------------------------------------------
dissolve_draw_tile:  jsr     dissolve_tile_value
        jsr     dissolve_tile_address
        lda     #$01
        sta     $5B
        sta     $5C
        lda     #$5B
        sta     $1E
        lda     #$00
        sta     $1F
        jsr     queue_vram_block
        rts

; ----------------------------------------------------------------------------
dissolve_tile_value:  lda     $77
        asl     a
        sta     $5B
        asl     a
        asl     a
        asl     a
        sec
        sbc     $5B
        sta     $5B
        txa
        clc
        adc     $5B
        tay
        lda     (data_ptr),y
        sta     $5D
        lda     scene_blank
        beq     LC14F
        lda     #$FF
        sta     $5D
LC14F:  rts

; ----------------------------------------------------------------------------
dissolve_tile_address:  lda     $77
        sta     $1C
        lda     #$00
        sta     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        txa
        clc
        adc     $1C
        sta     $1C
        lda     #$00
        adc     $1D
        sta     $1D
        lda     $1C
        clc
        adc     #$82
        sta     $1C
        lda     $1D
        adc     #$20
        sta     $1D
        rts

; ----------------------------------------------------------------------------
load_scene_attrs:
        jsr     scene_pointer
        lda     data_ptr
        clc
        adc     #$C4
        sta     data_ptr
        lda     data_ptr+1
        adc     #$00
        sta     data_ptr+1
        ldy     #$00
LC198:  ldx     scene_attr_positions,y
        lda     (data_ptr),y
        sta     attr_buffer_2000,x
        iny
        cpy     #$10
        bcc     LC198
        rts

; ----------------------------------------------------------------------------
; where each attribute byte goes, from $0300
scene_attr_positions:
        .byte   $08,$09,$0A,$0B,$10,$11,$12,$13
        .byte   $18,$19,$1A,$1B,$20,$21,$22,$23
; ----------------------------------------------------------------------------
blacken_palette_colors:  ldy     #$00
LC1B8:  lda     #$0F
        sta     palette_buffer+1,y
        sta     palette_buffer+5,y
        sta     palette_buffer+9,y
        iny
        cpy     #$03
        bne     LC1B8
        jsr     request_palette_upload
        rts

; ----------------------------------------------------------------------------
; all at once
draw_scene:
        jsr     scene_pointer
        jsr     copy_scene_tiles
        lda     #$0E
        sta     $0552
        sta     $0553
        lda     #$52
        sta     $1E
        lda     #$05
        sta     $1F
        lda     #$82
        sta     $1C
        lda     #$20
        sta     $1D
        jsr     queue_vram_block
        jsr     request_vram_queue
        jsr     wait_nmi_work_done
        rts

; ----------------------------------------------------------------------------
; 196 bytes to $0554
copy_scene_tiles:
        ldy     #$00
LC1F6:  lda     (data_ptr),y
        sta     scene_tiles,y
        iny
        cpy     #$C4
        bne     LC1F6
        rts

; ----------------------------------------------------------------------------
apply_room_settings:
        ldy     room
        cpy     #$33
        beq     LC210
        lda     room_settings,y
        and     #$0F
        jsr     play_sound
LC210:  rts

; ----------------------------------------------------------------------------
load_dissolve_pattern:  ldy     room
        lda     room_settings,y
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     dissolve_pattern
        rts

; ----------------------------------------------------------------------------
; per room ($0514): low nibble -> 7:F409, high -> $0530
room_settings:
        .byte   $12,$12,$12,$12,$12,$2C,$03,$03
        .byte   $03,$03,$12,$2C,$2D,$2C,$2C,$12
        .byte   $12,$2C,$2C,$2C,$2C,$2C,$2C,$2C
        .byte   $2C,$2C,$2C,$2C,$2C,$03,$2C,$2C
        .byte   $2C,$2C,$2C,$12,$03,$12,$12,$12
        .byte   $12,$12,$12,$12,$12,$12,$12,$2C
        .byte   $2C,$2C,$12,$00,$12,$12,$12,$12
        .byte   $03,$03,$03,$03,$03,$03,$2C,$12
        .byte   $12,$12,$12,$12,$12,$12,$12,$12
        .byte   $12,$12,$2C,$2C,$2C,$2C,$00,$00
; ----------------------------------------------------------------------------
test_hotspot:  tya
        pha
        ldy     #$00
LC273:  lda     (hotspot_ptr),y
        sta     $65,y
        iny
        cpy     #$04
        bcc     LC273
        jsr     cursor_in_hotspot
        pla
        tay
        rts

; ----------------------------------------------------------------------------
cursor_in_hotspot:  lda     script_flags
        and     #$FE
        sta     script_flags
        lda     cursor_x
        cmp     $65
        bcs     LC290
        rts

; ----------------------------------------------------------------------------
LC290:  cmp     $66
        bcc     LC295
        rts

; ----------------------------------------------------------------------------
LC295:  lda     cursor_y
        cmp     $67
        bcs     LC29C
        rts

; ----------------------------------------------------------------------------
LC29C:  cmp     $68
        bcc     LC2A1
        rts

; ----------------------------------------------------------------------------
LC2A1:  lda     script_flags
        ora     #$01
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
click_picture:
        lda     #$02
        jsr     push_and_switch_bank
        jsr     find_clicked_hotspot
        jsr     pop_bank
        jsr     pick_default_command
        rts

; ----------------------------------------------------------------------------
find_clicked_hotspot:  lda     #$AA
        sta     $6B
        lda     #$00
        sta     $6C
        lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LC2D2
        lda     #$AD
        sta     $6B
        lda     #$00
        sta     $6C
LC2D2:  ldy     #$00
        tya
        sta     ($6B),y
        iny
        lda     #$6F
        sta     ($6B),y
        lda     room
        asl     a
        tay
        lda     $9C00,y
        sta     hotspot_ptr
        lda     $9C01,y
        sta     hotspot_ptr+1
LC2EB:  ldy     #$00
        lda     (hotspot_ptr),y
        cmp     #$FF
        bne     LC2F4
        rts

; ----------------------------------------------------------------------------
LC2F4:  jsr     test_hotspot
        lda     script_flags
        and     #$01
        beq     LC31D
        ldy     #$04
        lda     (hotspot_ptr),y
        and     #$7F
        ldy     #$00
        sta     ($6B),y
        ldy     #$05
        jsr     jump_table
        .addr   LC365
        .addr   LC377
        .addr   LC377
        .addr   LC401
        .addr   LC433
        .addr   LC3D8
; ----------------------------------------------------------------------------
LC318:  ldy     #$00
        tya
        sta     ($6B),y
LC31D:  jsr     next_hotspot
        jmp     LC2EB

; ----------------------------------------------------------------------------
next_hotspot:  lda     hotspot_ptr
        clc
        adc     #$06
        sta     hotspot_ptr
        lda     hotspot_ptr+1
        adc     #$00
        sta     hotspot_ptr+1
        rts

; ----------------------------------------------------------------------------
LC331:  lda     scene
        cmp     #$73
        beq     LC377
        jmp     LC31D

; ----------------------------------------------------------------------------
LC33B:  lda     scene
        cmp     #$7A
        beq     LC377
        jmp     LC31D

; ----------------------------------------------------------------------------
LC345:  lda     scene
        cmp     #$40
        beq     LC377
        cmp     #$46
        beq     LC377
        cmp     #$49
        beq     LC377
        cmp     #$4C
        beq     LC377
        jmp     LC31D

; ----------------------------------------------------------------------------
LC35B:  lda     scene
        cmp     #$12
        beq     LC377
        jmp     LC31D

; ----------------------------------------------------------------------------
LC365:  lda     (hotspot_ptr),y
        cmp     #$6B
        beq     LC331
        cmp     #$5F
        beq     LC33B
        cmp     #$2E
        beq     LC345
        cmp     #$13
        beq     LC35B
LC377:  lda     (hotspot_ptr),y
        ldy     #$01
        sta     ($6B),y
        rts

; ----------------------------------------------------------------------------
LC37E:  lda     scene
        cmp     #$5E
        bcc     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC388:  lda     scene
        cmp     #$68
        bcc     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC392:  lda     scene
        cmp     #$6D
        bcc     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC39C:  lda     scene
        cmp     #$77
        beq     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3A6:  lda     scene
        cmp     #$5E
        bcs     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3B0:  lda     scene
        cmp     #$68
        bcs     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3BA:  lda     scene
        cmp     #$6D
        bcs     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3C4:  lda     scene
        cmp     #$78
        beq     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3CE:  lda     scene
        cmp     #$43
        beq     LC377
        jmp     LC318

; ----------------------------------------------------------------------------
LC3D8:  lda     (hotspot_ptr),y
        cmp     #$24
        beq     LC37E
        cmp     #$25
        beq     LC388
        cmp     #$26
        beq     LC392
        cmp     #$27
        beq     LC39C
        cmp     #$29
        beq     LC3A6
        cmp     #$2A
        beq     LC3B0
        cmp     #$2B
        beq     LC3BA
        cmp     #$2C
        beq     LC3C4
        cmp     #$47
        beq     LC3CE
        jmp     LC377

; ----------------------------------------------------------------------------
LC401:  lda     (hotspot_ptr),y
        cmp     #$2C
        bne     LC411
        lda     scene
        cmp     #$44
        beq     LC411
        jmp     LC318

; ----------------------------------------------------------------------------
LC411:  lda     (hotspot_ptr),y
        pha
        lsr     a
        lsr     a
        lsr     a
        tay
        pla
        and     #$07
        sta     $69
        lda     story_flags,y
        ldy     $69
        and     LC45A,y
        beq     LC42A
        jmp     LC318

; ----------------------------------------------------------------------------
LC42A:  ldy     #$05
        lda     (hotspot_ptr),y
        ldy     #$01
        sta     ($6B),y
        rts

; ----------------------------------------------------------------------------
LC433:  lda     (hotspot_ptr),y
        cmp     #$00
        beq     LC43D
        cmp     #$02
        bne     LC447
LC43D:  lda     scene
        cmp     #$00
        beq     LC447
        jmp     LC318

; ----------------------------------------------------------------------------
LC447:  lda     (hotspot_ptr),y
        tay
        lda     item_status,y
        and     #$01
        beq     LC454
        jmp     LC318

; ----------------------------------------------------------------------------
LC454:  tya
        ldy     #$01
        sta     ($6B),y
        rts

; ----------------------------------------------------------------------------
LC45A:  .byte   $01,$02,$04,$08,$10,$20,$40,$80
; ----------------------------------------------------------------------------
collect_room_sprites:  lda     #$02
        jsr     push_and_switch_bank
        lda     room
        asl     a
        tay
        lda     $9C00,y
        sta     hotspot_ptr
        lda     $9C01,y
        sta     hotspot_ptr+1
        lda     #$C5
        sta     $67
        lda     #$00
        sta     $68
LC47E:  ldy     #$00
        lda     (hotspot_ptr),y
        cmp     #$FF
        bne     LC48E
        ldy     #$00
        sta     ($67),y
        jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
LC48E:  ldy     #$04
        lda     (hotspot_ptr),y
        bmi     LC49A
        jsr     next_hotspot
        jmp     LC47E

; ----------------------------------------------------------------------------
LC49A:  and     #$7F
        sta     $BD
        iny
        lda     (hotspot_ptr),y
        sta     $BE
        lda     #$60
        jsr     far_call_8020
        jsr     next_hotspot
        jmp     LC47E

; ----------------------------------------------------------------------------
; A = message id, table at 5:9A00
print_message_5:
        sta     message_id
LC4B0:  lda     #$05
        sta     $84
        lda     #$00
        sta     $70
        lda     #$9A
        sta     $71
; $84 = bank, $70/$71 = table, $BF = id
print_message_from:
        txa
        pha
        tya
        pha
        lda     $84
        jsr     push_and_switch_bank
        lda     #$00
        sta     $74
        lda     message_id
        asl     a
        rol     $74
        clc
        adc     $70
        sta     $72
        lda     $71
        adc     $74
        sta     $73
        ldy     #$00
        lda     ($72),y
        sta     $00
        iny
        lda     ($72),y
        sta     $01
        jsr     print_message
        jsr     pop_bank
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; A = message id, table at 4:8000
print_message_4:
        sta     message_id
        lda     #$04
        sta     $84
        lda     #$00
        sta     $70
        lda     #$80
        sta     $71
        jsr     print_message_from
        rts

; ----------------------------------------------------------------------------
; A >= $80: 2:A500 entry A & $7F; else 5:9A00 entry A | $80
print_message_2_or_5:
        sta     message_id
        bpl     LC519
        and     #$7F
        sta     message_id
        lda     #$02
        sta     $84
        lda     #$00
        sta     $70
        lda     #$A5
        sta     $71
        jsr     print_message_from
        rts

; ----------------------------------------------------------------------------
LC519:  ora     #$80
        sta     message_id
        jmp     LC4B0

; ----------------------------------------------------------------------------
item_code_for_object:  tya
        pha
        ldy     script_table
        lda     LC551,y
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        ora     object_id
        sta     $C1
        pla
        tay
        rts

; ----------------------------------------------------------------------------
split_item_code:  sta     $BE
        tya
        pha
        lda     $BE
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        tay
        lda     LC557,y
        sta     $BD
        cmp     #$03
        beq     LC54E
        lda     $BE
        and     #$1F
        sta     $BE
LC54E:  pla
        tay
        rts

; ----------------------------------------------------------------------------
LC551:  .byte   $FF,$04,$05,$00,$03,$FF
LC557:  .byte   $03,$03,$03,$04,$01,$02
; ----------------------------------------------------------------------------
run_special_scene:  pha
        txa
        pha
        tya
        pha
        lda     repeat_sound
        pha
        jsr     enter_special_scene
        jsr     special_scene_action
        jsr     leave_special_scene
        pla
        jsr     play_sound
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
special_scene_pictures:  .byte   $97,$8F,$8F,$8F,$98,$8F,$94,$95
        .byte   $96,$98,$8F,$98,$54,$55,$8D,$53
        .byte   $40,$46,$49,$4C,$92,$93,$57,$90
        .byte   $8F,$8F,$8F
; ----------------------------------------------------------------------------
special_scene_action:  lda     special_scene
        jsr     jump_table
        .addr   LC5CE
        .addr   special_scene_with_message
        .addr   special_scene_with_message
        .addr   special_scene_with_message
        .addr   special_scene_with_message
        .addr   special_scene_with_message
        .addr   LC5E3
        .addr   LC5F4
        .addr   LC605
        .addr   LC615
        .addr   LC64E
        .addr   LC65D
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   LC65C
        .addr   special_scene_final
        .addr   special_scene_with_message
        .addr   special_scene_with_message
        .addr   special_scene_with_message
; ----------------------------------------------------------------------------
LC5CE:  lda     #$6F
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
special_scene_with_message:  lda     #$0B
        jsr     play_sound
        jsr     set_special_scene_flag
        jsr     special_scene_message
        jsr     clear_special_scene_flag
        rts

; ----------------------------------------------------------------------------
LC5E3:  lda     #$00
        jsr     play_sound
        jsr     wait_60_frames
        lda     #$20
        jsr     play_sound
        jsr     wait_60_frames
        rts

; ----------------------------------------------------------------------------
LC5F4:  lda     #$00
        jsr     play_sound
        jsr     wait_60_frames
        lda     #$1A
        jsr     play_sound
        jsr     wait_60_frames
        rts

; ----------------------------------------------------------------------------
LC605:  lda     #$00
        jsr     play_sound
        jsr     wait_60_frames
        lda     #$1F
        jsr     play_sound
        jmp     wait_180_frames

; ----------------------------------------------------------------------------
LC615:  lda     #$0B
        jsr     play_sound
        jsr     set_special_scene_flag
        lda     #$78
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LC645
        lda     #$B2
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LC63C
        lda     #$0A
        jsr     print_message_4
        jsr     clear_special_scene_flag
        rts

; ----------------------------------------------------------------------------
LC63C:  lda     #$32
        jsr     print_message_2_or_5
        jsr     clear_special_scene_flag
        rts

; ----------------------------------------------------------------------------
LC645:  lda     #$FC
        jsr     print_message_2_or_5
        jsr     clear_special_scene_flag
        rts

; ----------------------------------------------------------------------------
LC64E:  lda     #$0B
        jsr     play_sound
        jsr     set_special_scene_flag
        jsr     special_scene_message
        jsr     clear_special_scene_flag
LC65C:  rts

; ----------------------------------------------------------------------------
LC65D:  lda     #$0B
        jsr     play_sound
        jsr     set_special_scene_flag
        jsr     special_scene_message
        jsr     clear_special_scene_flag
        rts

; ----------------------------------------------------------------------------
special_scene_final:  lda     #$03
        sta     text_delay
        jsr     set_special_scene_flag
        jsr     special_scene_message
        jsr     clear_special_scene_flag
        lda     #$91
        sta     scene
        jsr     change_scene
        jsr     set_special_scene_flag
        lda     #$DD
        jsr     print_message_2_or_5
        lda     #$61
        jsr     far_call_8040
        rts

; ----------------------------------------------------------------------------
set_special_scene_flag:  ldy     special_scene
        lda     special_scene_flags,y
        jsr     set_event_flag
        lda     #$C5
        sta     $67
        lda     #$00
        sta     $68
        lda     #$00
        sta     $BD
        lda     #$6C
        sta     $BE
        lda     #$60
        jsr     far_call_8020
        ldy     #$00
        lda     #$FF
        sta     ($67),y
        rts

; ----------------------------------------------------------------------------
clear_special_scene_flag:  ldy     special_scene
        lda     special_scene_flags,y
        jsr     clear_event_flag
        jsr     collect_room_sprites
        rts

; ----------------------------------------------------------------------------
special_scene_flags:  .byte   $FF,$0B,$0D,$0E,$0F,$11,$FF,$FF
        .byte   $FF,$10,$0C,$12,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $11,$11,$11
; ----------------------------------------------------------------------------
special_scene_message:  lda     special_scene
        asl     a
        tay
        lda     LC700,y
        jsr     jump_table
        .addr   LC6EA
        .addr   LC6F1
        .addr   LC6F8
; ----------------------------------------------------------------------------
LC6EA:  lda     LC6FF,y
        jsr     print_message_5
        rts

; ----------------------------------------------------------------------------
LC6F1:  lda     LC6FF,y
        jsr     print_message_4
        rts

; ----------------------------------------------------------------------------
LC6F8:  lda     LC6FF,y
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
LC6FF:  .byte   $00
LC700:  .byte   $00,$95,$01,$83,$01,$BB,$01,$96
        .byte   $01,$1F,$00,$00,$00,$00,$00,$00
        .byte   $00,$00,$00,$A7,$02,$A8,$02,$00
        .byte   $00,$00,$00,$00,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$00,$F9,$01,$15
        .byte   $01,$1C,$00,$2F,$01
; ----------------------------------------------------------------------------
enter_special_scene:  lda     scene
        sta     saved_scene
        ldx     special_scene
        lda     special_scene_pictures,x
        sta     scene
        jsr     change_scene
        lda     #$FF
        sta     object_sprite_list
        rts

; ----------------------------------------------------------------------------
show_special_scene:  txa
        pha
        tya
        pha
        lda     scene
        sta     saved_scene
        ldx     special_scene
        lda     special_scene_pictures,x
        sta     scene
        jsr     change_scene
        lda     #$FF
        sta     object_sprite_list
        jsr     special_scene_extras
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
leave_special_scene:  lda     saved_scene
        sta     scene
        jsr     change_scene
        jsr     update_cursor_and_sprites
        rts

; ----------------------------------------------------------------------------
special_scene_extras:  lda     special_scene
        jsr     jump_table
        .addr   LC7B4
        .addr   LC7B4
        .addr   LC7B4
        .addr   LC7B4
        .addr   LC7B4
        .addr   LC7B4
        .addr   LC7B5
        .addr   LC7B8
        .addr   LC7BB
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BF
        .addr   LC7DA
        .addr   LC7BE
        .addr   LC7E0
        .addr   LC7BE
        .addr   LC7BE
        .addr   LC7BE
; ----------------------------------------------------------------------------
LC7B4:  rts

; ----------------------------------------------------------------------------
LC7B5:  jmp     LC5E3

; ----------------------------------------------------------------------------
LC7B8:  jmp     LC5F4

; ----------------------------------------------------------------------------
LC7BB:  jmp     LC605

; ----------------------------------------------------------------------------
LC7BE:  rts

; ----------------------------------------------------------------------------
LC7BF:  lda     #$00
        jsr     play_sound
        lda     #$28
        jsr     play_sound
        jsr     wait_180_frames
        lda     #$00
        jsr     play_sound
        jsr     wait_vblank
        lda     #$04
        jsr     play_sound
        rts

; ----------------------------------------------------------------------------
LC7DA:  lda     #$04
        jsr     play_sound
        rts

; ----------------------------------------------------------------------------
LC7E0:  jsr     special_scene_final
        rts

; ----------------------------------------------------------------------------
wait_180_frames:  jsr     wait_60_frames
        jsr     wait_60_frames
        jsr     wait_60_frames
        rts

; ----------------------------------------------------------------------------
wait_60_frames:  jsr     wait_30_frames
        jsr     wait_30_frames
        rts

; ----------------------------------------------------------------------------
wait_30_frames:  ldy     #$00
LC7F7:  jsr     wait_vblank
        iny
        cpy     #$1E
        bne     LC7F7
        rts

; ----------------------------------------------------------------------------
load_game:  txa
        pha
        tya
        pha
        jsr     protect_save_ram
        ldx     #$00
LC809:  lda     #$00
        sta     $64
        jsr     save_copy_address
        jsr     read_save_copy
        jsr     check_signature
        bne     LC81D
        jsr     check_checksum
        beq     LC828
LC81D:  jsr     shift_save_copies
        inx
        cpx     #$04
        bne     LC809
        jsr     reset_game_state
LC828:  jsr     save_game
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
read_save_copy:  jsr     clear_sum16
        lda     data_ptr
        sta     flag_array_ptr
        lda     data_ptr+1
        sta     flag_array_ptr+1
        ldy     #$00
LC83D:  lda     (flag_array_ptr),y
        sta     object_state,y
        jsr     add_to_sum16
        iny
        bne     LC83D
        inc     flag_array_ptr+1
LC84A:  lda     (flag_array_ptr),y
        sta     container_stack+1,y
        jsr     add_to_sum16
        iny
        cpy     #$10
        bne     LC84A
        rts

; ----------------------------------------------------------------------------
save_game:  txa
        pha
        tya
        pha
        jsr     unlock_save_ram
        lda     #$00
        sta     $64
LC863:  jsr     save_copy_address
        jsr     write_save_copy
        inc     $64
        lda     $64
        cmp     #$04
        bne     LC863
        jsr     disable_save_ram
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
write_save_copy:  jsr     clear_sum16
        ldy     #$00
LC87E:  lda     object_state,y
        sta     (data_ptr),y
        jsr     add_to_sum16
        iny
        bne     LC87E
        inc     data_ptr+1
LC88B:  lda     container_stack+1,y
        sta     (data_ptr),y
        jsr     add_to_sum16
        iny
        cpy     #$10
        bne     LC88B
        jsr     store_checksum
        jsr     store_signature
        rts

; ----------------------------------------------------------------------------
erase_save_file:  jsr     unlock_save_ram
        jsr     erase_save_copies
        jsr     disable_save_ram
        rts

; ----------------------------------------------------------------------------
check_all_save_files:  lda     save_file
        pha
        jsr     unlock_save_ram
        lda     #$00
        sta     save_file
        sta     scene_index
        jsr     check_save_file
        inc     save_file
        jsr     check_save_file
        inc     save_file
        jsr     check_save_file
        jsr     disable_save_ram
        pla
        sta     save_file
        rts

; ----------------------------------------------------------------------------
check_save_file:  jsr     clear_sum16
        lda     #$00
        sta     $64
LC8D0:  jsr     save_copy_address
        ldy     #$00
LC8D5:  lda     (data_ptr),y
        jsr     add_to_sum16
        iny
        bne     LC8D5
        inc     data_ptr+1
LC8DF:  lda     (data_ptr),y
        jsr     add_to_sum16
        iny
        cpy     #$10
        bne     LC8DF
        jsr     check_signature
        bne     LC8F3
        jsr     check_checksum
        beq     LC8FF
LC8F3:  inc     $64
        lda     $64
        cmp     #$04
        bne     LC8D0
        jsr     erase_save_copies
        rts

; ----------------------------------------------------------------------------
LC8FF:  jsr     repair_save_copies
        ldy     save_file
        lda     scene_index
        ora     save_file_bits,y
        sta     scene_index
        rts

; ----------------------------------------------------------------------------
save_file_bits:  .byte   $01,$02,$04
; ----------------------------------------------------------------------------
repair_save_copies:  lda     $64
        beq     LC94D
        jsr     save_copy_address
        ldy     #$00
LC918:  lda     (data_ptr),y
        sta     $7E00,y
        iny
        bne     LC918
        inc     data_ptr+1
        lda     (data_ptr),y
        sta     $7F00,y
        iny
        bne     LC918
        lda     #$00
        sta     $64
        jsr     save_copy_address
        ldy     #$00
LC933:  lda     $7E00,y
        sta     (data_ptr),y
        iny
        bne     LC933
        inc     data_ptr+1
LC93D:  lda     $7F00,y
        sta     (data_ptr),y
        iny
        bne     LC93D
        inc     $64
        lda     $64
        cmp     #$04
        bne     LC918
LC94D:  rts

; ----------------------------------------------------------------------------
unlock_save_ram:  jsr     wait_vblank
        lda     #$80
        sta     MMC3_RAM_PROTECT
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
protect_save_ram:  jsr     wait_vblank
        lda     #$C0
        sta     MMC3_RAM_PROTECT
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
disable_save_ram:  jsr     wait_vblank
        lda     #$40
        sta     MMC3_RAM_PROTECT
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
save_copy_address:  lda     save_file
        asl     a
        asl     a
        asl     a
        clc
        adc     #$60
        sta     data_ptr+1
        lda     #$10
        sta     data_ptr
        lda     $64
        asl     a
        clc
        adc     data_ptr+1
        sta     data_ptr+1
        lda     data_ptr
        clc
        adc     #$20
        sta     $57
        lda     data_ptr+1
        adc     #$01
        sta     $58
        lda     data_ptr
        clc
        adc     #$26
        sta     $59
        lda     data_ptr+1
        adc     #$01
        sta     $5A
        rts

; ----------------------------------------------------------------------------
shift_save_copies:  lda     #$00
        sta     $64
LC9A7:  jsr     save_copy_address
        jsr     next_copy_pointer
        jsr     copy_save_copy
        inc     $64
        lda     $64
        cmp     #$04
        bne     LC9A7
        lda     #$03
        sta     $64
        jsr     erase_save_copy
        rts

; ----------------------------------------------------------------------------
next_copy_pointer:  lda     data_ptr
        sta     $5B
        lda     data_ptr+1
        clc
        adc     #$02
        sta     $5C
        rts

; ----------------------------------------------------------------------------
copy_save_copy:  ldy     #$00
LC9CE:  lda     ($5B),y
        sta     (data_ptr),y
        iny
        bne     LC9CE
        inc     $5C
        inc     data_ptr+1
LC9D9:  lda     ($5B),y
        sta     (data_ptr),y
        iny
        bne     LC9D9
        rts

; ----------------------------------------------------------------------------
erase_save_copies:  lda     #$00
        sta     $64
LC9E5:  jsr     erase_save_copy
        inc     $64
        lda     $64
        cmp     #$04
        bne     LC9E5
        rts

; ----------------------------------------------------------------------------
erase_save_copy:  jsr     save_copy_address
        lda     data_ptr
        sta     $57
        lda     data_ptr+1
        clc
        adc     #$01
        sta     $58
        ldy     #$00
LCA01:  lda     #$00
        sta     (data_ptr),y
        sta     ($57),y
        iny
        bne     LCA01
        rts

; ----------------------------------------------------------------------------
clear_sum16:  lda     #$00
        sta     $65
        sta     $66
        rts

; ----------------------------------------------------------------------------
add_to_sum16:  clc
        adc     $65
        sta     $65
        lda     #$00
        adc     $66
        sta     $66
        rts

; ----------------------------------------------------------------------------
store_checksum:  ldy     #$00
        lda     $65
        sta     ($59),y
        iny
        lda     $66
        sta     ($59),y
        rts

; ----------------------------------------------------------------------------
check_checksum:  ldy     #$00
        lda     ($59),y
        cmp     $65
        bne     LCA3C
        iny
        lda     ($59),y
        cmp     $66
        bne     LCA3C
        lda     #$00
        rts

; ----------------------------------------------------------------------------
LCA3C:  lda     #$FF
        rts

; ----------------------------------------------------------------------------
check_signature:  ldy     #$00
LCA41:  lda     ($57),y
        cmp     save_signature,y
        bne     LCA50
        iny
        cpy     #$06
        bne     LCA41
        lda     #$00
        rts

; ----------------------------------------------------------------------------
LCA50:  lda     #$FF
        rts

; ----------------------------------------------------------------------------
save_signature:  .byte   $44,$45,$4A,$41,$56,$55
; ----------------------------------------------------------------------------
store_signature:  ldy     #$00
LCA5B:  lda     save_signature,y
        sta     ($57),y
        iny
        cpy     #$06
        bne     LCA5B
        rts

; ----------------------------------------------------------------------------
reset_game_state:  ldy     #$00
LCA68:  lda     #$00
        sta     object_state,y
        iny
        bne     LCA68
LCA70:  sta     container_stack+1,y
        iny
        cpy     #$10
        bne     LCA70
        jsr     new_game
        rts

; ----------------------------------------------------------------------------
backup_state_to_save_ram:  txa
        pha
        tya
        pha
        lda     #$B0
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LCABF
        jsr     unlock_save_ram
        jsr     clear_sum16
        ldy     #$00
LCA93:  lda     object_state,y
        sta     $7E00,y
        jsr     add_to_sum16
        iny
        bne     LCA93
LCA9F:  lda     container_stack+1,y
        sta     $7F00,y
        jsr     add_to_sum16
        iny
        cpy     #$10
        bne     LCA9F
        lda     $65
        sta     $7F36
        lda     $66
        sta     $7F37
        jsr     disable_save_ram
LCABA:  pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
LCABF:  lda     #$B0
        jsr     clear_event_flag
        jmp     LCABA

; ----------------------------------------------------------------------------
restore_state_backup:
        txa
        pha
        tya
        pha
        jsr     protect_save_ram
        jsr     clear_sum16
        ldy     #$00
LCAD3:  lda     $7E00,y
        sta     object_state,y
        jsr     add_to_sum16
        iny
        bne     LCAD3
LCADF:  lda     $7F00,y
        sta     container_stack+1,y
        jsr     add_to_sum16
        iny
        cpy     #$10
        bne     LCADF
        lda     $7F36
        cmp     $65
        bne     LCB00
        lda     $7F37
        cmp     $66
        bne     LCB00
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
LCB00:  jsr     load_game
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
pick_default_command:  lda     #$80
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LCB14
        rts

; ----------------------------------------------------------------------------
LCB14:  lda     command
        cmp     #$FF
        beq     LCB1B
        rts

; ----------------------------------------------------------------------------
LCB1B:  lda     #$BF
        jsr     set_event_flag
        lda     script_table
        jsr     jump_table
        .addr   LCB31
        .addr   LCB31
        .addr   LCB31
        .addr   LCB31
        .addr   LCB31
        .addr   LCB36
; ----------------------------------------------------------------------------
LCB31:  lda     #$00
        sta     command
        rts

; ----------------------------------------------------------------------------
LCB36:  lda     object_id
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        bne     LCB46
        lda     #$03
        sta     command
        rts

; ----------------------------------------------------------------------------
LCB46:  lda     #$0A
        sta     command
        rts

; ----------------------------------------------------------------------------
run_command:
        lda     #$63
        jsr     far_call_8020
        lda     script_slot
        cmp     #$FF
        bne     script_start
        lda     #$64
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
; continue a script after a yield (opcode $28/$48)
script_resume:
        lda     #$03
        jsr     push_and_switch_bank
        jmp     script_loop

; ----------------------------------------------------------------------------
; pick a script by $AA/$AB/$BC from the tables in bank 3
script_start:
        lda     #$03
        jsr     push_and_switch_bank
        lda     script_table
        and     #$7F
        asl     a
        tay
        lda     $8000,y
        sta     script_ptr
        lda     $8001,y
        sta     script_ptr+1
        lda     #$00
        sta     $65
        lda     object_id
        asl     a
        rol     $65
        clc
        adc     script_ptr
        sta     script_ptr
        lda     script_ptr+1
        adc     $65
        sta     script_ptr+1
        ldy     #$00
        lda     script_slot
        asl     a
        sta     $65
        lda     (script_ptr),y
        clc
        adc     $65
        pha
        iny
        lda     (script_ptr),y
        adc     #$00
        sta     script_ptr+1
        pla
        sta     script_ptr
        ldy     #$00
        lda     (script_ptr),y
        pha
        iny
        lda     (script_ptr),y
        sta     script_ptr+1
        pla
        sta     script_ptr
; runs opcodes until $FF
script_loop:
        jsr     script_fetch_byte
        cmp     #$FF
        bne     script_run_opcode
script_end:  jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
script_run_opcode:  jsr     script_dispatch
        jmp     script_loop

; ----------------------------------------------------------------------------
; A = byte at ($B6), pointer advances
script_fetch_byte:
        ldy     #$00
        lda     (script_ptr),y
        sta     script_args
        lda     #$01
; ($B6) += A
script_advance:
        clc
        adc     script_ptr
        sta     script_ptr
        lda     script_ptr+1
        adc     #$00
        sta     script_ptr+1
        lda     script_args
        rts

; ----------------------------------------------------------------------------
; operand bytes into $B8-$B9
script_fetch_2:
        ldy     #$00
        lda     (script_ptr),y
        sta     script_args
        iny
        lda     (script_ptr),y
        sta     script_args+1
        lda     #$02
        jmp     script_advance

; ----------------------------------------------------------------------------
; into $B8-$BA
script_fetch_3:
        ldy     #$00
        lda     (script_ptr),y
        sta     script_args
        iny
        lda     (script_ptr),y
        sta     script_args+1
        iny
        lda     (script_ptr),y
        sta     script_args+2
        lda     #$03
        jmp     script_advance

; ----------------------------------------------------------------------------
; into $B8-$BB
script_fetch_4:
        ldy     #$00
        lda     (script_ptr),y
        sta     script_args
        iny
        lda     (script_ptr),y
        sta     script_args+1
        iny
        lda     (script_ptr),y
        sta     script_args+2
        iny
        lda     (script_ptr),y
        sta     script_args+3
        lda     #$04
        jmp     script_advance

; ----------------------------------------------------------------------------
; A = opcode; 105-entry jump table follows
script_dispatch:
        jsr     jump_table
        .addr   op_00_message_5
        .addr   op_01_message_4
        .addr   op_02_message_2_or_5
        .addr   script_op_goto_if
        .addr   script_op_goto_if
        .addr   script_op_goto_if_not
        .addr   op_06
        .addr   op_07_test_story_flag_of_object
        .addr   op_08_set_story_flag_of_object
        .addr   op_09_clear_story_flag_of_object
        .addr   op_0A
        .addr   op_0B
        .addr   op_0C
        .addr   op_0D
        .addr   op_0E
        .addr   op_0F
        .addr   op_10
        .addr   op_11
        .addr   op_12
        .addr   op_13_test_door_flag_of_object
        .addr   op_14_set_door_flag_of_object
        .addr   op_15_clear_door_flag_of_object
        .addr   op_16_test_event_flag
        .addr   op_17_set_event_flag
        .addr   op_18_clear_event_flag
        .addr   op_19
        .addr   op_1A
        .addr   op_1B
        .addr   script_op_goto
        .addr   op_1D
        .addr   op_1E
        .addr   op_1F
        .addr   op_20_test_flag_045C_of_object
        .addr   op_21_set_flag_045C_of_object
        .addr   op_22_clear_flag_045C_of_object
        .addr   op_23_test_flag_047A_of_object
        .addr   op_24_set_flag_047A_of_object
        .addr   op_25_clear_flag_047A_of_object
        .addr   op_26
        .addr   op_27
        .addr   script_op_yield
        .addr   script_op_switch
        .addr   op_2A
        .addr   op_2B
        .addr   op_2C
        .addr   op_2D
        .addr   op_2E_test_door_flag
        .addr   op_2F_set_door_flag
        .addr   op_30_clear_door_flag
        .addr   op_31
        .addr   op_32
        .addr   op_33
        .addr   op_34
        .addr   op_35
        .addr   op_36
        .addr   op_37
        .addr   op_38
        .addr   op_39
        .addr   op_3A
        .addr   op_3B
        .addr   op_3C
        .addr   op_3D
        .addr   op_3E
        .addr   op_3F
        .addr   op_40
        .addr   op_41
        .addr   op_42
        .addr   op_43
        .addr   op_44
        .addr   op_45
        .addr   op_46_special_scene
        .addr   op_47
        .addr   op_48_yield_b5_80
        .addr   op_49
        .addr   op_4A
        .addr   op_4B
        .addr   op_4C
        .addr   op_4D
        .addr   op_4E
        .addr   op_4F
        .addr   op_50
        .addr   op_51
        .addr   op_52_test_story_flag
        .addr   op_53_set_flag_045C
        .addr   op_54_set_flag_047A
        .addr   op_55
        .addr   op_56
        .addr   op_57
        .addr   op_58
        .addr   op_59
        .addr   op_5A
        .addr   op_5B
        .addr   op_5C
        .addr   op_5D
        .addr   op_5E
        .addr   op_5F
        .addr   op_60
        .addr   op_61
        .addr   op_62
        .addr   op_63
        .addr   op_64
        .addr   script_op_push_goto
        .addr   script_op_pop_goto
        .addr   op_67
        .addr   op_68
; ----------------------------------------------------------------------------
op_68:  jsr     script_fetch_byte
LCCEF:  jsr     wait_vblank
        dec     script_args
        bne     LCCEF
        rts

; ----------------------------------------------------------------------------
op_67:  lda     coins
        cmp     #$02
        bcs     LCD02
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LCD02:  lda     #$FE
        sta     script_args
        jsr     add_coins_and_show
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_64:  lda     store_page
        cmp     #$0C
        bcc     LCD1D
        lda     #$73
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LCD1D:  jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_63:  jsr     script_condition_false
        lda     #$07
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        beq     LCD3E
        lda     #$07
        jsr     find_and_show_item
        lda     #$07
        jsr     clear_story_flag
        lda     #$07
        jsr     set_flag_045C
LCD3E:  jsr     script_condition_false
        lda     #$54
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        beq     LCD5B
        lda     #$54
        jsr     find_and_show_item
        lda     #$54
        jsr     clear_story_flag
        lda     #$07
        jsr     set_flag_045C
LCD5B:  lda     #$00
        sta     coins
        lda     #$00
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page_next
        lda     #$05
        sta     script_table
        lda     #$16
        sta     object_id
        jsr     enter_room
        rts

; ----------------------------------------------------------------------------
show_found_page_next:  lda     $5F
        cmp     inventory_page
        beq     LCD8B
        bcc     LCD91
        sta     inventory_page
        inc     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LCD8B:  lda     #$6A
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LCD91:  sta     inventory_page
        dec     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
op_61:  jsr     script_condition_false
        lda     $04A8
        bne     LCDA6
        rts

; ----------------------------------------------------------------------------
LCDA6:  lda     #$03
        sta     script_table
        lda     $04A8
        sta     object_id
        jsr     clear_flag_045C
        jsr     op_1D
        lda     $04B1
        sta     inventory_entry
        lda     #$00
        sta     $04A8
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_60:  lda     script_table
        pha
        lda     switch_key
        sta     script_table
        pla
        sta     switch_key
        lda     object_id
        pha
        lda     switch_key+1
        sta     object_id
        pla
        sta     switch_key+1
        rts

; ----------------------------------------------------------------------------
op_5F:  jsr     script_fetch_byte
        jsr     play_sound
        rts

; ----------------------------------------------------------------------------
; opcode $29
script_op_switch:
        jsr     script_fetch_4
        lda     script_args
        cmp     #$FE
        beq     LCDF3
        cmp     switch_key
        bne     script_op_switch
        lda     script_args+1
        cmp     switch_key+1
        bne     script_op_switch
LCDF3:  lda     script_args+2
        sta     script_ptr
        lda     script_args+3
        sta     script_ptr+1
        lda     #$80
        jsr     clear_event_flag
        rts

; ----------------------------------------------------------------------------
; opcode $28
script_op_yield:
        lda     #$80
        jsr     set_event_flag
        pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
op_31:  tya
        pha
        jsr     choice_is_table_2
        pla
        tay
        rts

; ----------------------------------------------------------------------------
choice_is_table_2:  jsr     script_condition_false
        lda     switch_key
        cmp     #$02
        bne     LCE20
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LCE20:  lda     #$02
        jsr     push_and_switch_bank
        lda     room
        asl     a
        tay
        lda     $9C00,y
        sta     hotspot_ptr
        lda     $9C01,y
        sta     hotspot_ptr+1
LCE34:  ldy     #$00
        lda     (hotspot_ptr),y
        cmp     #$FF
        beq     LCE68
        ldy     #$04
        lda     (hotspot_ptr),y
        and     #$7F
        cmp     script_table
        bne     LCE74
        iny
        lda     (hotspot_ptr),y
        cmp     object_id
        bne     LCE74
        jsr     test_hotspot
        lda     script_flags
        and     #$01
        beq     LCE74
        ldy     #$04
        lda     (hotspot_ptr),y
        sta     switch_key
        iny
        lda     (hotspot_ptr),y
        sta     switch_key+1
        jsr     pop_bank
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LCE68:  lda     #$80
        jsr     clear_event_flag
        jsr     pop_bank
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LCE74:  jsr     next_hotspot
        jmp     LCE34

; ----------------------------------------------------------------------------
op_32:  lda     script_table
        cmp     #$03
        bne     LCEBA
        lda     object_id
        cmp     #$00
        bne     LCE98
        lda     coins
        bne     LCEAD
LCE8B:  lda     #$29
        jsr     print_message_2_or_5
        lda     #$80
        jsr     clear_event_flag
        jmp     LCED7

; ----------------------------------------------------------------------------
LCE98:  cmp     #$08
        bne     LCEA4
        lda     $051C
        bne     LCEAD
        jmp     LCE8B

; ----------------------------------------------------------------------------
LCEA4:  cmp     #$2D
        bne     LCEBA
        lda     $051D
        beq     LCE8B
LCEAD:  lda     #$80
        jsr     clear_event_flag
        lda     #$7D
        jsr     print_message_2_or_5
        jmp     LCED7

; ----------------------------------------------------------------------------
LCEBA:  tya
        pha
        txa
        pha
        lda     script_flags
        and     #$BF
        sta     script_flags
        jsr     op_32_body
        lda     #$80
        jsr     clear_event_flag
        pla
        tax
        pla
        tay
        lda     script_flags
        and     #$40
        bne     LCED7
        rts

; ----------------------------------------------------------------------------
LCED7:  lda     script_flags
        and     #$BF
        sta     script_flags
        pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
op_32_body:  jsr     script_condition_false
        lda     switch_key
        sec
        sbc     #$02
        jsr     jump_table
        .addr   LCEF3
        .addr   LCF5F
        .addr   LCF73
; ----------------------------------------------------------------------------
LCEF3:  lda     switch_key+1
        cmp     #$04
        beq     LCF5B
        asl     a
        asl     a
        asl     a
        tay
        ldx     #$00
LCEFF:  lda     container_items,y
        cmp     #$FF
        beq     LCF18
        iny
        inx
        cpx     #$07
        bcc     LCEFF
        lda     #$76
        jsr     print_message_2_or_5
        lda     script_flags
        ora     #$40
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
LCF18:  jsr     item_code_for_object
        lda     $C1
        sta     container_items,y
op_5B:  lda     script_table
        cmp     #$03
        bne     LCF33
        lda     object_id
        jsr     clear_story_flag
        lda     object_id
        jsr     set_flag_045C
        jmp     LCF3D

; ----------------------------------------------------------------------------
LCF33:  lda     object_id
        jsr     clear_item_obtained
        lda     object_id
        jsr     set_flag_047A
LCF3D:  jsr     op_49
        lda     script_table
        sta     $BD
        lda     object_id
        sta     $BE
        lda     #$62
        jsr     far_call_8040
        lda     $C0
        sta     text_item
        lda     #$05
        jsr     print_message_2_or_5
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LCF5B:  jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LCF5F:  jsr     op_49
        lda     object_id
        jsr     clear_story_flag
        jsr     op_27
        lda     #$58
        jsr     print_message_2_or_5
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LCF73:  jsr     op_49
        lda     object_id
        jsr     clear_item_obtained
        jsr     op_27
        lda     #$58
        jsr     print_message_2_or_5
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_00_message_5:  jsr     script_fetch_byte
        jsr     print_message_5
        rts

; ----------------------------------------------------------------------------
op_01_message_4:  jsr     script_fetch_byte
        jsr     print_message_4
        rts

; ----------------------------------------------------------------------------
op_02_message_2_or_5:  jsr     script_fetch_byte
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
; opcodes $03, $04
script_op_goto_if:
        jsr     script_fetch_2
        lda     script_flags
        and     #$04
        bne     script_goto
        rts

; ----------------------------------------------------------------------------
; opcode $05
script_op_goto_if_not:
        jsr     script_fetch_2
        lda     script_flags
        and     #$04
        beq     script_goto
        rts

; ----------------------------------------------------------------------------
; ($B6) = $B8/$B9
script_goto:
        lda     script_args
        sta     script_ptr
        lda     script_args+1
        sta     script_ptr+1
        rts

; ----------------------------------------------------------------------------
; opcode $65
script_op_push_goto:
        jsr     script_fetch_2
        lda     script_args
        jsr     stack_push
        lda     script_args+1
        jsr     stack_push
        jsr     script_goto
        rts

; ----------------------------------------------------------------------------
; opcode $66
script_op_pop_goto:
        jsr     stack_pop
        sta     script_args+1
        jsr     stack_pop
        sta     script_args
        jsr     script_goto
        rts

; ----------------------------------------------------------------------------
; opcode $1C
script_op_goto:
        jsr     script_fetch_2
        jsr     script_goto
        rts

; ----------------------------------------------------------------------------
; sets bit 2 of $B5
script_condition_true:
        lda     script_flags
        ora     #$04
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
; clears bit 2 of $B5
script_condition_false:
        lda     script_flags
        and     #$FB
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
close_container_entry:  jsr     jump_table
        .addr   LD016
        .addr   LCFFC
        .addr   LD017
        .addr   LD016
        .addr   LD021
        .addr   LD016
; ----------------------------------------------------------------------------
LCFFC:  lda     container_stack+1,y
        jsr     close_object
        lda     #$47
        jsr     clear_door_flag
        lda     #$48
        jsr     clear_door_flag
        lda     #$4A
        jsr     clear_door_flag
        lda     #$4B
        jsr     clear_door_flag
LD016:  rts

; ----------------------------------------------------------------------------
LD017:  lda     container_stack+1,y
        clc
        adc     #$20
        jsr     clear_event_flag
        rts

; ----------------------------------------------------------------------------
LD021:  lda     container_stack+1,y
        jsr     clear_item_open
        rts

; ----------------------------------------------------------------------------
close_all_containers:  ldy     #$00
LD02A:  lda     container_stack,y
        cmp     #$FF
        beq     LD03A
        jsr     close_container_entry
        iny
        iny
        cpy     #$0E
        bcc     LD02A
LD03A:  lda     #$FF
        ldy     #$00
LD03E:  sta     open_containers,y
        iny
        cpy     #$10
        bcc     LD03E
        rts

; ----------------------------------------------------------------------------
clear_listed_story_flags:  lda     $051C
        bne     LD059
        ldy     #$00
LD04E:  lda     LD06C,y
        jsr     clear_story_flag
        iny
        cpy     #$06
        bcc     LD04E
LD059:  ldy     #$06
        lda     $051D
        bne     LD06B
LD060:  lda     LD06C,y
        jsr     clear_story_flag
        iny
        cpy     #$0C
        bcc     LD060
LD06B:  rts

; ----------------------------------------------------------------------------
LD06C:  .byte   $3D,$3E,$3F,$40,$41,$42,$4A,$4F
        .byte   $50,$51,$52,$53
; ----------------------------------------------------------------------------
op_06:  lda     object_id
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        bne     LD08D
        lda     #$63
        jsr     print_message_2_or_5
        pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
LD08D:  jsr     clear_listed_story_flags
        lda     #$A9
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LD0B6
        lda     room
        cmp     #$26
        beq     LD0A9
        cmp     #$27
        beq     LD0B1
        jmp     LD0B6

; ----------------------------------------------------------------------------
LD0A9:  lda     #$81
        jsr     set_event_flag
        jmp     LD0B6

; ----------------------------------------------------------------------------
LD0B1:  lda     #$A0
        jsr     set_event_flag
LD0B6:  lda     #$A9
        jsr     clear_event_flag
        jsr     backup_state_to_save_ram
        jsr     close_all_containers
        lda     inventory_page
        cmp     #$01
        bne     LD0CD
        lda     #$67
        jsr     far_call_8020
LD0CD:  jsr     script_fetch_byte
        lda     script_args
enter_room:  sta     room
        lda     #$61
        jsr     far_call_8020
        lda     script_flags
        and     #$08
        bne     LD0EA
        lda     #$67
        jsr     far_call_8000
        lda     #$62
        jsr     far_call_8020
LD0EA:  lda     script_flags
        and     #$08
        bne     LD0F6
        jsr     change_scene
        jmp     LD0F9

; ----------------------------------------------------------------------------
LD0F6:  jsr     change_scene_instant
LD0F9:  jsr     collect_room_sprites
        lda     script_flags
        and     #$F7
        sta     script_flags
        rts

; ----------------------------------------------------------------------------
op_52_test_story_flag:  jsr     script_fetch_byte
        jmp     test_story_flag

; ----------------------------------------------------------------------------
op_07_test_story_flag_of_object:  lda     object_id
test_story_flag:  jsr     select_story_flags
        jsr     test_flag_bit
        rts

; ----------------------------------------------------------------------------
op_08_set_story_flag_of_object:  lda     object_id
set_story_flag_refresh:  jsr     select_story_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        ora     LC45A,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        jsr     collect_room_sprites
        rts

; ----------------------------------------------------------------------------
op_09_clear_story_flag_of_object:  lda     object_id
clear_story_flag:  jsr     select_story_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        and     LDC9B,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
; A = bit number; ($61) = $0450
select_story_flags:
        sta     script_args
        lda     #$50
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
; condition = bit $B8 of the array at ($61)
test_flag_bit:
        jsr     script_condition_false
        txa
        pha
        tya
        pha
        jsr     load_flag_byte
        and     LC45A,x
        beq     LD15E
        jsr     script_condition_true
LD15E:  pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; A = byte holding bit $B8, X = bit within it
load_flag_byte:
        lda     script_args
        pha
        lsr     a
        lsr     a
        lsr     a
        tay
        pla
        and     #$07
        tax
        lda     (flag_array_ptr),y
        rts

; ----------------------------------------------------------------------------
op_5D:  jsr     script_fetch_byte
        jmp     test_item_obtained

; ----------------------------------------------------------------------------
op_0A:  lda     object_id
test_item_obtained:  sta     script_args
        tya
        pha
        ldy     script_args
        jsr     script_condition_false
        lda     item_status,y
        and     #$01
        beq     LD18C
        jsr     script_condition_true
LD18C:  pla
        tay
        rts

; ----------------------------------------------------------------------------
op_0B:  lda     object_id
        sta     script_args
        tya
        pha
        ldy     script_args
        lda     item_status,y
        ora     #$01
        sta     item_status,y
        pla
        tay
        jsr     collect_room_sprites
        rts

; ----------------------------------------------------------------------------
op_0C:  lda     object_id
clear_item_obtained:  sta     script_args
        tya
        pha
        ldy     script_args
        lda     item_status,y
        and     #$FE
        sta     item_status,y
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_0D:  lda     object_id
        jmp     test_object_open

; ----------------------------------------------------------------------------
op_2B:  jsr     script_fetch_byte
test_object_open:  sta     script_args
        tya
        pha
        ldy     script_args
        jsr     script_condition_false
        lda     object_state,y
        and     #$01
        beq     LD1D3
        jsr     script_condition_true
LD1D3:  pla
        tay
        rts

; ----------------------------------------------------------------------------
op_0E:  lda     object_id
        jmp     set_object_open

; ----------------------------------------------------------------------------
op_2C:  jsr     script_fetch_byte
set_object_open:  sta     script_args
        tya
        pha
        ldy     script_args
        lda     object_state,y
        ora     #$01
        sta     object_state,y
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_0F:  lda     object_id
        jmp     close_object

; ----------------------------------------------------------------------------
op_2D:  jsr     script_fetch_byte
close_object:  sta     script_args
        tya
        pha
        ldy     script_args
        lda     object_state,y
        and     #$FE
        sta     object_state,y
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_10:  lda     object_id
        sta     script_args
        tya
        pha
        ldy     script_args
        jsr     script_condition_false
        lda     item_status,y
        and     #$02
        beq     LD21D
        jsr     script_condition_true
LD21D:  pla
        tay
        rts

; ----------------------------------------------------------------------------
op_11:  lda     object_id
        sta     script_args
        tya
        pha
        ldy     script_args
        lda     item_status,y
        ora     #$02
        sta     item_status,y
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_12:  lda     object_id
clear_item_open:  sta     script_args
        tya
        pha
        ldy     script_args
        lda     item_status,y
        and     #$FD
        sta     item_status,y
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_13_test_door_flag_of_object:  lda     object_id
        jmp     test_door_flag

; ----------------------------------------------------------------------------
op_2E_test_door_flag:  jsr     script_fetch_byte
test_door_flag:  jsr     select_door_flags
        jsr     test_flag_bit
        rts

; ----------------------------------------------------------------------------
op_14_set_door_flag_of_object:  lda     object_id
        jmp     set_door_flag

; ----------------------------------------------------------------------------
op_2F_set_door_flag:  jsr     script_fetch_byte
set_door_flag:  jsr     select_door_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        ora     LC45A,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_15_clear_door_flag_of_object:  lda     object_id
        jmp     clear_door_flag

; ----------------------------------------------------------------------------
op_30_clear_door_flag:  jsr     script_fetch_byte
clear_door_flag:  jsr     select_door_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        and     LDC9B,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
select_door_flags:
        sta     script_args
        lda     #$80
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
op_16_test_event_flag:  jsr     script_fetch_byte
test_event_flag:  jsr     select_event_flags
        jsr     test_flag_bit
        rts

; ----------------------------------------------------------------------------
op_17_set_event_flag:  jsr     script_fetch_byte
set_event_flag:  jsr     select_event_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        ora     LC45A,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_18_clear_event_flag:  jsr     script_fetch_byte
clear_event_flag:  jsr     select_event_flags
        tya
        pha
        txa
        pha
        jsr     load_flag_byte
        and     LDC9B,x
        sta     (flag_array_ptr),y
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
select_event_flags:
        sta     script_args
        lda     #$90
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_D2DB:
        sta     script_args+1
        tya
        pha
        jsr     item_in_any_container
        pla
        tay
        rts

; ----------------------------------------------------------------------------
item_in_any_container:  jsr     select_flags_0430
        ldy     #$30
        jsr     script_condition_false
LD2ED:  dey
        cpy     #$FF
        beq     LD2FB
        lda     (flag_array_ptr),y
        cmp     script_args+1
        bne     LD2ED
        jsr     script_condition_true
LD2FB:  rts

; ----------------------------------------------------------------------------
select_flags_0430:  lda     #$30
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
op_19:  jsr     script_condition_false
        jsr     script_fetch_byte
        cmp     room
        beq     LD311
        rts

; ----------------------------------------------------------------------------
LD311:  jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_33:  lda     script_flags
        ora     #$08
        sta     script_flags
op_36:  jsr     script_fetch_byte
        jmp     LD329

; ----------------------------------------------------------------------------
op_1A:  lda     script_flags
        ora     #$08
        sta     script_flags
        lda     object_id
LD329:  jsr     set_door_flag
        lda     room
        jsr     enter_room
        rts

; ----------------------------------------------------------------------------
op_34:  lda     script_flags
        ora     #$08
        sta     script_flags
op_37:  jsr     script_fetch_byte
        jmp     LD347

; ----------------------------------------------------------------------------
op_1B:  lda     script_flags
        ora     #$08
        sta     script_flags
        lda     object_id
LD347:  jsr     clear_door_flag
        lda     room
        jsr     enter_room
        rts

; ----------------------------------------------------------------------------
op_1D:  tya
        pha
        txa
        pha
        jsr     op_1D_body
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_1D_body:  lda     #$00
        sta     $6F
        lda     #$66
        jsr     far_call_8020
        lda     screen_area
        cmp     #$02
        bne     LD388
        lda     inventory_page
        cmp     #$01
        bne     LD388
        lda     #$6C
        jsr     far_call_8020
        lda     $AF
        cmp     #$02
        bne     LD384
        jsr     find_item_in_container
        jsr     take_item_from_container
LD384:  lda     #$FF
        sta     $6F
LD388:  lda     inventory_page
        cmp     store_page
        bcs     LD39E
        jsr     select_store_slot
        dec     inventory_page
        lda     #$67
        jsr     far_call_8020
        jmp     LD3A6

; ----------------------------------------------------------------------------
LD39E:  jsr     select_store_slot
        lda     #$68
        jsr     far_call_8020
LD3A6:  inc     store_entry
        lda     store_entry
        cmp     #$07
        bcc     LD3B8
        lda     #$00
        sta     store_entry
        inc     store_page
LD3B8:  lda     $6F
        bne     LD3BD
        rts

; ----------------------------------------------------------------------------
LD3BD:  lda     #$1E
        jsr     wait_frames
        lda     #$02
        sta     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
take_item_from_container:  lda     script_table
        sec
        sbc     #$03
        jsr     jump_table
        .addr   op_22_clear_flag_045C_of_object
        .addr   op_25_clear_flag_047A_of_object
; ----------------------------------------------------------------------------
find_item_in_container:  ldx     #$00
        lda     $B0
        asl     a
        asl     a
        asl     a
        tay
LD3E1:  jsr     item_code_for_object
        lda     container_items,y
        cmp     $C1
        beq     LD3F2
        iny
        inx
        cpx     #$07
        bcc     LD3E1
        rts

; ----------------------------------------------------------------------------
LD3F2:  iny
        lda     container_items,y
        dey
        sta     container_items,y
        iny
        inx
        cpx     #$07
        bcc     LD3F2
        rts

; ----------------------------------------------------------------------------
select_store_slot:  lda     store_page
        sta     inventory_page
        lda     store_entry
        sta     inventory_entry
        rts

; ----------------------------------------------------------------------------
wait_frames:  sta     $6A
LD410:  jsr     wait_vblank
        dec     $6A
        bne     LD410
        rts

; ----------------------------------------------------------------------------
op_1E:  jsr     script_fetch_byte
        lda     script_args
        sta     text_item
        rts

; ----------------------------------------------------------------------------
op_1F:  jsr     script_fetch_byte
        lda     script_args
        sta     text_number
        rts

; ----------------------------------------------------------------------------
op_20_test_flag_045C_of_object:  lda     object_id
test_flag_045C:  jsr     select_flags_045C
        jsr     test_flag_bit
        rts

; ----------------------------------------------------------------------------
op_53_set_flag_045C:  jsr     script_fetch_byte
        jmp     set_flag_045C

; ----------------------------------------------------------------------------
op_21_set_flag_045C_of_object:  lda     object_id
set_flag_045C:  jsr     select_flags_045C
        txa
        pha
        jsr     load_flag_byte
        ora     LC45A,x
        sta     (flag_array_ptr),y
        pla
        tax
        rts

; ----------------------------------------------------------------------------
op_22_clear_flag_045C_of_object:  lda     object_id
clear_flag_045C:  jsr     select_flags_045C
        pha
        txa
        jsr     load_flag_byte
        and     LDC9B,x
        sta     (flag_array_ptr),y
        pla
        tax
        rts

; ----------------------------------------------------------------------------
select_flags_045C:
        sta     script_args
        lda     #$5C
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
op_23_test_flag_047A_of_object:  lda     object_id
test_flag_047A:  jsr     select_flags_047A
        jsr     test_flag_bit
        rts

; ----------------------------------------------------------------------------
op_54_set_flag_047A:  jsr     script_fetch_byte
        jmp     set_flag_047A

; ----------------------------------------------------------------------------
op_24_set_flag_047A_of_object:  lda     object_id
set_flag_047A:  jsr     select_flags_047A
        txa
        pha
        jsr     load_flag_byte
        ora     LC45A,x
        sta     (flag_array_ptr),y
        pla
        tax
        rts

; ----------------------------------------------------------------------------
op_25_clear_flag_047A_of_object:  lda     object_id
        jsr     select_flags_047A
        pha
        txa
        jsr     load_flag_byte
        and     LDC9B,x
        sta     (flag_array_ptr),y
        pla
        tax
        rts

; ----------------------------------------------------------------------------
select_flags_047A:
        sta     script_args
        lda     #$7A
        sta     flag_array_ptr
        lda     #$04
        sta     flag_array_ptr+1
        rts

; ----------------------------------------------------------------------------
op_26:  tya
        pha
        txa
        pha
        jsr     open_container
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
LD4B2:  lda     #$71
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
open_container:  lda     open_containers
        cmp     #$06
        beq     LD4B2
        inc     open_containers
        lda     open_containers
        sta     current_container
        asl     a
        tay
        lda     script_table
        sta     $AF
        sta     container_stack,y
        lda     object_id
        sta     $B0
        sta     container_stack+1,y
        jsr     load_container_view
        jsr     refresh_coat_page
        rts

; ----------------------------------------------------------------------------
load_container_view:  lda     $AF
        cmp     #$02
        bne     LD4E9
        jsr     copy_container_to_coat
        rts

; ----------------------------------------------------------------------------
LD4E9:  jsr     fill_coat_from_item_list
        rts

; ----------------------------------------------------------------------------
select_item_list:  lda     current_container
        asl     a
        tay
        lda     container_stack,y
        sta     $AF
        lda     container_stack+1,y
        sta     $B0
        rts

; ----------------------------------------------------------------------------
fill_coat_from_item_list:  lda     #$02
        jsr     push_and_switch_bank
        jsr     clear_coat_slots
        lda     #$00
        ldy     $AF
        cpy     #$01
        beq     LD50F
        lda     #$01
LD50F:  asl     a
        tay
        lda     $A400,y
        sta     $68
        lda     $A401,y
        sta     $69
        lda     $B0
        asl     a
        clc
        adc     $68
        sta     $68
        lda     $69
        adc     #$00
        sta     $69
        ldy     #$00
        lda     ($68),y
        sta     $6A
        iny
        lda     ($68),y
        sta     $6B
        ldy     #$00
        ldx     #$00
LD538:  lda     ($6A),y
        cmp     #$FF
        beq     LD554
        jsr     split_item_code
        jsr     item_list_entry_visible
        lda     script_flags
        and     #$04
        bne     LD550
        lda     ($6A),y
        sta     coat_slots,x
        inx
LD550:  iny
        jmp     LD538

; ----------------------------------------------------------------------------
LD554:  jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
refresh_coat_page:  lda     inventory_page
        cmp     #$01
        beq     LD561
        bcs     LD56C
LD561:  lda     #$00
        sta     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD56C:  lda     #$02
        sta     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
item_list_entry_visible:  tya
        pha
        txa
        pha
        jsr     item_list_entry_condition
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
item_list_entry_condition:  lda     $BD
        sec
        sbc     #$03
        jsr     jump_table
        .addr   LD58F
        .addr   LD5A1
; ----------------------------------------------------------------------------
LD58F:  lda     $BE
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        beq     LD59B
        rts

; ----------------------------------------------------------------------------
LD59B:  lda     $BE
        jsr     test_flag_045C
        rts

; ----------------------------------------------------------------------------
LD5A1:  lda     $BE
        jsr     test_item_obtained
        lda     script_flags
        and     #$04
        beq     LD5AD
        rts

; ----------------------------------------------------------------------------
LD5AD:  lda     $BE
        jsr     test_flag_047A
        rts

; ----------------------------------------------------------------------------
clear_coat_slots:  ldy     #$00
LD5B5:  lda     #$FF
        sta     coat_slots,y
        iny
        cpy     #$07
        bcc     LD5B5
        rts

; ----------------------------------------------------------------------------
copy_container_to_coat:  tya
        pha
        txa
        pha
        jsr     clear_coat_slots
        lda     $B0
        asl     a
        asl     a
        asl     a
        tax
        ldy     #$00
LD5CF:  lda     container_items,x
        cmp     #$FF
        beq     LD5DF
        sta     coat_slots,y
        iny
        inx
        cpy     #$07
        bcc     LD5CF
LD5DF:  pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
op_27:  jsr     collect_room_sprites
        lda     #$65
        jsr     far_call_8020
        jsr     wait_vblank_and_hide_sprites
        rts

; ----------------------------------------------------------------------------
op_2A:  lda     #$6C
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
op_35:  jsr     script_fetch_byte
        sta     room
        rts

; ----------------------------------------------------------------------------
op_38:  jsr     script_fetch_byte
        lda     script_args
        sta     text_number
        rts

; ----------------------------------------------------------------------------
op_39:  jsr     script_fetch_byte
add_coins:  lda     coins
        clc
        adc     script_args
        sta     coins
        lda     #$00
        sta     $C0
        rts

; ----------------------------------------------------------------------------
op_62:  jsr     script_fetch_byte
add_coins_and_show:  jsr     add_coins
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        rts

; ----------------------------------------------------------------------------
op_56:  jsr     script_fetch_byte
        lda     $051C
        clc
        adc     script_args
        sta     $051C
        lda     #$08
        sta     $C0
        rts

; ----------------------------------------------------------------------------
op_3A:  jsr     op_56
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_57:  lda     $051D
        inc     $051D
        lda     #$2D
        sta     $C0
        rts

; ----------------------------------------------------------------------------
op_3B:  jsr     op_57
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_3C:  jsr     script_fetch_byte
        lda     coins
        sec
        sbc     script_args
        bcc     LD683
        sta     coins
        lda     #$82
        jsr     set_event_flag
        lda     #$00
        sta     $C0
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD683:  lda     room
        cmp     #$26
        beq     LD692
        cmp     #$27
        beq     LD69B
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LD692:  lda     #$81
        jsr     set_event_flag
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LD69B:  lda     #$A0
        jsr     set_event_flag
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_3D:  lda     $051C
        beq     LD6BC
        dec     $051C
        lda     #$08
        sta     $C0
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD6BC:  lda     #$80
        jsr     clear_event_flag
        lda     #$77
        jsr     print_message_2_or_5
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_3E:  lda     $051D
        beq     LD6E2
        dec     $051D
        lda     #$2D
        sta     $C0
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD6E2:  lda     #$80
        jsr     clear_event_flag
        lda     #$78
        jsr     print_message_2_or_5
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_40:  jsr     script_condition_false
        lda     coins
        beq     LD70B
        dec     coins
        lda     #$00
        sta     $C0
        sta     script_args
        jsr     find_in_goods
        jsr     show_found_page
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD70B:  lda     #$72
        jsr     print_message_2_or_5
        rts

; ----------------------------------------------------------------------------
show_found_page:  lda     $60
        sta     inventory_entry
        lda     $5F
        cmp     inventory_page
        beq     LD72B
        bcc     LD731
        sta     inventory_page
        dec     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD72B:  lda     #$68
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD731:  sta     inventory_page
        inc     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
op_3F:  jsr     script_fetch_byte
        lda     script_args
        sta     text_keep_open
        rts

; ----------------------------------------------------------------------------
op_41:  jsr     script_condition_false
        lda     #$00
        sta     $04AE
        lda     inventory_page
        beq     LD763
        lda     inventory_page
        sta     $04AE
        lda     #$01
        sta     inventory_page
        lda     #$64
        jsr     far_call_8000
LD763:  lda     #$66
        jsr     far_call_8000
        ldy     $84
        cpy     #$FF
        beq     LD79F
        lda     address_book,y
        ldy     #$00
        cmp     #$A3
        beq     LD78B
        iny
        cmp     #$A0
        beq     LD78B
        iny
        cmp     #$A1
        beq     LD78B
        iny
        cmp     #$A2
        beq     LD78B
        iny
        cmp     #$A4
        beq     LD78B
LD78B:  lda     LD79A,y
        sta     $B2
        lda     room
        and     #$01
        beq     LD799
        inc     $B2
LD799:  rts

; ----------------------------------------------------------------------------
LD79A:  .byte   $26,$42,$44,$46,$48
; ----------------------------------------------------------------------------
LD79F:  jsr     op_44
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
op_44:  lda     $04AE
        beq     op_45
        sta     inventory_page
        dec     inventory_page
        lda     #$67
        jsr     far_call_8020
op_45:  lda     #$00
        sta     text_keep_open
        lda     #$00
        sta     split_nametable
        rts

; ----------------------------------------------------------------------------
op_43:  jsr     script_condition_false
        lda     room
        cmp     $B2
        bne     LD7CE
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD7CE:  lda     $B2
        sta     room
        lda     #$83
        jsr     set_event_flag
        rts

; ----------------------------------------------------------------------------
op_42:  jsr     script_fetch_byte
        sta     $C0
        ldy     addresses_found
        sta     address_book,y
        inc     addresses_found
        lda     inventory_page
        bne     LD7F2
        lda     #$68
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD7F2:  lda     inventory_page
        pha
        lda     #$01
        sta     inventory_page
        lda     #$64
        jsr     far_call_8000
        lda     #$28
        jsr     wait_frames
        pla
        sta     inventory_page
        dec     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
op_46_special_scene:  jsr     script_fetch_byte
        sta     special_scene
        jsr     run_special_scene
        rts

; ----------------------------------------------------------------------------
op_47:  tya
        pha
        jsr     close_container
        pla
        tay
        rts

; ----------------------------------------------------------------------------
close_container:  dec     open_containers
        ldy     #$00
LD828:  lda     container_stack,y
        cmp     script_table
        bne     LD836
        lda     container_stack+1,y
        cmp     object_id
        beq     LD83B
LD836:  iny
        iny
        jmp     LD828

; ----------------------------------------------------------------------------
LD83B:  lda     container_stack+2,y
        sta     container_stack,y
        lda     container_stack+3,y
        sta     container_stack+1,y
        iny
        iny
        cpy     #$0C
        bcc     LD83B
        lda     #$FF
        sta     container_stack,y
        sta     container_stack+1,y
        lda     open_containers
        cmp     #$FF
        beq     LD88D
        lda     current_container
        asl     a
        tay
        lda     container_stack,y
        cmp     #$FF
        bne     LD87C
        dec     current_container
        jsr     select_item_list
        jsr     load_container_view
        lda     #$02
        sta     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD87C:  jsr     select_item_list
        jsr     load_container_view
        lda     #$00
        sta     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
LD88D:  lda     inventory_page
        cmp     #$02
        bne     LD895
        rts

; ----------------------------------------------------------------------------
LD895:  lda     #$01
        sta     inventory_page
        lda     #$67
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
op_48_yield_b5_80:  lda     script_flags
        ora     #$80
        sta     script_flags
        pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
op_49:  lda     #$6B
        jsr     far_call_8020
        lda     #$6E
        jsr     far_call_8000
        lda     #$6A
        jsr     far_call_8020
        jsr     previous_store_entry
        rts

; ----------------------------------------------------------------------------
previous_store_entry:  dec     store_entry
        lda     store_entry
        cmp     #$FF
        beq     LD8C9
        rts

; ----------------------------------------------------------------------------
LD8C9:  lda     #$06
        sta     store_entry
        dec     store_page
        rts

; ----------------------------------------------------------------------------
op_4A:  jsr     script_condition_false
        lda     coins
        bne     LD8DD
        jsr     script_condition_true
LD8DD:  rts

; ----------------------------------------------------------------------------
op_4B:  jsr     script_condition_false
        jsr     script_fetch_byte
        cmp     $04A8
        bne     LD8EC
        jsr     script_condition_true
LD8EC:  rts

; ----------------------------------------------------------------------------
op_4C:  jsr     script_condition_false
        lda     $04A8
        bne     LD8FD
        lda     object_id
        sta     $04A8
        jsr     script_condition_true
LD8FD:  rts

; ----------------------------------------------------------------------------
op_4D:  jsr     script_condition_false
        lda     $04A8
        beq     LD90E
        lda     #$00
        sta     $04A8
        jsr     script_condition_true
LD90E:  rts

; ----------------------------------------------------------------------------
op_4E:  lda     $04A9
        cmp     #$06
        bcs     LD91D
        inc     $04A9
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD91D:  lda     #$80
        jsr     clear_event_flag
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_4F:  jsr     script_condition_false
        lda     $04A9
        beq     LD934
        dec     $04A9
        jsr     script_condition_true
LD934:  rts

; ----------------------------------------------------------------------------
op_50:  lda     $04AB
        cmp     #$06
        bcs     LD95C
        inc     $04AB
        lda     #$B1
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LD953
        lda     #$4B
        jsr     clear_story_flag
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD953:  lda     #$B1
        jsr     set_event_flag
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD95C:  lda     #$80
        jsr     clear_event_flag
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_51:  jsr     script_condition_false
        lda     $04AB
        beq     LD973
        dec     $04AB
        jsr     script_condition_true
LD973:  rts

; ----------------------------------------------------------------------------
op_58:  lda     $04AA
        cmp     #$06
        bcs     LD982
        inc     $04AA
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LD982:  lda     #$80
        jsr     clear_event_flag
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
op_59:  jsr     script_condition_false
        lda     $04AA
        beq     LD999
        dec     $04AA
        jsr     script_condition_true
LD999:  rts

; ----------------------------------------------------------------------------
op_55:  tya
        pha
        txa
        pha
        jsr     script_fetch_byte
        jsr     find_in_goods
        jsr     show_item_page_keep_cursor
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
find_and_show_item:  sta     script_args
        tya
        pha
        txa
        pha
        jsr     find_in_goods
        jsr     show_item_page_keep_cursor
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
find_in_goods:  lda     #$02
        sta     $5F
        ldx     #$00
        ldy     #$0E
LD9C5:  lda     address_book,y
        cmp     script_args
        bne     LD9CF
        jmp     LD9DD

; ----------------------------------------------------------------------------
LD9CF:  iny
        inx
        cpx     #$07
        bcc     LD9D9
        ldx     #$00
        inc     $5F
LD9D9:  cpy     #$54
        bcc     LD9C5
LD9DD:  stx     $60
        rts

; ----------------------------------------------------------------------------
show_item_page_keep_cursor:  lda     inventory_entry
        sta     saved_inventory_entry
        lda     inventory_page
        sta     saved_inventory_page
        lda     $60
        sta     inventory_entry
        lda     $5F
        cmp     inventory_page
        bcc     LDA07
        beq     show_page_then_op_49
        sta     inventory_page
        lda     #$6B
        jsr     far_call_8020
        jsr     previous_store_entry
        jsr     restore_inventory_cursor
        rts

; ----------------------------------------------------------------------------
LDA07:  jsr     show_page_then_op_49
        jsr     restore_inventory_cursor
        inc     inventory_page
        lda     #$6D
        jsr     far_call_8020
        rts

; ----------------------------------------------------------------------------
restore_inventory_cursor:  lda     saved_inventory_entry
        sta     inventory_entry
        lda     saved_inventory_page
        sta     inventory_page
        rts

; ----------------------------------------------------------------------------
show_page_then_op_49:  sta     inventory_page
        jsr     op_49
        rts

; ----------------------------------------------------------------------------
op_5A:  jsr     script_fetch_byte
        sta     special_scene
        jsr     show_special_scene
        rts

; ----------------------------------------------------------------------------
op_5C:  jsr     script_fetch_2
        lda     script_args
        sta     script_table
        lda     script_args+1
        sta     object_id
        rts

; ----------------------------------------------------------------------------
op_5E:  lda     object_id
        jsr     test_door_flag
        lda     script_flags
        and     #$04
        bne     LDA52
        lda     #$63
        jsr     print_message_2_or_5
        pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
LDA52:  jsr     backup_state_to_save_ram
        jsr     script_fetch_byte
        lda     script_args
        sta     script_args+3
        jsr     op_5E_special_check
        lda     script_flags
        and     #$04
        bne     LDA66
        rts

; ----------------------------------------------------------------------------
LDA66:  pla
        pla
        jmp     script_end

; ----------------------------------------------------------------------------
op_5E_special_check:  tya
        pha
        jsr     op_5E_special_check_body
        pla
        tay
LDA72:  rts

; ----------------------------------------------------------------------------
op_5E_special_check_body:  jsr     script_condition_false
        ldy     #$00
LDA78:  lda     LDC92,y
        cmp     #$FF
        beq     LDA72
        cmp     script_args+3
        bne     LDA97
        tya
        jsr     jump_table
        .addr   LDA9B
        .addr   LDAE8
        .addr   LDB12
        .addr   LDB4F
        .addr   LDB83
        .addr   LDBBC
        .addr   LDC04
        .addr   LDC4C
; ----------------------------------------------------------------------------
LDA97:  iny
        jmp     LDA78

; ----------------------------------------------------------------------------
LDA9B:  jsr     script_condition_false
        lda     #$07
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        bne     LDAB5
        lda     #$54
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        bne     LDAB5
        rts

; ----------------------------------------------------------------------------
LDAB5:  jsr     script_condition_false
        lda     coins
        bne     LDABE
        rts

; ----------------------------------------------------------------------------
LDABE:  lda     #$93
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDACD
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDACD:  jsr     script_condition_false
        jsr     next_random
        lda     random_value
        and     #$03
        beq     LDADA
        rts

; ----------------------------------------------------------------------------
LDADA:  lda     script_args+3
        jsr     enter_room
        lda     #$E0
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDAE8:  lda     #$92
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDAF7
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDAF7:  jsr     next_random
        lda     random_value
        and     #$01
        bne     LDB04
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDB04:  lda     script_args+3
        jsr     enter_room
        lda     #$E5
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDB12:  lda     #$02
        jsr     test_item_obtained
        lda     script_flags
        and     #$04
        bne     LDB29
        lda     #$07
        jsr     test_item_obtained
        lda     script_flags
        and     #$04
        bne     LDB29
        rts

; ----------------------------------------------------------------------------
LDB29:  jsr     script_condition_false
        lda     $04B7
        cmp     #$03
        bcc     LDB34
        rts

; ----------------------------------------------------------------------------
LDB34:  jsr     next_random
        lda     random_value
        and     #$01
        bne     LDB3E
        rts

; ----------------------------------------------------------------------------
LDB3E:  inc     $04B7
        lda     script_args+3
        jsr     enter_room
        lda     #$33
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDB4F:  jsr     next_random
        lda     random_value
        and     #$01
        beq     LDB5C
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDB5C:  lda     #$A8
        jsr     set_event_flag
        lda     script_args+3
        jsr     enter_room
        lda     #$91
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LDB7A
        lda     #$DB
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDB7A:  lda     #$DE
        jsr     print_message_2_or_5
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDB83:  lda     #$05
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDB92
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDB92:  lda     #$2A
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDBA1
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDBA1:  jsr     script_condition_false
        jsr     next_random
        lda     random_value
        and     #$18
        beq     LDBAE
        rts

; ----------------------------------------------------------------------------
LDBAE:  lda     script_args+3
        jsr     enter_room
        lda     #$EB
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDBBC:  lda     #$06
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDBCB
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDBCB:  lda     #$2A
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDBDA
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDBDA:  lda     #$05
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LDBE9
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDBE9:  jsr     script_condition_false
        jsr     next_random
        lda     random_value
        and     #$18
        beq     LDBF6
        rts

; ----------------------------------------------------------------------------
LDBF6:  lda     script_args+3
        jsr     enter_room
        lda     #$F1
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDC04:  lda     #$07
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDC13
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC13:  lda     #$2A
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDC22
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC22:  lda     #$06
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LDC31
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC31:  jsr     script_condition_false
        jsr     next_random
        lda     random_value
        and     #$18
        beq     LDC3E
        rts

; ----------------------------------------------------------------------------
LDC3E:  lda     script_args+3
        jsr     enter_room
        lda     #$F4
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDC4C:  lda     #$2A
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        beq     LDC5B
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC5B:  lda     #$07
        jsr     test_event_flag
        lda     script_flags
        and     #$04
        bne     LDC6A
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC6A:  lda     #$07
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        bne     LDC84
        lda     #$54
        jsr     test_story_flag
        lda     script_flags
        and     #$04
        bne     LDC84
        jsr     script_condition_false
        rts

; ----------------------------------------------------------------------------
LDC84:  lda     script_args+3
        jsr     enter_room
        lda     #$F6
        jsr     print_message_4
        jsr     script_condition_true
        rts

; ----------------------------------------------------------------------------
LDC92:  .byte   $1E,$20,$24,$1F,$4A,$4D,$4B,$4C
        .byte   $FF
LDC9B:  .byte   $FE,$FD,$FB,$F7,$EF,$DF,$BF,$7F
; 605 bytes of $FF
unused_7_DCA3:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
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
; ----------------------------------------------------------------------------
; next character code from the bit stream at ($00)
text_read_code:
        tax
        pha
        tya
        pha
        lda     #$00
        tax
        sta     text_value
LDF09:  jsr     text_read_bit
        lda     text_value
        bne     LDF14
        inx
        jmp     LDF09

; ----------------------------------------------------------------------------
LDF14:  lda     #$00
        sta     text_value
        jsr     text_read_bit
        jsr     text_read_bit
        jsr     text_read_bit
        txa
        asl     a
        asl     a
        asl     a
        ora     text_value
        tax
        lda     text_code_table,x
        sta     text_value
        pla
        tay
        pla
        tax
        lda     text_value
        rts

; ----------------------------------------------------------------------------
; next 8 bits as a byte
text_read_byte:
        tax
        pha
        tya
        pha
        lda     #$00
        tax
        sta     text_value
LDF3D:  jsr     text_read_bit
        inx
        cpx     #$08
        bne     LDF3D
        pla
        tay
        pla
        tax
        lda     text_value
        rts

; ----------------------------------------------------------------------------
; next bit into $3F; $3E = bit buffer, $3D = bits used
text_read_bit:
        lda     text_bits_used
        bne     LDF63
        ldy     #$00
        lda     ($00),y
        sta     text_bit_buffer
        lda     $00
        clc
        adc     #$01
        sta     $00
        lda     $01
        adc     #$00
        sta     $01
LDF63:  asl     text_bit_buffer
        rol     text_value
        inc     text_bits_used
        lda     text_bits_used
        cmp     #$08
        bne     LDF73
        lda     #$00
        sta     text_bits_used
LDF73:  rts

; ----------------------------------------------------------------------------
; ($00) = message; prints until an end code
print_message:
        lda     #$65
        jsr     far_call_8020
        jsr     wait_vblank_and_hide_sprites
        jsr     clear_text_window
        lda     #$00
        sta     text_bits_used
        sta     text_lines
print_message_loop:  jsr     text_read_code
        sta     text_char
        cmp     #$1F
        beq     LDFBF
        cmp     #$1E
        beq     LDFC2
        cmp     #$20
        bpl     LDFD6
        cmp     #$1B
        beq     LDFAA
        cmp     #$1C
        beq     LDFB0
        cmp     #$1D
        beq     LDFB9
        jsr     print_char
        jmp     print_message_loop

; ----------------------------------------------------------------------------
LDFAA:  jsr     text_wait_page
        jmp     print_message_loop

; ----------------------------------------------------------------------------
LDFB0:  jsr     text_wait_page
        jsr     print_clear
        jmp     print_message_loop

; ----------------------------------------------------------------------------
LDFB9:  jsr     text_newline
        jmp     print_message_loop

; ----------------------------------------------------------------------------
LDFBF:  jsr     text_wait_page
LDFC2:  lda     #$00
        sta     text_window_drawn
        dec     text_keep_open
        lda     text_keep_open
        bpl     LDFD5
        lda     #$00
        sta     split_nametable
        sta     text_keep_open
LDFD5:  rts

; ----------------------------------------------------------------------------
LDFD6:  and     #$1F
        jsr     print_control
        jmp     print_message_loop

; ----------------------------------------------------------------------------
print_control:  cmp     #$1C
        bne     LDFE5
        jmp     print_item_name_arg

; ----------------------------------------------------------------------------
LDFE5:  cmp     #$1D
        bne     LDFEC
        jmp     print_word_arg

; ----------------------------------------------------------------------------
LDFEC:  cmp     #$1F
        bne     LDFF3
        jmp     print_item_name_at

; ----------------------------------------------------------------------------
LDFF3:  cmp     #$1E
        bne     LDFFA
        jmp     print_number_at

; ----------------------------------------------------------------------------
LDFFA:  cmp     #$1B
        bne     LE001
        jmp     print_clear

; ----------------------------------------------------------------------------
LE001:  jsr     print_char
        rts

; ----------------------------------------------------------------------------
print_item_name_at:  ldy     #$00
        lda     ($00),y
        sta     $85
        iny
        lda     ($00),y
        sta     $86
        lda     #$00
        sta     text_bits_used
        lda     $00
        clc
        adc     #$02
        sta     $00
        lda     $01
        adc     #$00
        sta     $01
        ldy     #$00
        lda     ($85),y
        sta     $85
        jmp     LE02F

; ----------------------------------------------------------------------------
print_item_name_arg:  jsr     text_read_byte
        sta     $85
LE02F:  lda     #$02
        jsr     push_and_switch_bank
        lda     #$00
        sta     $86
        lda     $85
        asl     a
        rol     $86
        asl     a
        rol     $86
        asl     a
        rol     $86
        clc
        adc     #$00
        sta     $87
        lda     $86
        adc     #$8D
        sta     $88
        jsr     print_name
        jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
print_word_arg:  jsr     text_read_byte
        sta     $85
        lda     #$02
        jsr     push_and_switch_bank
        lda     #$00
        sta     $86
        lda     $85
        asl     a
        rol     $86
        asl     a
        rol     $86
        asl     a
        rol     $86
        clc
        adc     #$00
        sta     $87
        lda     $86
        adc     #$95
        sta     $88
        jsr     print_name
        jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
; 8 tiles at ($87), $FF ends early
print_name:
        ldx     #$00
LE082:  ldy     #$00
        lda     ($87),y
        sta     text_tile
        lda     $87
        clc
        adc     #$01
        sta     $87
        lda     $88
        adc     #$00
        sta     $88
        lda     text_tile
        cmp     #$FF
        beq     LE0A5
        jsr     print_tile
        inx
        cpx     #$08
        bne     LE082
LE0A5:  rts

; ----------------------------------------------------------------------------
print_number_at:  jsr     read_number_arg
        jsr     number_to_digits
        lda     $8B
        jsr     print_digit_skip_zero
        lda     $8C
        jsr     print_digit_skip_zero
        lda     $8D
        jsr     print_digit
        rts

; ----------------------------------------------------------------------------
text_wait_page:
        lda     text_lines
        cmp     #$03
        beq     LE0C9
        jsr     text_newline
        jmp     text_wait_page

; ----------------------------------------------------------------------------
LE0C9:  lda     text_fast
        beq     LE0D0
        jsr     request_vram_queue
LE0D0:  lda     #$00
        sta     text_fast
        jsr     wait_page_input
        rts

; ----------------------------------------------------------------------------
text_newline:
        lda     text_lines
        cmp     #$03
        beq     LE0E8
        lda     text_lines
        clc
        adc     #$01
        sta     text_lines
LE0E8:  jsr     text_line_feed
        rts

; ----------------------------------------------------------------------------
print_clear:  jsr     clear_text_window
        rts

; ----------------------------------------------------------------------------
print_digit_skip_zero:  sta     text_char
        bne     LE0FA
        lda     printed_digit
        bne     LE0FA
        rts

; ----------------------------------------------------------------------------
LE0FA:  lda     text_char
print_digit:  ora     #$30
        sta     text_char
        lda     #$FF
        sta     text_digit_mark
        sta     printed_digit
        jsr     print_char
        rts

; ----------------------------------------------------------------------------
number_to_digits:  lda     #$00
        sta     $86
        ldy     #$02
LE113:  tya
        asl     a
        tax
LE116:  lda     $85
        sta     $87
        sec
        sbc     LE14D,x
        sta     $85
        lda     $86
        sta     $88
        sbc     LE14E,x
        sta     $86
        bcc     LE137
        lda     $89,y
        clc
        adc     #$01
        sta     $89,y
        jmp     LE116

; ----------------------------------------------------------------------------
LE137:  lda     $87
        sta     $85
        lda     $88
        sta     $86
        iny
        cpy     #$04
        bne     LE113
        lda     $85
        sta     $8D
        lda     #$00
        sta     printed_digit
        rts

; ----------------------------------------------------------------------------
LE14D:  .byte   $10
LE14E:  .byte   $27,$E8,$03,$64,$00,$0A,$00,$01
        .byte   $00
; ----------------------------------------------------------------------------
read_number_arg:  ldy     #$00
        lda     ($00),y
        sta     $87
        iny
        lda     ($00),y
        sta     $88
        jsr     skip_inline_word
        ldy     #$00
        lda     ($87),y
        sta     $85
        iny
        lda     ($87),y
        sta     $86
        ldy     #$00
        lda     #$00
        sta     text_bits_used
LE176:  sta     $89,y
        iny
        cpy     #$05
        bne     LE176
        rts

; ----------------------------------------------------------------------------
skip_inline_word:  lda     $00
        clc
        adc     #$02
        jmp     LE18C

; ----------------------------------------------------------------------------
; no caller found
unreferenced_E187:
        lda     $00
        clc
        adc     #$01
LE18C:  sta     $00
        lda     $01
        adc     #$00
        sta     $01
        rts

; ----------------------------------------------------------------------------
clear_text_window:
        lda     #$02
        sta     text_window_x
        lda     #$14
        sta     text_window_y
        jsr     blank_text_window
        lda     #$00
        sta     text_lines
        sta     text_wait_enabled
        sta     text_fast
        sta     text_no_wrap
        lda     #$01
        sta     split_nametable
        rts

; ----------------------------------------------------------------------------
blank_text_window:  lda     text_window_drawn
        bne     LE1BD
        jsr     draw_text_window_sides
        lda     #$FF
        sta     text_window_drawn
LE1BD:  ldx     #$00
        stx     text_column
        lda     #$03
        sta     text_row
LE1C7:  lda     #$DE
        sta     text_window,x
        sta     text_window+52,x
        sta     text_window+104,x
        sta     text_window+156,x
        lda     #$7E
        sta     text_window+26,x
        sta     text_window+78,x
        sta     text_window+130,x
        sta     text_window+182,x
        inx
        cpx     #$1A
        bne     LE1C7
        jsr     upload_text_window
        jsr     blank_window_text_rows
        jsr     draw_window_rule_row
        rts

; ----------------------------------------------------------------------------
; $0620 = tile
print_tile:
        tya
        pha
        txa
        pha
        jsr     put_tile
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
; $061A = character code
print_char:
        tya
        pha
        txa
        pha
        ldy     text_char
        lda     text_tile_table,y
        sta     text_tile
        jsr     put_tile
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
; character code -> font tile
text_tile_table:
        .byte   $B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8
        .byte   $B9,$BA,$BB,$BC,$BD,$BE,$BF,$C0
        .byte   $C1,$C2,$C3,$C4,$C5,$C6,$C7,$C8
        .byte   $C9,$CA,$7E,$7E,$7E,$7E,$7E,$7E
        .byte   $A1,$A4,$A5,$CB,$D6,$D8,$D5,$8A
        .byte   $8B,$D7,$CD,$CC,$D9,$DA,$7E,$7E
        .byte   $80,$81,$82,$83,$84,$85,$86,$87
        .byte   $88,$89,$7E,$7E,$7E,$7E,$7E,$7E
; ----------------------------------------------------------------------------
put_tile:  lda     text_fast
        bne     LE26C
        jsr     read_joypads
        lda     joy_pressed
        and     #$80
        beq     LE26C
        lda     #$FF
        sta     text_fast
        lda     #$65
        jsr     far_call_8020
        jsr     wait_vblank_and_hide_sprites
LE26C:  lda     text_window_x
        sta     $0E
        lda     text_window_y
        sta     $0F
        inc     $0E
        lda     text_column
        clc
        adc     $0E
        sta     $0E
        lda     text_row
        asl     a
        clc
        adc     $0F
        sta     $0F
        inc     $0F
        jsr     text_cursor_vram_address
        lda     #$01
        sta     text_tile_block
        sta     text_tile_block+1
        lda     #$1E
        sta     $1E
        lda     #$06
        sta     $1F
        jsr     queue_vram_block
        lda     text_fast
        bne     text_store_tile
        jsr     request_vram_queue
        jsr     show_more_arrow
        lda     #$65
        jsr     far_call_8020
        jsr     wait_vblank_and_hide_sprites
        lda     #$31
        jsr     play_sound
        ldx     text_delay
LE2BA:  cpx     #$00
        beq     text_store_tile
        jsr     wait_vblank
        dex
        jmp     LE2BA

; ----------------------------------------------------------------------------
LE2C5:  .byte   $00,$34,$68,$9C,$D0
; ----------------------------------------------------------------------------
text_store_tile:  lda     text_row
        tax
        lda     LE2C5,x
        clc
        adc     text_column
        tax
        lda     #$DE
        sta     text_window,x
        lda     text_tile
        sta     text_window+26,x
        inc     text_column
        lda     text_column
        cmp     #$1A
        bne     LE2F2
        lda     text_no_wrap
        bne     LE2F3
        jsr     text_newline
LE2F2:  rts

; ----------------------------------------------------------------------------
LE2F3:  lda     #$19
        sta     text_column
        rts

; ----------------------------------------------------------------------------
text_line_feed:  inc     text_row
        lda     text_row
        cmp     #$04
        bne     LE309
        jsr     text_scroll
        dec     text_row
LE309:  lda     text_fast
        bne     LE310
        jsr     wait_for_more_button
LE310:  lda     #$00
        sta     text_column
        rts

; ----------------------------------------------------------------------------
text_scroll:  lda     #$DE
        jsr     text_scroll_rows
        jsr     blank_window_text_rows
        lda     #$7E
        jsr     text_scroll_rows
        jsr     draw_window_rule_row
        rts

; ----------------------------------------------------------------------------
text_scroll_rows:  pha
        ldx     #$00
LE32A:  lda     text_window+26,x
        sta     text_window,x
        inx
        cpx     #$B6
        bne     LE32A
        pla
LE336:  sta     text_window,x
        inx
        cpx     #$D0
        bne     LE336
        jsr     upload_text_window
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
text_cursor_vram_address:  lda     #$00
        sta     $1C
        lda     $0F
        lsr     a
        ror     $1C
        lsr     a
        ror     $1C
        lsr     a
        ror     $1C
        ora     #$24
        sta     $1D
        lda     $1C
        ora     $0E
        sta     $1C
        rts

; ----------------------------------------------------------------------------
upload_text_window:  lda     text_window_x
        sta     $0E
        lda     text_window_y
        sta     $0F
        inc     $0E
        jsr     text_cursor_vram_address
        lda     #$1A
        sta     text_window_block
        lda     #$08
        sta     text_window_block+1
        lda     #$26
        sta     $1E
        lda     #$06
        sta     $1F
        jsr     queue_vram_block
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
draw_window_rule_row:  lda     text_window_x
        sta     $0E
        inc     $0E
        lda     text_window_y
        clc
        adc     #$08
        sta     $0F
        jsr     text_cursor_vram_address
        lda     #$C9
        sta     $1E
        lda     #$E3
        sta     $1F
        jsr     queue_vram_fill
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
blank_window_text_rows:  lda     text_window_x
        sta     $0E
        inc     $0E
        lda     text_window_y
        clc
        adc     #$08
        sta     $0F
        jsr     text_cursor_vram_address
        lda     #$CC
        sta     $1E
        lda     #$E3
        sta     $1F
        jsr     queue_vram_fill
        jsr     request_vram_queue
        rts

; ----------------------------------------------------------------------------
        .byte   $1A,$01,$DE,$1A,$02,$7E
; ----------------------------------------------------------------------------
draw_text_window_sides:  jsr     request_vram_queue
        lda     text_window_x
        sta     $0E
        lda     text_window_y
        sta     $0F
        jsr     draw_window_side_column
        lda     text_window_x
        clc
        adc     #$1B
        sta     $0E
        lda     text_window_y
        sta     $0F
        jsr     draw_window_side_column
        rts

; ----------------------------------------------------------------------------
draw_window_side_column:  jsr     text_cursor_vram_address
        lda     #$02
        sta     $1E
        lda     #$E4
        sta     $1F
        jsr     queue_vram_column
        jsr     flush_vram_queue_down
        rts

; ----------------------------------------------------------------------------
        .byte   $0A,$7E,$7E,$7E,$7E,$7E,$7E,$7E
        .byte   $7E,$7E,$7E
; ----------------------------------------------------------------------------
text_pause:  pha
        txa
        pha
        tya
        pha
        jsr     request_vram_queue
        ldy     #$00
LE417:  jsr     wait_vblank
        iny
        cpy     #$14
        bne     LE417
        lda     joy_pressed
        pha
LE422:  jsr     read_joypads
        lda     joy_pressed
        and     #$C0
        bne     LE433
        jsr     wait_vblank
        iny
        cpy     #$32
        bne     LE422
LE433:  pla
        sta     joy_pressed
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
wait_page_input:  lda     #$00
        sta     text_lines
        lda     text_fast
        bne     LE448
        jsr     wait_for_more_button
LE448:  jsr     request_vram_queue
        lda     text_wait_enabled
        beq     LE45D
        ldx     text_delay
LE451:  jsr     text_pause
        cpx     #$00
        beq     LE45C
        dex
        jmp     LE451

; ----------------------------------------------------------------------------
LE45C:  rts

; ----------------------------------------------------------------------------
LE45D:  jsr     blink_more_arrow
        jsr     read_joypads
        lda     joy_pressed
        and     #$80
        beq     LE45D
LE469:  jsr     blink_more_arrow
        jsr     read_joypads
        lda     joy_held
        and     #$80
        bne     LE469
        rts

; ----------------------------------------------------------------------------
blink_more_arrow:  lda     #$65
        jsr     far_call_8020
        lda     frame_counter
        and     #$10
        beq     LE488
        lda     #$0A
        sta     $20
        jsr     draw_more_arrow
LE488:  jsr     wait_vblank_and_hide_sprites
        rts

; ----------------------------------------------------------------------------
show_more_arrow:  jsr     text_cursor_x_pixels
        jsr     draw_more_arrow
        rts

; ----------------------------------------------------------------------------
wait_for_more_button:  jsr     text_cursor_x_pixels
        lda     $20
        sta     text_column
LE49B:  jsr     draw_more_arrow_frame
        lda     text_column
        cmp     #$11
        bcc     LE4B8
        dec     text_column
        dec     text_column
        dec     text_column
        jsr     read_joypads
        lda     joy_held
        bmi     LE4B8
        jmp     LE49B

; ----------------------------------------------------------------------------
LE4B8:  lda     #$10
        sta     text_column
        jsr     draw_more_arrow_frame
        rts

; ----------------------------------------------------------------------------
draw_more_arrow_frame:  lda     #$65
        jsr     far_call_8020
        lda     text_column
        sta     $20
        jsr     draw_more_arrow
        jsr     wait_vblank_and_hide_sprites
        rts

; ----------------------------------------------------------------------------
text_cursor_x_pixels:  lda     text_window_x
        clc
        adc     text_column
        asl     a
        asl     a
        asl     a
        sta     $20
        rts

; ----------------------------------------------------------------------------
draw_more_arrow:  lda     #$D8
        sta     $21
        lda     #$00
        sta     $22
        lda     #$F3
        sta     $1E
        lda     #$E4
        sta     $1F
        jsr     draw_metasprite
        rts

; ----------------------------------------------------------------------------
        .byte   $22,$76,$02,$77,$02,$78,$02,$79
        .byte   $02
; bit-stream index -> character code
text_code_table:
        .byte   $1A,$04,$13,$0E,$00,$08,$12,$0D
        .byte   $11,$07,$0B,$14,$03,$18,$1D,$20
        .byte   $02,$0C,$06,$05,$1F,$01,$0F,$16
        .byte   $2A,$0A,$15,$25,$26,$27,$1C,$09
        .byte   $21,$24,$19,$28,$30,$17,$10,$32
        .byte   $3F,$33,$31,$34,$35,$2B,$36,$3E
        .byte   $38,$29,$39,$2E,$2C,$2D,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
; 196 bytes of $FF
unused_7_E53C:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
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
; reset jumps here after hardware init
main:   lda     #$00
        sta     same_room_redraws
        lda     #$FF
        sta     last_room
        lda     #$02
        sta     text_delay
        lda     #$00
        jsr     play_sound
        jsr     new_game
        lda     #$60
        jsr     far_call_8040
        lda     #$01
        jsr     play_sound
        lda     #$21
        jsr     far_call_8400
        lda     #$22
        jsr     far_call_8400
        jsr     backup_state_to_save_ram
game_screen_loop:  jsr     draw_game_screen
        lda     #$FF
        sta     script_table
        sta     object_id
        sta     command
        jsr     show_intro_once
input_loop:  lda     #$6A
        jsr     far_call_8000
        lda     #$65
        jsr     far_call_8020
        jsr     wait_vblank_and_hide_sprites
        lda     #$69
        jsr     far_call_8000
        lda     script_flags
        bpl     input_loop
        lda     room
        cmp     last_room
        bne     LE675
        inc     same_room_redraws
        lda     same_room_redraws
        cmp     #$03
        bne     LE67B
        lda     #$00
        sta     scene_index
        lda     #$02
        sta     $84
        jsr     erase_save_file
        jsr     check_all_save_files
        jsr     request_black_palette
        jsr     wait_nmi_work_done
        jmp     reset

; ----------------------------------------------------------------------------
LE675:  sta     last_room
        lda     #$01
        sta     same_room_redraws
LE67B:  lda     #$23
        jsr     far_call_8400
        asl     script_flags
        lsr     script_flags
        lda     scene_index
        jsr     jump_table
        .addr   game_screen_loop
        .addr   reset
; ----------------------------------------------------------------------------
draw_game_screen:  jsr     request_black_palette
        lda     #$A9
        jsr     clear_nametables
        jsr     clear_oam_buffer
        ldy     #$00
LE69A:  lda     #$FF
        sta     attr_buffer_2000,y
        iny
        cpy     #$80
        bcc     LE69A
        lda     #$6F
        jsr     far_call_8000
        lda     #$6C
        jsr     far_call_8000
        lda     #$62
        jsr     far_call_8020
        lda     #$6E
        jsr     far_call_8000
        lda     inventory_page
        bne     LE6C5
        lda     #$63
        jsr     far_call_8040
        jmp     LE6CA

; ----------------------------------------------------------------------------
LE6C5:  lda     #$6A
        jsr     far_call_8020
LE6CA:  lda     #$68
        jsr     far_call_8000
        lda     #$69
        jsr     far_call_8020
        jsr     load_current_scene
        lda     #$6D
        jsr     far_call_8000
        jsr     update_cursor_and_sprites
        lda     #$60
        jsr     far_call_8000
        lda     #$63
        jsr     far_call_8000
        rts

; ----------------------------------------------------------------------------
update_cursor_and_sprites:  lda     game_started
        beq     LE6FB
        lda     #$6A
        jsr     far_call_8000
        jsr     collect_room_sprites
        lda     #$65
        jsr     far_call_8020
LE6FB:  jsr     wait_vblank_and_hide_sprites
        rts

; ----------------------------------------------------------------------------
load_current_scene:  lda     game_started
        bne     LE708
        lda     #$8F
        sta     scene
LE708:  jsr     load_scene
        rts

; ----------------------------------------------------------------------------
show_intro_once:  lda     game_started
        bne     LE72D
        jsr     wait_60_frames
        jsr     wait_30_frames
        lda     #$11
        jsr     clear_event_flag
        lda     #$00
        sta     scene
        jsr     change_scene
        jsr     update_cursor_and_sprites
        dec     game_started
        lda     #$00
        jsr     print_message_5
LE72D:  rts

; ----------------------------------------------------------------------------
new_game:  lda     #$00
        tay
LE731:  sta     object_state,y
        iny
        bne     LE731
LE737:  sta     container_stack+1,y
        iny
        cpy     #$10
        bne     LE737
        lda     #$02
        sta     inventory_page
        sta     store_page
        lda     #$00
        sta     inventory_entry
        sta     store_entry
        lda     #$03
        sta     $04A9
        lda     #$06
        sta     $04AA
        lda     #$00
        sta     $04AB
        lda     #$07
        sta     $04AC
        lda     #$06
        sta     $04AD
        lda     #$4B
        jsr     set_story_flag_refresh
        lda     #$FF
        sta     script_table
        sta     object_id
        sta     command
        sta     switch_key
        sta     switch_key+1
        sta     $AF
        sta     $B0
        ldy     #$00
LE77F:  sta     address_book,y
        iny
        cpy     #$54
        bcc     LE77F
        ldy     #$00
LE789:  sta     open_containers,y
        iny
        cpy     #$10
        bcc     LE789
        ldy     #$00
LE793:  sta     container_items,y
        iny
        cpy     #$20
        bcc     LE793
        lda     #$0A
        sta     container_items
        lda     #$00
        sta     $04A8
        sta     scene
        sta     room
        lda     #$00
        sta     dissolve_pattern
        rts

; ----------------------------------------------------------------------------
next_random:  lda     rng_state
        clc
        adc     #$01
        sta     random_value
        clc
        adc     random_value
        clc
        adc     random_value
        clc
        adc     random_value
        clc
        adc     random_value
        sta     rng_state
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     random_value
        lda     rng_state
        asl     a
        asl     a
        asl     a
        asl     a
        ora     random_value
        sta     random_value
        rts

; ----------------------------------------------------------------------------
; 41 bytes of $FF
unused_7_E7D7:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF
; ----------------------------------------------------------------------------
reset:  sei
        cld
        ldx     #$00
        stx     PPUCTRL
LE807:  lda     PPUSTATUS
        bpl     LE807
LE80C:  lda     PPUSTATUS
        bpl     LE80C
        ldx     #$FE
        txs
        lda     #$00
        sta     SND_CHN
        sta     APU_FRAME
        sta     DMC_FREQ
        ldy     #$00
LE821:  sta     $00,y
        sta     $0100,y
        sta     oam_buffer,y
        sta     attr_buffer_2000,y
        sta     palette_buffer,y
        sta     goods_slots+50,y
        sta     scene_tiles+172,y
        sta     sound_channels,y
        iny
        bne     LE821
        lda     #$06
        jsr     set_current_bank
        lda     #$FF
        sta     stack_index
        lda     PPUSTATUS
        jsr     set_mirroring_vertical
        lda     #$00
        sta     PPUSCROLL
        sta     PPUSCROLL
        lda     #$00
        sta     sound_table_ptr
        lda     #$80
        sta     sound_table_ptr+1
        jsr     sound_reset
        lda     #$06
        sta     ppumask_shadow
        sta     PPUMASK
        lda     #$88
        sta     ppuctrl_shadow
        sta     PPUCTRL
        jsr     disable_save_ram
        jsr     wait_vblank
        jsr     set_mirroring_vertical
        jsr     wait_vblank
        jmp     main

; ----------------------------------------------------------------------------
set_mirroring_vertical:
        lda     #$00
        sta     MMC3_MIRRORING
        lda     #$47
        sta     mirroring_mode
        lda     system_flags
        and     #$F7
        sta     system_flags
        rts

; ----------------------------------------------------------------------------
set_mirroring_horizontal:
        lda     #$01
        sta     MMC3_MIRRORING
        lda     #$4F
        sta     mirroring_mode
        lda     system_flags
        ora     #$08
        sta     system_flags
        rts

; ----------------------------------------------------------------------------
; A = 16 KB bank for $8000-$BFFF
switch_prg_bank:
        sta     $010D
        lda     system_flags
        ora     #$08
        sta     system_flags
        txa
        pha
        lda     $010D
        asl     a
        tax
        lda     #$06
        jsr     mmc3_set_bank
        inx
        lda     #$07
        jsr     mmc3_set_bank
        lda     system_flags
        and     #$F7
        sta     system_flags
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; from shadow copies $0104/$0106
apply_chr_banks_r0_r1:
        txa
        pha
        lda     chr_bank_shadows
        and     #$FE
        tax
        lda     #$00
        jsr     mmc3_set_bank
        lda     chr_bank_shadows+2
        and     #$FE
        tax
        lda     #$01
        jsr     mmc3_set_bank
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; from shadow copies $0108-$010B
apply_chr_banks_r2_r5:
        txa
        pha
        ldx     chr_bank_shadows+4
        lda     #$02
        jsr     mmc3_set_bank
        ldx     chr_bank_shadows+5
        lda     #$03
        jsr     mmc3_set_bank
        ldx     chr_bank_shadows+6
        lda     #$04
        jsr     mmc3_set_bank
        ldx     chr_bank_shadows+7
        lda     #$05
        jsr     mmc3_set_bank
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; A = register, X = bank; keeps mode bits in $010C
mmc3_set_bank:
        pha
        lda     bank_select_shadow
        and     #$C0
        sta     bank_select_shadow
        pla
        ora     bank_select_shadow
        sta     bank_select_shadow
        sta     MMC3_BANK_SELECT
        stx     MMC3_BANK_DATA
        rts

; ----------------------------------------------------------------------------
; 8 bank numbers follow the JSR; returns after them
set_chr_banks_inline:
        pla
        sta     inline_ptr
        pla
        sta     inline_ptr+1
        txa
        pha
        tya
        pha
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+1
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+2
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+3
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+4
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+5
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+6
        jsr     advance_inline_ptr
        ldy     #$00
        lda     (inline_ptr),y
        sta     chr_bank_shadows+7
        jsr     advance_inline_ptr
        jsr     apply_chr_banks
        pla
        tay
        pla
        tax
        jmp     (inline_ptr)

; ----------------------------------------------------------------------------
apply_chr_banks:
        jsr     apply_chr_banks_r0_r1
        jsr     apply_chr_banks_r2_r5
        rts

; ----------------------------------------------------------------------------
; $37/$38 += 1
advance_inline_ptr:
        lda     inline_ptr
        clc
        adc     #$01
        sta     inline_ptr
        lda     inline_ptr+1
        adc     #$00
        sta     inline_ptr+1
        rts

; ----------------------------------------------------------------------------
; RLE nametable follows the JSR (no callers found)
draw_rle_screen_2400:
        pla
        sta     inline_ptr
        pla
        sta     inline_ptr+1
        txa
        pha
        tya
        pha
        jsr     request_black_palette
        jsr     wait_nmi_work_done
        jsr     hide_bg_and_sprites
        jsr     disable_nmi
        jsr     advance_inline_ptr
        lda     PPUSTATUS
        lda     #$24
        sta     PPUADDR
        lda     #$00
        sta     PPUADDR
        sta     $1E
        sta     $1F
        sta     $1C
        jsr     rle_to_ppu_2400
        jsr     enable_nmi
        jsr     show_bg_and_sprites
        jsr     request_oam_dma
        jsr     request_palette_upload
        jsr     wait_nmi_work_done
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; RLE nametable follows the JSR; returns to caller's caller
draw_rle_screen_2000:
        pla
        sta     inline_ptr
        pla
        sta     inline_ptr+1
        txa
        pha
        tya
        pha
        jsr     request_black_palette
        jsr     wait_nmi_work_done
        jsr     hide_bg_and_sprites
        jsr     disable_nmi
        jsr     advance_inline_ptr
        lda     PPUSTATUS
        lda     #$20
        sta     PPUADDR
        lda     #$00
        sta     PPUADDR
        sta     $1E
        sta     $1F
        sta     $1C
        jsr     rle_to_ppu_2000
        jsr     enable_nmi
        jsr     show_bg_and_sprites
        jsr     request_oam_dma
        jsr     request_palette_upload
        jsr     wait_nmi_work_done
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
rle_to_ppu_2000:
        jsr     read_inline_byte
        cmp     #$FF
        beq     LEA25
        jsr     rle_write_2000
        jmp     rle_to_ppu_2000

; ----------------------------------------------------------------------------
LEA25:  jsr     read_inline_byte
        cmp     #$FF
        bne     LEA39
        jsr     read_inline_byte
        cmp     #$00
        bne     LEA34
        rts

; ----------------------------------------------------------------------------
LEA34:  tax
        lda     #$FF
        bne     LEA3F
LEA39:  pha
        jsr     read_inline_byte
        tax
        pla
LEA3F:  jsr     rle_write_2000
        dex
        bne     LEA3F
        beq     rle_to_ppu_2000
rle_write_2000:  pha
        sta     PPUDATA
        sta     $1D
        lda     $1F
        cmp     #$03
        bne     LEA66
        lda     $1E
        cmp     #$C0
        bcc     LEA66
        txa
        pha
        ldx     $1C
        lda     $1D
        sta     attr_buffer_2000,x
        inc     $1C
        pla
        tax
LEA66:  lda     $1E
        clc
        adc     #$01
        sta     $1E
        lda     $1F
        adc     #$00
        sta     $1F
        pla
        rts

; ----------------------------------------------------------------------------
rle_to_ppu_2400:
        jsr     read_inline_byte
        cmp     #$FF
        beq     LEA82
        jsr     rle_write_2400
        jmp     rle_to_ppu_2400

; ----------------------------------------------------------------------------
LEA82:  jsr     read_inline_byte
        cmp     #$FF
        bne     LEA96
        jsr     read_inline_byte
        cmp     #$00
        bne     LEA91
        rts

; ----------------------------------------------------------------------------
LEA91:  tax
        lda     #$FF
        bne     LEA9C
LEA96:  pha
        jsr     read_inline_byte
        tax
        pla
LEA9C:  jsr     rle_write_2400
        dex
        bne     LEA9C
        beq     rle_to_ppu_2400
rle_write_2400:  pha
        sta     PPUDATA
        sta     $1D
        lda     $1F
        cmp     #$03
        bne     LEA66
        lda     $1E
        cmp     #$C0
        bcc     LEA66
        txa
        pha
        ldx     $1C
        lda     $1D
        sta     attr_buffer_2400,x
        inc     $1C
        pla
        tax
        jmp     LEA66

; ----------------------------------------------------------------------------
read_inline_byte:
        ldy     #$00
        lda     (inline_ptr),y
        pha
        jsr     advance_inline_ptr
        pla
        rts

; ----------------------------------------------------------------------------
; A = bank; remembers the current one
push_and_switch_bank:
        pha
        lda     current_bank
        jsr     stack_push
        pla
set_current_bank:  sta     current_bank
        and     #$0F
        jmp     switch_prg_bank

; ----------------------------------------------------------------------------
switch_to_sound_bank:  lda     #$05
        jmp     switch_prg_bank

; ----------------------------------------------------------------------------
restore_current_bank:  lda     current_bank
        and     #$0F
        jmp     switch_prg_bank

; ----------------------------------------------------------------------------
pop_bank:
        pha
        txa
        pha
        tya
        pha
        jsr     stack_pop
        sta     current_bank
        jsr     switch_prg_bank
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
; The stack sits at $0700 + stack_index (indexing from sound_channels' base);
; stack_index starts at $FF and counts down, so it lives above the sound slots.
stack_push:  pha
        tya
        pha
        txa
        pha
        tsx
        ldy     stack_index
        lda     $0103,x
        sta     sound_channels,y
        dec     stack_index
        pla
        tax
        pla
        tay
        pla
        rts

; ----------------------------------------------------------------------------
; The stack sits at $0700 + stack_index (indexing from sound_channels' base);
; stack_index starts at $FF and counts down, so it lives above the sound slots.
stack_pop:  pha
        tya
        pha
        txa
        pha
        tsx
        inc     stack_index
        ldy     stack_index
        lda     sound_channels,y
        sta     $0103,x
        pla
        tax
        pla
        tay
        pla
        rts

; ----------------------------------------------------------------------------
disable_nmi:  lda     ppuctrl_shadow
        and     #$7F
        sta     ppuctrl_shadow
        sta     PPUCTRL
        rts

; ----------------------------------------------------------------------------
enable_nmi:  lda     PPUSTATUS
        lda     ppuctrl_shadow
        ora     #$80
        sta     ppuctrl_shadow
        sta     PPUCTRL
        rts

; ----------------------------------------------------------------------------
; bare RTI: the MMC3 scanline IRQ is unused
irq:    rti

; ----------------------------------------------------------------------------
; reads twice until both reads match
read_joypads:
        tya
        pha
        txa
        pha
        jsr     read_joypads_once
LEB48:  ldy     joy_pressed
        lda     joy_pressed+1
        pha
        jsr     read_joypads_once
        pla
        cmp     joy_pressed+1
        bne     LEB48
        cpy     joy_pressed
        bne     LEB48
        ldx     #$01
LEB5B:  lda     joy_pressed,x
        tay
        eor     joy_held,x
        and     joy_pressed,x
        sta     joy_pressed,x
        sty     joy_held,x
        dex
        bpl     LEB5B
        pla
        tax
        pla
        tay
        rts

; ----------------------------------------------------------------------------
read_joypads_once:
        ldx     $FB
        inx
        stx     JOY1
        dex
        stx     JOY1
        ldx     #$08
LEB7A:  lda     JOY1
        lsr     a
        rol     joy_pressed
        lsr     a
        rol     L0010
        lda     JOY2
        lsr     a
        rol     joy_pressed+1
        lsr     a
        rol     $11
        dex
        bne     LEB7A
        lda     L0010
        ora     joy_pressed
        sta     joy_pressed
        lda     $11
        ora     joy_pressed+1
        sta     joy_pressed+1
        rts

; ----------------------------------------------------------------------------
; sets $F0 bit 1, waits for the NMI to clear it
wait_vblank:
        lda     system_flags
        ora     #$02
        sta     system_flags
        lda     #$02
LEBA4:  and     system_flags
        bne     LEBA4
        rts

; ----------------------------------------------------------------------------
request_nmi_work:  ora     nmi_requests
        sta     nmi_requests
        rts

; ----------------------------------------------------------------------------
wait_vram_queue_empty:  lda     #$08
        bne     LEBB4
wait_nmi_work_done:  lda     #$FF
LEBB4:  and     nmi_requests
        bne     LEBB4
        rts

; ----------------------------------------------------------------------------
wait_vblank_and_hide_sprites:
        jsr     request_oam_dma
        jsr     wait_vblank
clear_oam_buffer:  txa
        pha
        lda     #$00
        sta     sprite0_reserved
        ldx     #$00
        lda     system_flags
        lsr     a
        bcc     LEBD0
        inc     sprite0_reserved
        ldx     #$04
LEBD0:  lda     #$F8
LEBD2:  sta     oam_buffer,x
        inx
        inx
        inx
        inx
        bne     LEBD2
        pla
        tax
        rts

; ----------------------------------------------------------------------------
clear_and_upload_sprites:  jsr     clear_oam_buffer
request_oam_dma:  lda     #$80
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
; A = index; table of addresses follows the JSR
jump_table:
        sty     $12
        asl     a
        tay
        iny
        pla
        sta     L0010
        pla
        sta     $11
        lda     (L0010),y
        pha
        iny
        lda     (L0010),y
        sta     $11
        pla
        sta     L0010
        ldy     $12
        lda     $0B
        jmp     (L0010)

; ----------------------------------------------------------------------------
far_call_8000_body:
        pha
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        jsr     push_and_switch_bank
        pla
        and     #$0F
        asl     a
        sty     $0A
        tay
        lda     $8000,y
        sta     L0010
        lda     $8001,y
        sta     $11
        jsr     far_call_jump
        sta     $0B
        jsr     pop_bank
        lda     $0B
        rts

; ----------------------------------------------------------------------------
far_call_8020_body:
        pha
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        jsr     push_and_switch_bank
        pla
        and     #$0F
        asl     a
        sty     $0A
        tay
        lda     $8020,y
        sta     L0010
        lda     $8021,y
        sta     $11
        jsr     far_call_jump
        sta     $0B
        jsr     pop_bank
        lda     $0B
        rts

; ----------------------------------------------------------------------------
far_call_8040_body:
        pha
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        jsr     push_and_switch_bank
        pla
        and     #$0F
        asl     a
        sty     $0A
        tay
        lda     $8040,y
        sta     L0010
        lda     $8041,y
        sta     $11
        jsr     far_call_jump
        sta     $0B
        jsr     pop_bank
        lda     $0B
        rts

; ----------------------------------------------------------------------------
far_call_8400_body:
        pha
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        jsr     push_and_switch_bank
        pla
        and     #$0F
        asl     a
        sty     $0A
        tay
        lda     $8400,y
        sta     L0010
        lda     $8401,y
        sta     $11
        jsr     far_call_jump
        sta     $0B
        jsr     pop_bank
        lda     $0B
        rts

; ----------------------------------------------------------------------------
far_call_jump:
        lda     $0B
        ldy     $0A
        jmp     (L0010)

; ----------------------------------------------------------------------------
; OAM DMA, PPU updates, sprite-0 split
nmi:    pha
        txa
        pha
        tya
        pha
        lda     PPUSTATUS
        lda     ppuctrl_shadow
        and     #$67
        ora     #$08
        sta     ppuctrl_shadow
        sta     PPUCTRL
        lda     nmi_requests
        asl     a
        bcc     nmi_after_oam_dma
        lda     #$00
        sta     OAMADDR
        lda     #$02
        sta     OAMDMA
        lda     nmi_requests
        and     #$7F
        sta     nmi_requests
nmi_after_oam_dma:  jsr     nmi_ppu_updates
        lda     nmi_requests
        and     #$01
        beq     nmi_after_ppumask
        lda     ppumask_shadow
        sta     PPUMASK
        lda     nmi_requests
        and     #$FE
        sta     nmi_requests
nmi_after_ppumask:  jsr     nmi_set_scroll
        inc     frame_counter
        lda     split_nametable
        and     #$01
        sta     split_nametable
        jsr     sound_update
        lda     system_flags
        lsr     a
        bcs     nmi_check_split
        jmp     nmi_done

; ----------------------------------------------------------------------------
nmi_check_split:  lda     ppumask_shadow
        and     #$18
        cmp     #$18
        beq     nmi_wait_sprite0_clear
        jmp     nmi_done

; ----------------------------------------------------------------------------
nmi_wait_sprite0_clear:  lda     PPUSTATUS
        and     #$40
        bne     nmi_wait_sprite0_clear
nmi_wait_sprite0_hit:  lda     PPUSTATUS
        and     #$C0
        beq     nmi_wait_sprite0_hit
        lda     ppuctrl_shadow
        and     #$E7
        ora     #$10
        ora     split_nametable
        sta     ppuctrl_shadow
        sta     PPUCTRL
nmi_done:  lda     PPUSTATUS
        lda     ppuctrl_shadow
        ora     #$80
        sta     ppuctrl_shadow
        sta     PPUCTRL
        pla
        tay
        pla
        tax
        pla
        rti

; ----------------------------------------------------------------------------
nmi_ppu_updates:  lda     nmi_requests
        asl     a
        asl     a
        bcc     LED2D
        jmp     nmi_upload_palette

; ----------------------------------------------------------------------------
LED2D:  asl     a
        bcc     LED33
        jmp     nmi_upload_attrs_2000

; ----------------------------------------------------------------------------
LED33:  asl     a
        bcc     LED39
        jmp     nmi_upload_attrs_2nd

; ----------------------------------------------------------------------------
LED39:  asl     a
        bcc     LED3F
        jmp     nmi_flush_vram_queue

; ----------------------------------------------------------------------------
LED3F:  asl     a
        bcc     LED45
        jmp     nmi_black_palette

; ----------------------------------------------------------------------------
LED45:  asl     a
        bcc     LED4B
        jmp     nmi_flush_vram_queue_down

; ----------------------------------------------------------------------------
LED4B:  rts

; ----------------------------------------------------------------------------
nmi_upload_attrs_2000:  lda     PPUSTATUS
        lda     #$23
        sta     PPUADDR
        lda     #$C0
        sta     PPUADDR
        ldx     #$00
LED5B:  lda     attr_buffer_2000,x
        sta     PPUDATA
        inx
        cpx     #$40
        bne     LED5B
        lda     nmi_requests
        and     #$DF
        sta     nmi_requests
        rts

; ----------------------------------------------------------------------------
nmi_upload_attrs_2nd:  lda     PPUSTATUS
        ldx     #$27
        lda     mirroring_mode
        and     #$08
        beq     LED7A
        ldx     #$2B
LED7A:  txa
        sta     PPUADDR
        lda     #$C0
        sta     PPUADDR
        ldx     #$00
LED85:  lda     attr_buffer_2400,x
        sta     PPUDATA
        inx
        cpx     #$40
        bne     LED85
        lda     nmi_requests
        and     #$EF
        sta     nmi_requests
        rts

; ----------------------------------------------------------------------------
nmi_flush_vram_queue:  ldx     #$00
        jsr     flush_vram_queue
        lda     nmi_requests
        and     #$F7
        sta     nmi_requests
        rts

; ----------------------------------------------------------------------------
nmi_flush_vram_queue_down:  lda     ppuctrl_shadow
        ora     #$04
        sta     PPUCTRL
        ldx     #$00
        jsr     flush_vram_queue
        lda     nmi_requests
        and     #$FD
        sta     nmi_requests
        lda     ppuctrl_shadow
        sta     PPUCTRL
        rts

; ----------------------------------------------------------------------------
flush_vram_entry:  lda     PPUSTATUS
        lda     vram_queue,x
        sta     PPUADDR
        inx
        lda     vram_queue,x
        sta     PPUADDR
        inx
        dey
        dey
        dey
        sty     vram_queue_len
        ldy     vram_queue,x
        inx
LEDD6:  lda     vram_queue,x
        inx
        sta     PPUDATA
        dec     vram_queue_len
        dey
        bne     LEDD6
flush_vram_queue:  ldy     vram_queue_len
        bne     flush_vram_entry
        lda     #$00
        sta     PPUADDR
        sta     PPUADDR
        rts

; ----------------------------------------------------------------------------
nmi_black_palette:  lda     PPUSTATUS
        jsr     ppu_addr_palette
        tay
LEDF8:  lda     #$0F
        sta     PPUDATA
        iny
        cpy     #$20
        bne     LEDF8
        lda     nmi_requests
        and     #$FB
        sta     nmi_requests
        jmp     ppu_addr_leave_palette

; ----------------------------------------------------------------------------
nmi_upload_palette:  lda     PPUSTATUS
        jsr     ppu_addr_palette
        tay
LEE12:  lda     palette_buffer,y
        sta     PPUDATA
        iny
        cpy     #$20
        bne     LEE12
        lda     nmi_requests
        and     #$BF
        sta     nmi_requests
ppu_addr_leave_palette:  jsr     ppu_addr_palette
        sta     PPUADDR
        sta     PPUADDR
        rts

; ----------------------------------------------------------------------------
ppu_addr_palette:  lda     #$3F
        sta     PPUADDR
        lda     #$00
        sta     PPUADDR
        rts

; ----------------------------------------------------------------------------
nmi_set_scroll:  lda     scroll_x+1
        sta     $08
        lda     scroll_y+1
        sta     $09
        lda     scroll_x
        sta     ppuscroll_x
        lda     scroll_y
        sta     ppuscroll_y
        lda     ppuctrl_shadow
        lsr     a
        lsr     a
        lsr     $09
        rol     a
        lsr     $08
        rol     a
        sta     ppuctrl_shadow
        sta     PPUCTRL
        lda     PPUSTATUS
        lda     ppuscroll_x
        sta     PPUSCROLL
        lda     ppuscroll_y
        sta     PPUSCROLL
        rts

; ----------------------------------------------------------------------------
clear_nametables:  jsr     rendering_off
        jsr     fill_nametable_2000
LEE6B:  jsr     fill_nametable_2nd
rendering_on:  jsr     enable_nmi
        jsr     request_oam_dma
        jsr     wait_vblank
        lda     ppumask_shadow
        sta     PPUMASK
        ldx     $11
        ldy     $12
        lda     ppuscroll_x
        sta     PPUSCROLL
        lda     ppuscroll_y
        sta     PPUSCROLL
        rts

; ----------------------------------------------------------------------------
; no caller found
clear_nametable_2000:
        jsr     rendering_off
        jsr     fill_nametable_2000
        jmp     rendering_on

; ----------------------------------------------------------------------------
; no caller found
clear_nametable_2nd:
        jsr     rendering_off
        jmp     LEE6B

; ----------------------------------------------------------------------------
rendering_off:  sta     L0010
        stx     $11
        sty     $12
        jsr     disable_nmi
        lda     ppumask_shadow
        and     #$E7
        sta     PPUMASK
        rts

; ----------------------------------------------------------------------------
fill_nametable_2000:  lda     PPUSTATUS
        lda     #$20
        bne     LEEC0
fill_nametable_2nd:  lda     PPUSTATUS
        ldx     #$24
        lda     mirroring_mode
        and     #$08
        beq     LEEBF
        ldx     #$28
LEEBF:  txa
LEEC0:  sta     PPUADDR
        lda     #$00
        tax
        tay
        sta     PPUADDR
        lda     L0010
LEECC:  sta     PPUDATA
        inx
        bne     LEECC
        iny
        cpy     #$03
        bne     LEECC
LEED7:  sta     PPUDATA
        inx
        cpx     #$C0
        bne     LEED7
        rts

; ----------------------------------------------------------------------------
request_vram_queue:  lda     #$08
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
queue_vram_block:  txa
        pha
        tya
        pha
        jsr     vram_block_setup
LEEEC:  jsr     vram_copy_rows
        jsr     next_vram_row
        bne     LEEEC
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
queue_vram_fill:  txa
        pha
        tya
        pha
        jsr     vram_block_setup
LEF00:  jsr     vram_fill_rows
        jsr     next_vram_row
        bne     LEF00
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
vram_block_setup:  jsr     wait_vram_queue_empty
        ldy     #$00
        lda     ($1E),y
        sta     $12
        iny
        lda     ($1E),y
        sta     $13
        jsr     skip_block_header
        lda     $1C
        sta     L0010
        lda     $1D
        sta     $11
        ldy     #$00
        rts

; ----------------------------------------------------------------------------
vram_fill_rows:  lda     L0010
        sta     $15
        lda     $11
        sta     $16
        lda     $12
        sta     $14
LEF35:  lda     #$00
        sta     $17
        lda     vram_queue_len
        tax
        inx
        inx
        inx
        ldy     #$00
LEF42:  lda     ($1E),y
        sta     vram_queue,x
        inx
        inc     $17
        dec     $14
        beq     vram_queue_header
        jsr     next_vram_address
        lda     $1C
        and     #$1F
        bne     LEF42
        jsr     vram_wrap_up
        jsr     flush_vram_queue_if_full
        jmp     LEF35

; ----------------------------------------------------------------------------
vram_copy_rows:  lda     L0010
        sta     $15
        lda     $11
        sta     $16
        lda     $12
        sta     $14
LEF6C:  lda     #$00
        sta     $17
        lda     vram_queue_len
        tax
        inx
        inx
        inx
LEF77:  lda     ($1E),y
        sta     vram_queue,x
        inc     $17
        iny
        inx
        dec     $14
        beq     vram_queue_header
        jsr     next_vram_address
        lda     $1C
        and     #$1F
        bne     LEF77
        jsr     vram_wrap_up
        jsr     flush_vram_queue_if_full
        jmp     LEF6C

; ----------------------------------------------------------------------------
next_vram_address:  lda     $1C
        clc
        adc     #$01
        sta     $1C
        lda     $1D
        adc     #$00
        sta     $1D
        rts

; ----------------------------------------------------------------------------
vram_wrap_up:  jsr     vram_queue_header
        lda     $1C
        sec
        sbc     #$20
        sta     $1C
        lda     $1D
        sbc     #$00
        sta     $1D
        lda     mirroring_mode
        and     #$08
        bne     LEFC0
        lda     $1D
        eor     #$04
        sta     $1D
LEFC0:  lda     $1C
        sta     $15
        lda     $1D
        sta     $16
        rts

; ----------------------------------------------------------------------------
vram_queue_header:  lda     vram_queue_len
        tax
        clc
        adc     $17
        adc     #$03
        sta     vram_queue_len
        lda     $16
        sta     vram_queue,x
        inx
        lda     $15
        sta     vram_queue,x
        inx
        lda     $17
        sta     vram_queue,x
        rts

; ----------------------------------------------------------------------------
next_vram_row:  jsr     flush_vram_queue_if_full
        lda     #$20
        clc
        adc     L0010
        sta     L0010
        lda     #$00
        adc     $11
        sta     $11
        lda     $11
        and     #$03
        cmp     #$03
        bne     LF01F
        lda     L0010
        and     #$C0
        cmp     #$C0
        bne     LF01F
        lda     L0010
        and     #$1F
        sta     L0010
        lda     $11
        and     #$FC
        sta     $11
        lda     mirroring_mode
        and     #$08
        beq     LF01F
        lda     $11
        eor     #$08
        sta     $11
LF01F:  lda     L0010
        sta     $1C
        lda     $11
        sta     $1D
        dec     $13
        rts

; ----------------------------------------------------------------------------
skip_block_header:  lda     $1E
        clc
        adc     #$02
        sta     $1E
        lda     $1F
        adc     #$00
        sta     $1F
        rts

; ----------------------------------------------------------------------------
flush_vram_queue_if_full:  lda     vram_queue_len
        cmp     #$1E
        bmi     LF045
        jsr     request_vram_queue
        jsr     wait_vram_queue_empty
LF045:  rts

; ----------------------------------------------------------------------------
flush_vram_queue_down_if_full:  lda     vram_queue_len
        cmp     #$1E
        bmi     LF053
        jsr     flush_vram_queue_down
        jsr     wait_nmi_work_done
LF053:  rts

; ----------------------------------------------------------------------------
flush_vram_queue_down:  jsr     wait_nmi_work_done
        lda     #$02
        jsr     request_nmi_work
        jsr     wait_nmi_work_done
        rts

; ----------------------------------------------------------------------------
queue_vram_column:  txa
        pha
        tya
        pha
        jsr     vram_column_setup
        jsr     vram_copy_column
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
vram_column_setup:  jsr     wait_nmi_work_done
        ldy     #$00
        lda     ($1E),y
        sta     $12
        lda     $1C
        sta     L0010
        lda     $1D
        sta     $11
        ldy     #$00
        lda     $1E
        clc
        adc     #$01
        sta     $1E
        lda     $1F
        adc     #$00
        sta     $1F
        rts

; ----------------------------------------------------------------------------
vram_copy_column:  lda     L0010
        sta     $15
        lda     $11
        sta     $16
        lda     $12
        sta     $14
LF09C:  lda     #$00
        sta     $17
        lda     vram_queue_len
        tax
        inx
        inx
        inx
LF0A7:  lda     ($1E),y
        sta     vram_queue,x
        iny
        inx
        inc     $17
        dec     $14
        bne     LF0B7
        jmp     vram_queue_header

; ----------------------------------------------------------------------------
LF0B7:  jsr     vram_addr_down_row
        lda     $1D
        and     #$03
        cmp     #$03
        bne     LF0A7
        lda     $1C
        and     #$C0
        cmp     #$C0
        bne     LF0A7
        jsr     vram_wrap_column
        jsr     flush_vram_queue_down_if_full
        jmp     LF09C

; ----------------------------------------------------------------------------
vram_addr_down_row:  lda     $1C
        clc
        adc     #$20
        sta     $1C
        lda     $1D
        adc     #$00
        sta     $1D
        rts

; ----------------------------------------------------------------------------
vram_wrap_column:  jsr     vram_queue_header
        lda     $1C
        and     #$1F
        sta     $1C
        lda     $1D
        and     #$FC
        sta     $1D
        lda     mirroring_mode
        and     #$08
        beq     LF0FC
        lda     $1C
        eor     #$08
        sta     $1C
LF0FC:  lda     $1C
        sta     $15
        lda     $1D
        sta     $16
        rts

; ----------------------------------------------------------------------------
attr_position:  lda     #$00
        sta     $11
        lda     $1D
        and     #$03
        sta     L0010
        lda     $1D
        and     #$0C
        beq     LF11B
        lda     #$04
        ora     L0010
        sta     L0010
LF11B:  lda     $1C
        asl     a
        rol     L0010
        asl     a
        rol     $11
        asl     a
        asl     a
        rol     L0010
        asl     a
        rol     L0010
        asl     a
        rol     L0010
        asl     a
        rol     $11
        rts

; ----------------------------------------------------------------------------
set_attr_block:
        txa
        pha
        tya
        pha
        lda     $1C
        and     #$DE
        sta     $1C
        sta     $15
        lda     $1D
        sta     $16
        lda     $21
        sta     $13
LF145:  lda     $20
        sta     $12
LF149:  jsr     set_attr_bits
        jsr     next_attr_column
        dec     $12
        dec     $12
        beq     LF157
        bpl     LF149
LF157:  jsr     next_attr_row
        dec     $13
        dec     $13
        beq     LF162
        bpl     LF145
LF162:  pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
next_attr_row:  lda     $15
        clc
        adc     #$40
        sta     $15
        lda     $16
        adc     #$00
        sta     $16
        lda     $16
        and     #$03
        cmp     #$03
        bne     LF19C
        lda     $15
        and     #$C0
        cmp     #$C0
        bne     LF19C
        lda     $15
        and     #$1E
        sta     $15
        lda     $16
        and     #$FC
        sta     $16
        lda     mirroring_mode
        and     #$08
        beq     LF19C
        lda     $16
        eor     #$08
        sta     $16
LF19C:  lda     $15
        sta     $1C
        lda     $16
        sta     $1D
        rts

; ----------------------------------------------------------------------------
next_attr_column:  lda     $1C
        clc
        adc     #$02
        sta     $1C
        lda     $1D
        adc     #$00
        sta     $1D
        lda     $1C
        and     #$1F
        bne     LF1D1
        lda     $1C
        sec
        sbc     #$20
        sta     $1C
        lda     $1D
        sbc     #$00
        sta     $1D
        lda     mirroring_mode
        and     #$08
        bne     LF1D1
        lda     $1D
        eor     #$04
        sta     $1D
LF1D1:  rts

; ----------------------------------------------------------------------------
set_attr_bits:  jsr     attr_position
        ldx     L0010
        ldy     $11
        lda     LF20A,y
        and     attr_buffer_2000,x
        sta     attr_buffer_2000,x
        sty     $14
        lda     $22
        lsr     a
        rol     $14
        lsr     a
        rol     $14
        ldy     $14
        lda     LF1FA,y
        sta     $14
        ora     attr_buffer_2000,x
        sta     attr_buffer_2000,x
        rts

; ----------------------------------------------------------------------------
LF1FA:  .byte   $00,$02,$01,$03,$00,$08,$04,$0C
        .byte   $00,$20,$10,$30,$00,$80,$40,$C0
LF20A:  .byte   $FC,$F3,$CF,$3F
; ----------------------------------------------------------------------------
fill_all_attrs:
        jsr     pick_attr_fill
        jsr     fill_attr_buffer_2000
        jsr     fill_attr_buffer_2400
        jsr     restore_xy
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F21B:
        jsr     pick_attr_fill
        jsr     fill_attr_buffer_2000
        jsr     restore_xy
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F225:
        jsr     pick_attr_fill
        jsr     fill_attr_buffer_2400
        jsr     restore_xy
        rts

; ----------------------------------------------------------------------------
pick_attr_fill:  sta     L0010
        stx     $11
        sty     $12
        jsr     wait_nmi_work_done
        lda     L0010
        and     #$03
        tay
        lda     LF243,y
        sta     $13
        rts

; ----------------------------------------------------------------------------
LF243:  .byte   $00,$55,$AA,$FF
; ----------------------------------------------------------------------------
fill_attr_buffer_2000:  ldx     #$00
        beq     LF24D
fill_attr_buffer_2400:  ldx     #$40
LF24D:  lda     $13
        ldy     #$00
LF251:  sta     attr_buffer_2000,x
        inx
        iny
        cpy     #$40
        bne     LF251
        rts

; ----------------------------------------------------------------------------
restore_xy:  ldx     $11
        ldy     $12
        rts

; ----------------------------------------------------------------------------
request_attrs_2000:
        lda     #$20
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
; no caller found
request_attrs_2nd:
        lda     #$10
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
request_attr_upload:  lda     #$30
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
request_black_palette:  lda     #$04
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
show_bg_and_sprites:  lda     #$18
        bne     LF27E
; no caller found
show_sprites:
        lda     #$10
        bne     LF27E
        .byte   $A9,$08
; ----------------------------------------------------------------------------
LF27E:  ora     ppumask_shadow
        bne     LF28E
hide_bg_and_sprites:  lda     #$E7
        bne     LF28C
; no caller found
hide_sprites:
        lda     #$EF
        bne     LF28C
        .byte   $A9,$F7
; ----------------------------------------------------------------------------
LF28C:  and     ppumask_shadow
LF28E:  sta     ppumask_shadow
        lda     #$01
        jsr     request_nmi_work
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
request_palette_upload:  lda     #$40
        jmp     request_nmi_work

; ----------------------------------------------------------------------------
fade_in_palette:
        tya
        pha
        ldy     #$00
LF2A2:  lda     ($1E),y
        and     #$0F
        sta     palette_buffer,y
        iny
        cpy     #$20
        bne     LF2A2
        jsr     palette_fade_wait
        jsr     palette_brighten_step
        jsr     palette_fade_wait
        jsr     palette_brighten_step
        jsr     palette_fade_wait
        jsr     palette_brighten_step
        jsr     palette_fade_wait
        pla
        tay
        rts

; ----------------------------------------------------------------------------
palette_brighten_step:  ldy     #$00
LF2C8:  lda     palette_buffer,y
        cmp     ($1E),y
        beq     LF2D5
        clc
        adc     #$10
        sta     palette_buffer,y
LF2D5:  iny
        cpy     #$20
        bne     LF2C8
        rts

; ----------------------------------------------------------------------------
fade_out_palette:
        txa
        pha
        jsr     palette_darken_step
        jsr     palette_fade_wait
        jsr     palette_darken_step
        jsr     palette_fade_wait
        jsr     palette_darken_step
        jsr     palette_fade_wait
        jsr     palette_darken_step
        jsr     palette_fade_wait
        pla
        tax
        rts

; ----------------------------------------------------------------------------
palette_darken_step:  ldx     #$00
LF2FA:  lda     palette_buffer,x
        cmp     #$0F
        beq     LF316
        and     #$F0
        beq     LF311
        lda     palette_buffer,x
        sec
        sbc     #$10
        sta     palette_buffer,x
        jmp     LF316

; ----------------------------------------------------------------------------
LF311:  lda     #$0F
        sta     palette_buffer,x
LF316:  inx
        cpx     #$20
        bne     LF2FA
        rts

; ----------------------------------------------------------------------------
palette_fade_wait:  jsr     request_palette_upload
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        jsr     wait_vblank
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F32F:
        sta     L0010
        txa
        pha
        ldx     #$00
        beq     LF33D
; no caller found
unreferenced_F337:
        sta     L0010
        txa
        pha
        ldx     #$04
LF33D:  lda     L0010
        clc
        adc     scroll_x,x
        sta     scroll_x,x
        lda     scroll_x+1,x
        adc     #$00
        sta     scroll_x+1,x
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F34D:
        sta     L0010
        txa
        pha
        ldx     #$00
        beq     LF35B
; no caller found
unreferenced_F355:
        sta     L0010
        txa
        pha
        ldx     #$04
LF35B:  lda     scroll_x,x
        sec
        sbc     L0010
        sta     scroll_x,x
        lda     scroll_x+1,x
        sbc     #$00
        sta     scroll_x+1,x
        pla
        tax
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F36B:
        sta     L0010
        txa
        pha
        ldx     #$00
        beq     LF379
; no caller found
unreferenced_F373:
        sta     L0010
        txa
        pha
        ldx     #$04
LF379:  lda     L0010
        clc
        adc     scroll_y,x
        sta     scroll_y,x
        sec
        sbc     #$F0
        bcc     LF38D
        sta     scroll_y,x
        inc     scroll_y+1,x
        lda     scroll_y+1,x
        sta     scroll_y+1,x
LF38D:  pla
        tax
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_F390:
        sta     L0010
        txa
        pha
        ldx     #$00
        beq     LF39E
; no caller found
unreferenced_F398:
        sta     L0010
        txa
        pha
        ldx     #$04
LF39E:  lda     scroll_y,x
        sec
        sbc     L0010
        sta     scroll_y,x
        bcs     LF3B2
        sec
        sbc     #$10
        sta     scroll_y,x
        inc     scroll_y+1,x
        lda     scroll_y+1,x
        sta     scroll_y+1,x
LF3B2:  pla
        tax
        rts

; ----------------------------------------------------------------------------
far_call_8000:
        sta     $76
        txa
        pha
        tya
        pha
        lda     $76
        jsr     far_call_8000_body
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
far_call_8020:
        sta     $76
        txa
        pha
        tya
        pha
        lda     $76
        jsr     far_call_8020_body
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
far_call_8040:
        sta     $76
        txa
        pha
        tya
        pha
        lda     $76
        jsr     far_call_8040_body
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
far_call_8400:
        sta     $76
        txa
        pha
        tya
        pha
        lda     $76
        jsr     far_call_8400_body
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
sound_reset:  pha
        txa
        pha
        lda     #$00
        tax
LF3FB:  sta     sound_channels,x
        inx
        cpx     #$D8
        bne     LF3FB
        sta     repeat_sound
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
; A = sound number from 1; 0 stops all
play_sound:
        pha
        lda     system_flags
        ora     #$04
        sta     system_flags
        lda     #$05
        jsr     push_and_switch_bank
        pla
        sta     $44
        bne     LF420
        jsr     sound_reset
        jmp     LF429

; ----------------------------------------------------------------------------
LF420:  cmp     repeat_sound
        beq     LF429
        dec     $44
        jsr     start_sound
LF429:  jsr     pop_bank
        lda     system_flags
        and     #$FB
        sta     system_flags
        rts

; ----------------------------------------------------------------------------
sound_check_repeat:  lda     sound_channels
        ora     sound_channels+24
        ora     sound_channels+48
        ora     sound_channels+72
        bmi     LF44C
        lda     repeat_sound
        sta     $44
        beq     sound_reset
        dec     $44
        jsr     start_sound
LF44C:  rts

; ----------------------------------------------------------------------------
; $44 = number - 1
start_sound:
        txa
        pha
        tya
        pha
        lda     $44
        asl     a
        tay
        lda     (sound_table_ptr),y
        sta     $45
        iny
        lda     (sound_table_ptr),y
        sta     $46
        jsr     sound_start_by_type
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
sound_start_by_type:  ldy     #$00
        lda     ($45),y
        jsr     sound_jump_table
        .addr   sound_start_music
        .addr   sound_start_repeating_music
        .addr   sound_start_effect_3ch
        .addr   sound_start_effect_2ch
        .addr   sound_start_effect_2ch
; ----------------------------------------------------------------------------
sound_start_music:  ldx     #$00
        ldy     #$01
        jsr     sound_start_channel
        jsr     sound_start_channel
        jsr     sound_start_channel
        jsr     sound_start_channel
        iny
        iny
        lda     ($45),y
        sta     sound_channels+7
        sta     sound_channels+31
        sta     sound_channels+55
        sta     sound_channels+79
        lda     #$00
        sta     sound_channels+15
        lda     #$10
        sta     sound_channels+39
        lda     #$20
        sta     sound_channels+63
        lda     #$30
        sta     sound_channels+87
        lda     #$00
        sta     repeat_sound
        rts

; ----------------------------------------------------------------------------
sound_start_channel:  lda     #$80
        sta     sound_channels,x
        lda     ($45),y
        sta     sound_channels+1,x
        iny
        lda     ($45),y
        sta     sound_channels+2,x
        iny
        lda     #$00
        sta     sound_channels+8,x
        sta     sound_channels+6,x
        lda     #$01
        sta     sound_channels+5,x
        lda     sound_channels+1,x
        ora     sound_channels+2,x
        bne     LF4DB
        lda     #$00
        sta     sound_channels,x
LF4DB:  txa
        clc
        adc     #$18
        tax
        rts

; ----------------------------------------------------------------------------
sound_start_repeating_music:  jsr     sound_start_music
        lda     $44
        sta     repeat_sound
        inc     repeat_sound
        rts

; ----------------------------------------------------------------------------
sound_start_effect_3ch:  ldx     #$60
        ldy     #$01
        jsr     sound_start_channel
        jsr     sound_start_channel
        jsr     sound_start_channel
        iny
        iny
        iny
        iny
        lda     ($45),y
        sta     sound_channels+103
        sta     sound_channels+127
        sta     sound_channels+151
        lda     #$40
        sta     sound_channels+111
        lda     #$50
        sta     sound_channels+135
        lda     #$60
        sta     sound_channels+159
        rts

; ----------------------------------------------------------------------------
sound_start_effect_2ch:  ldy     #$03
        ldx     #$A8
        jsr     sound_start_channel
        iny
        iny
        jsr     sound_start_channel
        ldy     #$0B
        lda     ($45),y
        sta     sound_channels+175
        sta     sound_channels+199
        lda     #$70
        sta     sound_channels+183
        lda     #$80
        sta     sound_channels+207
        rts

; ----------------------------------------------------------------------------
; from the NMI
sound_update:
        lda     system_flags
        and     #$FD
        sta     system_flags
        and     #$0C
        beq     LF549
        lda     system_flags
        ora     #$80
        sta     system_flags
        rts

; ----------------------------------------------------------------------------
LF549:  jsr     switch_to_sound_bank
        jsr     sound_update_all
        jsr     restore_current_bank
        rts

; ----------------------------------------------------------------------------
sound_update_all:  lda     system_flags
        bpl     sound_step_all_channels
        and     #$7F
        sta     system_flags
        jsr     sound_step_all_channels
sound_step_all_channels:  ldx     #$00
        jsr     sound_step_pulse
        jsr     sound_pulse_envelope
        ldx     #$18
        jsr     sound_step_pulse
        jsr     sound_pulse_envelope
        ldx     #$30
        jsr     sound_step_triangle
        jsr     sound_triangle_countdown
        ldx     #$48
        jsr     sound_step_noise
        ldx     #$60
        jsr     sound_step_pulse
        jsr     sound_pulse_envelope
        ldx     #$78
        jsr     sound_step_pulse
        jsr     sound_pulse_envelope
        ldx     #$90
        jsr     sound_step_triangle
        jsr     sound_triangle_countdown
        ldx     #$A8
        jsr     sound_step_pulse
        jsr     sound_pulse_envelope
        ldx     #$C0
        jsr     sound_step_noise
        jsr     sound_write_apu
        rts

; ----------------------------------------------------------------------------
sound_step_pulse:  lda     sound_channels,x
        bpl     LF5FC
        jsr     sound_tick
        bpl     LF5FC
        jsr     sound_read_command
        bcs     LF60B
        cmp     #$7F
        beq     LF5FD
        asl     a
        tay
        lda     sound_channels+19,x
        and     #$F8
        sta     sound_channels+19,x
        lda     note_periods,y
        ora     sound_channels+19,x
        sta     sound_channels+19,x
        lda     LFB1C,y
        sta     sound_channels+18,x
        lda     sound_channels+9,x
        bpl     LF5E5
        lda     sound_channels,x
        ora     #$40
        and     #$DF
        sta     sound_channels,x
        lda     #$00
        sta     sound_channels+12,x
        rts

; ----------------------------------------------------------------------------
LF5E5:  lda     #$00
        sta     sound_channels+13,x
        sta     sound_channels+14,x
        lda     #$01
        sta     sound_channels+12,x
        lda     sound_channels,x
        ora     #$40
        and     #$DF
        sta     sound_channels,x
LF5FC:  rts

; ----------------------------------------------------------------------------
LF5FD:  lda     #$00
        sta     sound_channels+12,x
        lda     sound_channels,x
        ora     #$20
        sta     sound_channels,x
        rts

; ----------------------------------------------------------------------------
LF60B:  jsr     sound_check_repeat
        rts

; ----------------------------------------------------------------------------
sound_pulse_envelope:  lda     sound_channels+12,x
        beq     LF645
        ldy     sound_channels+13,x
        cpy     #$3F
        beq     LF629
        dec     sound_channels+14,x
        bpl     LF629
        lda     sound_channels+9,x
        sta     sound_channels+14,x
        inc     sound_channels+13,x
LF629:  lda     sound_channels+10,x
        sta     $46
        lda     sound_channels+11,x
        sta     $47
        lda     ($46),y
        and     #$0F
        sta     $48
        lda     sound_channels+16,x
        and     #$C0
        ora     #$30
        ora     $48
        sta     sound_channels+16,x
LF645:  rts

; ----------------------------------------------------------------------------
sound_step_triangle:  lda     sound_channels,x
        bpl     LF68F
        jsr     sound_tick
        bpl     LF68F
        jsr     sound_read_command
        bcs     LF690
        cmp     #$7F
        beq     LF5FD
        asl     a
        tay
        lda     sound_channels+19,x
        and     #$F8
        sta     sound_channels+19,x
        lda     note_periods,y
        sta     sound_channels+20,x
        ora     sound_channels+19,x
        sta     sound_channels+19,x
        lda     LFB1C,y
        sta     sound_channels+18,x
        lda     sound_channels+21,x
        cmp     #$01
        beq     LF694
        cmp     #$02
        beq     LF6AC
        lda     #$00
        sta     sound_channels+12,x
        lda     sound_channels,x
        ora     #$40
        and     #$DF
        sta     sound_channels,x
LF68F:  rts

; ----------------------------------------------------------------------------
LF690:  jsr     sound_check_repeat
        rts

; ----------------------------------------------------------------------------
LF694:  lda     #$01
        sta     sound_channels+12,x
        lda     sound_channels+13,x
LF69C:  jsr     multiply_8x8
        lda     $44
        sta     sound_channels+22,x
        lda     $45
        sta     sound_channels+23,x
        jmp     LF6D2

; ----------------------------------------------------------------------------
LF6AC:  lda     #$01
        sta     sound_channels+12,x
        lda     sound_channels+13,x
        sec
        sbc     #$01
        bcc     LF69C
        jsr     multiply_8x8
        sec
        lda     sound_channels+5,x
        sbc     $44
        sta     sound_channels+22,x
        lda     sound_channels+6,x
        sbc     $45
        sta     sound_channels+22,x
        bcs     LF6D2
        jmp     LF5FD

; ----------------------------------------------------------------------------
LF6D2:  lda     #$BF
        sta     sound_channels+16,x
        lda     sound_channels+20,x
        ora     #$08
        sta     sound_channels+19,x
        lda     sound_channels,x
        ora     #$40
        and     #$DF
        sta     sound_channels,x
        rts

; ----------------------------------------------------------------------------
multiply_8x8:  sta     $49
        lda     sound_channels+7,x
        sta     $48
        lda     #$00
        sta     $45
        sta     $44
        txa
        pha
        ldx     #$08
LF6FB:  lsr     $49
        bcc     LF706
        lda     $48
        clc
        adc     $45
        sta     $45
LF706:  ror     $45
        ror     $44
        dex
        bne     LF6FB
        pla
        tax
        rts

; ----------------------------------------------------------------------------
sound_triangle_countdown:  lda     sound_channels+12,x
        beq     LF732
        sec
        lda     sound_channels+22,x
        sbc     sound_channels+7,x
        sta     sound_channels+22,x
        lda     sound_channels+23,x
        sbc     sound_channels+8,x
        sta     sound_channels+23,x
        bcs     LF732
        lda     #$00
        sta     sound_channels+16,x
        sta     sound_channels+12,x
LF732:  rts

; ----------------------------------------------------------------------------
sound_step_noise:  lda     sound_channels,x
        bpl     LF78A
        jsr     sound_tick
        bpl     LF78A
        jsr     sound_read_command
        bcs     LF77E
        cmp     #$7F
        beq     LF782
        sec
        sbc     #$04
        bcc     LF782
LF74B:  sec
        sbc     #$0C
        bcs     LF74B
        adc     #$0C
        sta     $48
        asl     a
        adc     $48
        tay
        lda     sound_channels+10,x
        sta     $44
        lda     sound_channels+11,x
        sta     $45
        lda     ($44),y
        sta     sound_channels+16,x
        iny
        lda     ($44),y
        sta     sound_channels+18,x
        iny
        lda     ($44),y
        sta     sound_channels+19,x
        lda     sound_channels,x
        ora     #$40
        and     #$DF
        sta     sound_channels,x
        rts

; ----------------------------------------------------------------------------
LF77E:  jsr     sound_check_repeat
        rts

; ----------------------------------------------------------------------------
LF782:  lda     sound_channels,x
        ora     #$20
        sta     sound_channels,x
LF78A:  rts

; ----------------------------------------------------------------------------
sound_tick:  lda     sound_channels+5,x
        sec
        sbc     sound_channels+7,x
        sta     sound_channels+5,x
        lda     sound_channels+6,x
        sbc     sound_channels+8,x
        sta     sound_channels+6,x
        rts

; ----------------------------------------------------------------------------
sound_add_length:  lda     sound_channels+4,x
        sta     $48
        lda     sound_channels+3,x
        asl     a
        rol     $48
        asl     a
        rol     $48
        asl     a
        rol     $48
        clc
        adc     sound_channels+5,x
        sta     sound_channels+5,x
        lda     $48
        adc     sound_channels+6,x
        sta     sound_channels+6,x
        rts

; ----------------------------------------------------------------------------
; next command of the channel at X
sound_read_command:
        lda     sound_channels+1,x
        sta     $46
        lda     sound_channels+2,x
        sta     $47
        ldy     #$00
LF7CC:  lda     ($46),y
        sta     $4A
        iny
        cmp     #$FF
        beq     LF81A
        cmp     #$7E
        beq     LF7FD
        cmp     #$80
        bcc     LF7E8
        cmp     #$C0
        bcc     LF803
        cmp     #$E0
        bcs     LF827
        jmp     LF8A2

; ----------------------------------------------------------------------------
LF7E8:  jsr     sound_add_length
        tya
        clc
        adc     $46
        sta     sound_channels+1,x
        lda     $47
        adc     #$00
        sta     sound_channels+2,x
        lda     $4A
        clc
        rts

; ----------------------------------------------------------------------------
LF7FD:  jsr     sound_add_length
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF803:  sty     $4B
        and     #$1F
        asl     a
        tay
        lda     note_lengths,y
        sta     sound_channels+3,x
        lda     LFAF6,y
        sta     sound_channels+4,x
        ldy     $4B
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF81A:  lda     #$00
        sta     sound_channels,x
        sta     sound_channels+2,x
        jsr     sound_channel_done
        sec
        rts

; ----------------------------------------------------------------------------
LF827:  lda     ($46),y
        sta     $44
        iny
        lda     ($46),y
        sta     $45
        iny
        lda     $4A
        cmp     #$E0
        beq     LF846
        cmp     #$E1
        beq     LF846
        cmp     #$E2
        beq     LF875
        cmp     #$E3
        beq     LF895
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF846:  sty     $4B
        ldy     #$00
        lda     ($44),y
        sta     sound_channels+16,x
        iny
        lda     ($44),y
        sta     sound_channels+17,x
        iny
        lda     ($44),y
        sta     sound_channels+19,x
        iny
        lda     ($44),y
        sta     sound_channels+9,x
        iny
        tya
        clc
        adc     $44
        sta     sound_channels+10,x
        lda     #$00
        adc     $45
        sta     sound_channels+11,x
        ldy     $4B
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF875:  sty     $4B
        ldy     #$00
        lda     ($44),y
        sta     sound_channels+16,x
        iny
        lda     ($44),y
        sta     sound_channels+19,x
        iny
        lda     ($44),y
        sta     sound_channels+21,x
        iny
        lda     ($44),y
        sta     sound_channels+13,x
        ldy     $4B
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF895:  lda     $44
        sta     sound_channels+10,x
        lda     $45
        sta     sound_channels+11,x
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF8A2:  cmp     #$CF
        beq     LF8DB
        bcs     LF913
        and     #$0F
        sta     $48
        tya
        clc
        adc     $46
        sta     $46
        lda     $47
        adc     #$00
        sta     $47
        ldy     sound_channels+15,x
        lda     #$00
        sta     $0110,y
        lda     $48
        sta     $0111,y
        lda     $46
        sta     $0112,y
        lda     $47
        sta     $0113,y
        jsr     sound_loop_push
        tya
        sta     sound_channels+15,x
LF8D6:  ldy     #$00
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF8DB:  sty     $4B
        ldy     sound_channels+15,x
        jsr     sound_loop_pop
        bcs     LF904
        lda     $0110,y
        clc
        adc     #$01
        sta     $0110,y
        cmp     $0111,y
        beq     LF900
        lda     $0112,y
        sta     $46
        lda     $0113,y
        sta     $47
        jmp     LF8D6

; ----------------------------------------------------------------------------
LF900:  tya
        sta     sound_channels+15,x
LF904:  ldy     $4B
LF906:  lda     ($46),y
        iny
        and     #$F0
        cmp     #$D0
        beq     LF906
        dey
        jmp     LF7CC

; ----------------------------------------------------------------------------
LF913:  and     #$0F
        sta     $45
        dec     $45
        sty     $4B
        ldy     sound_channels+15,x
        jsr     sound_loop_pop
        lda     $0110,y
        cmp     $45
        beq     LF904
        ldy     $4B
LF92A:  lda     ($46),y
        sta     $48
        iny
        cpy     #$00
        bne     LF935
        inc     $47
LF935:  cmp     #$CF
        beq     LF8DB
        and     #$F0
        cmp     #$D0
        bne     LF92A
        lda     $48
        jmp     LF913

; ----------------------------------------------------------------------------
sound_loop_push:  tya
        and     #$F0
        sta     $48
        iny
        iny
        iny
        iny
        tya
        and     #$0F
        ora     $48
        tay
        rts

; ----------------------------------------------------------------------------
sound_loop_pop:  tya
        and     #$F0
        sta     $48
        tya
        and     #$0F
        beq     LF96A
        dey
        dey
        dey
        dey
        tya
        and     #$0F
        ora     $48
        tay
        clc
        rts

; ----------------------------------------------------------------------------
LF96A:  sec
        rts

; ----------------------------------------------------------------------------
sound_note_off:  lda     sound_channels,x
        and     #$BF
        sta     sound_channels,x
        rts

; ----------------------------------------------------------------------------
sound_channel_done:  cpx     #$60
        beq     LF98A
        cpx     #$78
        beq     LF99B
        cpx     #$90
        beq     LF9A4
        cpx     #$A8
        beq     LF993
        cpx     #$C0
        beq     LF9AD
        rts

; ----------------------------------------------------------------------------
LF98A:  lda     sound_channels
        ora     #$40
        sta     sound_channels
        rts

; ----------------------------------------------------------------------------
LF993:  lda     sound_channels+120
        ora     #$40
        sta     sound_channels+120
LF99B:  lda     sound_channels+24
        ora     #$40
        sta     sound_channels+24
        rts

; ----------------------------------------------------------------------------
LF9A4:  lda     sound_channels+48
        ora     #$40
        sta     sound_channels+48
        rts

; ----------------------------------------------------------------------------
LF9AD:  lda     sound_channels+72
        ora     #$40
        sta     sound_channels+72
        rts

; ----------------------------------------------------------------------------
sound_write_apu:  lda     #$00
        sta     $53
        lda     sound_channels+96
        bmi     LF9C4
        lda     sound_channels
        bpl     LF9CC
LF9C4:  and     #$20
        bne     LF9CC
        lda     #$01
        sta     $53
LF9CC:  lda     sound_channels+168
        bmi     LF9DB
        lda     sound_channels+120
        bmi     LF9DB
        lda     sound_channels+24
        bpl     LF9E5
LF9DB:  and     #$20
        bne     LF9E5
        lda     $53
        ora     #$02
        sta     $53
LF9E5:  lda     sound_channels+144
        bmi     LF9EF
        lda     sound_channels+48
        bpl     LF9F9
LF9EF:  and     #$20
        bne     LF9F9
        lda     $53
        ora     #$04
        sta     $53
LF9F9:  lda     sound_channels+192
        bmi     LFA03
        lda     sound_channels+72
        bpl     LFA0D
LFA03:  and     #$20
        bne     LFA0D
        lda     $53
        ora     #$08
        sta     $53
LFA0D:  lda     $53
        sta     SND_CHN
        ldx     #$60
        lda     sound_channels,x
        bmi     LFA20
        ldx     #$00
        lda     sound_channels,x
        bpl     LFA42
LFA20:  lda     sound_channels+16,x
        sta     SQ1_VOL
        lda     sound_channels,x
        and     #$40
        beq     LFA42
        lda     sound_channels+17,x
        sta     SQ1_SWEEP
        lda     sound_channels+18,x
        sta     SQ1_LO
        lda     sound_channels+19,x
        sta     SQ1_HI
        jsr     sound_note_off
LFA42:  ldx     #$A8
        lda     sound_channels,x
        bmi     LFA57
        ldx     #$78
        lda     sound_channels,x
        bmi     LFA57
        ldx     #$18
        lda     sound_channels,x
        bpl     LFA79
LFA57:  lda     sound_channels+16,x
        sta     SQ2_VOL
        lda     sound_channels,x
        and     #$40
        beq     LFA79
        lda     sound_channels+17,x
        sta     SQ2_SWEEP
        lda     sound_channels+18,x
        sta     SQ2_LO
        lda     sound_channels+19,x
        sta     SQ2_HI
        jsr     sound_note_off
LFA79:  ldx     #$90
        lda     sound_channels,x
        bmi     LFA87
        ldx     #$30
        lda     sound_channels,x
        bpl     LFAA9
LFA87:  lda     sound_channels+16,x
        sta     TRI_LINEAR
        lda     sound_channels,x
        and     #$40
        beq     LFAA9
        lda     sound_channels+17,x
        sta     $4009
        lda     sound_channels+18,x
        sta     TRI_LO
        lda     sound_channels+19,x
        sta     TRI_HI
        jsr     sound_note_off
LFAA9:  ldx     #$C0
        lda     sound_channels,x
        bmi     LFAB7
        ldx     #$48
        lda     sound_channels,x
        bpl     LFAD9
LFAB7:  lda     sound_channels+16,x
        sta     NOISE_VOL
        lda     sound_channels,x
        and     #$40
        beq     LFAD9
        lda     sound_channels+17,x
        sta     $400D
        lda     sound_channels+18,x
        sta     NOISE_LO
        lda     sound_channels+19,x
        sta     NOISE_HI
        jsr     sound_note_off
LFAD9:  rts

; ----------------------------------------------------------------------------
; same pattern, used by the sound code
sound_jump_table:
        sty     $53
        asl     a
        tay
        iny
        pla
        sta     L0051
        pla
        sta     $52
        lda     (L0051),y
        pha
        iny
        lda     (L0051),y
        sta     $52
        pla
        sta     L0051
        ldy     $53
        jmp     (L0051)

; ----------------------------------------------------------------------------
; length commands $80-$9F
note_lengths:
        .byte   $C0
LFAF6:  .byte   $00,$60,$00,$30,$00,$18,$00,$0C
        .byte   $00,$06,$00,$03,$00,$20,$01,$90
        .byte   $00,$48,$00,$24,$00,$12,$00,$09
        .byte   $00,$80,$00,$40,$00,$20,$00,$10
        .byte   $00,$08,$00,$04,$00
; timer value per note
note_periods:
        .byte   $0F
LFB1C:  .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$06
        .byte   $AE,$06,$4E,$05,$F3,$05,$9E,$05
        .byte   $4D,$05,$01,$04,$B9,$04,$75,$04
        .byte   $35,$03,$F8,$03,$BF,$03,$89,$03
        .byte   $57,$03,$27,$02,$F9,$02,$CF,$02
        .byte   $A6,$02,$80,$02,$5C,$02,$3A,$02
        .byte   $1A,$01,$FC,$01,$DF,$01,$C4,$01
        .byte   $AB,$01,$93,$01,$7C,$01,$67,$01
        .byte   $52,$01,$3F,$01,$2D,$01,$1C,$01
        .byte   $0C,$00,$FD,$00,$EE,$00,$E1,$00
        .byte   $D4,$00,$C8,$00,$BD,$00,$B2,$00
        .byte   $A8,$00,$9F,$00,$96,$00,$8D,$00
        .byte   $85,$00,$7E,$00,$76,$00,$70,$00
        .byte   $69,$00,$63,$00,$5E,$00,$58,$00
        .byte   $53,$00,$4F,$00,$4A,$00,$46,$00
        .byte   $42,$00,$3E,$00,$3A,$00,$37,$00
        .byte   $34,$00,$31,$00,$2E,$00,$2B,$00
        .byte   $29,$00,$27,$00,$24,$00,$22,$00
        .byte   $20,$00,$1E,$00,$1C,$00,$1A,$00
        .byte   $19,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF,$0F,$FF,$0F,$FF,$0F,$FF,$0F
        .byte   $FF
; ----------------------------------------------------------------------------
draw_layout_number:
        sta     data_ptr
        pha
; A = layout number, table at 6:BC00
draw_layout:
        txa
        pha
        tya
        pha
        lda     #$06
        jsr     push_and_switch_bank
        lda     #$00
        sta     data_ptr+1
        asl     data_ptr
        rol     data_ptr+1
        lda     #$00
        clc
        adc     data_ptr
        sta     data_ptr
        lda     #$BC
        adc     data_ptr+1
        sta     data_ptr+1
        ldy     #$00
        lda     (data_ptr),y
        sta     layout_ptr
        iny
        lda     (data_ptr),y
        sta     layout_ptr+1
        jsr     layout_run
        jsr     pop_bank
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
layout_run:  jsr     layout_reset_state
LFC28:  jsr     layout_fetch
        lda     $77
        cmp     #$FF
        beq     LFC37
        jsr     layout_command
        jmp     LFC28

; ----------------------------------------------------------------------------
LFC37:  rts

; ----------------------------------------------------------------------------
layout_reset_state:  lda     #$00
        sta     $98
        sta     $9A
        sta     $9B
        sta     $9E
        sta     $9F
        sta     $A0
        sta     $A1
        sta     $A2
        sta     $A3
        sta     $A4
        rts

; ----------------------------------------------------------------------------
layout_fetch:  ldy     #$00
        lda     (layout_ptr),y
        sta     $77
        iny
        lda     (layout_ptr),y
        sta     $78
        iny
        lda     (layout_ptr),y
        sta     $79
        rts

; ----------------------------------------------------------------------------
layout_advance_1:  lda     layout_ptr
        clc
        adc     #$01
        sta     layout_ptr
        lda     layout_ptr+1
        adc     #$00
        sta     layout_ptr+1
        rts

; ----------------------------------------------------------------------------
layout_advance_2:  jsr     layout_advance_1
        jmp     layout_advance_1

; ----------------------------------------------------------------------------
layout_advance_3:  jsr     layout_advance_1
        jmp     layout_advance_2

; ----------------------------------------------------------------------------
layout_put_tile:  lda     $9C
        sta     scene_index
        lda     $9D
        sta     $76
        jsr     layout_draw_tile
        inc     $9C
        rts

; ----------------------------------------------------------------------------
layout_command:
        lda     $77
        cmp     #$13
        bcs     LFCB7
        jsr     jump_table
        .addr   layout_name_by_index_command
        .addr   layout_byte_padded_command
        .addr   layout_word_padded_command
        .addr   layout_name_at_address_command
        .addr   layout_byte_command
        .addr   layout_word_command
        .addr   layout_position_command
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   LFCB7
        .addr   layout_name_in_ram_command
; ----------------------------------------------------------------------------
LFCB7:  lda     $77
        cmp     #$90
        beq     LFCC4
        cmp     #$A0
        beq     LFCC4
LFCC1:  jsr     layout_put_tile
LFCC4:  jsr     layout_advance_1
        rts

; ----------------------------------------------------------------------------
; no caller found
unreferenced_FCC8:
        lda     $98
        bmi     LFCC1
        bpl     LFCC4
layout_name_by_index_command:  jsr     layout_advance_2
        jsr     layout_name_from_index
        jsr     layout_print_name_buffer
        rts

; ----------------------------------------------------------------------------
layout_name_at_address_command:  jsr     layout_advance_3
        jsr     layout_name_from_address
        jsr     layout_print_name_buffer
        rts

; ----------------------------------------------------------------------------
layout_name_in_ram_command:  jsr     layout_advance_3
        jsr     layout_name_from_ram
        jsr     layout_print_name_buffer
        rts

; ----------------------------------------------------------------------------
layout_name_from_ram:  ldy     #$00
        lda     ($78),y
        jmp     LFCF5

; ----------------------------------------------------------------------------
layout_name_from_index:  lda     $78
LFCF5:  sta     $9A
        lda     #$00
        sta     $9B
        asl     $9A
        rol     $9B
        asl     $9A
        rol     $9B
        asl     $9A
        rol     $9B
        lda     #$00
        clc
        adc     $9A
        sta     $9A
        lda     #$8D
        adc     $9B
        sta     $9B
        lda     #$02
        jsr     push_and_switch_bank
        jsr     copy_name_to_buffer
        jsr     pop_bank
        rts

; ----------------------------------------------------------------------------
layout_name_from_address:  lda     $78
        sta     $9A
        lda     $79
        sta     $9B
        jsr     copy_name_to_buffer
        rts

; ----------------------------------------------------------------------------
copy_name_to_buffer:  ldy     #$00
LFD2E:  lda     ($9A),y
        sta     name_buffer,y
        iny
        cpy     #$08
        bne     LFD2E
        rts

; ----------------------------------------------------------------------------
layout_print_name_buffer:  lda     $98
        ora     #$40
        sta     $98
        ldx     #$00
LFD41:  lda     name_buffer,x
        sta     $77
        cmp     #$FF
        beq     LFD5F
        cmp     #$90
        beq     LFD5A
        cmp     #$A0
        beq     LFD5A
        lda     name_buffer+1,x
        sta     $78
        jsr     layout_put_tile
LFD5A:  inx
        cpx     #$08
        bne     LFD41
LFD5F:  lda     $98
        and     #$BF
        sta     $98
        rts

; ----------------------------------------------------------------------------
layout_byte_padded_command:  jsr     layout_advance_3
        jsr     layout_set_vram_dest
        jsr     layout_print_byte_padded
        jsr     layout_skip_3
        rts

; ----------------------------------------------------------------------------
layout_word_padded_command:  jsr     layout_advance_3
        jsr     layout_set_vram_dest
        jsr     layout_print_word_padded
        jsr     layout_skip_5
        rts

; ----------------------------------------------------------------------------
layout_byte_command:  jsr     layout_advance_3
        jsr     layout_set_vram_dest
        jsr     layout_print_byte
        jsr     layout_advance_x
        rts

; ----------------------------------------------------------------------------
layout_word_command:  jsr     layout_advance_3
        jsr     layout_set_vram_dest
        jsr     layout_print_word
        jsr     layout_advance_x
        rts

; ----------------------------------------------------------------------------
layout_advance_x:  lda     $9C
        clc
        adc     $84
        sta     $9C
        rts

; ----------------------------------------------------------------------------
layout_set_vram_dest:  lda     $78
        sta     $1C
        lda     $79
        sta     $1D
        lda     $9C
        sta     $0E
        lda     $9D
        sta     $0F
        rts

; ----------------------------------------------------------------------------
layout_skip_5:  inc     $9C
        inc     $9C
layout_skip_3:  inc     $9C
        inc     $9C
        inc     $9C
        rts

; ----------------------------------------------------------------------------
layout_position_command:  jsr     layout_advance_3
        lda     $78
        sta     $9C
        lda     $79
        sta     $9D
        rts

; ----------------------------------------------------------------------------
layout_draw_tile:  jsr     layout_tile_position
        jsr     layout_tile_kind
        jsr     tile_position_to_vram
        jsr     queue_tile
        rts

; ----------------------------------------------------------------------------
layout_tile_position:  lda     scene_index
        sta     $0E
        lda     $76
        sta     $0F
        rts

; ----------------------------------------------------------------------------
tile_position_to_vram:  lda     $0F
        sta     $1C
        lda     #$00
        sta     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        asl     $1C
        rol     $1D
        lda     $1C
        clc
        adc     $0E
        sta     $1C
        lda     $1D
        adc     #$20
        sta     $1D
        rts

; ----------------------------------------------------------------------------
layout_tile_kind:  dec     $0F
        lda     $77
        cmp     #$16
        bcc     LFE22
        cmp     #$50
        bcc     LFE3D
        cmp     #$60
        bcc     LFE32
        cmp     #$70
        bcc     LFE3D
        cmp     #$80
        bcc     LFE32
LFE22:  lda     $77
        cmp     #$FE
        beq     LFE2E
LFE28:  sta     $A1
        lda     #$DE
        bne     LFE46
LFE2E:  lda     #$7E
        bne     LFE28
LFE32:  lda     $77
        clc
        adc     #$70
        sta     $A1
        lda     #$A0
        bne     LFE46
LFE3D:  lda     $77
        clc
        adc     #$80
        sta     $A1
        lda     #$90
LFE46:  sta     $A0
        lda     #$02
        sta     $9F
        lda     #$01
        sta     $9E
        rts

; ----------------------------------------------------------------------------
queue_tile:  lda     #$9E
        sta     $1E
        lda     #$00
        sta     $1F
        jsr     queue_vram_block
        rts

; ----------------------------------------------------------------------------
draw_metasprite:  txa
        pha
        tya
        pha
        dec     $21
        lda     sprite0_reserved
        asl     a
        asl     a
        tax
        ldy     #$00
        lda     ($1E),y
        and     #$0F
        sta     $57
        lda     ($1E),y
        and     #$F0
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     data_ptr
        sta     data_ptr+1
        lda     $21
        sta     $59
        lda     $20
        sta     $58
        iny
LFE85:  lda     $59
        sta     oam_buffer,x
        lda     $58
        sta     oam_buffer+3,x
        lda     ($1E),y
        sta     oam_buffer+1,x
        iny
        lda     ($1E),y
        ora     $22
        sta     oam_buffer+2,x
        iny
        inx
        inx
        inx
        inx
        inc     sprite0_reserved
        lda     $58
        clc
        adc     #$08
        sta     $58
        dec     data_ptr
        bne     LFE85
        lda     $20
        sta     $58
        lda     $59
        clc
        adc     #$08
        sta     $59
        lda     data_ptr+1
        sta     data_ptr
        dec     $57
        bne     LFE85
        inc     $21
        pla
        tay
        pla
        tax
        rts

; ----------------------------------------------------------------------------
layout_print_byte_padded:  pha
        txa
        pha
        tya
        pha
        jsr     load_byte_value
        jsr     decimal_digits
        jsr     trim_leading_zeros
        jsr     digits_to_tiles_padded
        ldx     #$02
        jsr     print_decimal_digits
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
layout_print_byte:  pha
        txa
        pha
        tya
        pha
        jsr     load_byte_value
        jsr     decimal_digits
        jsr     trim_leading_zeros
        jsr     digits_to_tiles
        ldx     #$00
        jsr     print_decimal_digits
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
layout_print_word_padded:  pha
        txa
        pha
        tya
        pha
        jsr     load_word_value
        jsr     decimal_digits
        jsr     trim_leading_zeros
        jsr     digits_to_tiles_padded
        ldx     #$00
        jsr     print_decimal_digits
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
layout_print_word:  pha
        txa
        pha
        tya
        pha
        jsr     load_word_value
        jsr     decimal_digits
        jsr     trim_leading_zeros
        jsr     digits_to_tiles
        ldx     #$00
        jsr     print_decimal_digits
        pla
        tay
        pla
        tax
        pla
        rts

; ----------------------------------------------------------------------------
load_byte_value:  lda     #$00
        tay
        sta     data_ptr+1
        beq     LFF47
        rts

; ----------------------------------------------------------------------------
load_word_value:  ldy     #$01
        lda     ($1C),y
        sta     data_ptr+1
        dey
LFF47:  lda     ($1C),y
        sta     data_ptr
        rts

; ----------------------------------------------------------------------------
decimal_digits:  ldx     #$00
LFF4E:  lda     #$00
        sta     scene_index,x
        inx
        cpx     #$05
        bne     LFF4E
        ldx     #$00
        ldy     #$00
LFF5B:  lda     data_ptr
        sec
        sbc     LFF88,y
        lda     data_ptr+1
        sbc     LFF89,y
        bcc     LFF7C
        lda     data_ptr
        sec
        sbc     LFF88,y
        sta     data_ptr
        lda     data_ptr+1
        sbc     LFF89,y
        sta     data_ptr+1
        inc     scene_index,x
        jmp     LFF5B

; ----------------------------------------------------------------------------
LFF7C:  iny
        iny
        inx
        cpx     #$04
        bne     LFF5B
        lda     data_ptr
        sta     scene_index,x
        rts

; ----------------------------------------------------------------------------
LFF88:  .byte   $10
LFF89:  .byte   $27,$E8,$03,$64,$00,$0A,$00
; ----------------------------------------------------------------------------
trim_leading_zeros:  ldx     #$00
LFF92:  lda     #$FF
        sta     name_buffer,x
        inx
        cpx     #$05
        bne     LFF92
        ldx     #$00
LFF9E:  lda     scene_index,x
        bne     LFFA7
        inx
        cpx     #$04
        bne     LFF9E
LFFA7:  stx     $84
        rts

; ----------------------------------------------------------------------------
digits_to_tiles_padded:  lda     scene_index,x
        clc
        adc     #$80
        sta     name_buffer,x
        inx
        cpx     #$05
        bne     digits_to_tiles_padded
        lda     #$FE
        sta     name_buffer+5
        rts

; ----------------------------------------------------------------------------
digits_to_tiles:  ldy     #$00
LFFBF:  lda     scene_index,x
        clc
        adc     #$80
        sta     name_buffer,y
        iny
        inx
        cpx     #$05
        bne     LFFBF
        lda     #$FE
        sta     name_buffer,y
        sty     $84
        rts

; ----------------------------------------------------------------------------
print_decimal_digits:  lda     $0E
        sta     scene_index
        lda     $0F
        sta     $76
LFFDD:  lda     name_buffer,x
        cmp     #$FE
        beq     LFFEF
        sta     $77
        jsr     layout_draw_tile
        inc     scene_index
        inx
        jmp     LFFDD

; ----------------------------------------------------------------------------
LFFEF:  rts

; ----------------------------------------------------------------------------
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$9A,$EC
        .byte   $9A,$EC
; ----------------------------------------------------------------------------
; NMI, reset, IRQ
vectors:.addr   nmi
        .addr   reset
        .addr   irq
