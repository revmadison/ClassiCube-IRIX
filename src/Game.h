#ifndef CC_GAME_H
#define CC_GAME_H
#include "Core.h"
/* Represents the game and related structures.
   Copyright 2014-2022 ClassiCube | Licensed under BSD-3
*/

struct Bitmap;
struct Stream;
CC_VAR extern struct _GameData {
	/* Width and height of the window. (1 at minimum) */
	int Width, Height;
	/* Total time (in seconds) the game has been running for. */
	double Time;
	/* Number of chunks updated within last second. Resets to 0 after every second. */
	int ChunkUpdates;
} Game;

/* Stopwatch measurement of when current frame started */
extern cc_uint64 Game_FrameStart;
extern struct RayTracer Game_SelectedPos;
extern cc_bool Game_UseCPEBlocks;

extern cc_string Game_Username;
extern cc_string Game_Mppass;

#define DEFAULT_MAX_VIEWDIST 32768
extern int Game_ViewDistance;
extern int Game_MaxViewDistance;
extern int Game_UserViewDistance;

/* Strategy used to limit FPS (see FpsLimitMethod enum) */
extern int     Game_FpsLimit;
extern cc_bool Game_SimpleArmsAnim;
extern int     Game_Vertices;

extern cc_bool Game_ClassicMode;
extern cc_bool Game_ClassicHacks;
#define Game_PureClassic (Game_ClassicMode && !Game_ClassicHacks)
extern cc_bool Game_AllowCustomBlocks;
extern cc_bool Game_UseCPE;
extern cc_bool Game_AllowServerTextures;

extern cc_bool Game_ViewBobbing;
extern cc_bool Game_BreakableLiquids;
/* Whether a screenshot should be taken at the end of this frame */
extern cc_bool Game_ScreenshotRequested;
extern cc_bool Game_HideGui;
extern cc_bool Game_DefaultZipMissing;

enum GAME_VERSION_ {
	VERSION_0017 = 27, VERSION_0019 = 28, VERSION_0023 = 29, VERSION_0030 = 30, VERSION_CPE = 31
};
struct GameVersion { 
	const char* Name; 
	cc_uint8 Version, Protocol, MaxCoreBlock;
	cc_uint8 BlocksPerRow, InventorySize;
	const cc_uint8* Inventory; 
	const cc_uint8* Hotbar;
};
extern struct GameVersion Game_Version;
extern void GameVersion_Load(void);

enum FpsLimitMethod {
	FPS_LIMIT_VSYNC, FPS_LIMIT_30, FPS_LIMIT_60, FPS_LIMIT_120, FPS_LIMIT_144, FPS_LIMIT_NONE, FPS_LIMIT_COUNT
};
extern const char* const FpsLimit_Names[FPS_LIMIT_COUNT];

void Game_ToggleFullscreen(void);
void Game_CycleViewDistance(void);
/* Attempts to reduce VRAM usage (e.g. reducing view distance) */
/* Returns false if VRAM cannot be reduced any further */
cc_bool Game_ReduceVRAM(void);

void Game_SetViewDistance(int distance);
void Game_UserSetViewDistance(int distance);
void Game_Disconnect(const cc_string* title, const cc_string* reason);
void Game_Reset(void);

/* Sets the block in the map at the given coordinates, then updates state associated with the block. */
/* (updating state means recalculating light, redrawing chunk block is in, etc) */
/* NOTE: This does NOT notify the server, use Game_ChangeBlock for that. */
CC_API void Game_UpdateBlock(int x, int y, int z, BlockID block);
/* Calls Game_UpdateBlock, then informs server connection of the block change. */
/* In multiplayer this is sent to the server, in singleplayer just activates physics. */
CC_API void Game_ChangeBlock(int x, int y, int z, BlockID block);

cc_bool Game_CanPick(BlockID block);
cc_bool Game_UpdateTexture(GfxResourceID* texId, struct Stream* src, const cc_string* file, cc_uint8* skinType);
/* Checks that the given bitmap can be loaded into a native gfx texture. */
/* (must be power of two size and be <= Gfx_MaxTexWidth/Gfx_MaxHeight) */
cc_bool Game_ValidateBitmap(const cc_string* file, struct Bitmap* bmp);
/* Updates Game_Width and Game_Height. */
void Game_UpdateDimensions(void);
/* Sets the strategy/method used to limit frames per second. */
/* See FPS_LIMIT_ for valid strategies/methods */
void Game_SetFpsLimit(int method);

/* Runs the main game loop until the window is closed. */
void Game_Run(int width, int height, const cc_string* title);
/* Whether the game should be allowed to automatically close */
cc_bool Game_ShouldClose(void);

/* Represents a game component. */
struct IGameComponent;
struct IGameComponent {
	/* Called to init the component's state. (called when game is starting) */
	void (*Init)(void);
	/* Called to free the component's state. (called when game is closing) */
	void (*Free)(void);
	/* Called to reset the component's state. (e.g. reconnecting to server) */
	void (*Reset)(void);
	/* Called to update the component's state when the user begins loading a new map. */
	void (*OnNewMap)(void);
	/* Called to update the component's state when the user has finished loading a new map. */
	void (*OnNewMapLoaded)(void);
	/* Next component in linked list of components. */
	struct IGameComponent* next;
};
/* Adds a component to linked list of components. (always at end) */
CC_NOINLINE void Game_AddComponent(struct IGameComponent* comp);

/* Represents a task that periodically runs on the main thread every specified interval. */
struct ScheduledTask;
struct ScheduledTask {
	/* How long (in seconds) has elapsed since callback was last invoked */
	double accumulator;
	/* How long (in seconds) between invocations of the callback */
	double interval;
	/* Callback function that is periodically invoked */
	void (*Callback)(struct ScheduledTask* task);
};

typedef void (*ScheduledTaskCallback)(struct ScheduledTask* task);
/* Adds a task to list of scheduled tasks. (always at end) */
CC_API int ScheduledTask_Add(double interval, ScheduledTaskCallback callback);
#endif
