; iNES (NES 2.0) header for Deja Vu (USA): mapper 4 (MMC3), 128 KB PRG, 256 KB CHR,
; 8 KB battery-backed PRG RAM, horizontal mirroring.
        .segment "HEADER"
        .byte   "NES", $1A
        .byte   $08             ; PRG ROM: 8 x 16 KB
        .byte   $20             ; CHR ROM: 32 x 8 KB
        .byte   $42             ; mapper 4 (low nibble), battery
        .byte   $08             ; NES 2.0
        .byte   $00, $00
        .byte   $70             ; 8 KB battery-backed PRG RAM
        .byte   $00, $00, $00, $00
        .byte   $01             ; default expansion device: standard controllers
