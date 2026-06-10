;
; @file     hardware.inc
; @brief    Contains hardware constant definitions for the G10.Boy.
;
; @details  Adapted from the Game Boy `hardware.inc`, found here:
;           `https://github.com/gbdev/hardware.inc`
;

.once

; Memory Map *******************************************************************

.const mPRGROM  = $00000000
.const mVRAM    = $80008000
.const mOAM     = $8000FE00
.const mWAVRAM  = $8000FF30
.const mWRAM    = $A0000000
.const mSRAM    = $B0000000
.const mXPRAM   = $C0000000
.const mQRAM    = $FFFF0000
.const mIO      = $FFFFFF00

; Port Registers - G10 CPU Interrupt Registers *********************************

.const rIE0     = $00
.const rIE1     = $01
.const rIE2     = $02
.const rIE3     = $03
.const rIRQ0    = $04
.const rIRQ1    = $05
.const rIRQ2    = $06
.const rIRQ3    = $07

.const rIF      = rIRQ0
.const rIE      = rIE0

.const bINT_EXCEPTION       = 0
.const bINT_VBLANK          = 1
.const bINT_STAT            = 2
.const bINT_TIMER           = 3
.const bINT_SERIAL          = 4
.const bINT_JOYPAD          = 5

.const INT_EXCEPTION        = (1 << bINT_EXCEPTION)
.const INT_VBLANK           = (1 << bINT_VBLANK)
.const INT_STAT             = (1 << bINT_STAT)
.const INT_TIMER            = (1 << bINT_TIMER)
.const INT_SERIAL           = (1 << bINT_SERIAL)
.const INT_JOYPAD           = (1 << bINT_JOYPAD)

; Port Registers - G10 CPU Speed Control ***************************************

.const rSPD                 = $08
.const bSPD_DOUBLE          = 7
.const bSPD_SWITCHING       = 1
.const bSPD_PREPARE         = 0

.const SPD_SINGLE           = (0 << bSPD_DOUBLE)
.const SPD_DOUBLE           = (1 << bSPD_DOUBLE)
.const SPD_NOT_SWITCHING    = (0 << bSPD_SWITCHING)
.const SPD_SWITCHING        = (1 << bSPD_SWITCHING)
.const SPD_PREPARE          = (1 << bSPD_PREPARE)
.const SPD_SWITCH_ARMED     = (1 << bSPD_PREPARE)
.const SPD_SWITCH_UNARMED   = (0 << bSPD_PREPARE)

; Port Registers - `JOYP` / `P1` ***********************************************

.const rJOYP    = $80

.const bJOYP_GET_BUTTONS    = 5
.const bJOYP_GET_DPAD       = 4
.const bJOYP_START          = 3
.const bJOYP_SELECT         = 2
.const bJOYP_B              = 1
.const bJOYP_A              = 0
.const bJOYP_DOWN           = 3
.const bJOYP_UP             = 2
.const bJOYP_LEFT           = 1
.const bJOYP_RIGHT          = 0

.const JOYP_GET             = %00110000
.const JOYP_GET_BUTTONS     = %00010000
.const JOYP_GET_DPAD        = %00100000
.const JOYP_GET_NONE        = %00110000
.const JOYP_INPUTS          = %00001111
.const JOYP_START           = (1 << bJOYP_START)
.const JOYP_SELECT          = (1 << bJOYP_SELECT)
.const JOYP_B               = (1 << bJOYP_B)
.const JOYP_A               = (1 << bJOYP_A)
.const JOYP_DOWN            = (1 << bJOYP_DOWN)
.const JOYP_UP              = (1 << bJOYP_UP)
.const JOYP_LEFT            = (1 << bJOYP_LEFT)
.const JOYP_RIGHT           = (1 << bJOYP_RIGHT)

; Port Registers - `SB` and `SC` ***********************************************

.const rSB  = $20
.const rSC  = $21

.const bSC_START            = 7
.const bSC_SPEED            = 1
.const bSC_SOURCE           = 0

.const SC_START             = (1 << bSC_START)
.const SC_SPEED             = (1 << bSC_SPEED)
.const SC_SPEED_FAST        = (1 << bSC_SPEED)
.const SC_SPEED_SLOW        = (0 << bSC_SPEED)
.const SC_SOURCE            = (1 << bSC_SOURCE)
.const SC_SOURCE_INTERNAL   = (1 << bSC_SOURCE)
.const SC_SOURCE_EXTERNAL   = (0 << bSC_SOURCE)

; Port Registers - Infrared Communiction ***************************************

.const rRP      = $24

.const bRP_DATA_IN          = 1
.const bRP_LED_ON           = 0

.const RP_READ              = %11000000
.const RP_READ_ENABLE       = %11000000
.const RP_READ_DISABLE      = %00000000
.const RP_DATA_IN           = (1 << bRP_DATA_IN)
.const RP_LED_ON            = (1 << bRP_LED_ON)
.const RP_LED_OFF           = (0 << bRP_LED_ON)
.const RP_WRITE_HIGH        = (1 << bRP_LED_ON)
.const RP_WRITE_LOW         = (0 << bRP_LED_ON)

; Port Registers - Timer *******************************************************

.const rDIV     = $10
.const rTIMA    = $11
.const rTMA     = $12
.const rTAC     = $13

.const bTAC_START           = 2

.const TAC_START            = (1 << bTAC_START)
.const TAC_STOP             = (0 << bTAC_START)
.const TAC_CLOCK            = %00000011
.const TAC_CLOCK_4KHZ       = %00000000
.const TAC_CLOCK_262KHZ     = %00000001
.const TAC_CLOCK_65KHZ      = %00000010
.const TAC_CLOCK_16KHZ      = %00000011

; Port Registers - APU Pulse Channel 1 *****************************************

.const rNR10                = $30
.const rNR11                = $31
.const rNR12                = $32
.const rNR13                = $33
.const rNR14                = $34

; Frequency Sweep

.const rAUD1SWEEP           = rNR10
.const bAUD1SWEEP_DIR       = 3

.const AUD1SWEEP_TIME       = %01110000
.const AUD1SWEEP_DIR        = (1 << bAUD1SWEEP_DIR)
.const AUD1SWEEP_DIR_UP     = (0 << bAUD1SWEEP_DIR)
.const AUD1SWEEP_DIR_DOWN   = (1 << bAUD1SWEEP_DIR)
.const AUD1SWEEP_SHIFT      = %00000111

; Length Timer & Duty Cycle

.const rAUD1LEN             = rNR11

.const AUD1LEN_DUTY         = %11000000
.const AUD1LEN_DUTY_12_5    = %00000000
.const AUD1LEN_DUTY_25      = %01000000
.const AUD1LEN_DUTY_50      = %10000000
.const AUD1LEN_DUTY_75      = %11000000
.const AUD1LEN_TIMER        = %00111111

; Volume & Envelope

.const rAUD1ENV             = rNR12
.const bAUD1ENV_DIR         = 3

.const AUD1ENV_INIT_VOLUME  = %11110000
.const AUD1ENV_DIR          = (1 << bAUD1ENV_DIR)
.const AUD1ENV_DIR_DOWN     = (0 << bAUD1ENV_DIR)
.const AUD1ENV_DIR_UP       = (1 << bAUD1ENV_DIR)
.const AUD1ENV_PACE         = %00000111

; Period & Control

.const rAUD1LOW             = rNR13
.const rAUD1HIGH            = rNR14
.const bAUD1HIGH_RESTART    = 7
.const bAUD1HIGH_LEN_ENABLE = 6

.const AUD1HIGH_RESTART     = (1 << bAUD1HIGH_RESTART)
.const AUD1HIGH_LENGTH_OFF  = (0 << bAUD1HIGH_LEN_ENABLE)
.const AUD1HIGH_LENGTH_ON   = (1 << bAUD1HIGH_LEN_ENABLE)
.const AUD1HIGH_PERIOD_HIGH = %00000111

; Port Registers - APU Pulse Channel 2 *****************************************

.const rNR21                = $39
.const rNR22                = $3A
.const rNR23                = $3B
.const rNR24                = $3C

; Length Timer & Duty Cycle

.const rAUD2LEN             = rNR21

.const AUD2LEN_DUTY         = %11000000
.const AUD2LEN_DUTY_12_5    = %00000000
.const AUD2LEN_DUTY_25      = %01000000
.const AUD2LEN_DUTY_50      = %10000000
.const AUD2LEN_DUTY_75      = %11000000
.const AUD2LEN_TIMER        = %00111111

; Volume & Envelope

.const rAUD2ENV             = rNR22
.const bAUD2ENV_DIR         = 3

.const AUD2ENV_INIT_VOLUME  = %11110000
.const AUD2ENV_DIR          = (1 << bAUD2ENV_DIR)
.const AUD2ENV_DIR_DOWN     = (0 << bAUD2ENV_DIR)
.const AUD2ENV_DIR_UP       = (1 << bAUD2ENV_DIR)
.const AUD2ENV_PACE         = %00000111

; Period & Control

.const rAUD2LOW             = rNR23
.const rAUD2HIGH            = rNR24
.const bAUD2HIGH_RESTART    = 7
.const bAUD2HIGH_LEN_ENABLE = 6

.const AUD2HIGH_RESTART     = (1 << bAUD2HIGH_RESTART)
.const AUD2HIGH_LENGTH_OFF  = (0 << bAUD2HIGH_LEN_ENABLE)
.const AUD2HIGH_LENGTH_ON   = (1 << bAUD2HIGH_LEN_ENABLE)
.const AUD2HIGH_PERIOD_HIGH = %00000111

; Port Registers - APU Wave Channel ********************************************

.const rNR30                = $40
.const rNR31                = $41
.const rNR32                = $42
.const rNR33                = $43
.const rNR34                = $44

; DAC Enable

.const rAUD3ENA             = rNR30
.const bAUD3ENA_ENABLE      = 7

.const AUD3ENA_OFF          = (0 << bAUD3ENA_ENABLE)
.const AUD3ENA_ON           = (1 << bAUD3ENA_ENABLE)

; Length Timer

.const rAUD3LEN             = rNR31

; Output Level

.const rAUD3LEVEL           = rNR32

.const AUD3LEVEL_VOLUME         = %01100000
.const AUD3LEVEL_VOLUME_MUTE    = %00000000
.const AUD3LEVEL_VOLUME_FULL    = %00100000
.const AUD3LEVEL_VOLUME_HALF    = %01000000
.const AUD3LEVEL_VOLUME_QUARTER = %01100000

; Period & Control

.const rAUD3LOW             = rNR33
.const rAUD3HIGH            = rNR34
.const bAUD3HIGH_RESTART    = 7
.const bAUD3HIGH_LEN_ENABLE = 6

.const AUD3HIGH_RESTART     = (1 << bAUD3HIGH_RESTART)
.const AUD3HIGH_LENGTH_OFF  = (0 << bAUD3HIGH_LEN_ENABLE)
.const AUD3HIGH_LENGTH_ON   = (1 << bAUD3HIGH_LEN_ENABLE)
.const AUD3HIGH_PERIOD_HIGH = %00000111

; Port Registers - APU Noise Channel *******************************************

.const rNR41                = $49
.const rNR42                = $4A
.const rNR43                = $4B
.const rNR44                = $4C

; Length Timer

.const rAUD4LEN             = rNR41
.const AUD4LEN_TIMER        = %00111111

; Volume & Envelope

.const rAUD4ENV             = rNR42
.const bAUD4ENV_DIR         = 3

.const AUD4ENV_INIT_VOLUME  = %11110000
.const AUD4ENV_DIR          = (1 << bAUD4ENV_DIR)
.const AUD4ENV_DIR_DOWN     = (0 << bAUD4ENV_DIR)
.const AUD4ENV_DIR_UP       = (1 << bAUD4ENV_DIR)
.const AUD4ENV_PACE         = %00000111

; Period & Randomness (Polynomial)

.const rAUD4POLY            = rNR43
.const bAUD4POLY_WIDTH      = 3

.const AUD4POLY_SHIFT       = %11110000
.const AUD4POLY_WIDTH       = (1 << bAUD4POLY_WIDTH)
.const AUD4POLY_WIDTH_15    = (0 << bAUD4POLY_WIDTH)
.const AUD4POLY_WIDTH_7     = (1 << bAUD4POLY_WIDTH)
.const AUD4POLY_DIV         = %00000111

; Control

.const rAUD4GO              = rNR44
.const bAUD4GO_RESTART      = 7
.const bAUD4GO_LEN_ENABLE   = 6
.const AUD4GO_RESTART       = (1 << bAUD4GO_RESTART)
.const AUD4GO_LENGTH_OFF    = (0 << bAUD4GO_LEN_ENABLE)
.const AUD4GO_LENGTH_ON     = (1 << bAUD4GO_LEN_ENABLE)

; Port Registers - APU Master Control ******************************************

.const rNR50                = $50
.const rNR51                = $51
.const rNR52                = $52

; Master Volume & VIN Panning

.const rAUDVOL              = rNR50
.const bAUDVOL_VIN_LEFT     = 7
.const bAUDVOL_VIN_RIGHT    = 3

.const AUDVOL_LEFT          = %01110000
.const AUDVOL_RIGHT         = %00000111

; Sound Panning

.const rAUDTERM             = rNR51
.const bAUDTERM_CH4_LEFT    = 7
.const bAUDTERM_CH3_LEFT    = 6
.const bAUDTERM_CH2_LEFT    = 5
.const bAUDTERM_CH1_LEFT    = 4
.const bAUDTERM_CH4_RIGHT   = 3
.const bAUDTERM_CH3_RIGHT   = 2
.const bAUDTERM_CH2_RIGHT   = 1
.const bAUDTERM_CH1_RIGHT   = 0

.const AUDTERM_CH4_LEFT     = (1 << bAUDTERM_CH4_LEFT)
.const AUDTERM_CH3_LEFT     = (1 << bAUDTERM_CH3_LEFT)
.const AUDTERM_CH2_LEFT     = (1 << bAUDTERM_CH2_LEFT)
.const AUDTERM_CH1_LEFT     = (1 << bAUDTERM_CH1_LEFT)
.const AUDTERM_CH4_RIGHT    = (1 << bAUDTERM_CH4_RIGHT)
.const AUDTERM_CH3_RIGHT    = (1 << bAUDTERM_CH3_RIGHT)
.const AUDTERM_CH2_RIGHT    = (1 << bAUDTERM_CH2_RIGHT)
.const AUDTERM_CH1_RIGHT    = (1 << bAUDTERM_CH1_RIGHT)

; Master Enable

.const rAUDENA              = rNR52
.const bAUDENA_ENABLE       = 7
.const bAUDENA_ENABLE_CH4   = 3
.const bAUDENA_ENABLE_CH3   = 2
.const bAUDENA_ENABLE_CH2   = 1
.const bAUDENA_ENABLE_CH1   = 0

.const AUDENA_OFF           = (0 << bAUDENA_ENABLE)
.const AUDENA_ON            = (1 << bAUDENA_ENABLE)
.const AUDENA_CH4_OFF       = (0 << bAUDENA_ENABLE_CH4)
.const AUDENA_CH4_ON        = (1 << bAUDENA_ENABLE_CH4)
.const AUDENA_CH3_OFF       = (0 << bAUDENA_ENABLE_CH3)
.const AUDENA_CH3_ON        = (1 << bAUDENA_ENABLE_CH3)
.const AUDENA_CH2_OFF       = (0 << bAUDENA_ENABLE_CH2)
.const AUDENA_CH2_ON        = (1 << bAUDENA_ENABLE_CH2)
.const AUDENA_CH1_OFF       = (0 << bAUDENA_ENABLE_CH1)
.const AUDENA_CH1_ON        = (1 << bAUDENA_ENABLE_CH1)

; Port Registers - Audio Digital Outputs ***************************************

.const rPCM12               = $58
.const rPCM34               = $59

.const PCM12_CH1            = %00001111
.const PCM12_CH2            = %11110000
.const PCM34_CH3            = %00001111
.const PCM34_CH4            = %11110000

; Port Registers - PPU Display Control *****************************************

.const rLCDC                = $60
.const bLCDC_ENABLE         = 7
.const bLCDC_WIN_MAP        = 6
.const bLCDC_WINDOW         = 5
.const bLCDC_BLOCKS         = 4
.const bLCDC_BG_MAP         = 3
.const bLCDC_OBJ_SIZE       = 2
.const bLCDC_OBJS           = 1
.const bLCDC_BG             = 0
.const bLCDC_PRIO           = 0

.const LCDC_ENABLE          = (1 << bLCDC_ENABLE)
.const LCDC_OFF             = (0 << bLCDC_ENABLE)
.const LCDC_ON              = (1 << bLCDC_ENABLE)
.const LCDC_WIN_MAP         = (1 << bLCDC_WIN_MAP)
.const LCDC_WIN_9800        = (0 << bLCDC_WIN_MAP)
.const LCDC_WIN_9C00        = (1 << bLCDC_WIN_MAP)
.const LCDC_WINDOW          = (1 << bLCDC_WINDOW)
.const LCDC_WIN_OFF         = (0 << bLCDC_WINDOW)
.const LCDC_WIN_ON          = (1 << bLCDC_WINDOW)
.const LCDC_BLOCKS          = (1 << bLCDC_BLOCKS)
.const LCDC_BLOCK21         = (1 << bLCDC_BLOCKS)
.const LCDC_BLOCK01         = (0 << bLCDC_BLOCKS)
.const LCDC_BG_MAP          = (1 << bLCDC_BG_MAP)
.const LCDC_BG_9800         = (0 << bLCDC_BG_MAP)
.const LCDC_BG_9C00         = (1 << bLCDC_BG_MAP)
.const LCDC_OBJ_SIZE        = (1 << bLCDC_OBJ_SIZE)
.const LCDC_OBJ_8           = (0 << bLCDC_OBJ_SIZE)
.const LCDC_OBJ_16          = (1 << bLCDC_OBJ_SIZE)
.const LCDC_OBJS            = (1 << bLCDC_OBJS)
.const LCDC_OBJ_OFF         = (0 << bLCDC_OBJS)
.const LCDC_OBJ_ON          = (1 << bLCDC_OBJS)
.const LCDC_BG              = (1 << bLCDC_BG)
.const LCDC_BG_OFF          = (0 << bLCDC_BG)
.const LCDC_BG_ON           = (1 << bLCDC_BG)
.const LCDC_PRIO            = (1 << bLCDC_PRIO)
.const LCDC_PRIO_OFF        = (0 << bLCDC_PRIO)
.const LCDC_PRIO_ON         = (1 << bLCDC_PRIO)

; Port Registers - PPU Display Status ******************************************

.const rSTAT                = $61
.const bSTAT_LYC            = 6
.const bSTAT_MODE_2         = 5
.const bSTAT_OBJECT_SCAN    = 5
.const bSTAT_MODE_1         = 4
.const bSTAT_VBLANK         = 4
.const bSTAT_MODE_0         = 3
.const bSTAT_HBLANK         = 3
.const bSTAT_LYCF           = 2
.const bSTAT_COINCIDENCE    = 2
.const bSTAT_BUSY           = 1

.const STAT_LYC             = (1 << bSTAT_LYC)
.const STAT_MODE_2          = (1 << bSTAT_MODE_2)
.const STAT_MODE_1          = (1 << bSTAT_MODE_1)
.const STAT_MODE_0          = (1 << bSTAT_MODE_0)
.const STAT_LYCF            = (1 << bSTAT_LYCF)
.const STAT_BUSY            = (1 << bSTAT_BUSY)

.const STAT_MODE                = %00000011
.const STAT_HORIZONTAL_BLANK    = %00000000
.const STAT_VERTICAL_BLANK      = %00000001
.const STAT_OBJECT_SCAN         = %00000010
.const STAT_PIXEL_TRANSFER      = %00000011

; Port Registers - PPU Position Registers **************************************

.const rSCY                 = $62
.const rSCX                 = $63
.const rWY                  = $64
.const rWX                  = $65
.const rLY                  = $66
.const rLYC                 = $67

.const LY_VBLANK            = 144
.const WX_OFFSET            = 7

; Port Registers - PPU Monochrome Palettes *************************************

.const rBGP                 = $68
.const rOBP0                = $69
.const rOBP1                = $6A

; Port Registers - PPU Color Palettes ******************************************

.const rBGPI                = $6B
.const rBGPD                = $6C
.const rOBPI                = $6D
.const rOBPD                = $6E

.const bBGPI_AUTOINC        = 7
.const bOPBI_AUTOINC        = 7

.const BGPI_AUTOINC         = (1 << bBGPI_AUTOINC)
.const OBPI_AUTOINC         = (1 << bOPBI_AUTOINC)
.const BGPI_INDEX           = %00111111
.const OPBI_INDEX           = %00111111

; Port Registers - PPU Auxillary Settings **************************************

.const rPPUX                = $6F
.const bPPUX_VBK            = 0
.const bPPUX_OPRI           = 1
.const bPPUX_PALM           = 2

.const PPUX_VBK             = (1 << bPPUX_VBK)
.const PPUX_OPRI            = (1 << bPPUX_OPRI)
.const PPUX_OPRI_OAM        = (0 << bPPUX_OPRI)
.const PPUX_OPRI_CGB        = (0 << bPPUX_OPRI)
.const PPUX_OPRI_COORD      = (1 << bPPUX_OPRI)
.const PPUX_OPRI_DMG        = (1 << bPPUX_OPRI)
.const PPUX_PALM            = (1 << bPPUX_PALM)


; Port Registers - Direct Memory Access ****************************************

.const rODMAS               = $70
.const rODMA1               = $71
.const rODMA2               = $72
.const rODMA3               = $73
.const rVDMA0               = $74
.const rVDMA1               = $75
.const rVDMA2               = $76
.const rVDMA3               = $77
.const rVDMA4               = $78
.const rVDMA5               = $79
.const rVDMA6               = $7A

.const rODMA_START          = rODMAS
.const rODMA_SRC_1          = rODMA1
.const rODMA_SRC_2          = rODMA2
.const rODMA_SRC_3          = rODMA3

.const rVDMA_SRC_0          = rVDMA0
.const rVDMA_SRC_1          = rVDMA1
.const rVDMA_SRC_2          = rVDMA2
.const rVDMA_SRC_3          = rVDMA3
.const rVDMA_DEST_0         = rVDMA4
.const rVDMA_DEST_1         = rVDMA5
.const rVDMA_CTRL           = rVDMA6

.const bVDMA_CTRL_MODE      = 7
.const bVDMA_CTRL_BUSY      = 7

.const VDMA_CTRL_MODE           = (1 << bVDMA_CTRL_MODE)
.const VDMA_CTRL_MODE_GENERAL   = (0 << bVDMA_CTRL_MODE)
.const VDMA_CTRL_MODE_HBLANK    = (1 << bVDMA_CTRL_MODE)
.const VDMA_CTRL_BUSY           = (1 << bVDMA_CTRL_BUSY)
.const VDMA_CTRL_BUSY_NO        = (0 << bVDMA_CTRL_BUSY)
.const VDMA_CTRL_BUSY_YES       = (1 << bVDMA_CTRL_BUSY)

; Constants - Screen ***********************************************************

.const SCREEN_WIDTH_PX          = 160
.const SCREEN_HEIGHT_PX         = 144
.const SCREEN_AREA_PX           = SCREEN_WIDTH_PX * SCREEN_HEIGHT_PX
.const SCREEN_WIDTH_B           = 20
.const SCREEN_HEIGHT_B          = 18
.const SCREEN_AREA_B            = SCREEN_WIDTH_B * SCREEN_HEIGHT_B
.const TILEMAP_WIDTH_PX         = 256
.const TILEMAP_HEIGHT_PX        = 256
.const TILEMAP_WIDTH            = 32
.const TILEMAP_HEIGHT           = 32
.const TILEMAP_AREA             = TILEMAP_WIDTH * TILEMAP_HEIGHT
.const TILE_WIDTH               = 8
.const TILE_HEIGHT              = 8
.const TILE_SIZE                = 16
.const COLOR_SIZE               = 2
.const PAL_COLORS               = 4
.const PAL_SIZE                 = PAL_COLORS * COLOR_SIZE
.const COLOR_CH_WIDTH           = 5
.const COLOR_CH_MAX             = (1 << COLOR_CH_WIDTH) - 1
.const bCOLOR_RED               = (COLOR_CH_WIDTH * 0)
.const bCOLOR_GREEN             = (COLOR_CH_WIDTH * 1)
.const bCOLOR_BLUE              = (COLOR_CH_WIDTH * 2)
.const COLOR_RED                = %00011111
.const COLOR_GREEN_LOW          = %11100000
.const COLOR_GREEN_HIGH         = %00000011
.const COLOR_BLUE               = %01111100
.const SHADE_WHITE              = %00
.const SHADE_LIGHT              = %01
.const SHADE_DARK               = %10
.const SHADE_BLACK              = %11
.const TILEMAP0                 = $80009800
.const TILEMAP1                 = $80009C00
.const bBG_PRIO                 = 7
.const bBG_YFLIP                = 6
.const bBG_XFLIP                = 5
.const bBG_BANK1                = 3
.const BG_PALETTE               = %00000111
.const BG_PRIO                  = (1 << bBG_PRIO)
.const BG_YFLIP                 = (1 << bBG_YFLIP)
.const BG_XFLIP                 = (1 << bBG_XFLIP)
.const BG_BANK0                 = (0 << bBG_BANK1)
.const BG_BANK1                 = (1 << bBG_BANK1)

; Constants - Objects **********************************************************

.const OAMA_Y                   = 0
.const OAM_Y_OFS                = 16
.const OAMA_X                   = 1
.const OAM_X_OFS                = 8
.const OAMA_TILEID              = 2
.const OAMA_FLAGS               = 3
.const bOAM_PRIO                = 7
.const bOAM_YFLIP               = 6
.const bOAM_XFLIP               = 5
.const bOAM_PAL1                = 4
.const bOAM_BANK1               = 3
.const OAM_PALETTE              = %00000111
.const OAM_PRIO                 = (1 << bOAM_PRIO)
.const OAM_YFLIP                = (1 << bOAM_YFLIP)
.const OAM_XFLIP                = (1 << bOAM_XFLIP)
.const OAM_PAL0                 = (0 << bOAM_PAL1)
.const OAM_PAL1                 = (1 << bOAM_PAL1)
.const OAM_BANK0                = (0 << bOAM_BANK1)
.const OAM_BANK1                = (1 << bOAM_BANK1)
.const OBJ_SIZE                 = 4
.const OAM_COUNT                = 40
.const OAM_SIZE                 = OBJ_SIZE * OAM_COUNT

; Constants - Audio Channel RAM ************************************************

.const AUD1RAM                  = $FFFFFF30
.const AUD2RAM                  = $FFFFFF38
.const AUD3RAM                  = $FFFFFF40
.const AUD4RAM                  = $FFFFFF48
.const AUD3WAVERAM              = mWAVRAM
.const AUD3WAVERAM_SIZE         = 16
.const AUD3WAVERAM_NIBBLE_COUNT = 32

; Constants - Interrupt Vector Addresses ***************************************

.const INT_HANDLER_EXCEPTION    = $1000
.const INT_HANDLER_VBLANK       = $1080
.const INT_HANDLER_STAT         = $1100
.const INT_HANDLER_SERIAL       = $1180
.const INT_HANDLER_JOYPAD       = $1200
