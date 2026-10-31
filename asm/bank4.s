; Deja Vu (USA), NES: PRG bank 4 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK4"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
; 256 message pointers (tools/text.py)
messages_4:
        .addr   msg4_00
        .addr   msg4_01
        .addr   msg4_02
        .addr   msg4_03
        .addr   msg4_04
        .addr   msg4_05
        .addr   msg4_06
        .addr   msg4_07
        .addr   msg4_08
        .addr   msg4_09
        .addr   msg4_0A
        .addr   msg4_0B
        .addr   msg4_0C
        .addr   msg4_0D
        .addr   msg4_0E
        .addr   msg4_0F
        .addr   msg4_10
        .addr   msg4_11
        .addr   msg4_12
        .addr   msg4_13
        .addr   msg4_14
        .addr   msg4_15
        .addr   msg4_16
        .addr   msg4_17
        .addr   msg4_18
        .addr   msg4_19
        .addr   msg4_1A
        .addr   msg4_1B
        .addr   msg4_1C
        .addr   msg4_1D
        .addr   msg4_1E
        .addr   msg4_1F
        .addr   msg4_20
        .addr   msg4_21
        .addr   msg4_22
        .addr   msg4_23
        .addr   msg4_24
        .addr   msg4_25
        .addr   msg4_26
        .addr   msg4_27
        .addr   msg4_28
        .addr   msg4_29
        .addr   msg4_2A
        .addr   msg4_2B
        .addr   msg4_2C
        .addr   msg4_2D
        .addr   msg4_2E
        .addr   msg4_2F
        .addr   msg4_30
        .addr   msg4_31
        .addr   msg4_32
        .addr   msg4_33
        .addr   msg4_34
        .addr   msg4_35
        .addr   msg4_36
        .addr   msg4_37
        .addr   msg4_38
        .addr   msg4_39
        .addr   msg4_3A
        .addr   msg4_3B
        .addr   msg4_3C
        .addr   msg4_3D
        .addr   msg4_3E
        .addr   msg4_3F
        .addr   msg4_40
        .addr   msg4_41
        .addr   msg4_42
        .addr   msg4_43
        .addr   msg4_44
        .addr   msg4_45
        .addr   msg4_46
        .addr   msg4_47
        .addr   msg4_48
        .addr   msg4_49
        .addr   msg4_4A
        .addr   msg4_4B
        .addr   msg4_4C
        .addr   msg4_4D
        .addr   msg4_4E
        .addr   msg4_4F
        .addr   msg4_50
        .addr   msg4_51
        .addr   msg4_52
        .addr   msg4_53
        .addr   msg4_54
        .addr   msg4_55
        .addr   msg4_56
        .addr   msg4_57
        .addr   msg4_58
        .addr   msg4_59
        .addr   msg4_5A
        .addr   msg4_5B
        .addr   msg4_5C
        .addr   msg4_5D
        .addr   msg4_5E
        .addr   msg4_5F
        .addr   msg4_60
        .addr   msg4_61
        .addr   msg4_62
        .addr   msg4_63
        .addr   msg4_64
        .addr   msg4_65
        .addr   msg4_66
        .addr   msg4_67
        .addr   msg4_68
        .addr   msg4_69
        .addr   msg4_6A
        .addr   msg4_6B
        .addr   msg4_6C
        .addr   msg4_6D
        .addr   msg4_6E
        .addr   msg4_6F
        .addr   msg4_70
        .addr   msg4_71
        .addr   msg4_72
        .addr   msg4_73
        .addr   msg4_74
        .addr   msg4_75
        .addr   msg4_76
        .addr   msg4_77
        .addr   msg4_78
        .addr   msg4_79
        .addr   msg4_7A
        .addr   msg4_7B
        .addr   msg4_7C
        .addr   msg4_7D
        .addr   msg4_7E
        .addr   msg4_7F
        .addr   msg4_80
        .addr   msg4_81
        .addr   msg4_82
        .addr   msg4_83
        .addr   msg4_84
        .addr   msg4_85
        .addr   msg4_86
        .addr   msg4_87
        .addr   msg4_88
        .addr   msg4_89
        .addr   msg4_8A
        .addr   msg4_8B
        .addr   msg4_8C
        .addr   msg4_8D
        .addr   msg4_8E
        .addr   msg4_8F
        .addr   msg4_90
        .addr   msg4_91
        .addr   msg4_92
        .addr   msg4_93
        .addr   msg4_94
        .addr   msg4_95
        .addr   msg4_96
        .addr   msg4_97
        .addr   msg4_98
        .addr   msg4_99
        .addr   msg4_9A
        .addr   msg4_9B
        .addr   msg4_9C
        .addr   msg4_9D
        .addr   msg4_9E
        .addr   msg4_9F
        .addr   msg4_A0
        .addr   msg4_A1
        .addr   msg4_A2
        .addr   msg4_A3
        .addr   msg4_A4
        .addr   msg4_A5
        .addr   msg4_A6
        .addr   msg4_A7
        .addr   msg4_A8
        .addr   msg4_A9
        .addr   msg4_AA
        .addr   msg4_AB
        .addr   msg4_AC
        .addr   msg4_AD
        .addr   msg4_AE
        .addr   msg4_AF
        .addr   msg4_B0
        .addr   msg4_B1
        .addr   msg4_B2
        .addr   msg4_B3
        .addr   msg4_B4
        .addr   msg4_B5
        .addr   msg4_B6
        .addr   msg4_B7
        .addr   msg4_B8
        .addr   msg4_B9
        .addr   msg4_BA
        .addr   msg4_BB
        .addr   msg4_BC
        .addr   msg4_BD
        .addr   msg4_BE
        .addr   msg4_BF
        .addr   msg4_C0
        .addr   msg4_C1
        .addr   msg4_C2
        .addr   msg4_C3
        .addr   msg4_C4
        .addr   msg4_C5
        .addr   msg4_C6
        .addr   msg4_C7
        .addr   msg4_C8
        .addr   msg4_C9
        .addr   msg4_CA
        .addr   msg4_CB
        .addr   msg4_CC
        .addr   msg4_CD
        .addr   msg4_CE
        .addr   msg4_CF
        .addr   msg4_D0
        .addr   msg4_D1
        .addr   msg4_D2
        .addr   msg4_D3
        .addr   msg4_D4
        .addr   msg4_D5
        .addr   msg4_D6
        .addr   msg4_D7
        .addr   msg4_D8
        .addr   msg4_D9
        .addr   msg4_DA
        .addr   msg4_DB
        .addr   msg4_DC
        .addr   msg4_DD
        .addr   msg4_DE
        .addr   msg4_DF
        .addr   msg4_E0
        .addr   msg4_E1
        .addr   msg4_E2
        .addr   msg4_E3
        .addr   msg4_E4
        .addr   msg4_E5
        .addr   msg4_E6
        .addr   msg4_E7
        .addr   msg4_E8
        .addr   msg4_E9
        .addr   msg4_EA
        .addr   msg4_EB
        .addr   msg4_EC
        .addr   msg4_ED
        .addr   msg4_EE
        .addr   msg4_EF
        .addr   msg4_F0
        .addr   msg4_F1
        .addr   msg4_F2
        .addr   msg4_F3
        .addr   msg4_F4
        .addr   msg4_F5
        .addr   msg4_F6
        .addr   msg4_F7
        .addr   msg4_F8
        .addr   msg4_F9
        .addr   msg4_FA
        .addr   msg4_FB
        .addr   msg4_FC
        .addr   msg4_FD
        .addr   msg4_FE
        .addr   msg4_FF

; compressed messages: 13341 bytes from your ROM
messages_4_data:
        .incbin "messages_4_data.bin"
msg4_00         := messages_4_data + $0000
msg4_01         := messages_4_data + $0029
msg4_02         := messages_4_data + $004E
msg4_03         := messages_4_data + $00B7
msg4_04         := messages_4_data + $0105
msg4_05         := messages_4_data + $0127
msg4_06         := messages_4_data + $014B
msg4_07         := messages_4_data + $0175
msg4_08         := messages_4_data + $0185
msg4_09         := messages_4_data + $019F
msg4_0A         := messages_4_data + $01C7
msg4_0B         := messages_4_data + $01FA
msg4_0C         := messages_4_data + $022E
msg4_0D         := messages_4_data + $02B2
msg4_0E         := messages_4_data + $02E4
msg4_0F         := messages_4_data + $0315
msg4_10         := messages_4_data + $0346
msg4_11         := messages_4_data + $037F
msg4_12         := messages_4_data + $0393
msg4_13         := messages_4_data + $03AE
msg4_14         := messages_4_data + $03C8
msg4_15         := messages_4_data + $03E2
msg4_16         := messages_4_data + $0449
msg4_17         := messages_4_data + $046F
msg4_18         := messages_4_data + $0482
msg4_19         := messages_4_data + $048F
msg4_1A         := messages_4_data + $04AC
msg4_1B         := messages_4_data + $04E8
msg4_1C         := messages_4_data + $050F
msg4_1D         := messages_4_data + $052F
msg4_1E         := messages_4_data + $056B
msg4_1F         := messages_4_data + $058E
msg4_20         := messages_4_data + $05A5
msg4_21         := messages_4_data + $05C5
msg4_22         := messages_4_data + $05E0
msg4_23         := messages_4_data + $05FB
msg4_24         := messages_4_data + $0611
msg4_25         := messages_4_data + $0620
msg4_26         := messages_4_data + $0646
msg4_27         := messages_4_data + $065A
msg4_28         := messages_4_data + $0683
msg4_29         := messages_4_data + $06A2
msg4_2A         := messages_4_data + $06B2
msg4_2B         := messages_4_data + $06BE
msg4_2C         := messages_4_data + $06D5
msg4_2D         := messages_4_data + $0710
msg4_2E         := messages_4_data + $072F
msg4_2F         := messages_4_data + $074B
msg4_30         := messages_4_data + $07AC
msg4_31         := messages_4_data + $07D3
msg4_32         := messages_4_data + $07E8
msg4_33         := messages_4_data + $0812
msg4_34         := messages_4_data + $0839
msg4_35         := messages_4_data + $085D
msg4_36         := messages_4_data + $0878
msg4_37         := messages_4_data + $08D1
msg4_38         := messages_4_data + $0901
msg4_39         := messages_4_data + $0964
msg4_3A         := messages_4_data + $098A
msg4_3B         := messages_4_data + $09AE
msg4_3C         := messages_4_data + $0A00
msg4_3D         := messages_4_data + $0A2F
msg4_3E         := messages_4_data + $0A59
msg4_3F         := messages_4_data + $0A7E
msg4_40         := messages_4_data + $0B01
msg4_41         := messages_4_data + $0B33
msg4_42         := messages_4_data + $0B7A
msg4_43         := messages_4_data + $0B9C
msg4_44         := messages_4_data + $0BB5
msg4_45         := messages_4_data + $0BE5
msg4_46         := messages_4_data + $0C37
msg4_47         := messages_4_data + $0C56
msg4_48         := messages_4_data + $0C7D
msg4_49         := messages_4_data + $0CA9
msg4_4A         := messages_4_data + $0CC8
msg4_4B         := messages_4_data + $0CF1
msg4_4C         := messages_4_data + $0D12
msg4_4D         := messages_4_data + $0D28
msg4_4E         := messages_4_data + $0D3A
msg4_4F         := messages_4_data + $0D64
msg4_50         := messages_4_data + $0D8F
msg4_51         := messages_4_data + $0DA8
msg4_52         := messages_4_data + $0DC9
msg4_53         := messages_4_data + $0DDD
msg4_54         := messages_4_data + $0E0D
msg4_55         := messages_4_data + $0E24
msg4_56         := messages_4_data + $0E35
msg4_57         := messages_4_data + $0E58
msg4_58         := messages_4_data + $0E7A
msg4_59         := messages_4_data + $0EA5
msg4_5A         := messages_4_data + $0EB4
msg4_5B         := messages_4_data + $0ED5
msg4_5C         := messages_4_data + $0EEE
msg4_5D         := messages_4_data + $0F32
msg4_5E         := messages_4_data + $0F57
msg4_5F         := messages_4_data + $0F76
msg4_60         := messages_4_data + $0F90
msg4_61         := messages_4_data + $0FA3
msg4_62         := messages_4_data + $0FAF
msg4_63         := messages_4_data + $0FCA
msg4_64         := messages_4_data + $0FE8
msg4_65         := messages_4_data + $100B
msg4_66         := messages_4_data + $108B
msg4_67         := messages_4_data + $10B7
msg4_68         := messages_4_data + $10E2
msg4_69         := messages_4_data + $10F8
msg4_6A         := messages_4_data + $1121
msg4_6B         := messages_4_data + $1147
msg4_6C         := messages_4_data + $1183
msg4_6D         := messages_4_data + $119A
msg4_6E         := messages_4_data + $11A8
msg4_6F         := messages_4_data + $11C1
msg4_70         := messages_4_data + $11D1
msg4_71         := messages_4_data + $11F1
msg4_72         := messages_4_data + $1283
msg4_73         := messages_4_data + $129D
msg4_74         := messages_4_data + $12B7
msg4_75         := messages_4_data + $12CF
msg4_76         := messages_4_data + $12ED
msg4_77         := messages_4_data + $1327
msg4_78         := messages_4_data + $1334
msg4_79         := messages_4_data + $134F
msg4_7A         := messages_4_data + $1372
msg4_7B         := messages_4_data + $13A8
msg4_7C         := messages_4_data + $13CE
msg4_7D         := messages_4_data + $1402
msg4_7E         := messages_4_data + $1428
msg4_7F         := messages_4_data + $1463
msg4_80         := messages_4_data + $14D0
msg4_81         := messages_4_data + $1541
msg4_82         := messages_4_data + $171B
msg4_83         := messages_4_data + $1735
msg4_84         := messages_4_data + $1788
msg4_85         := messages_4_data + $179A
msg4_86         := messages_4_data + $17C3
msg4_87         := messages_4_data + $17FA
msg4_88         := messages_4_data + $1822
msg4_89         := messages_4_data + $184E
msg4_8A         := messages_4_data + $1870
msg4_8B         := messages_4_data + $188F
msg4_8C         := messages_4_data + $18B5
msg4_8D         := messages_4_data + $18D8
msg4_8E         := messages_4_data + $18F9
msg4_8F         := messages_4_data + $1929
msg4_90         := messages_4_data + $1943
msg4_91         := messages_4_data + $1966
msg4_92         := messages_4_data + $198C
msg4_93         := messages_4_data + $19AC
msg4_94         := messages_4_data + $19C6
msg4_95         := messages_4_data + $19EE
msg4_96         := messages_4_data + $1A4F
msg4_97         := messages_4_data + $1AD2
msg4_98         := messages_4_data + $1B0C
msg4_99         := messages_4_data + $1B44
msg4_9A         := messages_4_data + $1C24
msg4_9B         := messages_4_data + $1C4E
msg4_9C         := messages_4_data + $1C68
msg4_9D         := messages_4_data + $1CA0
msg4_9E         := messages_4_data + $1D4A
msg4_9F         := messages_4_data + $1D61
msg4_A0         := messages_4_data + $1D72
msg4_A1         := messages_4_data + $1D9D
msg4_A2         := messages_4_data + $1DB3
msg4_A3         := messages_4_data + $1DDA
msg4_A4         := messages_4_data + $1DF0
msg4_A5         := messages_4_data + $1E4C
msg4_A6         := messages_4_data + $1E85
msg4_A7         := messages_4_data + $1EB4
msg4_A8         := messages_4_data + $1EC4
msg4_A9         := messages_4_data + $1EE7
msg4_AA         := messages_4_data + $1EFA
msg4_AB         := messages_4_data + $1F15
msg4_AC         := messages_4_data + $1F5D
msg4_AD         := messages_4_data + $1F77
msg4_AE         := messages_4_data + $1FAC
msg4_AF         := messages_4_data + $1FC5
msg4_B0         := messages_4_data + $1FD0
msg4_B1         := messages_4_data + $1FED
msg4_B2         := messages_4_data + $200A
msg4_B3         := messages_4_data + $207F
msg4_B4         := messages_4_data + $20B6
msg4_B5         := messages_4_data + $20CD
msg4_B6         := messages_4_data + $20E8
msg4_B7         := messages_4_data + $20FA
msg4_B8         := messages_4_data + $212C
msg4_B9         := messages_4_data + $2153
msg4_BA         := messages_4_data + $2168
msg4_BB         := messages_4_data + $217D
msg4_BC         := messages_4_data + $21E8
msg4_BD         := messages_4_data + $220D
msg4_BE         := messages_4_data + $2223
msg4_BF         := messages_4_data + $2232
msg4_C0         := messages_4_data + $2256
msg4_C1         := messages_4_data + $2269
msg4_C2         := messages_4_data + $2287
msg4_C3         := messages_4_data + $22C1
msg4_C4         := messages_4_data + $22D1
msg4_C5         := messages_4_data + $237E
msg4_C6         := messages_4_data + $238E
msg4_C7         := messages_4_data + $243B
msg4_C8         := messages_4_data + $2443
msg4_C9         := messages_4_data + $2490
msg4_CA         := messages_4_data + $249D
msg4_CB         := messages_4_data + $24AC
msg4_CC         := messages_4_data + $24F9
msg4_CD         := messages_4_data + $2505
msg4_CE         := messages_4_data + $250D
msg4_CF         := messages_4_data + $2522
msg4_D0         := messages_4_data + $2540
msg4_D1         := messages_4_data + $2553
msg4_D2         := messages_4_data + $2561
msg4_D3         := messages_4_data + $2596
msg4_D4         := messages_4_data + $25B7
msg4_D5         := messages_4_data + $25F0
msg4_D6         := messages_4_data + $2850
msg4_D7         := messages_4_data + $28C7
msg4_D8         := messages_4_data + $28D2
msg4_D9         := messages_4_data + $294D
msg4_DA         := messages_4_data + $299E
msg4_DB         := messages_4_data + $29ED
msg4_DC         := messages_4_data + $2A20
msg4_DD         := messages_4_data + $2A38
msg4_DE         := messages_4_data + $2A5E
msg4_DF         := messages_4_data + $2A7D
msg4_E0         := messages_4_data + $2A8E
msg4_E1         := messages_4_data + $2AEB
msg4_E2         := messages_4_data + $2B2A
msg4_E3         := messages_4_data + $2B66
msg4_E4         := messages_4_data + $2BA3
msg4_E5         := messages_4_data + $2BDA
msg4_E6         := messages_4_data + $2C0F
msg4_E7         := messages_4_data + $2C3F
msg4_E8         := messages_4_data + $2C5D
msg4_E9         := messages_4_data + $2C96
msg4_EA         := messages_4_data + $2CA7
msg4_EB         := messages_4_data + $2CFF
msg4_EC         := messages_4_data + $2D36
msg4_ED         := messages_4_data + $2D68
msg4_EE         := messages_4_data + $2D90
msg4_EF         := messages_4_data + $2DAF
msg4_F0         := messages_4_data + $2DEB
msg4_F1         := messages_4_data + $2E10
msg4_F2         := messages_4_data + $2E73
msg4_F3         := messages_4_data + $2EAE
msg4_F4         := messages_4_data + $2EDE
msg4_F5         := messages_4_data + $2F18
msg4_F6         := messages_4_data + $2F52
msg4_F7         := messages_4_data + $2FC8
msg4_F8         := messages_4_data + $3000
msg4_F9         := messages_4_data + $3017
msg4_FA         := messages_4_data + $31E1
msg4_FB         := messages_4_data + $32CE
msg4_FC         := messages_4_data + $335C
msg4_FD         := messages_4_data + $3384
msg4_FE         := messages_4_data + $33EC
msg4_FF         := messages_4_data + $3410


; unused bytes: 2225 bytes from your ROM
unused_4_data:
        .incbin "unused_4_data.bin"
unused_4        := unused_4_data + $0000

; ----------------------------------------------------------------------------
; 306 unused bytes of $FF
free_space_4:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
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
