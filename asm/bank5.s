; Deja Vu (USA), NES: PRG bank 5 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK5"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
; address of each sound's header
sound_table:
        .addr   sound_01
        .addr   sound_02
        .addr   sound_03
        .addr   sound_04
        .addr   sound_05
        .addr   sound_06
        .addr   sound_07
        .addr   instrument_8866
        .addr   sound_09
        .addr   sound_0A
        .addr   sound_0B
        .addr   sound_0C
        .addr   sound_0D
        .addr   sound_0E
        .addr   sound_0F
        .addr   sound_0F
        .addr   sound_0F
        .addr   sound_12
        .addr   sound_13
        .addr   sound_14
        .addr   sound_15
        .addr   sound_16
        .addr   sound_17
        .addr   sound_18
        .addr   sound_19
        .addr   sound_1A
        .addr   sound_1B
        .addr   sound_1C
        .addr   sound_1D
        .addr   sound_1E
        .addr   sound_1F
        .addr   sound_20
        .addr   sound_21
        .addr   sound_22
        .addr   sound_23
        .addr   sound_24
        .addr   sound_25
        .addr   sound_26
        .addr   sound_27
        .addr   sound_28
        .addr   sound_29
        .addr   sound_2A
        .addr   sound_2B
        .addr   sound_2C
        .addr   sound_2D
        .addr   sound_2E
        .addr   sound_2F
        .addr   sound_30
        .addr   sound_31
        .addr   sound_32

; sound headers, channel streams and instruments: 6467 bytes from your ROM
sound_data:
        .incbin "sound_data.bin"
sound_01        := sound_data + $0000
sound_01_ch0    := sound_data + $000C
sound_01_ch1    := sound_data + $00AE
sound_01_ch2    := sound_data + $0152
sound_01_ch3    := sound_data + $019D
sound_02        := sound_data + $01E2
sound_02_ch0    := sound_data + $01EE
sound_02_ch1    := sound_data + $02A2
sound_02_ch2    := sound_data + $0358
sound_02_ch3    := sound_data + $03CE
sound_03        := sound_data + $03F9
sound_03_ch0    := sound_data + $0405
sound_03_ch1    := sound_data + $0491
sound_03_ch2    := sound_data + $051F
sound_03_ch3    := sound_data + $05FF
sound_04        := sound_data + $0624
sound_04_ch0    := sound_data + $0630
sound_04_ch1    := sound_data + $06A3
sound_04_ch2    := sound_data + $073B
sound_05        := sound_data + $075A
sound_05_ch0    := sound_data + $0766
sound_05_ch1    := sound_data + $0779
sound_06        := sound_data + $078C
sound_06_ch0    := sound_data + $0798
sound_06_ch1    := sound_data + $07AF
sound_07        := sound_data + $07CC
sound_07_ch0    := sound_data + $07D8
sound_07_ch1    := sound_data + $07ED
instrument_8866 := sound_data + $0802
instrument_88AA := sound_data + $0846
instrument_88EE := sound_data + $088A
unused_instrument_8932 := sound_data + $08CE
instrument_8976 := sound_data + $0912
instrument_89BA := sound_data + $0956
instrument_89FE := sound_data + $099A
instrument_8A42 := sound_data + $09DE
instrument_8A86 := sound_data + $0A22
sweep_8ACA      := sound_data + $0A66
sweep_8ACE      := sound_data + $0A6A
envelope_8AD2   := sound_data + $0A6E
instrument_8AF6 := sound_data + $0A92
sweep_8B3E      := sound_data + $0ADA
sweep_8B42      := sound_data + $0ADE
sound_09        := sound_data + $0B06
sound_09_ch0    := sound_data + $0B12
sound_09_ch1    := sound_data + $0B1E
sound_0A        := sound_data + $0B30
sound_0A_ch0    := sound_data + $0B3C
sound_0A_ch1    := sound_data + $0B47
sound_0A_ch2    := sound_data + $0B58
sound_0B        := sound_data + $0B61
sound_0B_ch0    := sound_data + $0B6D
sound_0B_ch1    := sound_data + $0B7E
sound_0B_ch2    := sound_data + $0B89
sound_0C        := sound_data + $0B92
sound_0C_ch0    := sound_data + $0B9E
sound_0C_ch1    := sound_data + $0D2B
sound_0C_ch2    := sound_data + $0EBA
sound_0C_ch3    := sound_data + $0EE6
sound_0D        := sound_data + $0F20
sound_0D_ch0    := sound_data + $0F2C
sound_0D_ch1    := sound_data + $0F93
sound_0D_ch2    := sound_data + $0FFD
sound_0D_ch3    := sound_data + $1063
sound_0E        := sound_data + $1088
sound_0E_ch0    := sound_data + $1094
sound_0E_ch1    := sound_data + $1170
sound_0E_ch2    := sound_data + $1250
sound_0E_ch3    := sound_data + $131B
sound_0F        := sound_data + $134B
sound_0F_ch0    := sound_data + $1357
sound_0F_ch1    := sound_data + $1360
sound_12        := sound_data + $1369
sound_12_ch0    := sound_data + $1375
sound_13        := sound_data + $1385
sound_13_ch1    := sound_data + $1391
sound_14        := sound_data + $1399
sound_14_ch0    := sound_data + $13A5
sound_14_ch1    := sound_data + $13B8
sound_15        := sound_data + $13CB
sound_15_ch0    := sound_data + $13D7
sound_16        := sound_data + $13EA
sound_16_ch0    := sound_data + $13F6
sound_17        := sound_data + $1405
sound_17_ch0    := sound_data + $1411
sound_17_ch1    := sound_data + $145D
sound_17_ch3    := sound_data + $1492
sound_18        := sound_data + $14B9
sound_18_ch0    := sound_data + $14C5
sound_18_ch1    := sound_data + $14FC
sound_18_ch3    := sound_data + $1522
instrument_95AD := sound_data + $1549
instrument_95F1 := sound_data + $158D
envelope_9635   := sound_data + $15D1
instrument_9659 := sound_data + $15F5
envelope_969D   := sound_data + $1639
instrument_96C1 := sound_data + $165D
instrument_9705 := sound_data + $16A1
sound_19        := sound_data + $16E5
sound_19_ch0    := sound_data + $16F1
sound_1A        := sound_data + $16F7
sound_1A_ch1    := sound_data + $1703
sound_1B        := sound_data + $1709
sound_1B_ch1    := sound_data + $1715
sound_1C        := sound_data + $171C
sound_1C_ch1    := sound_data + $1728
sound_1D        := sound_data + $172F
sound_1D_ch1    := sound_data + $173B
sound_1E        := sound_data + $1741
unused_stream_97B1 := sound_data + $174D
sound_1F        := sound_data + $1753
sound_1F_ch1    := sound_data + $175F
sound_20        := sound_data + $1776
sound_20_ch1    := sound_data + $1782
sound_21        := sound_data + $178A
sound_21_ch0    := sound_data + $1796
sound_22        := sound_data + $179F
sound_22_ch0    := sound_data + $17AB
sound_23        := sound_data + $17B1
sound_23_ch1    := sound_data + $17BD
sound_24        := sound_data + $17C5
sound_24_ch1    := sound_data + $17D1
sound_25        := sound_data + $17E6
sound_25_ch1    := sound_data + $17F2
sound_26        := sound_data + $17FD
sound_26_ch0    := sound_data + $1809
sound_27        := sound_data + $1818
sound_27_ch0    := sound_data + $1824
sound_28        := sound_data + $1847
sound_28_ch0    := sound_data + $1853
sound_29        := sound_data + $185E
sound_29_ch0    := sound_data + $186A
sound_2A        := sound_data + $1870
sound_2A_ch0    := sound_data + $187C
sound_2B        := sound_data + $1880
sound_2B_ch1    := sound_data + $188C
sound_2C        := sound_data + $1894
sound_2C_ch1    := sound_data + $18A0
sound_2D        := sound_data + $18A8
sound_2D_ch0    := sound_data + $18B4
sound_2E        := sound_data + $18C3
unused_stream_9933 := sound_data + $18CF
sound_2E_ch0    := sound_data + $18EB
sound_2F        := sound_data + $18EF
sound_2F_ch0    := sound_data + $18FB
sound_30        := sound_data + $1907
sound_30_ch1    := sound_data + $1913
sound_31        := sound_data + $191B
sound_31_ch1    := sound_data + $1927
sound_32        := sound_data + $192D
sound_32_ch0    := sound_data + $1939

; ----------------------------------------------------------------------------
; 89 bytes of $FF
unused_5_99A7:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF
; ----------------------------------------------------------------------------
; 256 message pointers (tools/text.py)
messages_5:
        .addr   msg5_00
        .addr   msg5_01
        .addr   msg5_02
        .addr   msg5_03
        .addr   msg5_04
        .addr   msg5_05
        .addr   msg5_06
        .addr   msg5_07
        .addr   msg5_08
        .addr   msg5_09
        .addr   msg5_0A
        .addr   msg5_0B
        .addr   msg5_0C
        .addr   msg5_0D
        .addr   msg5_0E
        .addr   msg5_0F
        .addr   msg5_10
        .addr   msg5_11
        .addr   msg5_12
        .addr   msg5_13
        .addr   msg5_14
        .addr   msg5_15
        .addr   msg5_16
        .addr   msg5_17
        .addr   msg5_18
        .addr   msg5_19
        .addr   msg5_1A
        .addr   msg5_1B
        .addr   msg5_1C
        .addr   msg5_1D
        .addr   msg5_1E
        .addr   msg5_1F
        .addr   msg5_20
        .addr   msg5_21
        .addr   msg5_22
        .addr   msg5_23
        .addr   msg5_24
        .addr   msg5_25
        .addr   msg5_26
        .addr   msg5_27
        .addr   msg5_28
        .addr   msg5_29
        .addr   msg5_2A
        .addr   msg5_2B
        .addr   msg5_2C
        .addr   msg5_2D
        .addr   msg5_2E
        .addr   msg5_2F
        .addr   msg5_30
        .addr   msg5_31
        .addr   msg5_32
        .addr   msg5_33
        .addr   msg5_34
        .addr   msg5_35
        .addr   msg5_36
        .addr   msg5_37
        .addr   msg5_38
        .addr   msg5_39
        .addr   msg5_3A
        .addr   msg5_3B
        .addr   msg5_3C
        .addr   msg5_3D
        .addr   msg5_3E
        .addr   msg5_3F
        .addr   msg5_40
        .addr   msg5_41
        .addr   msg5_42
        .addr   msg5_43
        .addr   msg5_44
        .addr   msg5_45
        .addr   msg5_46
        .addr   msg5_47
        .addr   msg5_48
        .addr   msg5_49
        .addr   msg5_4A
        .addr   msg5_4B
        .addr   msg5_4C
        .addr   msg5_4D
        .addr   msg5_4E
        .addr   msg5_4F
        .addr   msg5_50
        .addr   msg5_51
        .addr   msg5_52
        .addr   msg5_53
        .addr   msg5_54
        .addr   msg5_55
        .addr   msg5_56
        .addr   msg5_57
        .addr   msg5_58
        .addr   msg5_59
        .addr   msg5_5A
        .addr   msg5_5B
        .addr   msg5_5C
        .addr   msg5_5D
        .addr   msg5_5E
        .addr   msg5_5F
        .addr   msg5_60
        .addr   msg5_61
        .addr   msg5_62
        .addr   msg5_63
        .addr   msg5_64
        .addr   msg5_65
        .addr   msg5_66
        .addr   msg5_67
        .addr   msg5_68
        .addr   msg5_69
        .addr   msg5_6A
        .addr   msg5_6B
        .addr   msg5_6C
        .addr   msg5_6D
        .addr   msg5_6E
        .addr   msg5_6F
        .addr   msg5_70
        .addr   msg5_71
        .addr   msg5_72
        .addr   msg5_73
        .addr   msg5_74
        .addr   msg5_75
        .addr   msg5_76
        .addr   msg5_77
        .addr   msg5_78
        .addr   msg5_79
        .addr   msg5_7A
        .addr   msg5_7B
        .addr   msg5_7C
        .addr   msg5_7D
        .addr   msg5_7E
        .addr   msg5_7F
        .addr   msg5_80
        .addr   msg5_81
        .addr   msg5_82
        .addr   msg5_83
        .addr   msg5_84
        .addr   msg5_85
        .addr   msg5_86
        .addr   msg5_87
        .addr   msg5_88
        .addr   msg5_89
        .addr   msg5_8A
        .addr   msg5_8B
        .addr   msg5_8C
        .addr   msg5_8D
        .addr   msg5_8D
        .addr   msg5_8F
        .addr   msg5_90
        .addr   msg5_91
        .addr   msg5_92
        .addr   msg5_93
        .addr   msg5_93
        .addr   msg5_93
        .addr   msg5_96
        .addr   msg5_97
        .addr   msg5_98
        .addr   msg5_99
        .addr   msg5_9A
        .addr   msg5_9B
        .addr   msg5_9C
        .addr   msg5_9D
        .addr   msg5_9E
        .addr   msg5_9F
        .addr   msg5_9F
        .addr   msg5_A1
        .addr   msg5_A2
        .addr   msg5_A3
        .addr   msg5_A4
        .addr   msg5_A5
        .addr   msg5_A6
        .addr   msg5_A7
        .addr   msg5_A8
        .addr   msg5_A9
        .addr   msg5_AA
        .addr   msg5_AB
        .addr   msg5_AC
        .addr   msg5_AD
        .addr   msg5_AE
        .addr   msg5_AF
        .addr   msg5_B0
        .addr   msg5_B1
        .addr   msg5_B2
        .addr   msg5_B3
        .addr   msg5_B4
        .addr   msg5_B5
        .addr   msg5_B6
        .addr   msg5_B7
        .addr   msg5_B8
        .addr   msg5_B9
        .addr   msg5_BA
        .addr   msg5_BB
        .addr   msg5_BC
        .addr   msg5_BD
        .addr   msg5_BE
        .addr   msg5_BF
        .addr   msg5_C0
        .addr   msg5_C1
        .addr   msg5_C2
        .addr   msg5_C3
        .addr   msg5_C4
        .addr   msg5_C5
        .addr   msg5_C6
        .addr   msg5_C7
        .addr   msg5_C8
        .addr   msg5_C9
        .addr   msg5_CA
        .addr   msg5_CB
        .addr   msg5_CC
        .addr   msg5_CD
        .addr   msg5_CE
        .addr   msg5_CF
        .addr   msg5_D0
        .addr   msg5_D1
        .addr   msg5_D2
        .addr   msg5_D3
        .addr   msg5_D4
        .addr   msg5_D5
        .addr   msg5_D6
        .addr   msg5_D7
        .addr   msg5_D8
        .addr   msg5_D9
        .addr   msg5_DA
        .addr   msg5_DB
        .addr   msg5_DC
        .addr   msg5_DD
        .addr   msg5_DE
        .addr   msg5_DF
        .addr   msg5_E0
        .addr   msg5_E1
        .addr   msg5_E2
        .addr   msg5_E3
        .addr   msg5_E4
        .addr   msg5_E5
        .addr   msg5_E6
        .addr   msg5_E7
        .addr   msg5_E8
        .addr   msg5_E9
        .addr   msg5_EA
        .addr   msg5_EB
        .addr   msg5_EC
        .addr   msg5_ED
        .addr   msg5_EE
        .addr   msg5_EF
        .addr   msg5_F0
        .addr   msg5_F1
        .addr   msg5_F2
        .addr   msg5_F3
        .addr   msg5_F4
        .addr   msg5_F5
        .addr   msg5_F6
        .addr   msg5_F7
        .addr   msg5_F8
        .addr   msg5_F9
        .addr   msg5_FA
        .addr   msg5_FB
        .addr   msg5_FC
        .addr   msg5_FD
        .addr   msg5_FE
        .addr   msg5_FF

; compressed messages: 7640 bytes from your ROM
messages_5_data:
        .incbin "messages_5_data.bin"
msg5_00         := messages_5_data + $0000
msg5_01         := messages_5_data + $0110
msg5_02         := messages_5_data + $0132
msg5_03         := messages_5_data + $014D
msg5_04         := messages_5_data + $016A
msg5_05         := messages_5_data + $019F
msg5_06         := messages_5_data + $01AB
msg5_07         := messages_5_data + $01CF
msg5_08         := messages_5_data + $01D9
msg5_09         := messages_5_data + $01F6
msg5_0A         := messages_5_data + $0210
msg5_0B         := messages_5_data + $0214
msg5_0C         := messages_5_data + $024E
msg5_0D         := messages_5_data + $0266
msg5_0E         := messages_5_data + $0289
msg5_0F         := messages_5_data + $02AB
msg5_10         := messages_5_data + $02C8
msg5_11         := messages_5_data + $02EB
msg5_12         := messages_5_data + $030B
msg5_13         := messages_5_data + $031A
msg5_14         := messages_5_data + $032E
msg5_15         := messages_5_data + $0339
msg5_16         := messages_5_data + $0367
msg5_17         := messages_5_data + $0373
msg5_18         := messages_5_data + $0391
msg5_19         := messages_5_data + $0395
msg5_1A         := messages_5_data + $03BA
msg5_1B         := messages_5_data + $03D3
msg5_1C         := messages_5_data + $03E2
msg5_1D         := messages_5_data + $043E
msg5_1E         := messages_5_data + $0472
msg5_1F         := messages_5_data + $049D
msg5_20         := messages_5_data + $04D6
msg5_21         := messages_5_data + $04F2
msg5_22         := messages_5_data + $0501
msg5_23         := messages_5_data + $0514
msg5_24         := messages_5_data + $052D
msg5_25         := messages_5_data + $0540
msg5_26         := messages_5_data + $0555
msg5_27         := messages_5_data + $0565
msg5_28         := messages_5_data + $0577
msg5_29         := messages_5_data + $0584
msg5_2A         := messages_5_data + $05A1
msg5_2B         := messages_5_data + $05D2
msg5_2C         := messages_5_data + $05EC
msg5_2D         := messages_5_data + $05FF
msg5_2E         := messages_5_data + $061F
msg5_2F         := messages_5_data + $0632
msg5_30         := messages_5_data + $0643
msg5_31         := messages_5_data + $0654
msg5_32         := messages_5_data + $066B
msg5_33         := messages_5_data + $067B
msg5_34         := messages_5_data + $06AC
msg5_35         := messages_5_data + $06D2
msg5_36         := messages_5_data + $06F9
msg5_37         := messages_5_data + $072B
msg5_38         := messages_5_data + $0746
msg5_39         := messages_5_data + $076D
msg5_3A         := messages_5_data + $0786
msg5_3B         := messages_5_data + $07CC
msg5_3C         := messages_5_data + $0804
msg5_3D         := messages_5_data + $0828
msg5_3E         := messages_5_data + $0846
msg5_3F         := messages_5_data + $0870
msg5_40         := messages_5_data + $0882
msg5_41         := messages_5_data + $0897
msg5_42         := messages_5_data + $08B1
msg5_43         := messages_5_data + $08C4
msg5_44         := messages_5_data + $08DC
msg5_45         := messages_5_data + $0916
msg5_46         := messages_5_data + $092E
msg5_47         := messages_5_data + $0941
msg5_48         := messages_5_data + $0945
msg5_49         := messages_5_data + $095C
msg5_4A         := messages_5_data + $0989
msg5_4B         := messages_5_data + $099C
msg5_4C         := messages_5_data + $09AA
msg5_4D         := messages_5_data + $09B3
msg5_4E         := messages_5_data + $09C7
msg5_4F         := messages_5_data + $0A08
msg5_50         := messages_5_data + $0A34
msg5_51         := messages_5_data + $0A54
msg5_52         := messages_5_data + $0A8C
msg5_53         := messages_5_data + $0AA1
msg5_54         := messages_5_data + $0AB0
msg5_55         := messages_5_data + $0AC9
msg5_56         := messages_5_data + $0B00
msg5_57         := messages_5_data + $0B21
msg5_58         := messages_5_data + $0B34
msg5_59         := messages_5_data + $0B53
msg5_5A         := messages_5_data + $0B5B
msg5_5B         := messages_5_data + $0B74
msg5_5C         := messages_5_data + $0BF5
msg5_5D         := messages_5_data + $0C21
msg5_5E         := messages_5_data + $0C25
msg5_5F         := messages_5_data + $0C29
msg5_60         := messages_5_data + $0C4A
msg5_61         := messages_5_data + $0C63
msg5_62         := messages_5_data + $0C97
msg5_63         := messages_5_data + $0CAC
msg5_64         := messages_5_data + $0D13
msg5_65         := messages_5_data + $0D17
msg5_66         := messages_5_data + $0D2A
msg5_67         := messages_5_data + $0D38
msg5_68         := messages_5_data + $0D3C
msg5_69         := messages_5_data + $0D5F
msg5_6A         := messages_5_data + $0D77
msg5_6B         := messages_5_data + $0D7B
msg5_6C         := messages_5_data + $0D9A
msg5_6D         := messages_5_data + $0DB4
msg5_6E         := messages_5_data + $0DC6
msg5_6F         := messages_5_data + $0DE2
msg5_70         := messages_5_data + $0DF3
msg5_71         := messages_5_data + $0E33
msg5_72         := messages_5_data + $0E73
msg5_73         := messages_5_data + $0E8C
msg5_74         := messages_5_data + $0EC8
msg5_75         := messages_5_data + $0ED4
msg5_76         := messages_5_data + $0F0E
msg5_77         := messages_5_data + $0F29
msg5_78         := messages_5_data + $0F66
msg5_79         := messages_5_data + $0FC0
msg5_7A         := messages_5_data + $0FCF
msg5_7B         := messages_5_data + $0FF7
msg5_7C         := messages_5_data + $101B
msg5_7D         := messages_5_data + $102D
msg5_7E         := messages_5_data + $103C
msg5_7F         := messages_5_data + $105A
msg5_80         := messages_5_data + $10BE
msg5_81         := messages_5_data + $10C8
msg5_82         := messages_5_data + $10D4
msg5_83         := messages_5_data + $10DF
msg5_84         := messages_5_data + $10EC
msg5_85         := messages_5_data + $10F7
msg5_86         := messages_5_data + $1103
msg5_87         := messages_5_data + $1113
msg5_88         := messages_5_data + $1131
msg5_89         := messages_5_data + $1148
msg5_8A         := messages_5_data + $1158
msg5_8B         := messages_5_data + $1163
msg5_8C         := messages_5_data + $117A
msg5_8D         := messages_5_data + $119C
msg5_8F         := messages_5_data + $11BE
msg5_90         := messages_5_data + $11D4
msg5_91         := messages_5_data + $11F2
msg5_92         := messages_5_data + $120C
msg5_93         := messages_5_data + $121E
msg5_96         := messages_5_data + $122C
msg5_97         := messages_5_data + $1238
msg5_98         := messages_5_data + $124C
msg5_99         := messages_5_data + $126D
msg5_9A         := messages_5_data + $1278
msg5_9B         := messages_5_data + $1288
msg5_9C         := messages_5_data + $1294
msg5_9D         := messages_5_data + $12A0
msg5_9E         := messages_5_data + $12AC
msg5_9F         := messages_5_data + $12B8
msg5_A1         := messages_5_data + $12C7
msg5_A2         := messages_5_data + $12F3
msg5_A3         := messages_5_data + $130B
msg5_A4         := messages_5_data + $1323
msg5_A5         := messages_5_data + $1339
msg5_A6         := messages_5_data + $134F
msg5_A7         := messages_5_data + $1360
msg5_A8         := messages_5_data + $1371
msg5_A9         := messages_5_data + $1387
msg5_AA         := messages_5_data + $13A1
msg5_AB         := messages_5_data + $13B8
msg5_AC         := messages_5_data + $13D1
msg5_AD         := messages_5_data + $13DC
msg5_AE         := messages_5_data + $13E0
msg5_AF         := messages_5_data + $13E4
msg5_B0         := messages_5_data + $13E8
msg5_B1         := messages_5_data + $1409
msg5_B2         := messages_5_data + $1421
msg5_B3         := messages_5_data + $143C
msg5_B4         := messages_5_data + $1447
msg5_B5         := messages_5_data + $146B
msg5_B6         := messages_5_data + $149E
msg5_B7         := messages_5_data + $14D5
msg5_B8         := messages_5_data + $14E3
msg5_B9         := messages_5_data + $1519
msg5_BA         := messages_5_data + $151D
msg5_BB         := messages_5_data + $1569
msg5_BC         := messages_5_data + $1579
msg5_BD         := messages_5_data + $1590
msg5_BE         := messages_5_data + $15A7
msg5_BF         := messages_5_data + $15C3
msg5_C0         := messages_5_data + $15E3
msg5_C1         := messages_5_data + $15FD
msg5_C2         := messages_5_data + $1635
msg5_C3         := messages_5_data + $164E
msg5_C4         := messages_5_data + $1674
msg5_C5         := messages_5_data + $168D
msg5_C6         := messages_5_data + $1696
msg5_C7         := messages_5_data + $1767
msg5_C8         := messages_5_data + $1778
msg5_C9         := messages_5_data + $178E
msg5_CA         := messages_5_data + $1792
msg5_CB         := messages_5_data + $17AA
msg5_CC         := messages_5_data + $17B8
msg5_CD         := messages_5_data + $17C6
msg5_CE         := messages_5_data + $17D5
msg5_CF         := messages_5_data + $17E3
msg5_D0         := messages_5_data + $1801
msg5_D1         := messages_5_data + $1816
msg5_D2         := messages_5_data + $1824
msg5_D3         := messages_5_data + $183B
msg5_D4         := messages_5_data + $186D
msg5_D5         := messages_5_data + $1871
msg5_D6         := messages_5_data + $188B
msg5_D7         := messages_5_data + $18A2
msg5_D8         := messages_5_data + $18B5
msg5_D9         := messages_5_data + $18C9
msg5_DA         := messages_5_data + $18EE
msg5_DB         := messages_5_data + $1912
msg5_DC         := messages_5_data + $197B
msg5_DD         := messages_5_data + $1986
msg5_DE         := messages_5_data + $19BC
msg5_DF         := messages_5_data + $19EA
msg5_E0         := messages_5_data + $1A02
msg5_E1         := messages_5_data + $1A1A
msg5_E2         := messages_5_data + $1A31
msg5_E3         := messages_5_data + $1A76
msg5_E4         := messages_5_data + $1A8D
msg5_E5         := messages_5_data + $1A9D
msg5_E6         := messages_5_data + $1ABA
msg5_E7         := messages_5_data + $1ACC
msg5_E8         := messages_5_data + $1AE2
msg5_E9         := messages_5_data + $1AF9
msg5_EA         := messages_5_data + $1B21
msg5_EB         := messages_5_data + $1B3C
msg5_EC         := messages_5_data + $1B4D
msg5_ED         := messages_5_data + $1B7F
msg5_EE         := messages_5_data + $1BAD
msg5_EF         := messages_5_data + $1BDF
msg5_F0         := messages_5_data + $1C11
msg5_F1         := messages_5_data + $1C2A
msg5_F2         := messages_5_data + $1C47
msg5_F3         := messages_5_data + $1C55
msg5_F4         := messages_5_data + $1C78
msg5_F5         := messages_5_data + $1C7C
msg5_F6         := messages_5_data + $1C8C
msg5_F7         := messages_5_data + $1CA8
msg5_F8         := messages_5_data + $1CB9
msg5_F9         := messages_5_data + $1CCA
msg5_FA         := messages_5_data + $1CEE
msg5_FB         := messages_5_data + $1D04
msg5_FC         := messages_5_data + $1D1B
msg5_FD         := messages_5_data + $1D2F
msg5_FE         := messages_5_data + $1D50
msg5_FF         := messages_5_data + $1D85


; unused bytes: 1576 bytes from your ROM
unused_5_data:
        .incbin "unused_5_data.bin"
unused_5        := unused_5_data + $0000

; ----------------------------------------------------------------------------
