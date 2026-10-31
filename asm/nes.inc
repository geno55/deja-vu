; NES and MMC3 hardware registers, included at the top of every asm/bank*.s.
;
; Format as in ram.inc. "[write]" or "[read]" limits a name to stores or loads:
; the MMC3 registers share their addresses with ROM, so only stores to $8000-$FFFF
; are register writes, and $4017 is the second controller when read but the APU
; frame counter when written. tools/name_ram.py applies these names too.

; ---- PPU --------------------------------------------------------------------
PPUCTRL           = $2000
PPUMASK           = $2001
PPUSTATUS         = $2002
OAMADDR           = $2003
OAMDATA           = $2004
PPUSCROLL         = $2005
PPUADDR           = $2006
PPUDATA           = $2007

; ---- APU and I/O ------------------------------------------------------------
SQ1_VOL           = $4000
SQ1_SWEEP         = $4001
SQ1_LO            = $4002
SQ1_HI            = $4003
SQ2_VOL           = $4004
SQ2_SWEEP         = $4005
SQ2_LO            = $4006
SQ2_HI            = $4007
TRI_LINEAR        = $4008
TRI_LO            = $400A
TRI_HI            = $400B
NOISE_VOL         = $400C
NOISE_LO          = $400E
NOISE_HI          = $400F
DMC_FREQ          = $4010
DMC_RAW           = $4011
DMC_START         = $4012
DMC_LEN           = $4013
OAMDMA            = $4014
SND_CHN           = $4015
JOY1              = $4016
JOY2              = $4017 ; [read]
APU_FRAME         = $4017 ; [write]

; ---- MMC3 -------------------------------------------------------------------
MMC3_BANK_SELECT  = $8000 ; [write]
MMC3_BANK_DATA    = $8001 ; [write]
MMC3_MIRRORING    = $A000 ; [write]
MMC3_RAM_PROTECT  = $A001 ; [write]
MMC3_IRQ_LATCH    = $C000 ; [write]
MMC3_IRQ_RELOAD   = $C001 ; [write]
MMC3_IRQ_DISABLE  = $E000 ; [write]
MMC3_IRQ_ENABLE   = $E001 ; [write]
