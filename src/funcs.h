// The game's routines ported to C so far, named as in asm/. The rest is still
// original code, which ported code calls with emu_call (dv_cpu_infra.h).
//
// While original code still runs, each ported routine leaves the 6502's registers as the
// original does (dv_cpu_infra.h): it loads its arguments into the registers the original takes
// them in, starts with RUN_ORIGINAL_IF_LAG, and sets the registers the original returns.
#ifndef DV_FUNCS_H
#define DV_FUNCS_H

#include <stddef.h>
#include <stdint.h>

#include "dv_cpu_infra.h"

// Where the originals are, all in the fixed bank (7). Also for original code not ported yet
// that ported code calls.
enum {
  kChangeScene = 0xC000,
  kChangeSceneInstant = 0xC03F,
  kLoadScene = 0xC076,
  kSelectSceneBank = 0xC09F,
  kScenePointer = 0xC0CE,
  kDrawSceneDissolve = 0xC0DD,
  kDissolveDrawTile = 0xC119,
  kDissolveTileValue = 0xC131,
  kDissolveTileAddress = 0xC150,
  kLoadSceneAttrs = 0xC186,
  kSceneAttrPositions = 0xC1A6,  // [16] where each attribute byte goes in attr_buffer_2000
  kBlackenPaletteColors = 0xC1B6,
  kDrawScene = 0xC1CC,
  kCopySceneTiles = 0xC1F4,
  kApplyRoomSettings = 0xC201,
  kLoadDissolvePattern = 0xC211,
  kRoomSettings = 0xC21F,  // [80] per room: low nibble a sound, high the dissolve pattern
  kTestHotspot = 0xC26F,
  kCursorInHotspot = 0xC283,
  kClickPicture = 0xC2A8,
  kFindClickedHotspot = 0xC2B7,
  kBitMasks = 0xC45A,  // [8] 1 << n
  kCollectRoomSprites = 0xC462,
  kPrintMessage5 = 0xC4AE,
  kPrintMessageFrom = 0xC4BC,
  kPrintMessage4 = 0xC4EF,
  kPrintMessage2Or5 = 0xC501,
  kItemCodeForObject = 0xC520,
  kItemCodeTableNumbers = 0xC551,  // [6] by script table: the number in an item code's top bits
  kItemCodeTables = 0xC557,        // [6] the other way
  kSpecialScenePictures = 0xC578,  // [27] each special scene's picture
  kSpecialSceneAction = 0xC593,
  kSpecialSceneWithMessage = 0xC5D4,
  kSpecialSceneFinal = 0xC66C,
  kSetSpecialSceneFlag = 0xC68F,
  kClearSpecialSceneFlag = 0xC6B3,
  kSpecialSceneFlags = 0xC6BF,  // [27] each scene's event flag
  kSpecialSceneMessage = 0xC6DA,
  kSpecialSceneMessageIds = 0xC6FF,    // read at 2n: the byte before the kind
  kSpecialSceneMessageKinds = 0xC700,  // read at 2n: which printer
  kEnterSpecialScene = 0xC735,
  kLeaveSpecialScene = 0xC76C,
  kSpecialSceneExtras = 0xC779,
  kWait180Frames = 0xC7E4,
  kWait60Frames = 0xC7EE,
  kWait30Frames = 0xC7F5,
  kLoadGame = 0xC800,
  kReadSaveCopy = 0xC830,
  kSaveGame = 0xC858,
  kWriteSaveCopy = 0xC879,
  kCheckSaveFile = 0xC8C9,
  kSaveFileBits = 0xC90C,  // [3] each file's bit
  kRepairSaveCopies = 0xC90F,
  kUnlockSaveRam = 0xC94E,
  kProtectSaveRam = 0xC95A,
  kDisableSaveRam = 0xC966,
  kSaveCopyAddress = 0xC972,
  kShiftSaveCopies = 0xC9A3,
  kNextCopyPointer = 0xC9C0,
  kCopySaveCopy = 0xC9CC,
  kEraseSaveCopies = 0xC9E1,
  kEraseSaveCopy = 0xC9F1,
  kClearSum16 = 0xCA0B,
  kAddToSum16 = 0xCA12,
  kStoreChecksum = 0xCA1E,
  kCheckChecksum = 0xCA2A,
  kCheckSignature = 0xCA3F,
  kSaveSignature = 0xCA53,  // [6] "DEJAVU"
  kStoreSignature = 0xCA59,
  kResetGameState = 0xCA66,
  kRestoreStateBackup = 0xCAC7,
  kPickDefaultCommand = 0xCB08,
  kRunCommand = 0xCB4B,
  kScriptResume = 0xCB5C,
  kScriptStart = 0xCB64,
  kScriptLoop = 0xCBB1,
  kScriptRunOpcode = 0xCBBC,
  kScriptFetchByte = 0xCBC2,
  kScriptAdvance = 0xCBCA,
  kScriptFetch2 = 0xCBD8,
  kScriptFetch3 = 0xCBE8,
  kScriptFetch4 = 0xCBFD,
  kScriptDispatch = 0xCC17,
  kScriptOpTable = 0xCC1A,  // [105]
  kScriptOpSwitch = 0xCDE0,
  kScriptOpYield = 0xCE01,
  kOp31 = 0xCE0B,
  kOp00Message5 = 0xCF87,
  kOp01Message4 = 0xCF8E,
  kOp02Message2Or5 = 0xCF95,
  kScriptOpGotoIf = 0xCF9C,
  kScriptOpGotoIfNot = 0xCFA6,
  kScriptGoto = 0xCFB0,
  kScriptOpPushGoto = 0xCFB9,
  kScriptOpPopGoto = 0xCFCA,
  kScriptOpGoto = 0xCFD8,
  kScriptConditionTrue = 0xCFDF,
  kScriptConditionFalse = 0xCFE6,
  kCloseContainerEntry = 0xCFED,
  kOp52TestStoryFlag = 0xD103,
  kOp07TestStoryFlagOfObject = 0xD109,
  kTestStoryFlag = 0xD10B,
  kOp08SetStoryFlagOfObject = 0xD112,
  kSetStoryFlagRefresh = 0xD114,
  kOp09ClearStoryFlagOfObject = 0xD12B,
  kClearStoryFlag = 0xD12D,
  kSelectStoryFlags = 0xD141,
  kTestFlagBit = 0xD14C,
  kLoadFlagByte = 0xD163,
  kOp5D = 0xD171,
  kOp0A = 0xD177,
  kTestItemObtained = 0xD179,
  kOp0B = 0xD18F,
  kOp0C = 0xD1A5,
  kClearItemObtained = 0xD1A7,
  kOp0D = 0xD1B8,
  kOp2B = 0xD1BD,
  kTestObjectOpen = 0xD1C0,
  kOp0E = 0xD1D6,
  kOp2C = 0xD1DB,
  kSetObjectOpen = 0xD1DE,
  kOp0F = 0xD1EF,
  kOp2D = 0xD1F4,
  kCloseObject = 0xD1F7,
  kOp10 = 0xD208,
  kOp11 = 0xD220,
  kOp12 = 0xD233,
  kClearItemOpen = 0xD235,
  kOp13TestDoorFlagOfObject = 0xD246,
  kOp2ETestDoorFlag = 0xD24B,
  kTestDoorFlag = 0xD24E,
  kOp14SetDoorFlagOfObject = 0xD255,
  kOp2FSetDoorFlag = 0xD25A,
  kSetDoorFlag = 0xD25D,
  kOp15ClearDoorFlagOfObject = 0xD271,
  kOp30ClearDoorFlag = 0xD276,
  kClearDoorFlag = 0xD279,
  kSelectDoorFlags = 0xD28D,
  kOp16TestEventFlag = 0xD298,
  kTestEventFlag = 0xD29B,
  kOp17SetEventFlag = 0xD2A2,
  kSetEventFlag = 0xD2A5,
  kOp18ClearEventFlag = 0xD2B9,
  kClearEventFlag = 0xD2BC,
  kSelectEventFlags = 0xD2D0,
  kUnreferencedD2DB = 0xD2DB,
  kOp19 = 0xD305,
  kOp33 = 0xD315,
  kOp20TestFlag045COfObject = 0xD42A,
  kTestFlag045C = 0xD42C,
  kOp53SetFlag045C = 0xD433,
  kOp21SetFlag045COfObject = 0xD439,
  kSetFlag045C = 0xD43B,
  kOp22ClearFlag045COfObject = 0xD44B,
  kClearFlag045C = 0xD44D,
  kSelectFlags045C = 0xD45D,
  kOp23TestFlag047AOfObject = 0xD468,
  kTestFlag047A = 0xD46A,
  kOp54SetFlag047A = 0xD471,
  kOp24SetFlag047AOfObject = 0xD477,
  kSetFlag047A = 0xD479,
  kOp25ClearFlag047AOfObject = 0xD489,
  kSelectFlags047A = 0xD49B,
  kOp26 = 0xD4A6,
  kOp48YieldB580 = 0xD8A0,
  kOp49 = 0xD8AB,
  kBitClearMasks = 0xDC9B,  // [8] ~(1 << n)
  kNextHotspot = 0xC323,
  kHotspotChecks = 0xC331,
  kSplitItemCode = 0xC533,
  kRunSpecialScene = 0xC55D,
  kShowSpecialScene = 0xC74B,
  kOp68 = 0xCCEC,
  kOp67 = 0xCCF7,
  kOp64 = 0xCD0D,
  kOp63 = 0xCD21,
  kShowFoundPageNext = 0xCD76,
  kOp61 = 0xCD9D,
  kOp60 = 0xCDC4,
  kOp5F = 0xCDD9,
  kChoiceIsTable2 = 0xCE13,
  kOp32 = 0xCE7A,
  kOp32Body = 0xCEE2,
  kOp5B = 0xCF20,
  kOp32BodyEnd = 0xCF5B,
  kCloseAllContainers = 0xD028,
  kClearListedStoryFlags = 0xD047,
  kListedStoryFlags = 0xD06C,  // [12]
  kOp06 = 0xD078,
  kEnterRoom = 0xD0D2,
  kOp36 = 0xD31B,
  kOp1A = 0xD321,
  kOp34 = 0xD333,
  kOp37 = 0xD339,
  kOp1B = 0xD33F,
  kOp1D = 0xD351,
  kOp1DBody = 0xD35D,
  kTakeItemFromContainer = 0xD3CD,
  kFindItemInContainer = 0xD3D9,
  kSelectStoreSlot = 0xD401,
  kWaitFrames = 0xD40E,
  kOp1E = 0xD418,
  kOp1F = 0xD421,
  kOpenContainer = 0xD4B8,
  kLoadContainerView = 0xD4DF,
  kSelectItemList = 0xD4ED,
  kFillCoatFromItemList = 0xD4FD,
  kRefreshCoatPage = 0xD558,
  kItemListEntryVisible = 0xD577,
  kItemListEntryCondition = 0xD583,
  kClearCoatSlots = 0xD5B3,
  kCopyContainerToCoat = 0xD5C0,
  kOp27 = 0xD5E4,
  kOp2A = 0xD5F0,
  kOp35 = 0xD5F6,
  kOp38 = 0xD5FD,
  kOp39 = 0xD606,
  kAddCoins = 0xD609,
  kOp62 = 0xD617,
  kAddCoinsAndShow = 0xD61A,
  kOp56 = 0xD626,
  kOp3A = 0xD637,
  kOp57 = 0xD646,
  kOp3B = 0xD651,
  kOp3C = 0xD660,
  kOp3D = 0xD6A4,
  kOp3E = 0xD6CA,
  kOp40 = 0xD6F0,
  kShowFoundPage = 0xD711,
  kOp3F = 0xD73D,
  kOp41 = 0xD746,
  kAddressRooms = 0xD79A,  // [5]
  kOp44 = 0xD7A6,
  kOp45 = 0xD7B6,
  kOp43 = 0xD7C0,
  kOp42 = 0xD7D9,
  kOp46SpecialScene = 0xD812,
  kOp47 = 0xD81B,
  kCloseContainer = 0xD823,
  kPreviousStoreEntry = 0xD8BE,
  kOp4A = 0xD8D2,
  kOp4B = 0xD8DE,
  kOp4C = 0xD8ED,
  kOp4D = 0xD8FE,
  kOp4E = 0xD90F,
  kOp4F = 0xD926,
  kOp50 = 0xD935,
  kOp51 = 0xD965,
  kOp58 = 0xD974,
  kOp59 = 0xD98B,
  kOp55 = 0xD99A,
  kFindAndShowItem = 0xD9AC,
  kFindInGoods = 0xD9BD,
  kShowItemPageKeepCursor = 0xD9E0,
  kRestoreInventoryCursor = 0xDA16,
  kShowPageThenOp49 = 0xDA21,
  kOp5A = 0xDA28,
  kOp5C = 0xDA31,
  kOp5E = 0xDA3D,
  kOp5ESpecialCheck = 0xDA6B,
  kOp5ESpecialCheckBody = 0xDA73,
  kEncounterRooms = 0xDC92,
  kTextReadCode = 0xDF00,
  kTextReadByte = 0xDF34,
  kTextReadBit = 0xDF4C,
  kPrintMessage = 0xDF74,
  kPrintControl = 0xDFDE,
  kPrintItemNameAt = 0xE005,
  kPrintItemNameArg = 0xE02A,
  kPrintWordArg = 0xE055,
  kPrintName = 0xE080,
  kPrintNumberAt = 0xE0A6,
  kTextWaitPage = 0xE0BC,
  kTextNewline = 0xE0D8,
  kPrintClear = 0xE0EC,
  kPrintDigitSkipZero = 0xE0F0,
  kPrintDigit = 0xE0FD,
  kNumberToDigits = 0xE10D,
  kReadNumberArg = 0xE157,
  kSkipInlineWord = 0xE17F,
  kClearTextWindow = 0xE195,
  kBlankTextWindow = 0xE1B2,
  kPrintTile = 0xE1F2,
  kPrintChar = 0xE1FE,
  kTextTileTable = 0xE213,
  kPutTile = 0xE253,
  kTextLineFeed = 0xE2F9,
  kTextScroll = 0xE316,
  kTextScrollRows = 0xE327,
  kTextCursorVramAddress = 0xE345,
  kUploadTextWindow = 0xE35F,
  kDrawWindowRuleRow = 0xE387,
  kBlankWindowTextRows = 0xE3A8,
  kDrawTextWindowSides = 0xE3CF,
  kDrawWindowSideColumn = 0xE3F0,
  kTextPause = 0xE40D,
  kWaitPageInput = 0xE43C,
  kBlinkMoreArrow = 0xE476,
  kShowMoreArrow = 0xE48C,
  kWaitForMoreButton = 0xE493,
  kDrawMoreArrowFrame = 0xE4C1,
  kTextCursorXPixels = 0xE4D2,
  kDrawMoreArrow = 0xE4DF,
  kDrawMoreArrowEnd = 0xE4F3,  // the arrow's metasprite follows it
  kMain = 0xE600,
  kGameScreenLoop = 0xE62B,
  kMainJumpTable = 0xE689,  // the .addr list after main's jsr jump_table
  kDrawGameScreen = 0xE68D,
  kUpdateCursorAndSprites = 0xE6EA,
  kLoadCurrentScene = 0xE6FF,
  kShowIntroOnce = 0xE70C,
  kNewGame = 0xE72E,
  kNextRandom = 0xE7B1,
  kNextRandomEnd = 0xE7D7,  // nothing but $FF up to reset
  kReset = 0xE800,
  kSetMirroringVertical = 0xE87B,
  kSetMirroringHorizontal = 0xE88B,
  kSwitchPrgBank = 0xE89B,
  kApplyChrBanksR0R1 = 0xE8BF,
  kApplyChrBanksR2R5 = 0xE8DA,
  kMmc3SetBank = 0xE8FF,
  kSetChrBanksInline = 0xE916,
  kApplyChrBanks = 0xE97D,
  kAdvanceInlinePtr = 0xE984,
  kDrawRleScreen2400 = 0xE992,
  kDrawRleScreen2000 = 0xE9D5,
  kRleToPpu2000 = 0xEA18,
  kRleWrite2000 = 0xEA47,
  kRleToPpu2400 = 0xEA75,
  kReadInlineByte = 0xEAC6,
  kPushAndSwitchBank = 0xEAD0,
  kSetCurrentBank = 0xEAD7,
  kSwitchToSoundBank = 0xEADE,
  kRestoreCurrentBank = 0xEAE3,
  kPopBank = 0xEAEA,
  kStackPush = 0xEAFD,
  kStackPop = 0xEB13,
  kDisableNmi = 0xEB29,
  kEnableNmi = 0xEB33,
  kIrq = 0xEB40,
  kReadJoypads = 0xEB41,
  kReadJoypadsOnce = 0xEB6E,
  kWaitVblank = 0xEB9C,
  kRequestNmiWork = 0xEBA9,
  kWaitVramQueueEmpty = 0xEBAE,
  kWaitNmiWorkDone = 0xEBB2,
  kWaitVblankAndHideSprites = 0xEBB9,
  kClearOamBuffer = 0xEBBF,
  kClearAndUploadSprites = 0xEBDE,
  kRequestOamDma = 0xEBE1,
  kJumpTable = 0xEBE6,
  kFarCall8000Body = 0xEC03,
  kFarCall8020Body = 0xEC27,
  kFarCall8040Body = 0xEC4B,
  kFarCall8400Body = 0xEC6F,
  kFarCallJump = 0xEC93,
  kNmi = 0xEC9A,
  kNmiPpuUpdates = 0xED24,
  kNmiUploadAttrs2000 = 0xED4C,
  kNmiUploadAttrs2nd = 0xED6D,
  kNmiFlushVramQueue = 0xED97,
  kNmiFlushVramQueueDown = 0xEDA3,
  kFlushVramEntry = 0xEDBB,
  kFlushVramQueue = 0xEDE3,
  kNmiBlackPalette = 0xEDF1,
  kNmiUploadPalette = 0xEE0B,
  kPpuAddrLeavePalette = 0xEE23,
  kPpuAddrPalette = 0xEE2D,
  kNmiSetScroll = 0xEE38,
  kClearNametables = 0xEE65,
  kRenderingOn = 0xEE6E,
  kClearNametable2000 = 0xEE8B,
  kRenderingOff = 0xEE9A,
  kFillNametable2000 = 0xEEAB,
  kFillNametable2nd = 0xEEB2,
  kRequestVramQueue = 0xEEE0,
  kQueueVramBlock = 0xEEE5,
  kQueueVramFill = 0xEEF9,
  kVramBlockSetup = 0xEF0D,
  kVramFillRows = 0xEF29,
  kVramCopyRows = 0xEF60,
  kNextVramAddress = 0xEF96,
  kVramWrapUp = 0xEFA4,
  kVramQueueHeader = 0xEFC9,
  kNextVramRow = 0xEFE7,
  kSkipBlockHeader = 0xF02A,
  kFlushVramQueueIfFull = 0xF038,
  kFlushVramQueueDownIfFull = 0xF046,
  kFlushVramQueueDown = 0xF054,
  kQueueVramColumn = 0xF060,
  kVramColumnSetup = 0xF06F,
  kVramCopyColumn = 0xF090,
  kVramAddrDownRow = 0xF0D3,
  kVramWrapColumn = 0xF0E1,
  kAttrPosition = 0xF105,
  kSetAttrBlock = 0xF131,
  kNextAttrRow = 0xF167,
  kNextAttrColumn = 0xF1A5,
  kSetAttrBits = 0xF1D2,
  kSetAttrBitsEnd = 0xF1FA,  // two tables follow it
  kFillAllAttrs = 0xF20E,
  kUnreferencedF21B = 0xF21B,
  kPickAttrFill = 0xF22F,
  kPickAttrFillEnd = 0xF243,  // its table follows it
  kFillAttrBuffer2000 = 0xF247,
  kFillAttrBuffer2400 = 0xF24B,
  kRestoreXy = 0xF25B,
  kEraseSaveFile = 0xC89F,
  kCheckAllSaveFiles = 0xC8A9,
  kBackupStateToSaveRam = 0xCA7C,
  kRequestAttrs2000 = 0xF260,
  kRequestAttrs2nd = 0xF265,
  kRequestAttrUpload = 0xF26A,
  kRequestBlackPalette = 0xF26F,
  kShowBgAndSprites = 0xF274,
  kShowSprites = 0xF278,
  kHideBgAndSprites = 0xF282,
  kHideSprites = 0xF286,
  kRequestPaletteUpload = 0xF299,
  kFadeInPalette = 0xF29E,
  kPaletteBrightenStep = 0xF2C6,
  kFadeOutPalette = 0xF2DB,
  kPaletteDarkenStep = 0xF2F8,
  kPaletteFadeWait = 0xF31C,
  kUnreferencedF32F = 0xF32F,
  kFarCall8000 = 0xF3B5,
  kFarCall8020 = 0xF3C5,
  kFarCall8040 = 0xF3D5,
  kFarCall8400 = 0xF3E5,
  kSoundReset = 0xF3F5,
  kPlaySound = 0xF409,
  kSoundCheckRepeat = 0xF433,
  kStartSound = 0xF44D,
  kSoundStartByType = 0xF466,
  kSoundStartMusic = 0xF477,
  kSoundStartChannel = 0xF4B0,
  kSoundStartRepeatingMusic = 0xF4E1,
  kSoundStartEffect3ch = 0xF4EB,
  kSoundStartEffect2ch = 0xF517,
  kSoundUpdate = 0xF538,
  kSoundUpdateAll = 0xF553,
  kSoundStepAllChannels = 0xF55E,
  kSoundStepPulse = 0xF5A4,
  kSoundPulseEnvelope = 0xF60F,
  kSoundStepTriangle = 0xF646,
  kMultiply8x8 = 0xF6EA,
  kSoundTriangleCountdown = 0xF710,
  kSoundStepNoise = 0xF733,
  kSoundTick = 0xF78B,
  kSoundAddLength = 0xF79F,
  kSoundReadCommand = 0xF7C0,
  kSoundLoopPush = 0xF944,
  kSoundLoopPop = 0xF954,
  kSoundNoteOff = 0xF96C,
  kSoundChannelDone = 0xF975,
  kSoundWriteApu = 0xF9B6,
  kSoundJumpTable = 0xFADA,
  kNoteLengths = 0xFAF5,
  kNotePeriods = 0xFB1B,
  kDrawLayoutNumber = 0xFBED,
  kLayoutRun = 0xFC25,
  kLayoutResetState = 0xFC38,
  kLayoutFetch = 0xFC4F,
  kLayoutAdvance1 = 0xFC60,
  kLayoutAdvance2 = 0xFC6E,
  kLayoutAdvance3 = 0xFC74,
  kLayoutPutTile = 0xFC7A,
  kLayoutCommand = 0xFC88,
  kUnreferencedFCC8 = 0xFCC8,
  kLayoutNameByIndexCommand = 0xFCCE,
  kLayoutNameAtAddressCommand = 0xFCD8,
  kLayoutNameInRamCommand = 0xFCE2,
  kLayoutNameFromRam = 0xFCEC,
  kLayoutNameFromIndex = 0xFCF3,
  kLayoutNameFromAddress = 0xFD20,
  kCopyNameToBuffer = 0xFD2C,
  kLayoutPrintNameBuffer = 0xFD39,
  kLayoutBytePaddedCommand = 0xFD66,
  kLayoutWordPaddedCommand = 0xFD73,
  kLayoutByteCommand = 0xFD80,
  kLayoutWordCommand = 0xFD8D,
  kLayoutAdvanceX = 0xFD9A,
  kLayoutSetVramDest = 0xFDA2,
  kLayoutSkip5 = 0xFDB3,
  kLayoutSkip3 = 0xFDB7,
  kLayoutPositionCommand = 0xFDBE,
  kLayoutDrawTile = 0xFDCA,
  kLayoutTilePosition = 0xFDD7,
  kTilePositionToVram = 0xFDE0,
  kLayoutTileKind = 0xFE0A,
  kQueueTile = 0xFE51,
  kDrawMetasprite = 0xFE5D,
  kLayoutPrintBytePadded = 0xFEC8,
  kLayoutPrintByte = 0xFEE4,
  kLayoutPrintWordPadded = 0xFF00,
  kLayoutPrintWord = 0xFF1C,
  kLoadByteValue = 0xFF38,
  kLoadWordValue = 0xFF40,
  kDecimalDigits = 0xFF4C,
  kDecimalPlaces = 0xFF88,  // [4] 10000, 1000, 100, 10, little-endian (in decimal_digits's range)
  kTrimLeadingZeros = 0xFF90,
  kDigitsToTilesPadded = 0xFFAA,
  kDigitsToTiles = 0xFFBD,
  kPrintDecimalDigits = 0xFFD5,
  kPrintDecimalDigitsEnd = 0xFFF0,  // nothing but $FF up to the vectors
};

// In bank 3, where script_start maps it.
enum { kScriptTables = 0x8000 };

// In bank 2, which the routines that read them map.
enum {
  kRoomHotspots = 0x9C00,  // [rooms] a pointer to each room's hotspot records
  kItemLists = 0xA400,     // [2] pointers to the item lists by object, for tables 1 and others
  kNames = 0x8D00,         // [n] 8-byte names of items and words (layout_name_from_index)
  // The menus.
  kTitleScreen = 0x840C,
  kTitlePalette = 0x843D,
  kDrawTitlePicture = 0x845D,
  kFileSelectMenu = 0x86D0,
  kFileBits = 0x8741,  // [3]
  kFileSelectScreen = 0x8774,
  kMenuPalette = 0x87A6,
  kDrawFileList = 0x87C6,
  kDrawFileEntry = 0x87DE,
  kFileEntrySetup = 0x87EE,
  kDrawFileStatus = 0x8811,
  kFileEntryTiles = 0x882A,  // [5]
  kFileLetters = 0x882F,     // [3]
  kFileEntryVram = 0x8832,   // [3] low bytes
  kFileStatusVram = 0x8835,  // [3] low bytes
  kDrawFileOptions = 0x883D,
  kFileOptionBlocks = 0x8873,  // [3]
  kFileOptionVram = 0x88B9,    // [3]
  kClearFileArea = 0x88BF,
  kFillFileArea = 0x88CB,
  kClearFileBlock = 0x88D9,
  kDrawFileCursor = 0x88DC,
  kFileCursorYs = 0x88F7,  // [3]
  kDrawOptionCursor = 0x88FD,
  kOptionCursorYs = 0x890A,  // [3] tools labels them unreferenced_890A, as they decode as code
  kShowFileOptions = 0x890D,
  kYesNoVram = 0x894E,  // [2] low bytes
  kQuitDialog = 0x8950,
  kSaveDialog = 0x8965,
  kOpenConfirmDialog = 0x8984,
  kDrawDialog = 0x89A7,
  kDrawDialogText = 0x89B4,
  kDialogBlocks = 0x89D8,      // [6]
  kDialogVram = 0x89F6,        // [4]
  kQuitDialogBlocks = 0x8A1E,  // [2]
  kQuitDialogVram = 0x8A31,    // [2]
  kResetDialogAnswer = 0x8A35,
  kDrawDialogCursor = 0x8A3D,
  kDialogCursors = 0x8A57,  // [2 per dialog] x, y for each answer
  kConfirmDialogLoop = 0x8A63,
  kDrawDialogScreen = 0x8A88,
  kUnused2_8C01 = 0x8C01,
};

// In bank 6, which the far calls map at $8000-$BFFF.
enum {
  kLoadSceneChrBanks = 0x8070,
  kSceneChrBankTable = 0x80AC,  // [2 per scene] CHR banks R0 and R4
  kLoadScenePalette = 0x81DE,
  kLoadPalette = 0x81E7,
  kFar8000_63 = 0x81F7,
  kPalettePointer = 0x8213,
  kScenePaletteIds = 0x8223,  // [scenes] palette_table entry for each scene
  kPaletteTable = 0x82BC,     // [n] pointers to 32-byte palettes
  kDissolveRowMask = 0x9A50,
  kDissolvePatterns = 0x9A94,  // [3] pointers to each order's 6 x 14 row masks of 2 bytes
  kLayoutTable = 0xBC00,       // [n] a pointer to each screen layout
  kFar8000_6D = 0x804C,
  kSplitSprite = 0x806D,
  kUnreferenced81FE = 0x81FE,
  // Rooms, and what commands do outside the scripts.
  kSpecialCommandRoom1E = 0x8EE0,
  kSpecialCommandRoom20 = 0x8F21,
  kSpecialCommandRoom24 = 0x8F4A,
  kSpecialCommandRooms4A = 0x8F65,
  kSpecialCommandOtherRooms = 0x8F97,
  kRoomSceneList = 0x900C,
  kOtherRoomsRect = 0x901C,
  kRoomSceneLists = 0x9023,  // [rooms] a pointer to each room's list
  kCursorInRects933D = 0x9276,
  kOnPage04B0 = 0x92B2,
  kFar8040_64 = 0x92FC,
  kArrowRects = 0x933D,  // [2]
  kSelectRoomScene = 0x9345,
  kPickRoomVariant = 0x9367,
  kRoomScenes = 0x939B,        // [rooms] pointers to each room's scene by variant
  kRoomVariantSizes = 0x9522,  // [rooms] bytes of its list that pick the variant
  kFar8000_66 = 0x9947,
  kStepAddressChoice = 0x9994,
  kRedrawCursorAndSprites = 0x99B8,
  kPromptMessage2B = 0x99C2,  // only unreferenced_9847 calls it
  kFar8000_6F = 0x9C92,
  kFar8000_6C = 0x9CA9,
  kFar8000_67 = 0x9CBD,
  kFar8000_6E = 0x9CD4,
  kFar8020_6D = 0x9CEA,
  kFar8000_64 = 0x9D00,
  kPreviousPage = 0x9D10,
  kDrawPageStrings = 0x9D1F,
  kPageLabelsBack = 0x9D38,
  kFar8020_67 = 0x9D54,
  kNextPage = 0x9D6D,
  kPageLabelsOn = 0x9D7E,
  kDrawLabelString = 0x9D9A,
  kFixedLabels = 0x9DEF,
  kLabelStrings = 0x9EED,  // [n] pointers to each label's columns, ended by $FF
  kLabelColumns = 0x9F2D,  // [n] 18-byte blocks
  kDrawRoomTilePatches = 0x9FBD,
  kTilePatchHidden = 0x9FFA,
  kTilePatchHiddenAlt = 0xA00A,
  kTilePatchRoomRules = 0xA012,
  kTilePatchVramAddress = 0xA068,
  kTilePatchBlock = 0xA092,
  kDrawObjectSprite = 0xA098,
  kObjectSpritesLow = 0xA0D3,    // [2 per sprite] metasprite, attributes
  kMetaspritePointers = 0xA171,  // [n]
  kObjectSpritesHigh = 0xA1D3,   // [2 per sprite] the same, for sprites with bit 7
  kAddObjectSprites = 0xA6CC,
  kNextObjectSpriteRecord = 0xA6EF,
  kObjectSpriteTables = 0xA6FD,  // [5] by kind
  kObjectSpriteVisible = 0xA707,
  kObjectSpriteCondition = 0xA70F,
  kAppendObjectSprite = 0xA738,
  kSpriteValueIs0AOr16 = 0xA75B,
  kSpriteKind1 = 0xA774,
  kSpriteValueIs2C = 0xA7A6,
  kSpriteKind4 = 0xA7BB,
  kSpriteValueIs0Or2 = 0xA7DE,
  kSpriteKind2 = 0xA7F7,
  kDrawObjectSprites = 0xA808,
  kObjectSpriteData = 0xA825,
  kScriptSlotForCommand = 0xA96E,
  kScriptSlots = 0xA985,  // [11 per script table]
  kScriptTableIs0 = 0xA9C7,
  kFar8020_64 = 0xAA41,
  kSpecialRoomResponse = 0xAA98,
  kScriptTableIs0B = 0xAAC7,
  kDefaultResponses = 0xAAFC,  // [2 per command per script table] printer, and the next one's message
  // The inventory.
  kItemCodeInListAC53 = 0xAC53,
  kItemNumberForObject = 0xAC9C,
  kKindIs3 = 0xACAE,
  kGroupItems = 0xAD22,  // [7]
  kInventorySlotIndex = 0xAD60,
  kItemNumberBases = 0xAD70,  // [6] by script table
  kDrawInventoryPage = 0xAD76,
  kDrawAddressBook = 0xADA1,
  kDrawInventoryEntry = 0xADCC,
  kFar8020_6E = 0xAE2B,
  kInventoryPageHeader = 0xAE52,
  kEntryRows = 0xAE86,  // [7]
  kFar8020_69 = 0xAE8D,
  kDoorFlagsToSet = 0xAE9F,  // [$23]
  kDeleteInventoryEntry = 0xAEC2,
  kFar8020_6C = 0xAEE9,
  kUnreferencedAEF0 = 0xAEF0,
  // The slot machine.
  kPlaySlotMachine = 0xAF08,
  kSlotMachinePayout = 0xAF59,
  kSlotPayouts = 0xAF81,  // [8]
  kSpinAllReels = 0xAFA9,
  kStopReels = 0xAFC8,
  kSpinReel0 = 0xB032,
  kSpinReel1 = 0xB043,
  kSpinReel2 = 0xB054,
  kDrawReel0 = 0xB065,
  kDrawReel1 = 0xB069,
  kDrawReel2 = 0xB06D,
  kReelVramAddresses = 0xB0AA,  // [3]
  kReelAttributes = 0xB0B0,     // [8] by symbol
  kReelSymbols = 0xB0B8,        // [8] pointers to blocks
  kWait7Frames = 0xB0F8,
  // The logos and the ending.
  kOpeningLogos = 0xB10E,
  kBlackPaletteBuffer = 0xB147,
  kWait30FramesB6 = 0xB157,
  kLogoPaletteCycle1 = 0xB162,
  kLogoColours1 = 0xB184,  // [5 x 3]
  kLogoPaletteCycle2 = 0xB193,
  kLogoColours2 = 0xB1B5,  // [5 x 3]
  kWait6Frames = 0xB1C4,
  kDrawLogoScreen = 0xB1D7,
  kEndingSequence = 0xB26E,
  kEndingWaitButton = 0xB2C9,
  kEndingPromptColours = 0xB312,  // [4]
  kWait6Frames2 = 0xB319,
  kEndingSetup = 0xB32C,
  kEndingSprite0 = 0xB371,  // [4]
  kDrawEndingBox = 0xB395,
  kEndingBoxBlock = 0xB3AC,
  kDrawEndingScreen = 0xB3C2,
  kUnused6B5FD = 0xB5FD,
  // The cursor and the commands.
  kClickAtCursor = 0x8D3A,
  kFindScreenArea = 0x8D4D,
  kNextScreenRect = 0x8D6B,
  kScreenAreaRects = 0x8D79,  // [3] x1 x2 y1 y2: picture, commands, inventory
  kClickCommandArea = 0x8D85,
  kCommandRects = 0x8E2D,     // [14] the commands' rectangles
  kClickInventoryArea = 0x8E65,
  kEntryRects = 0x8EC4,       // [7] the inventory entries' rectangles
  kSpecialCommandByRoom = 0x8EFF,
  kClearSelectionUnlessWaiting = 0x9257,
  kResetCursor = 0x9570,
  kHandleInput = 0x9585,
  kGetInput = 0x958D,
  kMoveCursor = 0x959B,
  kReadInput = 0x95A8,
  kReadInputFree = 0x95BC,
  kReadInputWithRepeat = 0x95D1,
  kResetKeyRepeat = 0x960F,
  kDpadToDelta = 0x961A,
  kDpadDx = 0x962A,  // [16] by the d-pad's 4 bits
  kDpadDy = 0x963A,
  kMoveCursorInScene = 0x964A,
  kMoveCursorInInventory = 0x9692,
  kSnapCursorToCommand = 0x96E7,
  kCommandCursorXs = 0x96F4,  // [20] where the cursor sits on each command
  kCommandCursorYs = 0x9708,
  kMoveCursorX = 0x971C,
  kMoveCursorY = 0x9724,
  kMoveCursorInCommands = 0x972C,
  kPictureCursorXs = 0x975A,  // [15] where the cursor goes in the picture, by command
  kPictureCursorYs = 0x9769,
  kCommandGridPosition = 0x9789,
  kCommandGridXs = 0x9796,  // [20] each command's column and row in the grid
  kCommandGridYs = 0x97AA,
  kMoveInCommandGrid = 0x97BE,
  kCommandAtGrid = 0x97CD,
  kCommandGrid = 0x97E3,  // [7 per row] the command at each place: $20 picture, $30 inventory
  kHandleButtons = 0x983E,
  kUnreferenced9847 = 0x9847,
  kPressB = 0x9895,
  kPressA = 0x98B7,
  kDrawCursor = 0x9922,
  kCursorSprite = 0x993D,
  kHighlightCommand = 0xAB7F,
  kDrawCommandHighlight = 0xAB8A,
  kUnhighlightCommand = 0xAB9C,
  kCommandVramAddress = 0xABB5,
  kClearEntryHighlight = 0xABC4,
  kHighlightEntry = 0xABDD,
  kInventoryEntryVramAddress = 0xABE8,
  kHighlightBlocks = 0xABF8,          // [3] one-tile blocks: lit, unlit command, unlit entry
  kCommandVramAddresses = 0xAC01,     // [13] where each command's marker tile is
  kEntryVramAddresses = 0xAC1B,       // [7] the same for the inventory entries
  kStoreInInventorySlot = 0xAC29,
  kCheckInventorySlotEmpty = 0xAD29,
};

// At the top of a ported routine that returns nothing, with the inputs in the registers: runs
// the original instead when the reference's next NMI is a lag frame inside it (emu_lag_in).
#define RUN_ORIGINAL_IF_LAG(addr) \
  do {                            \
    if(emu_lag_in(addr)) {        \
      emu_call_original(addr);    \
      return;                     \
    }                             \
  } while(0)

// In a ported routine after a call that may have waited, where the original is at `resume`
// and has `pushed` bytes on the stack: runs the rest as original code if the reference's next
// NMI is a lag frame in `routine` (emu_resume_original).
#define RESUME_ORIGINAL_IF_LAG(routine, resume, pushed) \
  do {                                                  \
    if(emu_lag_in(routine)) {                           \
      emu_resume_original(resume, pushed);              \
      return;                                           \
    }                                                   \
  } while(0)

// In a ported routine, where the original jumps or branches to `addr` in the original code of
// `routine` (a tail they share, at a label), with the registers as the original has them there:
// runs the rest as original code from there if the reference's next NMI is a lag frame in
// `routine`, which the C code is about to run inline.
#define TAIL_ORIGINAL_IF_LAG(routine, addr, pushed) \
  do {                                              \
    if(emu_lag_in(routine)) {                       \
      emu_resume_original(addr, pushed);            \
      return;                                       \
    }                                               \
  } while(0)

// In a ported routine at its JSR jump_table (at `jsr`, with `count` addresses after it), with A and
// the flags as the original has them there: runs the rest as original code from that JSR if
// the reference's next NMI is a lag frame in jump_table (which C calls without a guard of its
// own), or if `index` is past the list (a made-up state: the original jumps wherever the bytes
// after it say). Otherwise the routine goes on with jump_table(jsr + 2, index).
#define JUMP_TABLE_OR_ORIGINAL(routine, jsr, pushed, index, count) \
  do {                                                             \
    if((index) >= (count)) {                                       \
      emu_note_bad_index();                                        \
    }                                                              \
    if((index) >= (count) || emu_lag_in(kJumpTable)) {             \
      emu_resume_original(jsr, pushed);                            \
      return;                                                      \
    }                                                              \
  } while(0)

// At a JSR set_chr_banks_inline in a ported routine, at `jsr` with its 8 bytes after it: as the
// original does, or from there as original code if the reference's next NMI is a lag frame in it.
#define SET_CHR_BANKS_INLINE(jsr, pushed)      \
  do {                                         \
    if(emu_lag_in(kSetChrBanksInline)) {       \
      emu_resume_original(jsr, pushed);        \
      return;                                  \
    }                                          \
    emu_push(((jsr) + 2) >> 8);                \
    emu_push(((jsr) + 2) & 0xFF);              \
    set_chr_banks_inline();                    \
  } while(0)

// Powers on the console with the ROM's PRG and CHR (the caller keeps them alive) and starts the game's
// coroutine, for emu_run_frame, with no reference. Returns 0, or -1 if it can't.
int dv_boot(const uint8_t *prg, const uint8_t *chr);

// Fixes the original's bugs: the check of the save files at the title (check_save_file,
// repair_save_copies; docs/saves.md) and the capsule warp (show_item_page_keep_cursor; docs/rooms.md).
// Off unless the settings' FixBugs turns it on (main.c), and always off in the harness and the tests.
extern int g_fix_bugs;

// The sound driver (sound.c).
void sound_reset(void);
void play_sound(uint8_t sound);
void sound_check_repeat(void);
void start_sound(void);
void sound_start_by_type(void);
void sound_start_music(void);
void sound_start_channel(void);
void sound_start_repeating_music(void);
void sound_start_effect_3ch(void);
void sound_start_effect_2ch(void);
void sound_update(void);
void sound_update_all(void);
void sound_step_all_channels(void);
void sound_step_pulse(void);
void sound_pulse_envelope(void);
void sound_step_triangle(void);
void multiply_8x8(void);
void sound_triangle_countdown(void);
void sound_step_noise(void);
void sound_tick(void);
void sound_add_length(void);
void sound_read_command(void);
void sound_loop_push(void);
void sound_loop_pop(void);
void sound_note_off(void);
void sound_channel_done(void);
void sound_write_apu(void);

// bank_calls.c: PRG and CHR banks, the bank stack, far calls.
void set_mirroring_vertical(void);
void switch_prg_bank(uint8_t bank);
void mmc3_set_bank(uint8_t reg, uint8_t bank);
void apply_chr_banks_r0_r1(void);
void apply_chr_banks_r2_r5(void);
void apply_chr_banks(void);
void advance_inline_ptr(void);
uint8_t read_inline_byte(void);
void push_and_switch_bank(uint8_t bank);
void set_current_bank(uint8_t bank);
void switch_to_sound_bank(void);
void restore_current_bank(void);
void pop_bank(void);
void stack_push(uint8_t v);
uint8_t stack_pop(void);
uint16_t jump_table(uint16_t pushed, uint8_t index);
void far_call_8000(uint8_t id);
void far_call_8020(uint8_t id);
void far_call_8040(uint8_t id);
void far_call_8400(uint8_t id);
uint16_t set_chr_banks_inline(void);

// dv_rtl.c: waiting for the NMI, where the game yields.
void wait_vblank(void);
void wait_vram_queue_empty(void);
void wait_nmi_work_done(void);

// input.c
void read_joypads(void);
void read_joypads_once(void);

// ppu_upload.c: sprites, and work asked of the NMI.
void disable_nmi(void);
void enable_nmi(void);
void request_nmi_work(uint8_t bits);
void wait_vblank_and_hide_sprites(void);
void clear_oam_buffer(void);
void clear_and_upload_sprites(void);
void request_oam_dma(void);
void request_vram_queue(void);
void request_attrs_2000(void);
void request_attr_upload(void);
void request_black_palette(void);
void request_palette_upload(void);
void show_bg_and_sprites(void);
void hide_bg_and_sprites(void);

// The original's 16-bit add on a pointer in zero page (lda lo, clc, adc #n, sta lo, lda hi,
// adc #0, sta hi): leaves A = hi and the last ADC's flags.
void add16(uint8_t *p, uint8_t n);

// ASL A, with N, Z and C.
uint8_t asl_a(uint8_t a);

// vram_queue.c: the VRAM queue and the attribute buffers.
void queue_vram_block(void);
void queue_vram_fill(void);
void vram_block_setup(void);
void vram_fill_rows(void);
void vram_copy_rows(void);
void next_vram_address(void);
void vram_wrap_up(void);
void vram_queue_header(void);
void next_vram_row(void);
void skip_block_header(void);
void flush_vram_queue_if_full(void);
void flush_vram_queue_down_if_full(void);
void flush_vram_queue_down(void);
void queue_vram_column(void);
void vram_column_setup(void);
void vram_copy_column(void);
void vram_addr_down_row(void);
void vram_wrap_column(void);
void attr_position(void);
void set_attr_block(void);
void next_attr_row(void);
void next_attr_column(void);
void set_attr_bits(void);
void fill_all_attrs(uint8_t palette);
void pick_attr_fill(uint8_t palette);
void fill_attr_buffer_2000(void);
void fill_attr_buffer_2400(void);
void restore_xy(void);

// screens.c: whole nametables, written with rendering off.
void clear_nametables(uint8_t tile);
void rendering_on(void);
void rendering_off(uint8_t tile);
void fill_nametable_2000(void);
void fill_nametable_2nd(void);
void rle_write_2000(uint8_t v);
void rle_to_ppu_2000(void);
uint16_t draw_rle_screen_2000(void);  // for original code: the data follows its JSR
void rle_screen_from(uint16_t routine);  // a routine that is JSR draw_rle_screen_2000 and its data

// palette.c: fades.
void fade_in_palette(void);
void palette_brighten_step(void);
void fade_out_palette(void);
void palette_darken_step(void);
void palette_fade_wait(void);

// text.c: messages, and the text window.
void print_message_5(uint8_t id);
void print_message_from(void);
void print_message_4(uint8_t id);
void print_message_2_or_5(void);
void print_message(void);
void print_control(uint8_t code);
void text_read_code(void);
void text_read_byte(void);
void text_read_bit(void);
void print_item_name_at(void);
void print_item_name_arg(void);
void print_word_arg(void);
void print_name(void);
void print_number_at(void);
void print_digit_skip_zero(void);
void print_digit(uint8_t digit);
void number_to_digits(void);
void read_number_arg(void);
void skip_inline_word(void);
void text_wait_page(void);
void text_newline(void);
void print_clear(void);
void clear_text_window(void);
void blank_text_window(void);
void print_tile(void);
void print_char(void);
void put_tile(void);
void text_line_feed(void);
void text_scroll(void);
void text_scroll_rows(uint8_t fill);
void text_cursor_vram_address(void);
void upload_text_window(void);
void draw_window_rule_row(void);
void blank_window_text_rows(void);
void draw_text_window_sides(void);
void draw_window_side_column(void);
void text_pause(void);
void wait_page_input(void);
void blink_more_arrow(void);
void show_more_arrow(void);
void wait_for_more_button(void);
void draw_more_arrow_frame(void);
void text_cursor_x_pixels(void);
void draw_more_arrow(void);

// scenes.c: the location picture.
void change_scene(void);
void change_scene_instant(void);
void load_scene(void);
void select_scene_bank(void);
void scene_pointer(void);
void load_scene_attrs(void);
void blacken_palette_colors(void);
void apply_room_settings(void);
void load_dissolve_pattern(void);
void draw_scene(void);
void copy_scene_tiles(void);
void draw_scene_dissolve(void);
void dissolve_draw_tile(void);
void dissolve_tile_value(void);
void dissolve_tile_address(void);
void load_scene_chr_banks(void);  // bank 6
void load_scene_palette(void);
void load_palette(void);
void palette_pointer(void);
void dissolve_row_mask(void);

// commands.c: the cursor, the buttons and the command and inventory areas (bank 6, but for
// test_hotspot and cursor_in_hotspot).
void reset_cursor(void);
void handle_input(void);
void get_input(void);
void move_cursor(void);
void read_input(void);
void read_input_free(void);
void read_input_with_repeat(void);
void reset_key_repeat(void);
void dpad_to_delta(void);
void move_cursor_in_scene(void);
void move_cursor_in_inventory(void);
void snap_cursor_to_command(void);
void move_cursor_x(void);
void move_cursor_y(void);
void move_cursor_in_commands(void);
void command_grid_position(void);
void move_in_command_grid(void);
void command_at_grid(void);
void draw_cursor(void);
void handle_buttons(void);
void press_a(void);
void press_b(void);
void click_at_cursor(void);
void find_screen_area(void);
void next_screen_rect(void);
void test_hotspot(void);
void cursor_in_hotspot(void);
void click_command_area(void);
void click_inventory_area(void);
void highlight_command(void);
void draw_command_highlight(void);
void unhighlight_command(void);
void command_vram_address(void);
void clear_entry_highlight(void);
void highlight_entry(void);
void inventory_entry_vram_address(void);

// scripts.c and flags.c: the script interpreter and the flag opcodes.
void run_command(void);
void script_resume(void);
void script_start(void);
void script_loop(void);
void script_fetch_byte(void);
void script_advance(void);
void script_fetch_2(void);
void script_fetch_4(void);
void script_op_switch(void);
void script_op_yield(void);
void op_00_message_5(void);
void op_01_message_4(void);
void op_02_message_2_or_5(void);
void script_op_goto_if(void);
void script_op_goto_if_not(void);
void script_goto(void);
void script_op_push_goto(void);
void script_op_pop_goto(void);
void script_op_goto(void);
void script_condition_true(void);
void script_condition_false(void);
void op_52_test_story_flag(void);
void op_07_test_story_flag_of_object(void);
void test_story_flag(uint8_t bit);
void op_08_set_story_flag_of_object(void);
void set_story_flag_refresh(uint8_t bit);
void op_09_clear_story_flag_of_object(void);
void clear_story_flag(uint8_t bit);
void select_story_flags(uint8_t bit);
void test_flag_bit(void);
void load_flag_byte(void);
void op_5D(void);
void op_0A(void);
void test_item_obtained(uint8_t bit);
void op_0B(void);
void op_0C(void);
void clear_item_obtained(uint8_t bit);
void op_0D(void);
void op_2B(void);
void test_object_open(uint8_t bit);
void op_0E(void);
void op_2C(void);
void set_object_open(uint8_t bit);
void op_0F(void);
void op_2D(void);
void close_object(uint8_t bit);
void op_10(void);
void op_11(void);
void op_12(void);
void clear_item_open(uint8_t bit);
void op_13_test_door_flag_of_object(void);
void op_2E_test_door_flag(void);
void test_door_flag(uint8_t bit);
void op_14_set_door_flag_of_object(void);
void op_2F_set_door_flag(void);
void set_door_flag(uint8_t bit);
void op_15_clear_door_flag_of_object(void);
void op_30_clear_door_flag(void);
void clear_door_flag(uint8_t bit);
void select_door_flags(uint8_t bit);
void op_16_test_event_flag(void);
void test_event_flag(uint8_t bit);
void op_17_set_event_flag(void);
void set_event_flag(uint8_t bit);
void op_18_clear_event_flag(void);
void clear_event_flag(uint8_t bit);
void select_event_flags(uint8_t bit);
void op_19(void);
void op_20_test_flag_045C_of_object(void);
void test_flag_045C(uint8_t bit);
void op_53_set_flag_045C(void);
void op_21_set_flag_045C_of_object(void);
void set_flag_045C(uint8_t bit);
void op_22_clear_flag_045C_of_object(void);
void clear_flag_045C(uint8_t bit);
void select_flags_045C(uint8_t bit);
void op_23_test_flag_047A_of_object(void);
void test_flag_047A(uint8_t bit);
void op_54_set_flag_047A(void);
void op_24_set_flag_047A_of_object(void);
void set_flag_047A(uint8_t bit);
void op_25_clear_flag_047A_of_object(void);
void select_flags_047A(uint8_t bit);
void op_48_yield_b5_80(void);

// opcodes.c: the other script opcodes, and what they use.
void end_script(void);  // in scripts.c
void next_hotspot(void);
void op_68(void);
void op_67(void);
void op_64(void);
void op_63(void);
void show_found_page_next(void);
void op_61(void);
void op_60(void);
void op_5F(void);
void op_31(void);
void choice_is_table_2(void);
void op_32(void);
void op_32_body(void);
void op_5B(void);
void close_container_entry(uint8_t arg);
void close_all_containers(void);
void clear_listed_story_flags(void);
void op_06(void);
void enter_room(uint8_t arg);
void op_33(void);
void op_36(void);
void op_1A(void);
void op_34(void);
void op_37(void);
void op_1B(void);
void op_1D(void);
void op_1D_body(void);
void take_item_from_container(void);
void find_item_in_container(void);
void select_store_slot(void);
void wait_frames(uint8_t arg);
void op_1E(void);
void op_1F(void);
void op_26(void);
void open_container(void);
void load_container_view(void);
void select_item_list(void);
void fill_coat_from_item_list(void);
void refresh_coat_page(void);
void item_list_entry_visible(void);
void item_list_entry_condition(void);
void clear_coat_slots(void);
void copy_container_to_coat(void);
void op_27(void);
void op_2A(void);
void op_35(void);
void op_38(void);
void op_39(void);
void add_coins(void);
void op_62(void);
void add_coins_and_show(void);
void op_56(void);
void op_3A(void);
void op_57(void);
void op_3B(void);
void op_3C(void);
void op_3D(void);
void op_3E(void);
void op_40(void);
void show_found_page(void);
void op_3F(void);
void op_41(void);
void op_44(void);
void op_45(void);
void op_43(void);
void op_42(void);
void op_46_special_scene(void);
void op_47(void);
void close_container(void);
void op_49(void);
void previous_store_entry(void);
void op_4A(void);
void op_4B(void);
void op_4C(void);
void op_4D(void);
void op_4E(void);
void op_4F(void);
void op_50(void);
void op_51(void);
void op_58(void);
void op_59(void);
void op_55(void);
void find_and_show_item(uint8_t arg);
void find_in_goods(void);
void show_item_page_keep_cursor(void);
void restore_inventory_cursor(void);
void show_page_then_op_49(uint8_t arg);
void op_5A(void);
void op_5C(void);
void op_5E(void);
void op_5E_special_check(void);
void op_5E_special_check_body(void);

// rooms.c: a room's hotspots, and the item codes.
void click_picture(void);
void find_clicked_hotspot(void);
void pick_default_command(void);
void collect_room_sprites(void);
void item_code_for_object(void);
void split_item_code(uint8_t code);

// special_scenes.c: the special scenes, and the waits they use.
void wait_180_frames(void);
void wait_60_frames(void);
void wait_30_frames(void);
void run_special_scene(void);
void enter_special_scene(void);
void show_special_scene(void);
void leave_special_scene(void);
void special_scene_action(void);
void special_scene_with_message(void);
void special_scene_final(void);
void set_special_scene_flag(void);
void clear_special_scene_flag(void);
void special_scene_message(void);
void special_scene_extras(void);

// saves.c: the save system.
void load_game(void);
void save_game(void);
void erase_save_file(void);
void check_all_save_files(void);
void check_save_file(void);
void repair_save_copies(void);
void read_save_copy(void);
void write_save_copy(void);
void save_copy_address(void);
void shift_save_copies(void);
void next_copy_pointer(void);
void copy_save_copy(void);
void erase_save_copies(void);
void erase_save_copy(void);
void clear_sum16(void);
void add_to_sum16(uint8_t v);
void store_checksum(void);
void check_checksum(void);
void check_signature(void);
void store_signature(void);
void unlock_save_ram(void);
void protect_save_ram(void);
void disable_save_ram(void);
void reset_game_state(void);
void backup_state_to_save_ram(void);
void restore_state_backup(void);

// layouts.c: screen layouts, the numbers they print, and metasprites.
void draw_layout_number(uint8_t number);
void layout_run(void);
void layout_reset_state(void);
void layout_fetch(void);
void layout_advance_1(void);
void layout_advance_2(void);
void layout_advance_3(void);
void layout_put_tile(void);
void layout_command(void);
void layout_name_by_index_command(void);
void layout_name_at_address_command(void);
void layout_name_in_ram_command(void);
void layout_byte_padded_command(void);
void layout_word_padded_command(void);
void layout_byte_command(void);
void layout_word_command(void);
void layout_position_command(void);
void layout_name_from_ram(void);
void layout_name_from_index(void);
void layout_name_from_address(void);
void copy_name_to_buffer(void);
void layout_print_name_buffer(void);
void layout_advance_x(void);
void layout_set_vram_dest(void);
void layout_skip_5(void);
void layout_skip_3(void);
void layout_draw_tile(void);
void layout_tile_position(void);
void tile_position_to_vram(void);
void layout_tile_kind(void);
void queue_tile(void);
void layout_print_byte_padded(void);
void layout_print_byte(void);
void layout_print_word_padded(void);
void layout_print_word(void);
void load_byte_value(void);
void load_word_value(void);
void decimal_digits(void);
void trim_leading_zeros(void);
void digits_to_tiles_padded(void);
void digits_to_tiles(void);
void print_decimal_digits(void);
void draw_metasprite(void);

// room_view.c: which scene a room shows, its tile patches and object sprites (bank 6).
void room_scene_list(void);
void select_room_scene(void);
void pick_room_variant(void);
void draw_room_tile_patches(void);
void tile_patch_hidden(void);
void tile_patch_hidden_alt(void);
void tile_patch_room_rules(void);
void tile_patch_vram_address(uint8_t place);
void draw_object_sprites(void);
void draw_object_sprite(uint8_t sprite);
void add_object_sprites(void);
void next_object_sprite_record(void);
void append_object_sprite(void);
void sprite_value_is_0A_or_16(void);
void sprite_value_is_2C(void);
void sprite_value_is_0_or_2(void);
void object_sprite_visible(void);

// room_commands.c: what commands do outside the scripts, and paging through the inventory (bank 6).
void script_slot_for_command(void);
void far_8020_64(void);
void script_table_is_0(void);
void script_table_is_0_b(void);
void special_command_room_1E(void);
void special_command_room_20(void);
void special_command_room_24(void);
void special_command_rooms_4A(void);
void special_command_by_room(void);
void special_command_other_rooms(void);
void special_room_response(void);
void clear_selection_unless_waiting(void);
void on_page_04B0(void);
void cursor_in_rects_933D(void);
void far_8040_64(void);
void far_8000_66(void);
void step_address_choice(void);
void redraw_cursor_and_sprites(void);

// inventory.c: the inventory area (bank 6).
void store_in_inventory_slot(void);
void item_code_in_list_AC53(void);
void item_number_for_object(void);
void kind_is_3(void);
void check_inventory_slot_empty(void);
void inventory_slot_index(void);
void draw_inventory_page(void);
void draw_address_book(void);
void draw_inventory_entry(void);
void far_8020_6E(void);
void inventory_page_header(void);
void far_8000_6E(void);
void far_8020_6D(void);
void far_8000_64(void);
void previous_page(void);
void draw_page_strings(void);
void far_8020_67(void);
void next_page(void);
void draw_label_string(void);
void far_8000_6F(void);
void far_8000_6C(void);
void far_8000_67(void);
void far_8020_69(void);
void delete_inventory_entry(void);
void far_8020_6C(void);
void far_8000_6D(void);
void far_8000_63(void);

// slot_machine.c: the slot machine, the logos and the ending (bank 6).
void play_slot_machine(void);
void slot_machine_payout(void);
void spin_all_reels(void);
void stop_reels(void);
void spin_reel_0(void);
void spin_reel_1(void);
void spin_reel_2(void);
void draw_reel_0(void);
void draw_reel_1(void);
void draw_reel_2(void);
void wait_7_frames(void);
void opening_logos(void);
void black_palette_buffer(void);
void wait_30_frames_b6(void);
void logo_palette_cycle_1(void);
void logo_palette_cycle_2(void);
void wait_6_frames(void);
void draw_logo_screen(void);
void ending_sequence(void);
void ending_wait_button(void);
void wait_6_frames_2(void);
void ending_setup(void);
void draw_ending_box(void);
void draw_ending_screen(void);

// menus.c: the title, the file select screen and the dialogs (bank 2).
void draw_title_picture(void);
void draw_dialog_screen(void);
void title_screen(void);
void file_select_menu(void);
void file_select_screen(void);
void draw_file_list(void);
void draw_file_entry(void);
void file_entry_setup(void);
void draw_file_status(void);
void draw_file_options(void);
void clear_file_area(void);
void fill_file_area(void);
void draw_file_cursor(void);
void draw_option_cursor(void);
void show_file_options(void);
void quit_dialog(void);
void save_dialog(void);
void open_confirm_dialog(void);
void draw_dialog(void);
void draw_dialog_text(void);
void reset_dialog_answer(void);
void draw_dialog_cursor(void);
void confirm_dialog_loop(void);

// nmi.c: the NMI handler.
void nmi(void);
void nmi_ppu_updates(void);
void nmi_upload_attrs_2000(void);
void nmi_upload_attrs_2nd(void);
void nmi_flush_vram_queue(void);
void nmi_flush_vram_queue_down(void);
void flush_vram_queue(void);
void nmi_black_palette(void);
void nmi_upload_palette(void);
void ppu_addr_leave_palette(void);
void ppu_addr_palette(void);
void nmi_set_scroll(void);

// game.c
uint16_t game_reset(void);  // reset; returns where the game carries on (main)
uint16_t game_main(void);  // main; returns where the game carries on (reset)
void draw_game_screen(void);
void update_cursor_and_sprites(void);
void load_current_scene(void);
void show_intro_once(void);
void new_game(void);
void next_random(void);

// The ported routines, for emu_set_hooks: the C side runs them in place of the original code.
extern const EmuHook kPortedRoutines[];
extern const size_t kPortedRoutineCount;

#endif  // DV_FUNCS_H
