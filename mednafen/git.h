#ifndef _GIT_H
#define _GIT_H

#include <algorithm>
#include <string>
#include <vector>
#include <libretro.h>

#include "video.h"
#include "state.h"
#include "settings-common.h"

#if 0
template<typename T> void showtype(T foo);
#endif

typedef struct
{
   const char *extension;
   const char *description;
} FileExtensionSpecStruct;

enum
{
   MDFN_ROTATE0 = 0,
   MDFN_ROTATE90,
   MDFN_ROTATE180,
   MDFN_ROTATE270
};

typedef enum
{
   VIDSYS_NONE,
   VIDSYS_PAL,
   VIDSYS_PAL_M,
   VIDSYS_NTSC,
   VIDSYS_SECAM
} VideoSystems;

typedef enum
{
   GMT_CART,
   GMT_ARCADE,
   GMT_DISK,
   GMT_CDROM,
   GMT_PLAYER
} GameMediumTypes;

typedef enum
{
   IDIT_BUTTON,
   IDIT_BUTTON_CAN_RAPID,

   IDIT_SWITCH,		/* ceil(log2(n))-bit */
   IDIT_STATUS,		/* ceil(log2(n))-bit */

   IDIT_X_AXIS,
   IDIT_Y_AXIS,
   IDIT_X_AXIS_REL,
   IDIT_Y_AXIS_REL,

   IDIT_BYTE_SPECIAL,
   IDIT_RESET_BUTTON,	/* 1-bit */

   IDIT_BUTTON_ANALOG,
   IDIT_RUMBLE
} InputDeviceInputType;

#define IDIT_BUTTON_ANALOG_FLAG_SQLR	0x01	// Denotes analog data that may need to be scaled to ensure a more squareish logical range(for emulated analog sticks).
#define IDIT_FLAG_AUX_SETTINGS_UNDOC	0x80

struct IDIIS_StatusState
{
	const char* ShortName;
	const char* Name;
	int32 Color;	// (msb)0RGB(lsb), -1 for unused.
};

struct IDIIS_SwitchPos
{
	const char* SettingName;
	const char* Name;
	const char* Description;
};

struct InputDeviceInputInfoStruct
{
	const char *SettingName;	// No spaces, shouldbe all a-z0-9 and _. Definitely no ~!
	const char *Name;
        int ConfigOrder;          // Configuration order during in-game config process, -1 for no config.
	InputDeviceInputType Type;
	const char *ExcludeName;	// SettingName of a button that can't be pressed at the same time as this button
					// due to physical limitations.

	const char *RotateName[3];	// 90, 180, 270
	uint8 Flags;
	uint8 BitSize;
	uint16 BitOffset;

	union
	{
         struct
         {
	  const IDIIS_SwitchPos* Pos;
	  uint32 NumPos;
         } Switch;

	 struct
	 {
	  const IDIIS_StatusState* States;
	  uint32 NumStates;
	 } Status;
	};
};

struct IDIISG : public std::vector<InputDeviceInputInfoStruct>
{
 IDIISG();
 IDIISG(std::initializer_list<InputDeviceInputInfoStruct> l);
 uint32 InputByteSize;
};

extern const IDIISG IDII_Empty;

struct IDIIS_Switch : public InputDeviceInputInfoStruct
{
	IDIIS_Switch(const char* sname, const char* name, int co, const IDIIS_SwitchPos* spn, const uint32 spn_num, bool undoc_defpos = true)
	{
	 SettingName = sname;
	 Name = name;
	 ConfigOrder = co;
	 Type = IDIT_SWITCH;

	 ExcludeName = NULL;
	 Flags = undoc_defpos ? IDIT_FLAG_AUX_SETTINGS_UNDOC : 0;
	 Switch.Pos = spn;
	 Switch.NumPos = spn_num;
	}
};

struct IDIIS_Status : public InputDeviceInputInfoStruct
{
	IDIIS_Status(const char* sname, const char* name, const IDIIS_StatusState* ss, const uint32 ss_num)
	{
	 SettingName = sname;
	 Name = name;
	 ConfigOrder = -1;
	 Type = IDIT_STATUS;

	 ExcludeName = NULL;
	 RotateName[0] = RotateName[1] = RotateName[2] = NULL;
	 Flags = 0;
	 Status.States = ss;
	 Status.NumStates = ss_num;
	}
};

struct InputDeviceInfoStruct
{
 const char *ShortName;
 const char *FullName;
 const char *Description;

 const IDIISG& IDII;

 unsigned Flags;

 enum
 {
  FLAG_KEYBOARD = (1U << 0)
 };
};

struct InputPortInfoStruct
{
 const char *ShortName;
 const char *FullName;
 const std::vector<InputDeviceInfoStruct> &DeviceInfo;
 const char *DefaultDevice;	// Default device for this port.
};

typedef struct
{
   int InputPorts;
   const InputPortInfoStruct *Types;
} InputInfoStruct;

struct MemoryPatch;

struct CheatFormatStruct
{
   const char *FullName;
   const char *Description;

   bool (*DecodeCheat)(const std::string& cheat_string, MemoryPatch* patch);
};

struct CheatFormatInfoStruct
{
   unsigned NumFormats;

   CheatFormatStruct *Formats;
};

extern const std::vector<CheatFormatStruct> CheatFormatInfo_Empty;

struct CheatInfoStruct
{
 //
 // InstallReadPatch and RemoveReadPatches should be non-NULL(even if only pointing to dummy functions) if the emulator module supports
 // read-substitution and read-substitution-with-compare style(IE Game Genie-style) cheats.
 //
 // See also "SubCheats" global stuff in mempatcher.h.
 //
 void (*InstallReadPatch)(uint32 address, uint8 value, int compare); // Compare is >= 0 when utilized.
 void (*RemoveReadPatches)(void);
 uint8 (*MemRead)(uint32 addr);
 void (*MemWrite)(uint32 addr, uint8 val);

 const std::vector<CheatFormatStruct>& CheatFormatInfo;

 bool BigEndian;	// UI default for cheat search and new cheats.
};

enum
{
   MDFN_MSC_RESET = 0x01,
   MDFN_MSC_POWER = 0x02,

   MDFN_MSC_INSERT_COIN = 0x07,

   MDFN_MSC_TOGGLE_DIP0 = 0x10,
   MDFN_MSC_TOGGLE_DIP1,
   MDFN_MSC_TOGGLE_DIP2,
   MDFN_MSC_TOGGLE_DIP3,
   MDFN_MSC_TOGGLE_DIP4,
   MDFN_MSC_TOGGLE_DIP5,
   MDFN_MSC_TOGGLE_DIP6,
   MDFN_MSC_TOGGLE_DIP7,
   MDFN_MSC_TOGGLE_DIP8,
   MDFN_MSC_TOGGLE_DIP9,
   MDFN_MSC_TOGGLE_DIP10,
   MDFN_MSC_TOGGLE_DIP11,
   MDFN_MSC_TOGGLE_DIP12,
   MDFN_MSC_TOGGLE_DIP13,
   MDFN_MSC_TOGGLE_DIP14,
   MDFN_MSC_TOGGLE_DIP15,


   // n of DISKn translates to is emulation module specific.
   MDFN_MSC_INSERT_DISK0 = 0x20,
   MDFN_MSC_INSERT_DISK1,
   MDFN_MSC_INSERT_DISK2,
   MDFN_MSC_INSERT_DISK3,
   MDFN_MSC_INSERT_DISK4,
   MDFN_MSC_INSERT_DISK5,
   MDFN_MSC_INSERT_DISK6,
   MDFN_MSC_INSERT_DISK7,
   MDFN_MSC_INSERT_DISK8,
   MDFN_MSC_INSERT_DISK9,
   MDFN_MSC_INSERT_DISK10,
   MDFN_MSC_INSERT_DISK11,
   MDFN_MSC_INSERT_DISK12,
   MDFN_MSC_INSERT_DISK13,
   MDFN_MSC_INSERT_DISK14,
   MDFN_MSC_INSERT_DISK15,

   MDFN_MSC_INSERT_DISK	= 0x30,
   MDFN_MSC_EJECT_DISK 	= 0x31,

   MDFN_MSC_SELECT_DISK	= 0x32,

   MDFN_MSC__LAST = 0x3F
};

struct RMD_Media
{
 std::string Name;
 unsigned MediaType;	// Index into RMD_Layout::MediaTypes
 std::vector<std::string> Orientations;	// The vector may be empty.
};

struct RMD_MediaType
{
 std::string Name;
};

struct RMD_State
{
 std::string Name;

 bool MediaPresent;
 bool MediaUsable;	// Usually the same as MediaPresent.
 bool MediaCanChange;
};

struct RMD_Drive
{
 std::string Name;

 std::vector<RMD_State> PossibleStates;	// Ideally, only one state will have MediaPresent == true.
 std::vector<unsigned> CompatibleMedia;	// Indexes into RMD_Layout::MediaTypes
 unsigned MediaMtoPDelay;		// Recommended minimum delay, in milliseconds, between a MediaPresent == false state and a MediaPresent == true state; to be enforced
					// by the media changing user interface.
};

struct RMD_Layout
{
 std::vector<RMD_Drive> Drives;
 std::vector<RMD_MediaType> MediaTypes;
 std::vector<RMD_Media> Media;
};

typedef struct
{
   // Pitch(32-bit) must be equal to width and >= the "fb_width" specified in the MDFNGI struct for the emulated system.
   // Height must be >= to the "fb_height" specified in the MDFNGI struct for the emulated system.
   // The framebuffer pointed to by surface->pixels is written to by the system emulation code.
   MDFN_Surface *surface;

   // Will be set to TRUE if the video pixel format has changed since the last call to Emulate(), FALSE otherwise.
   // Will be set to TRUE on the first call to the Emulate() function/method
   bool VideoFormatChanged;

   // Set by the system emulation code every frame, to denote the horizontal and vertical offsets of the image, and the size
   // of the image.  If the emulated system sets the elements of LineWidths, then the horizontal offset(x) and width(w) of this structure
   // are ignored while drawing the image.
   MDFN_Rect DisplayRect;

   // Pointer to an array of MDFN_Rect, number of elements = fb_height, set by the driver code.  Individual MDFN_Rect structs written
   // to by system emulation code.  If the emulated system doesn't support multiple screen widths per frame, or if you handle
   // such a situation by outputting at a constant width-per-frame that is the least-common-multiple of the screen widths, then
   // you can ignore this.  If you do wish to use this, you must set all elements every frame.
   int32_t *LineWidths;

   // TODO
   bool *IsFMV;

   // Set(optionally) by emulation code.  If InterlaceOn is true, then assume field height is 1/2 DisplayRect.h, and
   // only every other line in surface (with the start line defined by InterlacedField) has valid data
   // (it's up to internal Mednafen code to deinterlace it).
   bool InterlaceOn;
   bool InterlaceField;

   // Skip rendering this frame if true.  Set by the driver code.
   int skip;

   //
   // If sound is disabled, the driver code must set SoundRate to false, SoundBuf to NULL, SoundBufMaxSize to 0.

   // Will be set to TRUE if the sound format(only rate for now, at least) has changed since the last call to Emulate(), FALSE otherwise.
   // Will be set to TRUE on the first call to the Emulate() function/method
   bool SoundFormatChanged;

   // Sound rate.  Set by driver side.
   double SoundRate;

   // Pointer to sound buffer, set by the driver code, that the emulation code should render sound to.
   // Guaranteed to be at least 500ms in length, but emulation code really shouldn't exceed 40ms or so.  Additionally, if emulation code
   // generates >= 100ms,
   // DEPRECATED: Emulation code may set this pointer to a sound buffer internal to the emulation module.
   int16_t *SoundBuf;

   // Maximum size of the sound buffer, in frames.  Set by the driver code.
   int32_t SoundBufMaxSize;

   // Number of frames currently in internal sound buffer.  Set by the system emulation code, to be read by the driver code.
   int32_t SoundBufSize;
   int32_t SoundBufSizeALMS;	// SoundBufSize value at last MidSync(), 0
   // if mid sync isn't implemented for the emulation module in use.

   // Number of cycles that this frame consumed, using MDFNGI::MasterClock as a time base.
   // Set by emulation code.
   int64_t MasterCycles;
   int64_t MasterCyclesALMS;	// MasterCycles value at last MidSync(), 0
   // if mid sync isn't implemented for the emulation module in use.

   // Current sound volume(0.000...<=volume<=1.000...).  If, after calling Emulate(), it is still != 1, Mednafen will handle it internally.
   // Emulation modules can handle volume themselves if they like, for speed reasons.  If they do, afterwards, they should set its value to 1.
   double SoundVolume;

   // Current sound speed multiplier.  Set by the driver code.  If, after calling Emulate(), it is still != 1, Mednafen will handle it internally
   // by resampling the audio.  This means that emulation modules can handle(and set the value to 1 after handling it) it if they want to get the most
   // performance possible.  HOWEVER, emulation modules must make sure the value is in a range(with minimum and maximum) that their code can handle
   // before they try to handle it.
   double soundmultiplier;

   // True if we want to rewind one frame.  Set by the driver code.
   bool NeedRewind;

   // Sound reversal during state rewinding is normally done in mednafen.cpp, but
   // individual system emulation code can also do it if this is set, and clear it after it's done.
   // (Also, the driver code shouldn't touch this variable)
   bool NeedSoundReverse;

} EmulateSpecStruct;

typedef enum
{
   MODPRIO_INTERNAL_EXTRA_LOW = 0,	// For "cdplay" module, mostly.

   MODPRIO_INTERNAL_LOW = 10,
   MODPRIO_EXTERNAL_LOW = 20,
   MODPRIO_INTERNAL_HIGH = 30,
   MODPRIO_EXTERNAL_HIGH = 40
} ModPrio;


class CDIF;

#define MDFN_MASTERCLOCK_FIXED(n)	((int64_t)((double)(n) * (INT64_C(1) << 32)))

typedef struct
{
   const MDFNSetting *Settings;

   // Time base for EmulateSpecStruct::MasterCycles
   int64_t MasterClock;

   uint32_t fps; // frames per second * 65536 * 256, truncated

   // multires is a hint that, if set, indicates that the system has fairly programmable video modes(particularly, the ability
   // to display multiple horizontal resolutions, such as the PCE, PC-FX, or Genesis).  In practice, it will cause the driver
   // code to set the linear interpolation on by default.
   //
   // lcm_width and lcm_height are the least common multiples of all possible
   // resolutions in the frame buffer as specified by DisplayRect/LineWidths(Ex for PCE: widths of 256, 341.333333, 512,
   // lcm = 1024)
   //
   // nominal_width and nominal_height specify the resolution that Mednafen should display
   // the framebuffer image in at 1x scaling, scaled from the dimensions of DisplayRect, and optionally the LineWidths array
   // passed through espec to the Emulate() function.
   //
   bool multires;

   int lcm_width;
   int lcm_height;

   void *dummy_separator;	//

   int nominal_width;
   int nominal_height;

   int fb_width;		// Width of the framebuffer(not necessarily width of the image).  MDFN_Surface width should be >= this.
   int fb_height;		// Height of the framebuffer passed to the Emulate() function(not necessarily height of the image)

   int soundchan; 	// Number of output sound channels.


   int rotated;

   uint8_t MD5[16];
   uint8_t GameSetMD5[16];	/* A unique ID for the game set this CD belongs to, only used in PC-FX emulation. */
   bool GameSetMD5Valid; /* True if GameSetMD5 is valid. */

   uint8_t StateMD5[16];	// ID to use in save state naming and netplay session IDs, if
   bool StateMD5Valid;	// StateMD5Valid is true(useful for systems with multiple BIOS revisions, e.g. PS1).

   int soundrate;  /* For Ogg Vorbis expansion sound wacky support.  0 for default. */

   VideoSystems VideoSystem;
   GameMediumTypes GameType;

   RMD_Layout* RMD;

   //int DiskLogicalCount;	// A single double-sided disk would be 2 here.
   //const char *DiskNames;	// Null-terminated.

   const char *cspecial;  /* Special cart expansion: DIP switches, barcode reader, etc. */

   std::vector<const char *>DesiredInput; // Desired input device for the input ports, NULL for don't care


   // For absolute coordinates(IDIT_X_AXIS and IDIT_Y_AXIS), usually mapped to a mouse(hence the naming).
   float mouse_scale_x, mouse_scale_y;
   float mouse_offs_x, mouse_offs_y;
} MDFNGI;

int StateAction(StateMem *sm, int load, int data_only);

extern retro_log_printf_t log_cb;

#endif
