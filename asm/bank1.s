; Deja Vu (USA), NES: PRG bank 1 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK1"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
; scenes $4C-$97
scene_pictures_1:
        .addr   scene_4C
        .addr   scene_4D
        .addr   scene_4E
        .addr   scene_4F
        .addr   scene_50
        .addr   scene_51
        .addr   scene_52
        .addr   scene_53
        .addr   scene_54
        .addr   scene_55
        .addr   scene_56
        .addr   scene_57
        .addr   scene_58
        .addr   scene_59
        .addr   scene_5A
        .addr   scene_5B
        .addr   scene_5C
        .addr   scene_5D
        .addr   scene_5E
        .addr   scene_5F
        .addr   scene_60
        .addr   scene_61
        .addr   scene_62
        .addr   scene_63
        .addr   scene_64
        .addr   scene_65
        .addr   scene_66
        .addr   scene_67
        .addr   scene_68
        .addr   scene_69
        .addr   scene_6A
        .addr   scene_6B
        .addr   scene_6C
        .addr   scene_6D
        .addr   scene_6E
        .addr   scene_6F
        .addr   scene_70
        .addr   scene_71
        .addr   scene_72
        .addr   scene_73
        .addr   scene_74
        .addr   scene_75
        .addr   scene_76
        .addr   scene_77
        .addr   scene_78
        .addr   scene_79
        .addr   scene_7A
        .addr   scene_7B
        .addr   scene_7C
        .addr   scene_7D
        .addr   scene_7E
        .addr   scene_7F
        .addr   scene_80
        .addr   scene_81
        .addr   scene_82
        .addr   scene_83
        .addr   scene_84
        .addr   scene_85
        .addr   scene_86
        .addr   scene_87
        .addr   scene_88
        .addr   scene_89
        .addr   scene_8A
        .addr   scene_8B
        .addr   scene_8C
        .addr   scene_8D
        .addr   scene_8E
        .addr   scene_8F
        .addr   scene_90
        .addr   scene_91
        .addr   scene_92
        .addr   scene_93
        .addr   scene_94
        .addr   scene_95
        .addr   scene_96
        .addr   scene_97

; scene pictures: 16112 bytes from your ROM
scenes_1_data:
        .incbin "scenes_1_data.bin"
scene_4C        := scenes_1_data + $0000
scene_4D        := scenes_1_data + $00D4
scene_4E        := scenes_1_data + $01A8
scene_4F        := scenes_1_data + $027C
scene_50        := scenes_1_data + $0350
scene_51        := scenes_1_data + $0424
scene_52        := scenes_1_data + $04F8
scene_53        := scenes_1_data + $05CC
scene_54        := scenes_1_data + $06A0
scene_55        := scenes_1_data + $0774
scene_56        := scenes_1_data + $0848
scene_57        := scenes_1_data + $091C
scene_58        := scenes_1_data + $09F0
scene_59        := scenes_1_data + $0AC4
scene_5A        := scenes_1_data + $0B98
scene_5B        := scenes_1_data + $0C6C
scene_5C        := scenes_1_data + $0D40
scene_5D        := scenes_1_data + $0E14
scene_5E        := scenes_1_data + $0EE8
scene_5F        := scenes_1_data + $0FBC
scene_60        := scenes_1_data + $1090
scene_61        := scenes_1_data + $1164
scene_62        := scenes_1_data + $1238
scene_63        := scenes_1_data + $130C
scene_64        := scenes_1_data + $13E0
scene_65        := scenes_1_data + $14B4
scene_66        := scenes_1_data + $1588
scene_67        := scenes_1_data + $165C
scene_68        := scenes_1_data + $1730
scene_69        := scenes_1_data + $1804
scene_6A        := scenes_1_data + $18D8
scene_6B        := scenes_1_data + $19AC
scene_6C        := scenes_1_data + $1A80
scene_6D        := scenes_1_data + $1B54
scene_6E        := scenes_1_data + $1C28
scene_6F        := scenes_1_data + $1CFC
scene_70        := scenes_1_data + $1DD0
scene_71        := scenes_1_data + $1EA4
scene_72        := scenes_1_data + $1F78
scene_73        := scenes_1_data + $204C
scene_74        := scenes_1_data + $2120
scene_75        := scenes_1_data + $21F4
scene_76        := scenes_1_data + $22C8
scene_77        := scenes_1_data + $239C
scene_78        := scenes_1_data + $2470
scene_79        := scenes_1_data + $2544
scene_7A        := scenes_1_data + $2618
scene_7B        := scenes_1_data + $26EC
scene_7C        := scenes_1_data + $27C0
scene_7D        := scenes_1_data + $2894
scene_7E        := scenes_1_data + $2968
scene_7F        := scenes_1_data + $2A3C
scene_80        := scenes_1_data + $2B10
scene_81        := scenes_1_data + $2BE4
scene_82        := scenes_1_data + $2CB8
scene_83        := scenes_1_data + $2D8C
scene_84        := scenes_1_data + $2E60
scene_85        := scenes_1_data + $2F34
scene_86        := scenes_1_data + $3008
scene_87        := scenes_1_data + $30DC
scene_88        := scenes_1_data + $31B0
scene_89        := scenes_1_data + $3284
scene_8A        := scenes_1_data + $3358
scene_8B        := scenes_1_data + $342C
scene_8C        := scenes_1_data + $3500
scene_8D        := scenes_1_data + $35D4
scene_8E        := scenes_1_data + $36A8
scene_8F        := scenes_1_data + $377C
scene_90        := scenes_1_data + $3850
scene_91        := scenes_1_data + $3924
scene_92        := scenes_1_data + $39F8
scene_93        := scenes_1_data + $3ACC
scene_94        := scenes_1_data + $3BA0
scene_95        := scenes_1_data + $3C74
scene_96        := scenes_1_data + $3D48
scene_97        := scenes_1_data + $3E1C

; ----------------------------------------------------------------------------
; 120 unused bytes of $FF
free_space_1:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
