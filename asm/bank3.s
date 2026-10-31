; Deja Vu (USA), NES: PRG bank 3 ($8000-$BFFF)
; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.
; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.

        .setcpu "6502"
        .segment "BANK3"
        .include "nes.inc"
        .include "ram.inc"

; ----------------------------------------------------------------------------
script_tables:
        .addr   scripts_aa0
        .addr   scripts_aa1
        .addr   scripts_aa2
        .addr   scripts_aa3
        .addr   scripts_aa4
        .addr   scripts_aa5
scripts_aa0:
        .addr   scripts_aa0_ab00
        .addr   scripts_aa0_ab01
        .addr   scripts_aa0_ab02
        .addr   scripts_aa0_ab03
        .addr   scripts_aa0_ab04
        .addr   scripts_aa0_ab05
        .addr   scripts_aa0_ab06
        .addr   scripts_aa0_ab07
        .addr   scripts_aa0_ab08
        .addr   scripts_aa0_ab09
        .addr   scripts_aa0_ab0A
        .addr   scripts_aa0_ab0B
        .addr   scripts_aa0_ab0C
        .addr   scripts_aa0_ab0D
        .addr   scripts_aa0_ab0E
        .addr   scripts_aa0_ab0F
        .addr   scripts_aa0_ab10
        .addr   scripts_aa0_ab11
        .addr   scripts_aa0_ab12
        .addr   scripts_aa0_ab13
        .addr   scripts_aa0_ab14
        .addr   scripts_aa0_ab15
        .addr   scripts_aa0_ab16
        .addr   scripts_aa0_ab17
        .addr   scripts_aa0_ab18
        .addr   scripts_aa0_ab19
        .addr   scripts_aa0_ab1A
        .addr   scripts_aa0_ab1B
        .addr   scripts_aa0_ab1C
        .addr   scripts_aa0_ab1D
        .addr   scripts_aa0_ab1E
        .addr   scripts_aa0_ab1F
        .addr   scripts_aa0_ab20
        .addr   scripts_aa0_ab21
        .addr   scripts_aa0_ab22
        .addr   scripts_aa0_ab23
        .addr   scripts_aa0_ab24
        .addr   scripts_aa0_ab25
        .addr   scripts_aa0_ab26
        .addr   scripts_aa0_ab27
        .addr   scripts_aa0_ab28
        .addr   scripts_aa0_ab29
        .addr   scripts_aa0_ab2A
        .addr   scripts_aa0_ab2B
        .addr   scripts_aa0_ab2C
        .addr   scripts_aa0_ab2D
        .addr   scripts_aa0_ab2E
        .addr   scripts_aa0_ab2F
        .addr   scripts_aa0_ab30
        .addr   scripts_aa0_ab31
        .addr   scripts_aa0_ab32
        .addr   scripts_aa0_ab33
        .addr   scripts_aa0_ab34
        .addr   scripts_aa0_ab35
        .addr   scripts_aa0_ab36
        .addr   scripts_aa0_ab37
        .addr   scripts_aa0_ab38
        .addr   scripts_aa0_ab39
        .addr   scripts_aa0_ab3A
        .addr   scripts_aa0_ab3B
        .addr   scripts_aa0_ab3C
        .addr   scripts_aa0_ab3D
        .addr   scripts_aa0_ab3E
        .addr   scripts_aa0_ab3F
        .addr   scripts_aa0_ab40
        .addr   scripts_aa0_ab41
        .addr   scripts_aa0_ab42
        .addr   scripts_aa0_ab43
        .addr   scripts_aa0_ab44
        .addr   scripts_aa0_ab45
        .addr   scripts_aa0_ab46
        .addr   scripts_aa0_ab47
        .addr   scripts_aa0_ab48
        .addr   scripts_aa0_ab49
        .addr   scripts_aa0_ab4A
        .addr   scripts_aa0_ab4B
        .addr   scripts_aa0_ab4C
        .addr   scripts_aa0_ab4D
        .addr   scripts_aa0_ab4E
        .addr   scripts_aa0_ab4F
        .addr   scripts_aa0_ab50
        .addr   scripts_aa0_ab51
        .addr   scripts_aa0_ab52
        .addr   scripts_aa0_ab53
        .addr   scripts_aa0_ab54
        .addr   scripts_aa0_ab55
        .addr   scripts_aa0_ab56
        .addr   scripts_aa0_ab57
        .addr   scripts_aa0_ab58
        .addr   scripts_aa0_ab59
        .addr   scripts_aa0_ab5A
        .addr   scripts_aa0_ab5B
        .addr   scripts_aa0_ab5C
        .addr   scripts_aa0_ab5D
        .addr   scripts_aa0_ab5E
        .addr   scripts_aa0_ab5F
        .addr   scripts_aa0_ab60
        .addr   scripts_aa0_ab61
        .addr   scripts_aa0_ab62
        .addr   scripts_aa0_ab63
        .addr   scripts_aa0_ab64
        .addr   scripts_aa0_ab65
        .addr   scripts_aa0_ab66
        .addr   scripts_aa0_ab67
        .addr   scripts_aa0_ab68
        .addr   scripts_aa0_ab69
        .addr   scripts_aa0_ab6A
        .addr   scripts_aa0_ab6B
        .addr   scripts_aa0_ab6C
        .addr   scripts_aa0_ab6D
        .addr   scripts_aa0_ab6E
        .addr   scripts_aa0_ab6F
        .addr   scripts_aa0_ab70
        .addr   scripts_aa0_ab71
        .addr   scripts_aa0_ab72
        .addr   scripts_aa0_ab73
        .addr   scripts_aa0_ab74
        .addr   scripts_aa0_ab75
        .addr   scripts_aa0_ab76
        .addr   scripts_aa0_ab77
scripts_aa0_ab00:
        .addr   script_0_00_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab01:
        .addr   script_0_01_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab02:
        .addr   script_0_02_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab03:
        .addr   script_0_03_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab04:
        .addr   script_0_04_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab05:
        .addr   script_0_05_0
        .addr   script_0_00_1
        .addr   script_0_05_2
        .addr   script_0_00_3
scripts_aa0_ab06:
        .addr   script_0_06_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab07:
        .addr   script_0_07_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab08:
        .addr   script_0_08_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab09:
        .addr   script_0_09_0
        .addr   script_0_00_1
        .addr   script_0_05_2
        .addr   script_0_02_3
scripts_aa0_ab0A:
        .addr   script_0_0A_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab0B:
        .addr   script_0_0B_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab0C:
        .addr   script_0_0C_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab0D:
        .addr   script_0_0D_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab0E:
        .addr   script_0_0E_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab0F:
        .addr   script_0_0F_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab10:
        .addr   script_0_10_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab11:
        .addr   script_0_11_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab12:
        .addr   script_0_12_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab13:
        .addr   script_0_13_0
        .addr   script_0_00_1
        .addr   script_0_13_2
        .addr   script_0_00_3
scripts_aa0_ab14:
        .addr   script_0_14_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab15:
        .addr   script_0_15_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab16:
        .addr   script_0_16_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_16_3
scripts_aa0_ab17:
        .addr   script_0_16_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_16_3
scripts_aa0_ab18:
        .addr   script_0_18_0
        .addr   script_0_00_1
        .addr   script_0_18_2
        .addr   script_0_18_2
scripts_aa0_ab19:
        .addr   script_0_19_0
        .addr   script_0_00_1
        .addr   script_0_19_2
        .addr   script_0_19_2
scripts_aa0_ab1A:
        .addr   script_0_18_0
        .addr   script_0_00_1
        .addr   script_0_1A_2
        .addr   script_0_1A_2
scripts_aa0_ab1B:
        .addr   script_0_18_0
        .addr   script_0_00_1
        .addr   script_0_1B_2
        .addr   script_0_1B_2
scripts_aa0_ab1C:
        .addr   script_0_1C_0
        .addr   script_0_00_1
        .addr   script_0_1C_2
        .addr   script_0_1C_2
scripts_aa0_ab1D:
        .addr   script_0_1D_0
        .addr   script_0_00_1
        .addr   script_0_1D_2
        .addr   script_0_1D_2
scripts_aa0_ab1E:
        .addr   script_0_1E_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab1F:
        .addr   script_0_1F_0
        .addr   script_0_01_3
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab20:
        .addr   script_0_20_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab21:
        .addr   script_0_21_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab22:
        .addr   script_0_22_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab23:
        .addr   script_0_23_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab24:
        .addr   script_0_24_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab25:
        .addr   script_0_25_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab26:
        .addr   script_0_26_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab27:
        .addr   script_0_27_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab28:
        .addr   script_0_28_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab29:
        .addr   script_0_29_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab2A:
        .addr   script_0_2A_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab2B:
        .addr   script_0_2B_0
        .addr   script_0_2B_1
        .addr   script_0_2B_2
        .addr   script_0_01_3
scripts_aa0_ab2C:
        .addr   script_0_2C_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab2D:
        .addr   script_0_2D_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab2E:
        .addr   script_0_2E_0
        .addr   script_0_2E_0
        .addr   script_0_2E_2
        .addr   script_0_2E_0
scripts_aa0_ab2F:
        .addr   script_0_2F_0
        .addr   script_0_2F_0
        .addr   script_0_2F_2
        .addr   script_0_01_3
scripts_aa0_ab30:
        .addr   script_0_30_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab31:
        .addr   script_0_31_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab32:
        .addr   script_0_32_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab33:
        .addr   script_0_0B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab34:
        .addr   script_0_34_0
        .addr   script_0_01_3
        .addr   script_0_34_2
        .addr   script_0_01_3
scripts_aa0_ab35:
        .addr   script_0_35_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab36:
        .addr   script_0_36_0
        .addr   script_0_36_1
        .addr   script_0_36_2
        .addr   script_0_01_3
scripts_aa0_ab37:
        .addr   script_0_37_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab38:
        .addr   script_0_38_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab39:
        .addr   script_0_39_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab3A:
        .addr   script_0_3A_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab3B:
        .addr   script_0_3B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab3C:
        .addr   script_0_3C_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab3D:
        .addr   script_0_0E_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab3E:
        .addr   script_0_3E_0
        .addr   script_0_00_1
        .addr   script_0_3E_2
        .addr   script_0_3E_2
scripts_aa0_ab3F:
        .addr   script_0_3F_0
        .addr   script_0_3F_1
        .addr   script_0_3F_2
        .addr   script_0_01_3
scripts_aa0_ab40:
        .addr   script_0_40_0
        .addr   script_0_40_1
        .addr   script_0_40_2
        .addr   script_0_01_3
scripts_aa0_ab41:
        .addr   script_0_41_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab42:
        .addr   script_0_42_0
        .addr   script_0_42_1
        .addr   script_0_42_2
        .addr   script_0_01_3
scripts_aa0_ab43:
        .addr   script_0_43_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab44:
        .addr   script_0_44_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab45:
        .addr   script_0_45_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab46:
        .addr   script_0_46_0
        .addr   script_0_46_1
        .addr   script_0_42_2
        .addr   script_0_01_3
scripts_aa0_ab47:
        .addr   script_0_43_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab48:
        .addr   script_0_48_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab49:
        .addr   script_0_45_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab4A:
        .addr   script_0_4A_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab4B:
        .addr   script_0_4B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab4C:
        .addr   script_0_4C_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab4D:
        .addr   script_0_4D_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab4E:
        .addr   script_0_4E_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab4F:
        .addr   script_0_4F_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab50:
        .addr   script_0_50_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab51:
        .addr   script_0_51_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab52:
        .addr   script_0_52_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab53:
        .addr   script_0_53_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab54:
        .addr   script_0_54_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab55:
        .addr   script_0_55_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab56:
        .addr   script_0_56_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab57:
        .addr   script_0_57_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab58:
        .addr   script_0_58_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab59:
        .addr   script_0_59_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab5A:
        .addr   script_0_5A_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab5B:
        .addr   script_0_5B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab5C:
        .addr   script_0_5C_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab5D:
        .addr   script_0_5D_0
        .addr   script_0_5D_1
        .addr   script_0_5D_1
        .addr   script_0_5D_1
scripts_aa0_ab5E:
        .addr   script_0_5E_0
        .addr   script_0_00_1
        .addr   script_0_5E_2
        .addr   script_0_5E_2
scripts_aa0_ab5F:
        .addr   script_0_5F_0
        .addr   script_0_5F_1
        .addr   script_0_5F_2
        .addr   script_0_5F_1
scripts_aa0_ab60:
        .addr   script_0_60_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab61:
        .addr   script_0_61_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab62:
        .addr   script_0_62_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab63:
        .addr   script_0_63_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab64:
        .addr   script_0_64_0
        .addr   script_0_64_1
        .addr   script_0_64_2
        .addr   script_0_01_3
scripts_aa0_ab65:
        .addr   script_0_65_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab66:
        .addr   script_0_66_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab67:
        .addr   script_0_67_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab68:
        .addr   script_0_68_0
        .addr   script_0_68_1
        .addr   script_0_64_2
        .addr   script_0_01_3
scripts_aa0_ab69:
        .addr   script_0_69_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab6A:
        .addr   script_0_6A_0
        .addr   script_0_6A_1
        .addr   script_0_6A_1
        .addr   script_0_6A_1
scripts_aa0_ab6B:
        .addr   script_0_6B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab6C:
        .addr   script_0_0B_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_02_3
scripts_aa0_ab6D:
        .addr   script_0_0E_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab6E:
        .addr   script_0_0E_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab6F:
        .addr   script_0_6F_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab70:
        .addr   script_0_0B_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab71:
        .addr   script_0_0E_0
        .addr   script_0_01_3
        .addr   script_0_01_3
        .addr   script_0_01_3
scripts_aa0_ab72:
        .addr   script_0_72_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_01_3
scripts_aa0_ab73:
        .addr   script_0_73_0
        .addr   script_0_73_1
        .addr   script_0_73_2
        .addr   script_0_01_3
scripts_aa0_ab74:
        .addr   script_0_74_0
        .addr   script_0_00_1
        .addr   script_0_00_2
        .addr   script_0_00_3
scripts_aa0_ab75:
        .addr   script_0_2E_0
        .addr   script_0_2E_0
        .addr   script_0_2E_2
        .addr   script_0_2E_0
scripts_aa0_ab76:
        .addr   script_0_2E_0
        .addr   script_0_2E_0
        .addr   script_0_2E_2
        .addr   script_0_2E_0
scripts_aa0_ab77:
        .addr   script_0_2E_0
        .addr   script_0_2E_0
        .addr   script_0_2E_2
        .addr   script_0_2E_0

; bytecode scripts: 1058 bytes from your ROM
scripts_84BC_data:
        .incbin "scripts_84BC_data.bin"
script_0_00_0   := scripts_84BC_data + $0000
script_0_00_1   := scripts_84BC_data + $0003
script_0_5D_1   := scripts_84BC_data + $0006
script_0_00_2   := scripts_84BC_data + $0007
script_0_00_3   := scripts_84BC_data + $000A
script_0_01_0   := scripts_84BC_data + $000D
script_0_02_0   := scripts_84BC_data + $0010
script_0_02_3   := scripts_84BC_data + $0013
script_0_03_0   := scripts_84BC_data + $0016
script_0_04_0   := scripts_84BC_data + $002B
script_0_05_0   := scripts_84BC_data + $002E
script_0_05_2   := scripts_84BC_data + $003C
script_0_06_0   := scripts_84BC_data + $003F
script_0_07_0   := scripts_84BC_data + $0042
script_0_08_0   := scripts_84BC_data + $004A
script_0_09_0   := scripts_84BC_data + $004D
script_0_0A_0   := scripts_84BC_data + $0050
script_0_0C_0   := scripts_84BC_data + $0053
script_0_0D_0   := scripts_84BC_data + $0056
script_0_0F_0   := scripts_84BC_data + $005E
script_0_10_0   := scripts_84BC_data + $0061
script_0_11_0   := scripts_84BC_data + $0064
script_0_12_0   := scripts_84BC_data + $0071
script_0_13_0   := scripts_84BC_data + $0074
script_0_13_2   := scripts_84BC_data + $007F
script_0_14_0   := scripts_84BC_data + $0094
script_0_15_0   := scripts_84BC_data + $0097
script_0_16_0   := scripts_84BC_data + $00A2
script_0_16_3   := scripts_84BC_data + $00BB
script_0_18_2   := scripts_84BC_data + $00BE
script_0_19_0   := scripts_84BC_data + $00CD
script_0_19_2   := scripts_84BC_data + $00D0
script_0_18_0   := scripts_84BC_data + $00EF
script_0_1A_2   := scripts_84BC_data + $00F2
script_0_1B_2   := scripts_84BC_data + $0111
script_0_1C_0   := scripts_84BC_data + $0130
script_0_1C_2   := scripts_84BC_data + $0133
script_0_1D_0   := scripts_84BC_data + $0155
script_0_1D_2   := scripts_84BC_data + $0158
script_0_1E_0   := scripts_84BC_data + $0167
script_0_1F_0   := scripts_84BC_data + $0172
script_0_20_0   := scripts_84BC_data + $0175
script_0_21_0   := scripts_84BC_data + $0178
script_0_22_0   := scripts_84BC_data + $017B
script_0_23_0   := scripts_84BC_data + $017E
script_0_24_0   := scripts_84BC_data + $0181
script_0_25_0   := scripts_84BC_data + $0188
script_0_26_0   := scripts_84BC_data + $018B
script_0_27_0   := scripts_84BC_data + $018E
script_0_28_0   := scripts_84BC_data + $01A1
script_0_29_0   := scripts_84BC_data + $01A4
script_0_2A_0   := scripts_84BC_data + $01A7
script_0_2B_0   := scripts_84BC_data + $01AA
script_0_2B_1   := scripts_84BC_data + $01AD
script_0_2B_2   := scripts_84BC_data + $01B8
script_0_2C_0   := scripts_84BC_data + $01BB
script_0_2D_0   := scripts_84BC_data + $01C3
script_0_2E_0   := scripts_84BC_data + $01C6
script_0_2E_2   := scripts_84BC_data + $01D9
script_0_2F_0   := scripts_84BC_data + $0232
script_0_2F_2   := scripts_84BC_data + $023E
script_0_30_0   := scripts_84BC_data + $024A
script_0_31_0   := scripts_84BC_data + $024D
script_0_32_0   := scripts_84BC_data + $0250
script_0_34_0   := scripts_84BC_data + $0253
script_0_34_2   := scripts_84BC_data + $025F
script_0_35_0   := scripts_84BC_data + $026F
script_0_01_3   := scripts_84BC_data + $0272
script_0_36_0   := scripts_84BC_data + $0275
script_0_36_1   := scripts_84BC_data + $0278
script_0_36_2   := scripts_84BC_data + $027B
script_0_37_0   := scripts_84BC_data + $0280
script_0_38_0   := scripts_84BC_data + $0283
script_0_39_0   := scripts_84BC_data + $0286
script_0_3A_0   := scripts_84BC_data + $0289
script_0_3B_0   := scripts_84BC_data + $028C
script_0_3C_0   := scripts_84BC_data + $028F
script_0_3E_0   := scripts_84BC_data + $0292
script_0_3E_2   := scripts_84BC_data + $0295
script_0_3F_0   := scripts_84BC_data + $02A3
script_0_3F_1   := scripts_84BC_data + $02C0
script_0_3F_2   := scripts_84BC_data + $02C3
script_0_40_0   := scripts_84BC_data + $02C6
script_0_40_1   := scripts_84BC_data + $02C9
script_0_40_2   := scripts_84BC_data + $02CC
script_0_41_0   := scripts_84BC_data + $02D6
script_0_42_0   := scripts_84BC_data + $02D9
script_0_42_1   := scripts_84BC_data + $02DC
script_0_42_2   := scripts_84BC_data + $02FA
script_0_43_0   := scripts_84BC_data + $02FD
script_0_44_0   := scripts_84BC_data + $0300
script_0_45_0   := scripts_84BC_data + $0303
script_0_46_0   := scripts_84BC_data + $0306
script_0_46_1   := scripts_84BC_data + $0309
script_0_48_0   := scripts_84BC_data + $0327
script_0_4A_0   := scripts_84BC_data + $032A
script_0_4B_0   := scripts_84BC_data + $032D
script_0_4C_0   := scripts_84BC_data + $033C
script_0_4D_0   := scripts_84BC_data + $033F
script_0_4E_0   := scripts_84BC_data + $0342
script_0_4F_0   := scripts_84BC_data + $0345
script_0_50_0   := scripts_84BC_data + $0348
script_0_51_0   := scripts_84BC_data + $034B
script_0_52_0   := scripts_84BC_data + $034E
script_0_53_0   := scripts_84BC_data + $0351
script_0_54_0   := scripts_84BC_data + $0354
script_0_55_0   := scripts_84BC_data + $035C
script_0_56_0   := scripts_84BC_data + $035F
script_0_57_0   := scripts_84BC_data + $0362
script_0_58_0   := scripts_84BC_data + $0365
script_0_59_0   := scripts_84BC_data + $0368
script_0_5A_0   := scripts_84BC_data + $036B
script_0_5B_0   := scripts_84BC_data + $036E
script_0_5C_0   := scripts_84BC_data + $0371
script_0_5D_0   := scripts_84BC_data + $037C
script_0_5E_0   := scripts_84BC_data + $037D
script_0_5E_2   := scripts_84BC_data + $0380
script_0_5F_0   := scripts_84BC_data + $0390
script_0_5F_1   := scripts_84BC_data + $0393
script_0_5F_2   := scripts_84BC_data + $03A1
script_0_60_0   := scripts_84BC_data + $03AA
script_0_61_0   := scripts_84BC_data + $03AD
script_0_62_0   := scripts_84BC_data + $03B0
script_0_63_0   := scripts_84BC_data + $03B3
script_0_64_0   := scripts_84BC_data + $03B6
script_0_64_1   := scripts_84BC_data + $03C1
script_0_64_2   := scripts_84BC_data + $03C4
script_0_65_0   := scripts_84BC_data + $03C7
script_0_66_0   := scripts_84BC_data + $03CF
script_0_67_0   := scripts_84BC_data + $03D2
script_0_68_0   := scripts_84BC_data + $03D5
script_0_68_1   := scripts_84BC_data + $03DA
script_0_69_0   := scripts_84BC_data + $03DD
script_0_6A_0   := scripts_84BC_data + $03E0
script_0_6A_1   := scripts_84BC_data + $03E3
script_0_6B_0   := scripts_84BC_data + $03EB
script_0_0B_0   := scripts_84BC_data + $040A
script_0_0E_0   := scripts_84BC_data + $040D
script_0_6F_0   := scripts_84BC_data + $0410
script_0_72_0   := scripts_84BC_data + $0413
script_0_73_0   := scripts_84BC_data + $0416
script_0_73_1   := scripts_84BC_data + $0419
script_0_73_2   := scripts_84BC_data + $041C
script_0_74_0   := scripts_84BC_data + $041F

; ----------------------------------------------------------------------------
scripts_aa1:
        .addr   scripts_aa1_ab00
        .addr   scripts_aa1_ab01
        .addr   scripts_aa1_ab02
        .addr   scripts_aa1_ab03
        .addr   scripts_aa1_ab04
        .addr   scripts_aa1_ab05
        .addr   scripts_aa1_ab06
        .addr   scripts_aa1_ab07
        .addr   scripts_aa1_ab08
        .addr   scripts_aa1_ab09
        .addr   scripts_aa1_ab0A
        .addr   scripts_aa1_ab0B
        .addr   scripts_aa1_ab0C
scripts_aa1_ab00:
        .addr   script_1_00_0
        .addr   script_1_00_1
        .addr   script_1_00_2
        .addr   script_1_00_3
scripts_aa1_ab01:
        .addr   script_1_01_0
        .addr   script_1_01_1
        .addr   script_1_01_2
        .addr   script_1_01_3
scripts_aa1_ab02:
        .addr   script_1_02_0
        .addr   script_1_02_1
        .addr   script_1_02_2
        .addr   script_1_02_3
scripts_aa1_ab03:
        .addr   script_1_03_0
        .addr   script_1_03_1
        .addr   script_1_03_2
        .addr   script_1_01_3
scripts_aa1_ab04:
        .addr   script_1_04_0
        .addr   script_1_04_1
        .addr   script_1_04_2
        .addr   script_1_00_3
scripts_aa1_ab05:
        .addr   script_1_05_0
        .addr   script_1_05_1
        .addr   script_1_05_2
        .addr   script_1_01_3
scripts_aa1_ab06:
        .addr   script_1_06_0
        .addr   script_1_06_1
        .addr   script_1_06_2
        .addr   script_1_02_3
scripts_aa1_ab07:
        .addr   script_1_07_0
        .addr   script_1_07_1
        .addr   script_1_07_2
        .addr   script_1_00_3
scripts_aa1_ab08:
        .addr   script_1_08_0
        .addr   script_1_08_1
        .addr   script_1_06_2
        .addr   script_1_01_3
scripts_aa1_ab09:
        .addr   script_1_09_0
        .addr   script_1_09_1
        .addr   script_1_09_2
        .addr   script_1_00_3
scripts_aa1_ab0A:
        .addr   script_1_0A_0
        .addr   script_1_0A_1
        .addr   script_1_0A_2
        .addr   script_1_01_3
scripts_aa1_ab0B:
        .addr   script_1_0A_0
        .addr   script_1_0B_1
        .addr   script_1_0B_2
        .addr   script_1_00_3
scripts_aa1_ab0C:
        .addr   script_1_0C_0
        .addr   script_1_0C_1
        .addr   script_1_0C_2
        .addr   script_1_02_3

; bytecode scripts: 503 bytes from your ROM
scripts_8960_data:
        .incbin "scripts_8960_data.bin"
script_1_00_0   := scripts_8960_data + $0000
script_1_00_1   := scripts_8960_data + $000B
script_1_01_1   := scripts_8960_data + $001B
script_1_02_1   := scripts_8960_data + $002B
script_1_03_1   := scripts_8960_data + $0037
script_1_04_1   := scripts_8960_data + $0047
script_1_05_1   := scripts_8960_data + $0059
script_1_07_1   := scripts_8960_data + $0069
script_1_06_1   := scripts_8960_data + $0079
script_1_08_1   := scripts_8960_data + $0088
script_1_09_1   := scripts_8960_data + $009B
script_1_0A_1   := scripts_8960_data + $00AB
script_1_0B_1   := scripts_8960_data + $00BB
script_1_0C_1   := scripts_8960_data + $00CB
script_1_00_2   := scripts_8960_data + $00E1
script_1_01_2   := scripts_8960_data + $00F1
script_1_02_2   := scripts_8960_data + $0101
script_1_03_2   := scripts_8960_data + $0113
script_1_04_2   := scripts_8960_data + $0123
script_1_05_2   := scripts_8960_data + $0135
script_1_06_2   := scripts_8960_data + $0145
script_1_07_2   := scripts_8960_data + $0156
script_1_09_2   := scripts_8960_data + $0166
script_1_0A_2   := scripts_8960_data + $0176
script_1_0B_2   := scripts_8960_data + $0186
script_1_0C_2   := scripts_8960_data + $0196
script_1_00_3   := scripts_8960_data + $01AC
script_1_01_0   := scripts_8960_data + $01AF
script_1_01_3   := scripts_8960_data + $01B7
script_1_02_0   := scripts_8960_data + $01BA
script_1_03_0   := scripts_8960_data + $01C5
script_1_04_0   := scripts_8960_data + $01C8
script_1_05_0   := scripts_8960_data + $01CF
script_1_06_0   := scripts_8960_data + $01D2
script_1_02_3   := scripts_8960_data + $01DD
script_1_07_0   := scripts_8960_data + $01E0
script_1_08_0   := scripts_8960_data + $01E3
script_1_09_0   := scripts_8960_data + $01EE
script_1_0A_0   := scripts_8960_data + $01F1
script_1_0C_0   := scripts_8960_data + $01F4

; ----------------------------------------------------------------------------
scripts_aa2:
        .addr   scripts_aa2_ab00
        .addr   scripts_aa2_ab01
        .addr   scripts_aa2_ab02
        .addr   scripts_aa2_ab03
        .addr   scripts_aa2_ab04
scripts_aa2_ab00:
        .addr   script_2_00_0
        .addr   script_2_00_1
        .addr   script_2_00_2
        .addr   script_2_00_3
scripts_aa2_ab01:
        .addr   script_2_01_0
        .addr   script_2_01_1
        .addr   script_2_01_2
        .addr   script_2_01_3
scripts_aa2_ab02:
        .addr   script_2_02_0
        .addr   script_2_02_1
        .addr   script_2_02_2
        .addr   script_2_00_3
scripts_aa2_ab03:
        .addr   script_2_02_0
        .addr   script_2_03_1
        .addr   script_2_03_2
        .addr   script_2_01_3
scripts_aa2_ab04:
        .addr   script_2_04_0
        .addr   script_2_00_3
        .addr   script_2_00_3
        .addr   script_2_00_3

; bytecode scripts: 173 bytes from your ROM
scripts_8B89_data:
        .incbin "scripts_8B89_data.bin"
script_2_00_0   := scripts_8B89_data + $0000
script_2_00_1   := scripts_8B89_data + $0003
script_2_00_2   := scripts_8B89_data + $0023
script_2_00_3   := scripts_8B89_data + $0038
script_2_01_0   := scripts_8B89_data + $003B
script_2_01_1   := scripts_8B89_data + $003E
script_2_01_2   := scripts_8B89_data + $004E
script_2_01_3   := scripts_8B89_data + $005E
script_2_02_1   := scripts_8B89_data + $0061
script_2_02_2   := scripts_8B89_data + $0071
script_2_02_0   := scripts_8B89_data + $0081
script_2_03_1   := scripts_8B89_data + $0084
script_2_03_2   := scripts_8B89_data + $0092
script_2_04_0   := scripts_8B89_data + $00A0

; ----------------------------------------------------------------------------
scripts_aa3:
        .addr   scripts_aa3_ab00
        .addr   scripts_aa3_ab01
        .addr   scripts_aa3_ab02
        .addr   scripts_aa3_ab03
        .addr   scripts_aa3_ab04
        .addr   scripts_aa3_ab05
        .addr   scripts_aa3_ab06
        .addr   scripts_aa3_ab07
        .addr   scripts_aa3_ab08
        .addr   scripts_aa3_ab09
        .addr   scripts_aa3_ab0A
        .addr   scripts_aa3_ab0B
        .addr   scripts_aa3_ab0C
        .addr   scripts_aa3_ab0D
        .addr   scripts_aa3_ab0E
        .addr   scripts_aa3_ab0F
        .addr   scripts_aa3_ab10
        .addr   scripts_aa3_ab11
        .addr   scripts_aa3_ab12
        .addr   scripts_aa3_ab13
        .addr   scripts_aa3_ab14
        .addr   scripts_aa3_ab15
        .addr   scripts_aa3_ab16
        .addr   scripts_aa3_ab17
        .addr   scripts_aa3_ab18
        .addr   scripts_aa3_ab19
        .addr   scripts_aa3_ab1A
        .addr   scripts_aa3_ab1B
        .addr   scripts_aa3_ab1C
        .addr   scripts_aa3_ab1D
        .addr   scripts_aa3_ab1E
        .addr   scripts_aa3_ab1F
        .addr   scripts_aa3_ab20
        .addr   scripts_aa3_ab21
        .addr   scripts_aa3_ab22
        .addr   scripts_aa3_ab23
        .addr   scripts_aa3_ab24
        .addr   scripts_aa3_ab25
        .addr   scripts_aa3_ab26
        .addr   scripts_aa3_ab27
        .addr   scripts_aa3_ab28
        .addr   scripts_aa3_ab29
        .addr   scripts_aa3_ab2A
        .addr   scripts_aa3_ab2B
        .addr   scripts_aa3_ab2C
        .addr   scripts_aa3_ab2D
        .addr   scripts_aa3_ab2E
        .addr   scripts_aa3_ab2F
        .addr   scripts_aa3_ab30
        .addr   scripts_aa3_ab31
        .addr   scripts_aa3_ab32
        .addr   scripts_aa3_ab33
        .addr   scripts_aa3_ab34
        .addr   scripts_aa3_ab35
        .addr   scripts_aa3_ab36
        .addr   scripts_aa3_ab37
        .addr   scripts_aa3_ab38
        .addr   scripts_aa3_ab39
        .addr   scripts_aa3_ab3A
        .addr   scripts_aa3_ab3B
        .addr   scripts_aa3_ab3C
        .addr   scripts_aa3_ab3D
        .addr   scripts_aa3_ab3E
        .addr   scripts_aa3_ab3F
        .addr   scripts_aa3_ab40
        .addr   scripts_aa3_ab41
        .addr   scripts_aa3_ab42
        .addr   scripts_aa3_ab43
        .addr   scripts_aa3_ab44
        .addr   scripts_aa3_ab45
        .addr   scripts_aa3_ab46
        .addr   scripts_aa3_ab47
        .addr   scripts_aa3_ab48
        .addr   scripts_aa3_ab49
        .addr   scripts_aa3_ab4A
        .addr   scripts_aa3_ab4B
        .addr   scripts_aa3_ab4C
        .addr   scripts_aa3_ab4D
        .addr   scripts_aa3_ab4E
        .addr   scripts_aa3_ab4F
        .addr   scripts_aa3_ab50
        .addr   scripts_aa3_ab51
        .addr   scripts_aa3_ab52
        .addr   scripts_aa3_ab53
        .addr   scripts_aa3_ab54
        .addr   scripts_aa3_ab55
        .addr   scripts_aa3_ab56
        .addr   scripts_aa3_ab57
        .addr   scripts_aa3_ab58
        .addr   scripts_aa3_ab59
        .addr   scripts_aa3_ab5A
        .addr   scripts_aa3_ab5B
        .addr   scripts_aa3_ab5C
scripts_aa3_ab00:
        .addr   script_3_00_0
        .addr   script_3_00_1
        .addr   script_3_00_2
        .addr   script_3_00_3
scripts_aa3_ab01:
        .addr   script_3_01_0
        .addr   script_3_01_1
        .addr   script_3_00_2
        .addr   script_3_01_3
scripts_aa3_ab02:
        .addr   script_3_02_0
        .addr   script_3_02_1
        .addr   script_3_02_2
        .addr   script_3_02_3
scripts_aa3_ab03:
        .addr   script_3_03_0
        .addr   script_3_03_1
        .addr   script_3_02_2
        .addr   script_3_03_3
scripts_aa3_ab04:
        .addr   script_3_04_0
        .addr   script_3_04_1
        .addr   script_3_02_2
        .addr   script_3_04_3
scripts_aa3_ab05:
        .addr   script_3_05_0
        .addr   script_3_05_1
        .addr   script_3_00_2
        .addr   script_3_05_3
scripts_aa3_ab06:
        .addr   script_3_06_0
        .addr   script_3_06_1
        .addr   script_3_00_2
        .addr   script_3_06_3
scripts_aa3_ab07:
        .addr   script_3_07_0
        .addr   script_3_07_1
        .addr   script_3_00_2
        .addr   script_3_07_3
scripts_aa3_ab08:
        .addr   script_3_08_0
        .addr   script_3_08_1
        .addr   script_3_08_2
        .addr   script_3_08_3
scripts_aa3_ab09:
        .addr   script_3_09_0
        .addr   script_3_09_1
        .addr   script_3_09_0
        .addr   script_3_08_1
scripts_aa3_ab0A:
        .addr   script_3_0A_0
        .addr   script_3_0A_1
        .addr   script_3_00_2
        .addr   script_3_0A_3
scripts_aa3_ab0B:
        .addr   script_3_0B_0
        .addr   script_3_0B_1
        .addr   script_3_00_2
        .addr   script_3_0B_3
scripts_aa3_ab0C:
        .addr   script_3_0C_0
        .addr   script_3_0C_1
        .addr   script_3_00_2
        .addr   script_3_0C_3
scripts_aa3_ab0D:
        .addr   script_3_0D_0
        .addr   script_3_0D_1
        .addr   script_3_00_2
        .addr   script_3_0D_3
scripts_aa3_ab0E:
        .addr   script_3_0E_0
        .addr   script_3_0E_1
        .addr   script_3_0E_2
        .addr   script_3_08_1
scripts_aa3_ab0F:
        .addr   script_3_0F_0
        .addr   script_3_0F_1
        .addr   script_3_00_2
        .addr   script_3_0F_3
scripts_aa3_ab10:
        .addr   script_3_10_0
        .addr   script_3_10_1
        .addr   script_3_10_2
        .addr   script_3_08_1
scripts_aa3_ab11:
        .addr   script_3_11_0
        .addr   script_3_11_1
        .addr   script_3_11_2
        .addr   script_3_08_1
scripts_aa3_ab12:
        .addr   script_3_12_0
        .addr   script_3_12_1
        .addr   script_3_12_2
        .addr   script_3_08_1
scripts_aa3_ab13:
        .addr   script_3_13_0
        .addr   script_3_13_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab14:
        .addr   script_3_14_0
        .addr   script_3_14_1
        .addr   script_3_02_2
        .addr   script_3_03_3
scripts_aa3_ab15:
        .addr   script_3_15_0
        .addr   script_3_15_1
        .addr   script_3_00_2
        .addr   script_3_15_3
scripts_aa3_ab16:
        .addr   script_3_16_0
        .addr   script_3_16_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab17:
        .addr   script_3_17_0
        .addr   script_3_17_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab18:
        .addr   script_3_18_0
        .addr   script_3_18_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab19:
        .addr   script_3_19_0
        .addr   script_3_19_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab1A:
        .addr   script_3_1A_0
        .addr   script_3_1A_1
        .addr   script_3_00_2
        .addr   script_3_08_1
scripts_aa3_ab1B:
        .addr   script_3_1B_0
        .addr   script_3_1B_1
        .addr   script_3_1B_2
        .addr   script_3_08_1
scripts_aa3_ab1C:
        .addr   script_3_1C_0
        .addr   script_3_1C_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab1D:
        .addr   script_3_1D_0
        .addr   script_3_1D_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab1E:
        .addr   script_3_1E_0
        .addr   script_3_1E_1
        .addr   script_3_00_2
        .addr   script_3_08_1
scripts_aa3_ab1F:
        .addr   script_3_1F_0
        .addr   script_3_1F_1
        .addr   script_3_02_2
        .addr   script_3_1F_3
scripts_aa3_ab20:
        .addr   script_3_20_0
        .addr   script_3_20_1
        .addr   script_3_02_2
        .addr   script_3_20_3
scripts_aa3_ab21:
        .addr   script_3_21_0
        .addr   script_3_21_1
        .addr   script_3_02_2
        .addr   script_3_03_3
scripts_aa3_ab22:
        .addr   script_3_22_0
        .addr   script_3_22_1
        .addr   script_3_00_2
        .addr   script_3_08_1
scripts_aa3_ab23:
        .addr   script_3_23_0
        .addr   script_3_23_1
        .addr   script_3_02_2
        .addr   script_3_23_3
scripts_aa3_ab24:
        .addr   script_3_24_0
        .addr   script_3_24_1
        .addr   script_3_00_2
        .addr   script_3_08_1
scripts_aa3_ab25:
        .addr   script_3_25_0
        .addr   script_3_25_1
        .addr   script_3_02_2
        .addr   script_3_25_3
scripts_aa3_ab26:
        .addr   script_3_26_0
        .addr   script_3_26_1
        .addr   script_3_02_2
        .addr   script_3_26_3
scripts_aa3_ab27:
        .addr   script_3_27_0
        .addr   script_3_27_1
        .addr   script_3_02_2
        .addr   script_3_27_3
scripts_aa3_ab28:
        .addr   script_3_28_0
        .addr   script_3_28_1
        .addr   script_3_28_2
        .addr   script_3_08_1
scripts_aa3_ab29:
        .addr   script_3_29_0
        .addr   script_3_29_1
        .addr   script_3_00_2
        .addr   script_3_29_3
scripts_aa3_ab2A:
        .addr   script_3_2A_0
        .addr   script_3_28_1
        .addr   script_3_28_2
        .addr   script_3_08_1
scripts_aa3_ab2B:
        .addr   script_3_2B_0
        .addr   script_3_2B_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab2C:
        .addr   script_3_2C_0
        .addr   script_3_2C_1
        .addr   script_3_02_2
        .addr   script_3_03_3
scripts_aa3_ab2D:
        .addr   script_3_2D_0
        .addr   script_3_08_1
        .addr   script_3_2D_2
        .addr   script_3_2D_3
scripts_aa3_ab2E:
        .addr   script_3_2E_0
        .addr   script_3_2E_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab2F:
        .addr   script_3_2F_0
        .addr   script_3_2F_1
        .addr   script_3_2F_2
        .addr   script_3_08_1
scripts_aa3_ab30:
        .addr   script_3_30_0
        .addr   script_3_30_1
        .addr   script_3_02_2
        .addr   script_3_03_3
scripts_aa3_ab31:
        .addr   script_3_31_0
        .addr   script_3_31_1
        .addr   script_3_00_2
        .addr   script_3_31_3
scripts_aa3_ab32:
        .addr   script_3_32_0
        .addr   script_3_32_1
        .addr   script_3_00_2
        .addr   script_3_32_3
scripts_aa3_ab33:
        .addr   script_3_33_0
        .addr   script_3_33_1
        .addr   script_3_00_2
        .addr   script_3_33_3
scripts_aa3_ab34:
        .addr   script_3_34_0
        .addr   script_3_34_1
        .addr   script_3_00_2
        .addr   script_3_34_3
scripts_aa3_ab35:
        .addr   script_3_35_0
        .addr   script_3_35_1
        .addr   script_3_00_2
        .addr   script_3_35_3
scripts_aa3_ab36:
        .addr   script_3_36_0
        .addr   script_3_36_1
        .addr   script_3_00_2
        .addr   script_3_36_3
scripts_aa3_ab37:
        .addr   script_3_37_0
        .addr   script_3_37_1
        .addr   script_3_02_2
        .addr   script_3_37_3
scripts_aa3_ab38:
        .addr   script_3_38_0
        .addr   script_3_38_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab39:
        .addr   script_3_39_0
        .addr   script_3_39_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab3A:
        .addr   script_3_3A_0
        .addr   script_3_3A_1
        .addr   script_3_02_2
        .addr   script_3_08_1
scripts_aa3_ab3B:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab3C:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab3D:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab3E:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab3F:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab40:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab41:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab42:
        .addr   script_3_08_0
        .addr   script_3_3D_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab43:
        .addr   script_3_08_0
        .addr   script_3_08_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab44:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab45:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab46:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab47:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab48:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab49:
        .addr   script_3_08_0
        .addr   script_3_49_1
        .addr   script_3_08_2
        .addr   script_3_3D_3
scripts_aa3_ab4A:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab4B:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab4C:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab4D:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab4E:
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
        .addr   script_3_09_0
scripts_aa3_ab4F:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab50:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab51:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab52:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab53:
        .addr   script_3_2D_0
        .addr   script_3_4A_1
        .addr   script_3_2D_2
        .addr   script_3_3D_3
scripts_aa3_ab54:
        .addr   script_3_07_0
        .addr   script_3_54_1
        .addr   script_3_00_2
        .addr   script_3_54_3
scripts_aa3_ab55:
        .addr   script_3_55_0
        .addr   script_3_55_1
        .addr   script_3_00_2
        .addr   script_3_55_3
scripts_aa3_ab56:
        .addr   script_3_55_0
        .addr   script_3_55_1
        .addr   script_3_00_2
        .addr   script_3_56_3
scripts_aa3_ab57:
        .addr   script_3_55_0
        .addr   script_3_55_1
        .addr   script_3_00_2
        .addr   script_3_57_3
scripts_aa3_ab58:
        .addr   script_3_29_0
        .addr   script_3_29_1
        .addr   script_3_00_2
        .addr   script_3_58_3
scripts_aa3_ab59:
        .addr   script_3_29_0
        .addr   script_3_29_1
        .addr   script_3_00_2
        .addr   script_3_59_3
scripts_aa3_ab5A:
        .addr   script_3_33_0
        .addr   script_3_33_1
        .addr   script_3_00_2
        .addr   script_3_5A_3
scripts_aa3_ab5B:
        .addr   script_3_35_0
        .addr   script_3_35_1
        .addr   script_3_00_2
        .addr   script_3_5B_3
scripts_aa3_ab5C:
        .addr   script_3_35_0
        .addr   script_3_35_1
        .addr   script_3_00_2
        .addr   script_3_5C_3

; bytecode scripts: 3548 bytes from your ROM
scripts_8FD8_data:
        .incbin "scripts_8FD8_data.bin"
script_3_00_0   := scripts_8FD8_data + $0000
script_3_00_1   := scripts_8FD8_data + $0003
script_3_00_3   := scripts_8FD8_data + $0012
script_3_04_3   := scripts_8FD8_data + $00CB
script_3_05_3   := scripts_8FD8_data + $00F7
script_3_08_3   := scripts_8FD8_data + $0148
script_3_0B_3   := scripts_8FD8_data + $0188
script_3_0F_3   := scripts_8FD8_data + $01F7
script_3_03_3   := scripts_8FD8_data + $0254
script_3_20_3   := scripts_8FD8_data + $027C
script_3_33_3   := scripts_8FD8_data + $02AB
script_3_5A_3   := scripts_8FD8_data + $02C8
script_3_35_3   := scripts_8FD8_data + $02E5
script_3_5B_3   := scripts_8FD8_data + $0302
script_3_5C_3   := scripts_8FD8_data + $031F
script_3_37_3   := scripts_8FD8_data + $033C
script_3_01_0   := scripts_8FD8_data + $036B
script_3_01_1   := scripts_8FD8_data + $036E
script_3_02_1   := scripts_8FD8_data + $037D
script_3_03_1   := scripts_8FD8_data + $038C
script_3_04_1   := scripts_8FD8_data + $039B
script_3_05_1   := scripts_8FD8_data + $03AA
script_3_06_1   := scripts_8FD8_data + $03B9
script_3_07_1   := scripts_8FD8_data + $03C8
script_3_54_1   := scripts_8FD8_data + $03D7
script_3_3D_1   := scripts_8FD8_data + $03E8
script_3_4A_1   := scripts_8FD8_data + $040C
script_3_49_1   := scripts_8FD8_data + $042E
script_3_09_1   := scripts_8FD8_data + $044A
script_3_0B_1   := scripts_8FD8_data + $044B
script_3_0C_1   := scripts_8FD8_data + $045A
script_3_0D_1   := scripts_8FD8_data + $0469
script_3_0E_1   := scripts_8FD8_data + $0478
script_3_0F_1   := scripts_8FD8_data + $0489
script_3_10_1   := scripts_8FD8_data + $049A
script_3_11_1   := scripts_8FD8_data + $04A9
script_3_12_1   := scripts_8FD8_data + $04B8
script_3_13_1   := scripts_8FD8_data + $04C7
script_3_14_1   := scripts_8FD8_data + $04D6
script_3_15_1   := scripts_8FD8_data + $04E5
script_3_16_1   := scripts_8FD8_data + $04F4
script_3_17_1   := scripts_8FD8_data + $0505
script_3_18_1   := scripts_8FD8_data + $0514
script_3_19_1   := scripts_8FD8_data + $0523
script_3_1A_1   := scripts_8FD8_data + $0532
script_3_1B_1   := scripts_8FD8_data + $0541
script_3_1C_1   := scripts_8FD8_data + $0550
script_3_1D_1   := scripts_8FD8_data + $055F
script_3_1E_1   := scripts_8FD8_data + $056E
script_3_1F_1   := scripts_8FD8_data + $057F
script_3_20_1   := scripts_8FD8_data + $058E
script_3_21_1   := scripts_8FD8_data + $059D
script_3_22_1   := scripts_8FD8_data + $05AC
script_3_23_1   := scripts_8FD8_data + $05BD
script_3_24_1   := scripts_8FD8_data + $05CC
script_3_25_1   := scripts_8FD8_data + $05DB
script_3_26_1   := scripts_8FD8_data + $05EA
script_3_27_1   := scripts_8FD8_data + $05F9
script_3_29_1   := scripts_8FD8_data + $0608
script_3_2C_1   := scripts_8FD8_data + $0617
script_3_2E_1   := scripts_8FD8_data + $0628
script_3_2F_1   := scripts_8FD8_data + $0637
script_3_30_1   := scripts_8FD8_data + $0646
script_3_31_1   := scripts_8FD8_data + $0655
script_3_32_1   := scripts_8FD8_data + $0664
script_3_34_1   := scripts_8FD8_data + $0673
script_3_36_1   := scripts_8FD8_data + $0682
script_3_37_1   := scripts_8FD8_data + $0691
script_3_38_1   := scripts_8FD8_data + $06A0
script_3_39_1   := scripts_8FD8_data + $06AF
script_3_3A_1   := scripts_8FD8_data + $06BE
script_3_00_2   := scripts_8FD8_data + $06CD
script_3_08_2   := scripts_8FD8_data + $06DF
script_3_2D_2   := scripts_8FD8_data + $06F2
script_3_1B_2   := scripts_8FD8_data + $0705
script_3_28_2   := scripts_8FD8_data + $071D
script_3_02_2   := scripts_8FD8_data + $0723
script_3_0E_2   := scripts_8FD8_data + $0734
script_3_10_2   := scripts_8FD8_data + $074C
script_3_11_2   := scripts_8FD8_data + $0762
script_3_12_2   := scripts_8FD8_data + $0778
script_3_2F_2   := scripts_8FD8_data + $0790
script_3_01_3   := scripts_8FD8_data + $07A6
script_unreferenced_97FA := scripts_8FD8_data + $0822
script_3_08_1   := scripts_8FD8_data + $0845
script_3_28_1   := scripts_8FD8_data + $0848
script_3_3D_3   := scripts_8FD8_data + $09AC
script_3_06_3   := scripts_8FD8_data + $09AF
script_3_0D_3   := scripts_8FD8_data + $09F9
script_3_15_3   := scripts_8FD8_data + $0A44
script_3_1F_3   := scripts_8FD8_data + $0AAE
script_3_23_3   := scripts_8FD8_data + $0ADD
script_3_26_3   := scripts_8FD8_data + $0B0E
script_3_27_3   := scripts_8FD8_data + $0B3D
script_3_29_3   := scripts_8FD8_data + $0B76
script_3_58_3   := scripts_8FD8_data + $0B93
script_3_59_3   := scripts_8FD8_data + $0BB0
script_3_2D_3   := scripts_8FD8_data + $0BD0
script_3_36_3   := scripts_8FD8_data + $0BEC
script_3_34_3   := scripts_8FD8_data + $0C08
script_3_02_0   := scripts_8FD8_data + $0C25
script_3_02_3   := scripts_8FD8_data + $0C2B
script_3_03_0   := scripts_8FD8_data + $0C5C
script_3_04_0   := scripts_8FD8_data + $0C5F
script_3_05_0   := scripts_8FD8_data + $0C62
script_3_06_0   := scripts_8FD8_data + $0C65
script_3_07_0   := scripts_8FD8_data + $0C70
script_3_54_3   := scripts_8FD8_data + $0C73
script_3_07_3   := scripts_8FD8_data + $0D25

; ----------------------------------------------------------------------------
        .byte   $FF

; bytecode scripts: 1537 bytes from your ROM
scripts_9DB5_data:
        .incbin "scripts_9DB5_data.bin"
script_3_08_0   := scripts_9DB5_data + $0021
script_3_09_0   := scripts_9DB5_data + $0024
script_3_0A_0   := scripts_9DB5_data + $0025
script_3_0A_1   := scripts_9DB5_data + $0094
script_3_0A_3   := scripts_9DB5_data + $00C1
script_3_0B_0   := scripts_9DB5_data + $039B
script_3_0C_0   := scripts_9DB5_data + $039E
script_3_0C_3   := scripts_9DB5_data + $03A1
script_3_0D_0   := scripts_9DB5_data + $03B6
script_3_0F_0   := scripts_9DB5_data + $03B9
script_3_0E_0   := scripts_9DB5_data + $03BC
script_3_10_0   := scripts_9DB5_data + $03D1
script_3_11_0   := scripts_9DB5_data + $03EB
script_3_12_0   := scripts_9DB5_data + $03F7
script_3_13_0   := scripts_9DB5_data + $0414
script_3_14_0   := scripts_9DB5_data + $0417
script_3_15_0   := scripts_9DB5_data + $041A
script_3_16_0   := scripts_9DB5_data + $041D
script_3_17_0   := scripts_9DB5_data + $0420
script_3_18_0   := scripts_9DB5_data + $0423
script_3_19_0   := scripts_9DB5_data + $0426
script_3_1A_0   := scripts_9DB5_data + $0429
script_3_1B_0   := scripts_9DB5_data + $043B
script_3_1C_0   := scripts_9DB5_data + $0442
script_3_1D_0   := scripts_9DB5_data + $0445
script_3_1E_0   := scripts_9DB5_data + $0448
script_3_1F_0   := scripts_9DB5_data + $044B
script_3_20_0   := scripts_9DB5_data + $044E
script_3_21_0   := scripts_9DB5_data + $0451
script_3_22_0   := scripts_9DB5_data + $0454
script_3_23_0   := scripts_9DB5_data + $0457
script_3_24_0   := scripts_9DB5_data + $045A
script_3_25_0   := scripts_9DB5_data + $0469
script_3_25_3   := scripts_9DB5_data + $046C
script_3_26_0   := scripts_9DB5_data + $0484
script_3_27_0   := scripts_9DB5_data + $0487
script_3_28_0   := scripts_9DB5_data + $0492
script_3_29_0   := scripts_9DB5_data + $049F
script_3_2A_0   := scripts_9DB5_data + $04AC
script_3_2B_0   := scripts_9DB5_data + $04B9
script_3_2B_1   := scripts_9DB5_data + $04C4
script_3_2C_0   := scripts_9DB5_data + $04E8
script_3_2D_0   := scripts_9DB5_data + $04EB
script_3_2E_0   := scripts_9DB5_data + $04EE
script_3_2F_0   := scripts_9DB5_data + $04F1
script_3_30_0   := scripts_9DB5_data + $050B
script_3_31_0   := scripts_9DB5_data + $050E
script_3_31_3   := scripts_9DB5_data + $0511
script_3_32_0   := scripts_9DB5_data + $053C
script_3_32_3   := scripts_9DB5_data + $053F
script_3_37_0   := scripts_9DB5_data + $0556
script_3_38_0   := scripts_9DB5_data + $0559
script_3_39_0   := scripts_9DB5_data + $055C
script_3_3A_0   := scripts_9DB5_data + $0561
script_3_55_0   := scripts_9DB5_data + $0564
script_3_55_1   := scripts_9DB5_data + $0569
script_3_55_3   := scripts_9DB5_data + $0578
script_3_56_3   := scripts_9DB5_data + $0595
script_3_57_3   := scripts_9DB5_data + $05B2
script_3_33_1   := scripts_9DB5_data + $05CF
script_3_33_0   := scripts_9DB5_data + $05DE
script_3_35_1   := scripts_9DB5_data + $05E3
script_3_34_0   := scripts_9DB5_data + $05F2
script_3_35_0   := scripts_9DB5_data + $05F7
script_3_36_0   := scripts_9DB5_data + $05FC

; ----------------------------------------------------------------------------
scripts_aa4:
        .addr   scripts_aa4_ab00
        .addr   scripts_aa4_ab01
        .addr   scripts_aa4_ab02
        .addr   scripts_aa4_ab03
        .addr   scripts_aa4_ab04
        .addr   scripts_aa4_ab05
        .addr   scripts_aa4_ab06
        .addr   scripts_aa4_ab07
        .addr   scripts_aa4_ab08
        .addr   scripts_aa4_ab09
        .addr   scripts_aa4_ab0A
        .addr   scripts_aa4_ab0B
        .addr   scripts_aa4_ab0C
        .addr   scripts_aa4_ab0D
        .addr   scripts_aa4_ab0E
        .addr   scripts_aa4_ab0F
        .addr   scripts_aa4_ab10
        .addr   scripts_aa4_ab11
scripts_aa4_ab00:
        .addr   script_4_00_0
        .addr   script_4_00_1
        .addr   script_4_00_2
        .addr   script_4_00_3
        .addr   script_4_00_4
        .addr   script_4_00_5
scripts_aa4_ab01:
        .addr   script_4_01_0
        .addr   script_4_01_1
        .addr   script_4_01_2
        .addr   script_4_01_3
        .addr   script_4_01_4
        .addr   script_4_00_5
scripts_aa4_ab02:
        .addr   script_4_02_0
        .addr   script_4_02_1
        .addr   script_4_02_2
        .addr   script_4_02_3
        .addr   script_4_02_4
        .addr   script_4_02_5
scripts_aa4_ab03:
        .addr   script_4_03_0
        .addr   script_4_03_1
        .addr   script_4_03_2
        .addr   script_4_03_3
        .addr   script_4_03_4
        .addr   script_4_00_5
scripts_aa4_ab04:
        .addr   script_4_04_0
        .addr   script_4_04_1
        .addr   script_4_04_2
        .addr   script_4_04_3
        .addr   script_4_04_4
        .addr   script_4_00_5
scripts_aa4_ab05:
        .addr   script_4_05_0
        .addr   script_4_05_1
        .addr   script_4_05_2
        .addr   script_4_05_3
        .addr   script_4_05_4
        .addr   script_4_00_5
scripts_aa4_ab06:
        .addr   script_4_06_0
        .addr   script_4_06_1
        .addr   script_4_06_2
        .addr   script_4_06_3
        .addr   script_4_06_4
        .addr   script_4_00_5
scripts_aa4_ab07:
        .addr   script_4_07_0
        .addr   script_4_07_1
        .addr   script_4_07_2
        .addr   script_4_07_3
        .addr   script_4_07_4
        .addr   script_4_07_5
scripts_aa4_ab08:
        .addr   script_4_08_0
        .addr   script_4_08_1
        .addr   script_4_08_2
        .addr   script_4_08_3
        .addr   script_4_08_4
        .addr   script_4_00_5
scripts_aa4_ab09:
        .addr   script_4_09_0
        .addr   script_4_09_1
        .addr   script_4_09_2
        .addr   script_4_09_3
        .addr   script_4_09_4
        .addr   script_4_00_5
scripts_aa4_ab0A:
        .addr   script_4_0A_0
        .addr   script_4_0A_1
        .addr   script_4_0A_2
        .addr   script_4_0A_3
        .addr   script_4_0A_4
        .addr   script_4_00_5
scripts_aa4_ab0B:
        .addr   script_4_0B_0
        .addr   script_4_0B_1
        .addr   script_4_0B_2
        .addr   script_4_0B_3
        .addr   script_4_07_4
        .addr   script_4_00_5
scripts_aa4_ab0C:
        .addr   script_4_0C_0
        .addr   script_4_0C_1
        .addr   script_4_0C_2
        .addr   script_4_0C_3
        .addr   script_4_07_4
        .addr   script_4_00_5
scripts_aa4_ab0D:
        .addr   script_4_0D_0
        .addr   script_4_0D_1
        .addr   script_4_0D_2
        .addr   script_4_0D_3
        .addr   script_4_0D_4
        .addr   script_4_00_5
scripts_aa4_ab0E:
        .addr   script_4_0E_0
        .addr   script_4_0E_1
        .addr   script_4_0E_2
        .addr   script_4_0E_3
        .addr   script_4_07_4
        .addr   script_4_00_5
scripts_aa4_ab0F:
        .addr   script_4_0F_0
        .addr   script_4_0F_1
        .addr   script_4_0F_2
        .addr   script_4_0F_3
        .addr   script_4_07_4
        .addr   script_4_00_5
scripts_aa4_ab10:
        .addr   script_4_10_0
        .addr   script_4_10_1
        .addr   script_4_10_2
        .addr   script_4_10_3
        .addr   script_4_07_4
        .addr   script_4_00_5
scripts_aa4_ab11:
        .addr   script_4_11_0
        .addr   script_4_11_1
        .addr   script_4_11_2
        .addr   script_4_11_3
        .addr   script_4_07_4
        .addr   script_4_11_5

; bytecode scripts: 2436 bytes from your ROM
scripts_A4B2_data:
        .incbin "scripts_A4B2_data.bin"
script_4_00_0   := scripts_A4B2_data + $0000
script_4_00_1   := scripts_A4B2_data + $000E
script_4_01_1   := scripts_A4B2_data + $0022
script_4_02_1   := scripts_A4B2_data + $0032
script_4_03_1   := scripts_A4B2_data + $0044
script_4_04_1   := scripts_A4B2_data + $0054
script_4_05_1   := scripts_A4B2_data + $0064
script_4_06_1   := scripts_A4B2_data + $0074
script_4_07_1   := scripts_A4B2_data + $0084
script_4_08_1   := scripts_A4B2_data + $0096
script_4_09_1   := scripts_A4B2_data + $00B0
script_4_0A_1   := scripts_A4B2_data + $00C2
script_4_0B_1   := scripts_A4B2_data + $00D2
script_4_0C_1   := scripts_A4B2_data + $00E2
script_4_0D_1   := scripts_A4B2_data + $00F2
script_4_0E_1   := scripts_A4B2_data + $0102
script_4_0F_1   := scripts_A4B2_data + $0112
script_4_10_1   := scripts_A4B2_data + $0122
script_4_11_1   := scripts_A4B2_data + $0132
script_4_00_2   := scripts_A4B2_data + $0144
script_4_01_2   := scripts_A4B2_data + $0154
script_4_02_2   := scripts_A4B2_data + $0164
script_4_03_2   := scripts_A4B2_data + $0174
script_4_04_2   := scripts_A4B2_data + $0184
script_4_05_2   := scripts_A4B2_data + $0194
script_4_06_2   := scripts_A4B2_data + $01A4
script_4_07_2   := scripts_A4B2_data + $01B4
script_4_08_2   := scripts_A4B2_data + $01C4
script_4_09_2   := scripts_A4B2_data + $01D4
script_4_0A_2   := scripts_A4B2_data + $01E4
script_4_0B_2   := scripts_A4B2_data + $01F4
script_4_0C_2   := scripts_A4B2_data + $0204
script_4_0D_2   := scripts_A4B2_data + $0214
script_4_0E_2   := scripts_A4B2_data + $0224
script_4_0F_2   := scripts_A4B2_data + $0234
script_4_10_2   := scripts_A4B2_data + $0244
script_4_11_2   := scripts_A4B2_data + $0254
script_4_00_3   := scripts_A4B2_data + $0264
script_4_00_4   := scripts_A4B2_data + $0285
script_4_01_4   := scripts_A4B2_data + $02BD
script_4_00_5   := scripts_A4B2_data + $02CF
script_4_01_0   := scripts_A4B2_data + $02D5
script_4_02_3   := scripts_A4B2_data + $02D8
script_4_01_3   := scripts_A4B2_data + $02E7
script_4_03_3   := scripts_A4B2_data + $02F6
script_4_04_3   := scripts_A4B2_data + $0305
script_4_05_3   := scripts_A4B2_data + $0314
script_4_06_3   := scripts_A4B2_data + $0323
script_4_07_3   := scripts_A4B2_data + $0332
script_4_08_3   := scripts_A4B2_data + $0341
script_4_09_3   := scripts_A4B2_data + $0350
script_4_0A_3   := scripts_A4B2_data + $035F
script_4_0B_3   := scripts_A4B2_data + $036E
script_4_0C_3   := scripts_A4B2_data + $037D
script_4_0D_3   := scripts_A4B2_data + $038C
script_4_0E_3   := scripts_A4B2_data + $039B
script_4_0F_3   := scripts_A4B2_data + $03AA
script_4_10_3   := scripts_A4B2_data + $03B9
script_4_11_3   := scripts_A4B2_data + $03C8
script_4_02_0   := scripts_A4B2_data + $041B
script_4_02_4   := scripts_A4B2_data + $041E
script_4_02_5   := scripts_A4B2_data + $0440
script_4_03_0   := scripts_A4B2_data + $0733
script_4_03_4   := scripts_A4B2_data + $0742
script_4_04_0   := scripts_A4B2_data + $0758
script_4_04_4   := scripts_A4B2_data + $075B
script_4_05_0   := scripts_A4B2_data + $0771
script_4_05_4   := scripts_A4B2_data + $0774
script_4_06_0   := scripts_A4B2_data + $078A
script_4_06_4   := scripts_A4B2_data + $078D
script_4_07_0   := scripts_A4B2_data + $07A3
script_4_07_5   := scripts_A4B2_data + $07A6
script_4_08_0   := scripts_A4B2_data + $0855
script_4_08_4   := scripts_A4B2_data + $086B
script_4_09_0   := scripts_A4B2_data + $087B
script_4_09_4   := scripts_A4B2_data + $087E
script_4_0A_0   := scripts_A4B2_data + $0894
script_4_0A_4   := scripts_A4B2_data + $0897
script_4_0B_0   := scripts_A4B2_data + $08AD
script_4_07_4   := scripts_A4B2_data + $08B0
script_4_0C_0   := scripts_A4B2_data + $08C1
script_4_0D_0   := scripts_A4B2_data + $08C4
script_4_0D_4   := scripts_A4B2_data + $08C7
script_4_0E_0   := scripts_A4B2_data + $08DD
script_4_0F_0   := scripts_A4B2_data + $08E0
script_4_10_0   := scripts_A4B2_data + $08E3
script_4_11_0   := scripts_A4B2_data + $08E6
script_4_11_5   := scripts_A4B2_data + $08E9

; ----------------------------------------------------------------------------
scripts_aa5:
        .addr   scripts_aa5_ab00
        .addr   scripts_aa5_ab01
        .addr   scripts_aa5_ab02
        .addr   scripts_aa5_ab03
        .addr   scripts_aa5_ab04
        .addr   scripts_aa5_ab05
        .addr   scripts_aa5_ab06
        .addr   scripts_aa5_ab07
        .addr   scripts_aa5_ab08
        .addr   scripts_aa5_ab09
        .addr   scripts_aa5_ab0A
        .addr   scripts_aa5_ab0B
        .addr   scripts_aa5_ab0C
        .addr   scripts_aa5_ab0D
        .addr   scripts_aa5_ab0E
        .addr   scripts_aa5_ab0F
        .addr   scripts_aa5_ab10
        .addr   scripts_aa5_ab11
        .addr   scripts_aa5_ab12
        .addr   scripts_aa5_ab13
        .addr   scripts_aa5_ab14
        .addr   scripts_aa5_ab15
        .addr   scripts_aa5_ab16
        .addr   scripts_aa5_ab17
        .addr   scripts_aa5_ab18
        .addr   scripts_aa5_ab19
        .addr   scripts_aa5_ab1A
        .addr   scripts_aa5_ab1B
        .addr   scripts_aa5_ab1C
        .addr   scripts_aa5_ab1D
        .addr   scripts_aa5_ab1E
        .addr   scripts_aa5_ab1F
        .addr   scripts_aa5_ab20
        .addr   scripts_aa5_ab21
        .addr   scripts_aa5_ab22
        .addr   scripts_aa5_ab23
        .addr   scripts_aa5_ab24
        .addr   scripts_aa5_ab25
        .addr   scripts_aa5_ab26
        .addr   scripts_aa5_ab27
        .addr   scripts_aa5_ab28
        .addr   scripts_aa5_ab29
        .addr   scripts_aa5_ab2A
        .addr   scripts_aa5_ab2B
        .addr   scripts_aa5_ab2C
        .addr   scripts_aa5_ab2D
        .addr   scripts_aa5_ab2E
        .addr   scripts_aa5_ab2F
        .addr   scripts_aa5_ab30
        .addr   scripts_aa5_ab31
        .addr   scripts_aa5_ab32
        .addr   scripts_aa5_ab33
        .addr   scripts_aa5_ab34
        .addr   scripts_aa5_ab35
        .addr   scripts_aa5_ab36
        .addr   scripts_aa5_ab37
        .addr   scripts_aa5_ab38
        .addr   scripts_aa5_ab39
        .addr   scripts_aa5_ab3A
        .addr   scripts_aa5_ab3B
        .addr   scripts_aa5_ab3C
        .addr   scripts_aa5_ab3D
        .addr   scripts_aa5_ab3E
        .addr   scripts_aa5_ab3F
        .addr   scripts_aa5_ab40
        .addr   scripts_aa5_ab41
        .addr   scripts_aa5_ab42
        .addr   scripts_aa5_ab43
        .addr   scripts_aa5_ab44
        .addr   scripts_aa5_ab45
        .addr   scripts_aa5_ab46
        .addr   scripts_aa5_ab47
        .addr   scripts_aa5_ab48
        .addr   scripts_aa5_ab49
        .addr   scripts_aa5_ab4A
        .addr   scripts_aa5_ab4B
        .addr   scripts_aa5_ab4C
        .addr   scripts_aa5_ab4D
scripts_aa5_ab00:
        .addr   script_5_00_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_00_3
scripts_aa5_ab01:
        .addr   script_5_01_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_01_3
scripts_aa5_ab02:
        .addr   script_5_02_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_02_3
scripts_aa5_ab03:
        .addr   script_5_03_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_03_3
scripts_aa5_ab04:
        .addr   script_5_04_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_04_3
scripts_aa5_ab05:
        .addr   script_5_05_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_05_3
scripts_aa5_ab06:
        .addr   script_5_06_0
        .addr   script_5_06_1
        .addr   script_5_00_2
        .addr   script_5_06_3
scripts_aa5_ab07:
        .addr   script_5_07_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_07_3
scripts_aa5_ab08:
        .addr   script_5_08_0
        .addr   script_5_08_1
        .addr   script_5_08_2
        .addr   script_5_08_3
scripts_aa5_ab09:
        .addr   script_5_09_0
        .addr   script_5_09_1
        .addr   script_5_09_2
        .addr   script_5_09_3
scripts_aa5_ab0A:
        .addr   script_5_0A_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_0A_3
scripts_aa5_ab0B:
        .addr   script_5_0B_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_0B_3
scripts_aa5_ab0C:
        .addr   script_5_0C_0
        .addr   script_5_0C_1
        .addr   script_5_0C_2
        .addr   script_5_0C_3
scripts_aa5_ab0D:
        .addr   script_5_0D_0
        .addr   script_5_0D_0
        .addr   script_5_0D_0
        .addr   script_5_0D_0
scripts_aa5_ab0E:
        .addr   script_5_0C_0
        .addr   script_5_0C_1
        .addr   script_5_0C_2
        .addr   script_5_0D_0
scripts_aa5_ab0F:
        .addr   script_5_0C_0
        .addr   script_5_0C_1
        .addr   script_5_0C_2
        .addr   script_5_0F_3
scripts_aa5_ab10:
        .addr   script_5_10_0
        .addr   script_5_10_1
        .addr   script_5_10_2
        .addr   script_5_10_3
scripts_aa5_ab11:
        .addr   script_5_11_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_11_3
scripts_aa5_ab12:
        .addr   script_5_12_0
        .addr   script_5_10_1
        .addr   script_5_10_2
        .addr   script_5_12_3
scripts_aa5_ab13:
        .addr   script_5_13_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_13_3
scripts_aa5_ab14:
        .addr   script_5_14_0
        .addr   script_5_14_1
        .addr   script_5_00_2
        .addr   script_5_14_3
scripts_aa5_ab15:
        .addr   script_5_15_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_15_3
scripts_aa5_ab16:
        .addr   script_5_16_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_16_3
scripts_aa5_ab17:
        .addr   script_5_17_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_17_3
scripts_aa5_ab18:
        .addr   script_5_18_0
        .addr   script_5_18_1
        .addr   script_5_18_2
        .addr   script_5_18_3
scripts_aa5_ab19:
        .addr   script_5_19_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_19_3
scripts_aa5_ab1A:
        .addr   script_5_1A_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_1A_3
scripts_aa5_ab1B:
        .addr   script_5_1B_0
        .addr   script_5_1B_1
        .addr   script_5_00_2
        .addr   script_5_1B_3
scripts_aa5_ab1C:
        .addr   script_5_1C_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_1C_3
scripts_aa5_ab1D:
        .addr   script_5_1D_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_1D_3
scripts_aa5_ab1E:
        .addr   script_5_1E_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_1E_3
scripts_aa5_ab1F:
        .addr   script_5_1F_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_1F_3
scripts_aa5_ab20:
        .addr   script_5_20_0
        .addr   script_5_20_1
        .addr   script_5_00_2
        .addr   script_5_20_1
scripts_aa5_ab21:
        .addr   script_5_21_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_21_3
scripts_aa5_ab22:
        .addr   script_5_22_0
        .addr   script_5_18_1
        .addr   script_5_18_2
        .addr   script_5_22_3
scripts_aa5_ab23:
        .addr   script_5_23_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_23_3
scripts_aa5_ab24:
        .addr   script_5_24_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_24_3
scripts_aa5_ab25:
        .addr   script_5_25_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_25_3
scripts_aa5_ab26:
        .addr   script_5_26_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_26_3
scripts_aa5_ab27:
        .addr   script_5_27_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_27_3
scripts_aa5_ab28:
        .addr   script_5_28_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_28_3
scripts_aa5_ab29:
        .addr   script_5_29_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_29_3
scripts_aa5_ab2A:
        .addr   script_5_2A_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_2A_3
scripts_aa5_ab2B:
        .addr   script_5_2B_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_2B_3
scripts_aa5_ab2C:
        .addr   script_5_2C_0
        .addr   script_5_23_1
        .addr   script_5_0C_2
        .addr   script_5_2C_3
scripts_aa5_ab2D:
        .addr   script_5_2D_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_2D_3
scripts_aa5_ab2E:
        .addr   script_5_2E_0
        .addr   script_5_2E_0
        .addr   script_5_2E_0
        .addr   script_5_2E_0
scripts_aa5_ab2F:
        .addr   script_5_2E_0
        .addr   script_5_0C_1
        .addr   script_5_0C_2
        .addr   script_5_2F_3
scripts_aa5_ab30:
        .addr   script_5_30_0
        .addr   script_5_0C_1
        .addr   script_5_0C_2
        .addr   script_5_30_3
scripts_aa5_ab31:
        .addr   script_5_31_0
        .addr   script_5_31_1
        .addr   script_5_00_2
        .addr   script_5_31_3
scripts_aa5_ab32:
        .addr   script_5_32_0
        .addr   script_5_32_0
        .addr   script_5_32_0
        .addr   script_5_32_0
scripts_aa5_ab33:
        .addr   script_5_32_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_33_3
scripts_aa5_ab34:
        .addr   script_5_34_0
        .addr   script_5_34_0
        .addr   script_5_34_0
        .addr   script_5_34_0
scripts_aa5_ab35:
        .addr   script_5_35_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_35_3
scripts_aa5_ab36:
        .addr   script_5_36_0
        .addr   script_5_36_1
        .addr   script_5_00_2
        .addr   script_5_36_3
scripts_aa5_ab37:
        .addr   script_5_37_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_37_3
scripts_aa5_ab38:
        .addr   script_5_38_0
        .addr   script_5_38_1
        .addr   script_5_00_2
        .addr   script_5_38_3
scripts_aa5_ab39:
        .addr   script_5_39_0
        .addr   script_5_39_1
        .addr   script_5_0C_2
        .addr   script_5_39_3
scripts_aa5_ab3A:
        .addr   script_5_3A_0
        .addr   script_5_3A_0
        .addr   script_5_3A_0
        .addr   script_5_3A_0
scripts_aa5_ab3B:
        .addr   script_5_3A_0
        .addr   script_5_3A_0
        .addr   script_5_3A_0
        .addr   script_5_3A_0
scripts_aa5_ab3C:
        .addr   script_5_3C_0
        .addr   script_5_3C_1
        .addr   script_5_00_2
        .addr   script_5_3C_3
scripts_aa5_ab3D:
        .addr   script_5_3D_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_3D_3
scripts_aa5_ab3E:
        .addr   script_5_0C_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_3E_3
scripts_aa5_ab3F:
        .addr   script_5_0C_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_3F_3
scripts_aa5_ab40:
        .addr   script_5_0C_0
        .addr   script_5_00_1
        .addr   script_5_00_2
        .addr   script_5_40_3
scripts_aa5_ab41:
        .addr   script_5_41_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_41_3
scripts_aa5_ab42:
        .addr   script_5_42_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_42_3
scripts_aa5_ab43:
        .addr   script_5_43_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_43_3
scripts_aa5_ab44:
        .addr   script_5_44_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_44_3
scripts_aa5_ab45:
        .addr   script_5_45_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_45_3
scripts_aa5_ab46:
        .addr   script_5_46_0
        .addr   script_5_07_1
        .addr   script_5_07_1
        .addr   script_5_46_3
scripts_aa5_ab47:
        .addr   script_5_47_0
        .addr   script_5_47_1
        .addr   script_5_00_2
        .addr   script_5_07_1
scripts_aa5_ab48:
        .addr   script_5_48_0
        .addr   script_5_48_0
        .addr   script_5_48_0
        .addr   script_5_48_0
scripts_aa5_ab49:
        .addr   script_5_49_0
        .addr   script_5_49_1
        .addr   script_5_49_2
        .addr   script_5_49_3
scripts_aa5_ab4A:
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
scripts_aa5_ab4B:
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
scripts_aa5_ab4C:
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
scripts_aa5_ab4D:
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0
        .addr   script_5_4A_0

; bytecode scripts: 2638 bytes from your ROM
scripts_B142_data:
        .incbin "scripts_B142_data.bin"
script_5_00_0   := scripts_B142_data + $0000
script_5_00_1   := scripts_B142_data + $0016
script_5_23_1   := scripts_B142_data + $0020
script_5_3C_1   := scripts_B142_data + $0025
script_5_06_1   := scripts_B142_data + $003E
script_5_14_1   := scripts_B142_data + $004A
script_5_1B_1   := scripts_B142_data + $0056
script_5_38_1   := scripts_B142_data + $0062
script_5_36_1   := scripts_B142_data + $006E
script_5_31_1   := scripts_B142_data + $007A
script_5_09_1   := scripts_B142_data + $0086
script_5_00_2   := scripts_B142_data + $0092
script_5_09_2   := scripts_B142_data + $00A1
script_5_00_3   := scripts_B142_data + $00B1
script_5_01_0   := scripts_B142_data + $00CA
script_5_07_1   := scripts_B142_data + $00DF
script_5_01_3   := scripts_B142_data + $00E2
script_5_02_0   := scripts_B142_data + $00FE
script_5_02_3   := scripts_B142_data + $010A
script_5_03_0   := scripts_B142_data + $011E
script_5_03_3   := scripts_B142_data + $0133
script_5_04_0   := scripts_B142_data + $0147
script_5_04_3   := scripts_B142_data + $015C
script_5_05_0   := scripts_B142_data + $0170
script_5_05_3   := scripts_B142_data + $017D
script_5_06_0   := scripts_B142_data + $0198
script_5_06_3   := scripts_B142_data + $01A7
script_5_07_0   := scripts_B142_data + $01BD
script_5_07_3   := scripts_B142_data + $01CA
script_5_08_0   := scripts_B142_data + $01EF
script_5_08_1   := scripts_B142_data + $0203
script_5_08_2   := scripts_B142_data + $020B
script_5_08_3   := scripts_B142_data + $0220
script_5_09_0   := scripts_B142_data + $0234
script_5_09_3   := scripts_B142_data + $024D
script_5_0A_0   := scripts_B142_data + $0261
script_5_0A_3   := scripts_B142_data + $026C
script_5_0B_0   := scripts_B142_data + $027F
script_5_0B_3   := scripts_B142_data + $0280
script_5_0C_0   := scripts_B142_data + $028A
script_5_0C_1   := scripts_B142_data + $0293
script_5_39_1   := scripts_B142_data + $0297
script_5_0C_2   := scripts_B142_data + $029A
script_5_0C_3   := scripts_B142_data + $029D
script_5_0D_0   := scripts_B142_data + $02B1
script_5_0F_3   := scripts_B142_data + $02EF
script_5_10_0   := scripts_B142_data + $030D
script_5_10_1   := scripts_B142_data + $0320
script_5_10_2   := scripts_B142_data + $032F
script_5_10_3   := scripts_B142_data + $033E
script_5_11_0   := scripts_B142_data + $0347
script_5_11_3   := scripts_B142_data + $0348
script_5_12_0   := scripts_B142_data + $0352
script_5_12_3   := scripts_B142_data + $035E
script_5_13_0   := scripts_B142_data + $0368
script_5_13_3   := scripts_B142_data + $0370
script_5_14_0   := scripts_B142_data + $037F
script_5_14_3   := scripts_B142_data + $038B
script_5_15_0   := scripts_B142_data + $039F
script_5_15_3   := scripts_B142_data + $03A7
script_5_16_3   := scripts_B142_data + $03AF
script_5_17_3   := scripts_B142_data + $03B7
script_5_17_0   := scripts_B142_data + $03CC
script_5_16_0   := scripts_B142_data + $03CD
script_5_18_0   := scripts_B142_data + $03D5
script_5_18_1   := scripts_B142_data + $03E4
script_5_18_2   := scripts_B142_data + $03F3
script_5_18_3   := scripts_B142_data + $0402
script_5_19_0   := scripts_B142_data + $0412
script_5_19_3   := scripts_B142_data + $0413
script_5_1A_0   := scripts_B142_data + $0421
script_5_1A_3   := scripts_B142_data + $0422
script_5_1B_0   := scripts_B142_data + $0436
script_5_1B_3   := scripts_B142_data + $0442
script_5_1C_0   := scripts_B142_data + $0455
script_5_1C_3   := scripts_B142_data + $0456
script_5_1D_0   := scripts_B142_data + $0474
script_5_1D_3   := scripts_B142_data + $0475
script_5_1E_0   := scripts_B142_data + $0491
script_5_1E_3   := scripts_B142_data + $049D
script_5_1F_0   := scripts_B142_data + $04B0
script_5_1F_3   := scripts_B142_data + $04B1
script_5_20_0   := scripts_B142_data + $04C2
script_5_21_0   := scripts_B142_data + $04C8
script_5_20_1   := scripts_B142_data + $04C9
script_unreferenced_B63D := scripts_B142_data + $04FB
script_5_21_3   := scripts_B142_data + $0561
script_5_22_0   := scripts_B142_data + $056B
script_5_22_3   := scripts_B142_data + $0577
script_5_23_0   := scripts_B142_data + $0581
script_5_23_3   := scripts_B142_data + $058C
script_5_24_0   := scripts_B142_data + $05C0
script_5_24_3   := scripts_B142_data + $05C6
script_5_25_0   := scripts_B142_data + $05F3
script_5_25_3   := scripts_B142_data + $05F9
script_5_26_0   := scripts_B142_data + $062F
script_5_26_3   := scripts_B142_data + $0635
script_5_27_0   := scripts_B142_data + $066B
script_5_27_3   := scripts_B142_data + $0671
script_5_28_0   := scripts_B142_data + $06A7
script_5_28_3   := scripts_B142_data + $06B2
script_5_29_0   := scripts_B142_data + $06EA
script_5_29_3   := scripts_B142_data + $06F0
script_5_2A_0   := scripts_B142_data + $071D
script_5_2A_3   := scripts_B142_data + $0723
script_5_2B_0   := scripts_B142_data + $0741
script_5_2B_3   := scripts_B142_data + $0747
script_5_2C_0   := scripts_B142_data + $0765
script_5_2C_3   := scripts_B142_data + $076B
script_5_2D_0   := scripts_B142_data + $0789
script_5_2D_3   := scripts_B142_data + $0797
script_5_2E_0   := scripts_B142_data + $07AB
script_5_2F_3   := scripts_B142_data + $07BA
script_5_30_0   := scripts_B142_data + $07C4
script_5_30_3   := scripts_B142_data + $07D0
script_5_31_0   := scripts_B142_data + $07E9
script_5_31_3   := scripts_B142_data + $0809
script_5_32_0   := scripts_B142_data + $081D
script_5_33_3   := scripts_B142_data + $0829
script_5_34_0   := scripts_B142_data + $0833
script_5_35_0   := scripts_B142_data + $0834
script_5_35_3   := scripts_B142_data + $083C
script_5_36_0   := scripts_B142_data + $0879
script_5_36_3   := scripts_B142_data + $088D
script_5_37_0   := scripts_B142_data + $08A6
script_5_37_3   := scripts_B142_data + $08AE
script_5_38_0   := scripts_B142_data + $08B6
script_5_38_3   := scripts_B142_data + $08CA
script_5_39_0   := scripts_B142_data + $0908
script_5_39_3   := scripts_B142_data + $0910
script_5_3A_0   := scripts_B142_data + $0924
script_5_3C_0   := scripts_B142_data + $0925
script_5_3C_3   := scripts_B142_data + $092B
script_5_3D_0   := scripts_B142_data + $093F
script_5_3D_3   := scripts_B142_data + $0945
script_5_3E_3   := scripts_B142_data + $0954
script_5_3F_3   := scripts_B142_data + $0968
script_5_40_3   := scripts_B142_data + $097C
script_5_41_0   := scripts_B142_data + $0984
script_5_41_3   := scripts_B142_data + $098C
script_5_42_0   := scripts_B142_data + $09A0
script_5_42_3   := scripts_B142_data + $09A8
script_5_43_0   := scripts_B142_data + $09B9
script_5_43_3   := scripts_B142_data + $09C4
script_5_44_0   := scripts_B142_data + $09CC
script_5_44_3   := scripts_B142_data + $09D4
script_5_45_0   := scripts_B142_data + $09DC
script_5_45_3   := scripts_B142_data + $09E4
script_5_46_0   := scripts_B142_data + $09F6
script_5_46_3   := scripts_B142_data + $09F7
script_5_47_0   := scripts_B142_data + $0A08
script_5_47_1   := scripts_B142_data + $0A0B
script_5_48_0   := scripts_B142_data + $0A2B
script_5_49_0   := scripts_B142_data + $0A2C
script_5_49_1   := scripts_B142_data + $0A2F
script_5_49_2   := scripts_B142_data + $0A41

; ----------------------------------------------------------------------------
        .byte   $FF
script_5_49_3:
        .byte   $16,$27,$04,$A0,$BB,$19,$31,$04
        .byte   $9D,$BB,$06,$31,$01,$A0,$FF,$06
        .byte   $1C,$01,$9F,$FF
; also 5.4A.1, 5.4A.2, 5.4A.3...
script_5_4A_0:
        .byte   $FF
; 1114 unused bytes of $FF
free_space_3:
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
        .byte   $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
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
