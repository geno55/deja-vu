; Deja Vu (USA), NES: PRG bank 0 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK0"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
; scenes $00-$4B
scene_pictures_0:
        .addr   scene_00
        .addr   scene_01
        .addr   scene_02
        .addr   scene_03
        .addr   scene_04
        .addr   scene_05
        .addr   scene_06
        .addr   scene_07
        .addr   scene_08
        .addr   scene_09
        .addr   scene_0A
        .addr   scene_0B
        .addr   scene_0C
        .addr   scene_0D
        .addr   scene_0E
        .addr   scene_0F
        .addr   scene_10
        .addr   scene_11
        .addr   scene_12
        .addr   scene_13
        .addr   scene_14
        .addr   scene_15
        .addr   scene_16
        .addr   scene_17
        .addr   scene_18
        .addr   scene_19
        .addr   scene_1A
        .addr   scene_1B
        .addr   scene_1C
        .addr   scene_1D
        .addr   scene_1E
        .addr   scene_1F
        .addr   scene_20
        .addr   scene_21
        .addr   scene_22
        .addr   scene_23
        .addr   scene_24
        .addr   scene_25
        .addr   scene_26
        .addr   scene_27
        .addr   scene_28
        .addr   scene_29
        .addr   scene_2A
        .addr   scene_2B
        .addr   scene_2C
        .addr   scene_2D
        .addr   scene_2E
        .addr   scene_2F
        .addr   scene_30
        .addr   scene_31
        .addr   scene_32
        .addr   scene_33
        .addr   scene_34
        .addr   scene_35
        .addr   scene_36
        .addr   scene_37
        .addr   scene_38
        .addr   scene_39
        .addr   scene_3A
        .addr   scene_3B
        .addr   scene_3C
        .addr   scene_3D
        .addr   scene_3E
        .addr   scene_3F
        .addr   scene_40
        .addr   scene_41
        .addr   scene_42
        .addr   scene_43
        .addr   scene_44
        .addr   scene_45
        .addr   scene_46
        .addr   scene_47
        .addr   scene_48
        .addr   scene_49
        .addr   scene_4A
        .addr   scene_4B

; scene pictures: 16112 bytes from your ROM
scenes_0_data:
        .incbin "scenes_0_data.bin"
scene_00        := scenes_0_data + $0000
scene_01        := scenes_0_data + $00D4
scene_02        := scenes_0_data + $01A8
scene_03        := scenes_0_data + $027C
scene_04        := scenes_0_data + $0350
scene_05        := scenes_0_data + $0424
scene_06        := scenes_0_data + $04F8
scene_07        := scenes_0_data + $05CC
scene_08        := scenes_0_data + $06A0
scene_09        := scenes_0_data + $0774
scene_0A        := scenes_0_data + $0848
scene_0B        := scenes_0_data + $091C
scene_0C        := scenes_0_data + $09F0
scene_0D        := scenes_0_data + $0AC4
scene_0E        := scenes_0_data + $0B98
scene_0F        := scenes_0_data + $0C6C
scene_10        := scenes_0_data + $0D40
scene_11        := scenes_0_data + $0E14
scene_12        := scenes_0_data + $0EE8
scene_13        := scenes_0_data + $0FBC
scene_14        := scenes_0_data + $1090
scene_15        := scenes_0_data + $1164
scene_16        := scenes_0_data + $1238
scene_17        := scenes_0_data + $130C
scene_18        := scenes_0_data + $13E0
scene_19        := scenes_0_data + $14B4
scene_1A        := scenes_0_data + $1588
scene_1B        := scenes_0_data + $165C
scene_1C        := scenes_0_data + $1730
scene_1D        := scenes_0_data + $1804
scene_1E        := scenes_0_data + $18D8
scene_1F        := scenes_0_data + $19AC
scene_20        := scenes_0_data + $1A80
scene_21        := scenes_0_data + $1B54
scene_22        := scenes_0_data + $1C28
scene_23        := scenes_0_data + $1CFC
scene_24        := scenes_0_data + $1DD0
scene_25        := scenes_0_data + $1EA4
scene_26        := scenes_0_data + $1F78
scene_27        := scenes_0_data + $204C
scene_28        := scenes_0_data + $2120
scene_29        := scenes_0_data + $21F4
scene_2A        := scenes_0_data + $22C8
scene_2B        := scenes_0_data + $239C
scene_2C        := scenes_0_data + $2470
scene_2D        := scenes_0_data + $2544
scene_2E        := scenes_0_data + $2618
scene_2F        := scenes_0_data + $26EC
scene_30        := scenes_0_data + $27C0
scene_31        := scenes_0_data + $2894
scene_32        := scenes_0_data + $2968
scene_33        := scenes_0_data + $2A3C
scene_34        := scenes_0_data + $2B10
scene_35        := scenes_0_data + $2BE4
scene_36        := scenes_0_data + $2CB8
scene_37        := scenes_0_data + $2D8C
scene_38        := scenes_0_data + $2E60
scene_39        := scenes_0_data + $2F34
scene_3A        := scenes_0_data + $3008
scene_3B        := scenes_0_data + $30DC
scene_3C        := scenes_0_data + $31B0
scene_3D        := scenes_0_data + $3284
scene_3E        := scenes_0_data + $3358
scene_3F        := scenes_0_data + $342C
scene_40        := scenes_0_data + $3500
scene_41        := scenes_0_data + $35D4
scene_42        := scenes_0_data + $36A8
scene_43        := scenes_0_data + $377C
scene_44        := scenes_0_data + $3850
scene_45        := scenes_0_data + $3924
scene_46        := scenes_0_data + $39F8
scene_47        := scenes_0_data + $3ACC
scene_48        := scenes_0_data + $3BA0
scene_49        := scenes_0_data + $3C74
scene_4A        := scenes_0_data + $3D48
scene_4B        := scenes_0_data + $3E1C

; ----------------------------------------------------------------------------
; 120 unused bytes of $FF
free_space_0:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
