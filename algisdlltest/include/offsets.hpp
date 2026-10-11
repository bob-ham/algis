#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define REBASE(addr)(addr)

namespace Offsets
{
    namespace Luau
    {
        const uintptr_t LuaVMLoad = REBASE(0x41F51A0);
        const uintptr_t Lua_SandBoxThread = REBASE(0x1D6A630);
        const uintptr_t OpcodeLookupTable = REBASE(0x6F67BF0);
        const uintptr_t f_luaopen = REBASE(0x26444F0);
        const uintptr_t loadsafe = REBASE(0x26B6440);
        const uintptr_t luaB_assert = REBASE(0x26A4FE0);
        const uintptr_t luaB_error = REBASE(0x269E340);
        const uintptr_t luaB_gcinfo = REBASE(0x26A3920);
        const uintptr_t luaB_getfenv = REBASE(0x269F940);
        const uintptr_t luaB_getmetatable = REBASE(0x269ED80);
        const uintptr_t luaB_newproxy = REBASE(0x26A5F20);
        const uintptr_t luaB_next = REBASE(0x26A4530);
        const uintptr_t luaB_print = REBASE(0x269DEE0);
        const uintptr_t luaB_rawequal = REBASE(0x269FC30);
        const uintptr_t luaB_rawget = REBASE(0x269FD40);
        const uintptr_t luaB_rawlen = REBASE(0x26A3880);
        const uintptr_t luaB_rawset = REBASE(0x26A0760);
        const uintptr_t luaB_select = REBASE(0x26A5090);
        const uintptr_t luaB_setfenv = REBASE(0x269FA00);
        const uintptr_t luaB_setmetatable = REBASE(0x269EF10);
        const uintptr_t luaB_tonumber = REBASE(0x5797560);
        const uintptr_t luaB_tostring = REBASE(0x26A5EC0);
        const uintptr_t luaB_type = REBASE(0x26A3940);
        const uintptr_t luaB_typeof = REBASE(0x26A3F40);
        const uintptr_t luaC_reallymarkobject = REBASE(0x2663D60);
        const uintptr_t luaD_call = REBASE(0x2641400);
        const uintptr_t luaD_checkCstack = REBASE(0x2652140);
        const uintptr_t luaD_growstack = REBASE(0x263FAB0);
        const uintptr_t luaD_rawrunprotected = REBASE(0x2652080);
        const uintptr_t luaD_reallocstack = REBASE(0x263FF00);
        const uintptr_t luaD_seterrorobj = REBASE(0x2653220);
        const uintptr_t luaD_throw = REBASE(0x26520B0);
        const uintptr_t luaF_close = REBASE(0x267BBE0);
        const uintptr_t luaF_freeproto = REBASE(0x267BCD0);
        const uintptr_t luaF_newLclosure = REBASE(0x267B820);
        const uintptr_t luaF_newUpval = REBASE(0x267BA00);
        const uintptr_t luaF_newproto = REBASE(0x267B5F0);
        const uintptr_t luaG_aritherror = REBASE(0x267A600);
        const uintptr_t luaG_concaterror = REBASE(0x267A5B0);
        const uintptr_t luaG_methoderror = REBASE(0x267A820);
        const uintptr_t luaG_missingmembererror = REBASE(0x267A7B0);
        const uintptr_t luaG_ordererror = REBASE(0x267A6A0);
        const uintptr_t luaG_readonlyerror = REBASE(0x267A860);
        const uintptr_t luaG_runerror = REBASE(0x267AE80);
        const uintptr_t luaG_typeerror = REBASE(0x267A530);
        const uintptr_t luaH_clone = REBASE(0x26758E0);
        const uintptr_t luaH_dummynode = REBASE(0x6504798);
        const uintptr_t luaH_getn = REBASE(0x2675760);
        const uintptr_t luaL_checklstring = REBASE(0x2649580);
        const uintptr_t luaL_errorL = REBASE(0x2647EE0);
        const uintptr_t luaL_getmetafield = REBASE(0x2649AF0);
        const uintptr_t luaL_optlstring = REBASE(0x26496D0);
        const uintptr_t luaL_prepbuffsize = REBASE(0x264E560);
        const uintptr_t luaL_tolstring = REBASE(0x2650270);
        const uintptr_t luaL_where = REBASE(0x2647430);
        const uintptr_t luaM_realloc = REBASE(0x2647080);
        const uintptr_t luaM_toobig = REBASE(0x267FF50);
        const uintptr_t luaO_nilobject = REBASE(0x6507CD8);
        const uintptr_t luaO_str2d = REBASE(0x2679720);
        const uintptr_t luaT_eventname = REBASE(0x6529DF0);
        const uintptr_t luaT_init = REBASE(0x267C2A0);
        const uintptr_t luaT_objtypename = REBASE(0x267CD80);
        const uintptr_t luaT_typenames = REBASE(0x6529D80);
        const uintptr_t luaV_gettable = REBASE(0x265B170);
        const uintptr_t luaV_settable = REBASE(0x265CE90);
        const uintptr_t lua_addfinalizer = REBASE(0x2641DF0);
        const uintptr_t lua_close = REBASE(0x2645590);
        const uintptr_t lua_exception_unexpectedexceptionstatus = REBASE(0x2651F40);
        const uintptr_t lua_getfenv = REBASE(0x2640FA0);
        const uintptr_t lua_newstate = REBASE(0x2645CB0);
        const uintptr_t lua_pushthread = REBASE(0x2640E90);
        const uintptr_t lua_resume = REBASE(0x2654910);
        const uintptr_t lua_setfenv = REBASE(0x2641050);
        const uintptr_t lua_yield = REBASE(0x2656A70);
        const uintptr_t luau_api_error = REBASE(0x4DE6880);
        const uintptr_t luau_api_tracer = REBASE(0x4DE59C0);
        const uintptr_t luau_execute = REBASE(0x2682560);
        const uintptr_t luau_load = REBASE(0x41F47B0);
    }
    namespace lua_State
    {
        const uintptr_t base = 0x60;
        const uintptr_t extraSpace = 0x28;
        const uintptr_t global = 0x48;
        const uintptr_t marked = 0x2;
        const uintptr_t memcat = 0x1;
        const uintptr_t stack_last = 0x68;
        const uintptr_t top = 0x58;
        const uintptr_t tt = 0x0;
    }
    namespace global_State
    {
        const uintptr_t GCthreshold = 0x38;
        const uintptr_t currentwhite = 0x48;
        const uintptr_t frealloc = 0x10;
        const uintptr_t gray = 0x30;
        const uintptr_t mainthread = 0x90;
        const uintptr_t totalbytes = 0x40;
        const uintptr_t ud = 0x18;
    }
    namespace Proto
    {
        const uintptr_t size = 0xD8;
    }
    namespace Identity
    {
        const uintptr_t AssetId = 0x8;
        const uintptr_t Identity = 0x0;
        const uintptr_t LuaState = 0x20;
        const uintptr_t Restore = 0x38;
        const uintptr_t Script = 0x30;
        const uintptr_t GetCapabilities = REBASE(0x1D6A7C0);
        const uintptr_t GetTssData = REBASE(0x7320);
        const uintptr_t Impersonator = REBASE(0x1D6A880);
        const uintptr_t Pointer = REBASE(0x82D23D8);
        const uintptr_t printidentity = REBASE(0x42AED40);
    }
    namespace ExtraSpace
    {
        const uintptr_t Capabilities = 0x30;
        const uintptr_t IsAurora = 0xA9;
        const uintptr_t ScriptContext = 0x18;
    }
    namespace Instance
    {
        const uintptr_t Attributes = 0x40;
        const uintptr_t Capabilities = 0x60;
        const uintptr_t Children = 0x78;
        const uintptr_t ChildrenCtrl = 0x80;
        const uintptr_t ClassDescriptor = 0x18;
        const uintptr_t Name = 0x70;
        const uintptr_t Parent = 0x68;
        const uintptr_t PropertyDescriptorBitFlags = 0x8C;
        const uintptr_t ScriptableMask = 0x10;
        const uintptr_t AddTagCore = REBASE(0x328FC40);
        const uintptr_t AddTagOpsPerFrame = REBASE(0x33A4AE0);
        const uintptr_t CircularParent = REBASE(0x1D854A0);
        const uintptr_t CircularParentInternal = REBASE(0x8911A0);
        const uintptr_t ClearAllChildren = REBASE(0x5D13E0);
        const uintptr_t FindFirstChild = REBASE(0x5D5620);
        const uintptr_t FindFirstChildCore = REBASE(0x1D745B0);
        const uintptr_t FindFirstChildInternal = REBASE(0x158C170);
        const uintptr_t FindFirstFile = REBASE(0x5B23E20);
        const uintptr_t GetActor = REBASE(0x5D1B00);
        const uintptr_t GetAttribute = REBASE(0x5D2980);
        const uintptr_t GetAttributeChangedSignal = REBASE(0x5D2BF0);
        const uintptr_t GetAttributeInternal = REBASE(0x158CA80);
        const uintptr_t GetAttributes = REBASE(0x5D2E00);
        const uintptr_t GetCapabilities = REBASE(0x41F2010);
        const uintptr_t GetChildren = REBASE(0x5D30D0);
        const uintptr_t GetDebugId = REBASE(0x5D5B80);
        const uintptr_t GetDebugIdInternal = REBASE(0x158BD10);
        const uintptr_t GetDescendants = REBASE(0x5D3390);
        const uintptr_t GetFullName = REBASE(0x5D38F0);
        const uintptr_t GetPropertyChangedSignal = REBASE(0x5D77A0);
        const uintptr_t GetPropertyChangedSignalInternal = REBASE(0x158B440);
        const uintptr_t GetStyled = REBASE(0x5D6E60);
        const uintptr_t GetStyledInternal = REBASE(0x158B5E0);
        const uintptr_t GetStyledPropertyChangedSignal = REBASE(0x5D7070);
        const uintptr_t IsAncestorOf = REBASE(0x5D3BB0);
        const uintptr_t IsDescendantOf = REBASE(0x5D3E30);
        const uintptr_t NotScriptablePropertyInternal = REBASE(0x1D83850);
        const uintptr_t ParentLockedFmt = REBASE(0x1D85430);
        const uintptr_t ParentLockedFmtInternal = REBASE(0x1D88050);
        const uintptr_t PropertySignalLookup = REBASE(0x4AF3C30);
        const uintptr_t SetAttribute = REBASE(0x158A4D0);
        const uintptr_t SetAttributeCore = REBASE(0x1D7D870);
        const uintptr_t SetAttributeInternal = REBASE(0x158B8A0);
        const uintptr_t SetParent = REBASE(0x1D819E0);
        const uintptr_t TouchInterest = REBASE(0xBF1DC0);
        const uintptr_t WaitForChild = REBASE(0x5D4760);
        const uintptr_t WaitForChildInternal = REBASE(0x158CF40);
        const uintptr_t WaitForChildYield = REBASE(0x1586C50);
        const uintptr_t clone = REBASE(0x5D4C30);
        const uintptr_t destroy = REBASE(0x5D66A0);
        const uintptr_t getAttribute = REBASE(0x1D7FAE0);
        const uintptr_t getChildren = REBASE(0x5D4F00);
        const uintptr_t validateAttributeName = REBASE(0x1D7C6C0);
    }
    namespace DataModel
    {
        const uintptr_t WriteLock = 0x6D8;
        const uintptr_t WriteRequestingThread = 0x6D0;
        const uintptr_t WriteRequestingThreadState = 0x6C8;
        const uintptr_t FindService = REBASE(0x5D79D0);
        const uintptr_t FindServiceInternal = REBASE(0x158B2A0);
        const uintptr_t Get = REBASE(0x4121B50);
        const uintptr_t GetCurrentThreadId = REBASE(0x7510);
        const uintptr_t HttpGetAsync = REBASE(0x5E17F0);
        const uintptr_t HttpGetAsyncInternal = REBASE(0x168D880);
        const uintptr_t HttpPostAsync = REBASE(0x5E1A10);
        const uintptr_t Load = REBASE(0x5DFDE0);
        const uintptr_t LoadInternal = REBASE(0x1689120);
        const uintptr_t Shutdown = REBASE(0x5E0B10);
        const uintptr_t currentThreadHasWriteLock = REBASE(0x4125C40);
        const uintptr_t onWriteBorrow = REBASE(0x4125460);
        const uintptr_t onWriteLock = REBASE(0x4125290);
        const uintptr_t onWriteReturn = REBASE(0x4125620);
        const uintptr_t onWriteUnlock = REBASE(0x4125940);
    }
    namespace TaskScheduler
    {
        const uintptr_t Init = REBASE(0x2A34FB0);
        const uintptr_t PostSimulation = REBASE(0x32D7C0);
        const uintptr_t PreSimulation = REBASE(0x32D8E0);
        const uintptr_t TaskScheduler = REBASE(0x31357B0);
        const uintptr_t WaitingHybridScriptsJob = REBASE(0x42D5440);
    }
    namespace Reflection
    {
        const uintptr_t GetProperty = REBASE(0x5ED520);
        const uintptr_t GetPropertyInternal = REBASE(0x178DC10);
        const uintptr_t PropertyDescriptor = REBASE(0x8916A0);
        const uintptr_t SetProperty = REBASE(0x178CE90);
        const uintptr_t SetPropertyInternal = REBASE(0x178D7B0);
    }
    namespace Humanoid
    {
        const uintptr_t AddAccessory = REBASE(0x4D2110);
        const uintptr_t AddAccessoryInternal = REBASE(0x8A6520);
        const uintptr_t ApplyDescription = REBASE(0x4D7DF0);
        const uintptr_t ApplyDescriptionReset = REBASE(0x4D8080);
        const uintptr_t BuildRigFromAttachments = REBASE(0x4D2580);
        const uintptr_t ChangeState = REBASE(0x4D68F0);
        const uintptr_t ChangeStateInternal = REBASE(0x1025240);
        const uintptr_t GetAppliedDescription = REBASE(0x4D3080);
        const uintptr_t GetAppliedDescriptionInternal = REBASE(0x1033BD0);
        const uintptr_t GetBodyPartR15 = REBASE(0x4D32D0);
        const uintptr_t GetLimb = REBASE(0x4D34F0);
        const uintptr_t GetLimbInternal = REBASE(0x1026300);
        const uintptr_t RemoveAccessories = REBASE(0x4D4F60);
        const uintptr_t ReplaceBodyPartR15 = REBASE(0x4D5400);
        const uintptr_t SetStateEnabledInternal = REBASE(0x1025700);
        const uintptr_t TakeDamage = REBASE(0x4D5A90);
        const uintptr_t TakeDamageInternal = REBASE(0x1025570);
        const uintptr_t UnequipTools = REBASE(0x4D5CB0);
    }
    namespace Player
    {
        const uintptr_t DistanceFromCharacter = REBASE(0x26A7D0);
        const uintptr_t DistanceFromCharacterInternal = REBASE(0xB1FDE0);
        const uintptr_t GetFriendsOnline = REBASE(0x276BB0);
        const uintptr_t GetFriendsOnlineInternal = REBASE(0xB1D400);
        const uintptr_t GetJoinData = REBASE(0x26B550);
        const uintptr_t GetNetworkPing = REBASE(0x26C760);
        const uintptr_t GetRankInGroup = REBASE(0x274940);
        const uintptr_t GetRankInGroupInternal = REBASE(0xB1FA70);
        const uintptr_t GetRoleInGroup = REBASE(0x274B60);
        const uintptr_t GetRoleInGroupInternal = REBASE(0xB1F8E0);
        const uintptr_t HasAppearanceLoaded = REBASE(0x26D040);
        const uintptr_t IsFriendsWith = REBASE(0x274D80);
        const uintptr_t IsFriendsWithInternal = REBASE(0xB1F5B0);
        const uintptr_t IsInGroup = REBASE(0x274FA0);
        const uintptr_t IsInGroupInternal = REBASE(0xB1F420);
        const uintptr_t Kick = REBASE(0x3E01590);
        const uintptr_t LoadCharacter = REBASE(0x2751D0);
        const uintptr_t LoadCharacterAsync = REBASE(0x26E170);
        const uintptr_t LoadCharacterBlocking = REBASE(0x26E5E0);
        const uintptr_t LoadCharacterWithAvatarRules = REBASE(0xB15700);
        const uintptr_t LoadCharacterWithHumanoidDescription = REBASE(0x275400);
        const uintptr_t LoadCharacter_Failedtospawnplayer__s = REBASE(0x5FB8750);
        const uintptr_t RequestStreamAroundAsync = REBASE(0x272D10);
        const uintptr_t RequestStreamAroundAsyncInternal = REBASE(0xB1CE80);
        const uintptr_t SetAccountAge = REBASE(0x26FEC0);
    }
    namespace Players
    {
        const uintptr_t CreateHumanoidModelFromDescription = REBASE(0x263EC0);
        const uintptr_t GetCharacterAppearanceAsync = REBASE(0x25FAF0);
        const uintptr_t GetCharacterAppearanceInfoAsync = REBASE(0x25FD00);
        const uintptr_t GetPlayerByUserId = REBASE(0x260A40);
        const uintptr_t GetPlayerFromCharacter = REBASE(0x260C50);
        const uintptr_t GetPlayerFromCharacterInternal = REBASE(0xAF7F10);
        const uintptr_t GetPlayerImageDefaultTimeout = REBASE(0x3F1CB20);
        const uintptr_t GetPlayerSegmentsAsync = REBASE(0x3DB830);
        const uintptr_t GetPlayersByPartyId = REBASE(0x313DA0);
    }
    namespace Workspace
    {
        const uintptr_t Blockcast = REBASE(0xEECBC0);
        const uintptr_t BlockcastInternal = REBASE(0xEF24A0);
        const uintptr_t FindPartOnRay = REBASE(0x3FAA70);
        const uintptr_t FindPartOnRayInternal = REBASE(0xEE9D10);
        const uintptr_t FindPartsInRegion3 = REBASE(0x3FB130);
        const uintptr_t GetPartBoundsInBox = REBASE(0xEF1850);
        const uintptr_t GetPartBoundsInRadius = REBASE(0xEF1670);
        const uintptr_t GetPartsInPart = REBASE(0xEF0E50);
        const uintptr_t GetPhysicsThrottling = REBASE(0x2A82D0);
        const uintptr_t GetRealPhysicsFPS = REBASE(0x2A8510);
        const uintptr_t GetServerTimeNow = REBASE(0x2A8750);
        const uintptr_t Raycast = REBASE(0xEEB2D0);
        const uintptr_t RaycastInternal = REBASE(0xEF08D0);
        const uintptr_t Spherecast = REBASE(0xEEA530);
        const uintptr_t SpherecastInternal = REBASE(0xEEF830);
    }
    namespace Lua
    {
        const uintptr_t LuaArguments = REBASE(0x4158490);
    }
    namespace Script
    {
        const uintptr_t EnableLoadModule = REBASE(0x8635A08);
        const uintptr_t FailedtocreateLuastate = REBASE(0x428A680);
        const uintptr_t GetScriptContext = REBASE(0x41FC130);
        const uintptr_t ScriptContextResume = REBASE(0x42A9110);
        const uintptr_t ScriptContextRun = REBASE(0x42C8240);
        const uintptr_t loadmodule = REBASE(0x42B5E30);
    }
    namespace Scripts
    {
        const uintptr_t RunContextFilter = REBASE(0x4234740);
        const uintptr_t filterRunContextInternal = REBASE(0x1D2F120);
    }
    namespace ScriptContext
    {
        const uintptr_t ExtraSpace = 0x18;
        const uintptr_t ExtraSpaceCapabilities = 0x30;
        const uintptr_t Facet = 0x13F0;
        const uintptr_t CallbacksCannotYield = REBASE(0x42AA2A0);
        const uintptr_t FailedToCreateLuaState = REBASE(0x428A680);
        const uintptr_t Get = REBASE(0x41FC130);
        const uintptr_t ParallelLuauHybridCallerCallInParallelCa = REBASE(0x18586C0);
        const uintptr_t ParallelLuauWorkQueueExecutePendingWorkF = REBASE(0x4317740);
        const uintptr_t Require = REBASE(0x42B6720);
        const uintptr_t Resume = REBASE(0x42A9110);
        const uintptr_t ResumeDelayedThreads = REBASE(0x42D9570);
        const uintptr_t ResumeWaitingThreads = REBASE(0x4365F30);
        const uintptr_t ResumeWaitingThreadsInternal = REBASE(0x4367470);
        const uintptr_t RomarkProfilerFacetPopProfilerUserActive = REBASE(0x4358D50);
        const uintptr_t Run = REBASE(0x42C8240);
        const uintptr_t ScriptContextAdd = REBASE(0x3BA23E0);
        const uintptr_t ScriptContextClose = REBASE(0x3BA2470);
        const uintptr_t ScriptContextRemove = REBASE(0x3BA2410);
        const uintptr_t ScriptContextShutdownTimeoutSeconds = REBASE(0x705070);
        const uintptr_t ScriptContextTaskQueue = REBASE(0x4359CC0);
        const uintptr_t ScriptContextTaskQueueInternal = REBASE(0x2A37AB0);
    }
    namespace Lighting
    {
        const uintptr_t GetMinutesAfterMidnight = REBASE(0x3721C0);
        const uintptr_t GetMoonDirection = REBASE(0x372400);
        const uintptr_t GetMoonPhase = REBASE(0x372640);
        const uintptr_t GetSunDirection = REBASE(0x3728A0);
        const uintptr_t SetMinutesAfterMidnight = REBASE(0x372AE0);
        const uintptr_t SetMinutesAfterMidnightInternal = REBASE(0xCC5390);
    }
    namespace Camera
    {
        const uintptr_t GetLargestCutoffDistance = REBASE(0xD51DE0);
        const uintptr_t GetLargestCutoffDistanceInternal = REBASE(0xB82E60);
        const uintptr_t GetPanSpeed = REBASE(0x3BF640);
        const uintptr_t GetPartsObscuringTarget = REBASE(0x3BF8D0);
        const uintptr_t GetPartsObscuringTargetInternal = REBASE(0xD53DC0);
        const uintptr_t ScreenPointToRay = REBASE(0x3C15F0);
        const uintptr_t SetCameraPanMode = REBASE(0x3C1880);
        const uintptr_t SetCameraPanModeInternal = REBASE(0xD527C0);
        const uintptr_t ViewportPointToRay = REBASE(0x3C1B00);
        const uintptr_t WorldToScreenPoint = REBASE(0x3C0D30);
        const uintptr_t WorldToViewportPoint = REBASE(0x3C0F80);
    }
    namespace BasePart
    {
        const uintptr_t ApplyAngularImpulse = REBASE(0x45E6A0);
        const uintptr_t ApplyImpulse = REBASE(0x45E8F0);
        const uintptr_t ApplyImpulseAtPosition = REBASE(0x45EB40);
        const uintptr_t ApplyImpulseAtPositionInternal = REBASE(0xF5A650);
        const uintptr_t CanCollideWith = REBASE(0x45F020);
        const uintptr_t CanCollideWithInternal = REBASE(0xF5A4B0);
        const uintptr_t CanSetNetworkOwnership = REBASE(0x45F290);
        const uintptr_t GetConnectedParts = REBASE(0x461BC0);
        const uintptr_t GetConnectedPartsInternal = REBASE(0xF59D20);
        const uintptr_t GetJoints = REBASE(0x45F750);
        const uintptr_t GetMass = REBASE(0x45FA10);
        const uintptr_t GetNetworkOwner = REBASE(0x45FCD0);
        const uintptr_t GetNetworkOwnershipAuto = REBASE(0x45FF90);
        const uintptr_t GetRootPart = REBASE(0x460830);
        const uintptr_t GetTouchingParts = REBASE(0x460A20);
        const uintptr_t GetVelocityAtPosition = REBASE(0x460CB0);
        const uintptr_t GetVelocityAtPositionInternal = REBASE(0xF5A170);
        const uintptr_t SetNetworkOwner = REBASE(0x461E30);
        const uintptr_t SetNetworkOwnerInternal = REBASE(0xF59B80);
        const uintptr_t SetNetworkOwnershipAuto = REBASE(0x4616A0);
    }
    namespace ProximityPrompt
    {
        const uintptr_t InputHoldBegin = REBASE(0x33C140);
        const uintptr_t InputHoldEnd = REBASE(0x33C3A0);
        const uintptr_t ProximityPromptAnalyticsSampleRate = REBASE(0x69FA80);
        const uintptr_t ProximityPromptIndicators = REBASE(0x69FA00);
        const uintptr_t ProximityPromptService = REBASE(0xC6AA90);
        const uintptr_t ProximityPrompts = REBASE(0x69F9A0);
    }
    namespace AnimationTrack
    {
        const uintptr_t AdjustSpeed = REBASE(0x183460);
        const uintptr_t AdjustSpeedInternal = REBASE(0x8E9830);
        const uintptr_t GetMarkerReachedSignal = REBASE(0x183060);
        const uintptr_t GetMarkerReachedSignalInternal = REBASE(0x8E8E40);
        const uintptr_t GetTimeOfKeyframe = REBASE(0x183260);
        const uintptr_t GetTimeOfKeyframeInternal = REBASE(0x8E8CA0);
    }
    namespace AssetService
    {
        const uintptr_t CreatePlaceAsync = REBASE(0x19BE20);
        const uintptr_t CreatePlaceAsyncInternal = REBASE(0x974670);
        const uintptr_t GetBundleDetailsAsync = REBASE(0x19AF30);
        const uintptr_t GetBundleDetailsAsyncInternal = REBASE(0x9755F0);
        const uintptr_t SavePlaceAsync = REBASE(0x19B7F0);
        const uintptr_t SavePlaceAsyncInternal = REBASE(0x974EB0);
        const uintptr_t logTelemetryForAssetPublish = REBASE(0x33D9E00);
    }
    namespace AvatarEditorService
    {
        const uintptr_t GetBatchItemDetails = REBASE(0x531CB0);
        const uintptr_t GetBatchItemDetailsInternal = REBASE(0x1076F80);
        const uintptr_t GetFavorite = REBASE(0x531EE0);
        const uintptr_t GetFavoriteInternal = REBASE(0x1076CD0);
        const uintptr_t GetInventory = REBASE(0x532110);
        const uintptr_t GetInventoryInternal = REBASE(0x1076B00);
        const uintptr_t PromptAllowInventoryReadAccess = REBASE(0x52D4B0);
        const uintptr_t PromptSetFavorite = REBASE(0x52DA50);
    }
    namespace GroupService
    {
        const uintptr_t GetAlliesAsync = REBASE(0x38AD90);
        const uintptr_t GetEnemiesAsync = REBASE(0x38AFA0);
        const uintptr_t GetGroupInfoAsync = REBASE(0x38B1B0);
        const uintptr_t GetGroupInfoAsyncInternal = REBASE(0xCE9700);
        const uintptr_t GetGroupsAsync = REBASE(0x38B3C0);
        const uintptr_t GetGroupsAsyncInternal = REBASE(0xCE9560);
    }
    namespace LogService
    {
        const uintptr_t GetLogHistory = REBASE(0x36D1E0);
        const uintptr_t GetLogHistoryInternal = REBASE(0xCBADD0);
    }
    namespace Stats
    {
        const uintptr_t GetTotalMemoryUsageMb = REBASE(0x302F70);
    }
    namespace MemoryStoreService
    {
        const uintptr_t GetHashMap = REBASE(0x3593D0);
        const uintptr_t GetHashMapInternal = REBASE(0xCA2C60);
        const uintptr_t GetQueue = REBASE(0x3595D0);
        const uintptr_t GetQueueInternal = REBASE(0xCA2960);
        const uintptr_t GetSortedMap = REBASE(0x359810);
        const uintptr_t GetSortedMapInternal = REBASE(0xCA2780);
    }
    namespace HttpService
    {
        const uintptr_t GenerateGUID = REBASE(0x380D30);
        const uintptr_t GenerateGUIDInternal = REBASE(0xCD31B0);
        const uintptr_t JSONEncode = REBASE(0x3800E0);
        const uintptr_t JSONEncodeInternal = REBASE(0xCD40E0);
        const uintptr_t UrlEncode = REBASE(0x380B20);
        const uintptr_t UrlEncodeInternal = REBASE(0xCD3350);
    }
    namespace TweenService
    {
        const uintptr_t Create = REBASE(0x2E5930);
        const uintptr_t updateGenericTweens = REBASE(0x4AE3FD0);
    }
    namespace MarketplaceService
    {
        const uintptr_t GetDeveloperProductsAsync = REBASE(0x5ACF90);
        const uintptr_t GetProductInfo = REBASE(0x5B6CB0);
        const uintptr_t GetProductInfoInternal = REBASE(0x14542F0);
        const uintptr_t PlayerOwnsAsset = REBASE(0x5B6630);
        const uintptr_t PlayerOwnsAssetInternal = REBASE(0x1458D10);
        const uintptr_t UserOwnsGamePassAsync = REBASE(0x5B2680);
    }
    namespace TeleportService
    {
        const uintptr_t GetLocalPlayerTeleportData = REBASE(0x2F5340);
        const uintptr_t GetPlayerPlaceInstanceAsync = REBASE(0x2F55A0);
        const uintptr_t initiateTeleportOnServer = REBASE(0x44D7F30);
        const uintptr_t initiateTeleportReservedServer = REBASE(0x44DE870);
    }
    namespace UserInputService
    {
        const uintptr_t GetConnectedGamepads = REBASE(0x239CF0);
        const uintptr_t GetGamepadState = REBASE(0x23AFA0);
        const uintptr_t GetGamepadStateInternal = REBASE(0xA91800);
        const uintptr_t GetKeysPressed = REBASE(0x23B1A0);
        const uintptr_t GetKeysPressedInternal = REBASE(0xAA1570);
        const uintptr_t GetLastInputType = REBASE(0x23B400);
        const uintptr_t GetLastInputTypeInternal = REBASE(0x8666B0);
        const uintptr_t GetMouseDelta = REBASE(0x23B8C0);
        const uintptr_t GetMouseLocation = REBASE(0x23BB20);
        const uintptr_t IsKeyDown = REBASE(0x23CAA0);
        const uintptr_t IsKeyDownInternal = REBASE(0xA91060);
        const uintptr_t IsMouseButtonPressed = REBASE(0x23CCA0);
        const uintptr_t IsMouseButtonPressedInternal = REBASE(0xA919A0);
        const uintptr_t UserInputService = REBASE(0xA88DF0);
    }
    namespace RunService
    {
        const uintptr_t BindToRenderStep = REBASE(0x3285C0);
        const uintptr_t BindToRenderStepInternal = REBASE(0xC50190);
        const uintptr_t IsClient = REBASE(0x3297C0);
        const uintptr_t IsRunMode = REBASE(0x329A40);
        const uintptr_t IsRunning = REBASE(0x329F40);
        const uintptr_t IsServer = REBASE(0x32A440);
        const uintptr_t UnbindFromRenderStep = REBASE(0x32BB90);
        const uintptr_t UnbindFromRenderStepInternal = REBASE(0xC4FFF0);
    }
    namespace ContextActionService
    {
        const uintptr_t BindAction = REBASE(0x250060);
        const uintptr_t BindActionAtPriority = REBASE(0x250290);
        const uintptr_t BindActionAtPriorityInternal = REBASE(0xAB15F0);
        const uintptr_t BindActionInternal = REBASE(0xAB1C50);
        const uintptr_t GetBoundActionInfo = REBASE(0x251630);
        const uintptr_t GetBoundActionInfoInternal = REBASE(0xAB0AE0);
        const uintptr_t UnbindAction = REBASE(0x252720);
        const uintptr_t UnbindActionInternal = REBASE(0xAB0230);
    }
    namespace GuiService
    {
        const uintptr_t GetEmotesMenuOpen = REBASE(0x1F6E70);
        const uintptr_t GetGuiInset = REBASE(0x1F7E50);
        const uintptr_t InspectPlayerFromUserId = REBASE(0x1FA070);
    }
    namespace StarterGui
    {
        const uintptr_t GetCore = REBASE(0x1D7F50);
        const uintptr_t GetCoreGuiEnabledInternal = REBASE(0x9E2100);
        const uintptr_t GetCoreInternal = REBASE(0x9E22A0);
        const uintptr_t SetCore = REBASE(0x9E0920);
        const uintptr_t SetCoreGuiEnabledInternal = REBASE(0x9E1870);
        const uintptr_t SetCoreInternal = REBASE(0x9E1B30);
    }
    namespace PathfindingService
    {
        const uintptr_t CreatePath = REBASE(0x193470);
        const uintptr_t CreatePathInternal = REBASE(0x958B90);
        const uintptr_t FindPathAsync = REBASE(0x193250);
        const uintptr_t FindPathAsyncInternal = REBASE(0x958D30);
    }
    namespace BadgeService
    {
        const uintptr_t AwardBadge = REBASE(0x3CCC90);
        const uintptr_t UserHasBadgeAsync = REBASE(0x3CC630);
    }
    namespace DataStoreService
    {
        const uintptr_t GetDataStore = REBASE(0xB44AE0);
        const uintptr_t GetDataStoreInternal = REBASE(0xB45620);
        const uintptr_t GetGlobalDataStore = REBASE(0x28F310);
        const uintptr_t GetGlobalDataStoreInternal = REBASE(0xB46BA0);
        const uintptr_t GetOrderedDataStore = REBASE(0x28FAE0);
        const uintptr_t GetOrderedDataStoreInternal = REBASE(0xB45320);
    }
    namespace SocialService
    {
        const uintptr_t CanSendGameInviteAsync = REBASE(0x313130);
        const uintptr_t PromptGameInvite = REBASE(0x3137D0);
        const uintptr_t PromptGameInviteInternal = REBASE(0xC32040);
        const uintptr_t createExpiringDeepLinkAsync = REBASE(0x35426F0);
        const uintptr_t getUpcomingExperienceEventsAsync = REBASE(0x353F3E0);
        const uintptr_t validateAndUpdateParty = REBASE(0x352A1F0);
        const uintptr_t validateAndUpdatePartyInternal = REBASE(0x11A8D80);
    }
    namespace InsertService
    {
        const uintptr_t GetBaseSets = REBASE(0x379420);
        const uintptr_t LoadAsset = REBASE(0x37A4D0);
        const uintptr_t LoadAssetVersion = REBASE(0x37A6F0);
    }
    namespace LocalizationService
    {
        const uintptr_t GetTranslatorForPlayer = REBASE(0x5C7380);
        const uintptr_t GetTranslatorForPlayerAsync = REBASE(0x5C5CE0);
    }
    namespace SoundService
    {
        const uintptr_t PlayLocalSound = REBASE(0x30C650);
    }
    namespace TextChatService
    {
        const uintptr_t DisplayBubble = REBASE(0x491C60);
    }
    namespace Raknet
    {
        const uintptr_t AckNak = REBASE(0x631750);
        const uintptr_t AddedConnectRequestQueue = REBASE(0x286DA00);
        const uintptr_t AppLevelConnectionAccepted = REBASE(0x287A2E0);
        const uintptr_t AttemptsSerializeRakNetACKsFailedMaxData = REBASE(0x289E4A0);
        const uintptr_t BadOfflineMessageReceivedMessageId = REBASE(0x2873A30);
        const uintptr_t BufferedCommands = REBASE(0x6313F0);
        const uintptr_t ClearingRequestedConnectionList = REBASE(0x2872CC0);
        const uintptr_t ClientRecvFromRaknet = REBASE(0x28624E0);
        const uintptr_t ConnectRequestedAttemptCountTimeBetween = REBASE(0x2867280);
        const uintptr_t CreateOpenRequest1MTU = REBASE(0x2877BE0);
        const uintptr_t DatagramHistory = REBASE(0x6316F0);
        const uintptr_t ExtDrop = REBASE(0x2863680);
        const uintptr_t Memory = REBASE(0x6312B0);
        const uintptr_t OtherStatsDeadOutgoingWaitingAcksWaitingNetCapac = REBASE(0x287B650);
        const uintptr_t PacketReturn = REBASE(0x631450);
        const uintptr_t ProcessNetworkPacket = REBASE(0x287E250);
        const uintptr_t RakNetCapabilityBitNegotiated = REBASE(0x2882360);
        const uintptr_t RakNetConnectTrace = REBASE(0x2862570);
        const uintptr_t RakNetConnectionFailedAfterTooMany = REBASE(0x28783C0);
        const uintptr_t RakNetHasProducedPacketsFramesMs = REBASE(0x2869130);
        const uintptr_t RakNetMissingPingPong = REBASE(0x2862510);
        const uintptr_t RakNetPingFrequencyMillisecond = REBASE(0x631C00);
        const uintptr_t RakNetPktTrace = REBASE(0x2862A20);
        const uintptr_t RakNetResendRttMultiple = REBASE(0x634670);
        const uintptr_t RakNetStoppedProduction = REBASE(0x2863310);
        const uintptr_t RaknetRawpeerDetachingPeerPlugin = REBASE(0x47C8B80);
        const uintptr_t Receive = REBASE(0x2893F90);
        const uintptr_t RecvBuffer = REBASE(0x631390);
        const uintptr_t RecvFromLoop = REBASE(0x287EDC0);
        const uintptr_t ReliabilityLayerLog = REBASE(0x2891690);
        const uintptr_t RxHandling = REBASE(0x631690);
        const uintptr_t RxOrderedHeap = REBASE(0x631570);
        const uintptr_t RxOutput = REBASE(0x631630);
        const uintptr_t RxSplitReassembly = REBASE(0x631280);
        const uintptr_t SendPacketsToSelf = REBASE(0x2867580);
        const uintptr_t SerializeAcks = REBASE(0x289E080);
        const uintptr_t Startup = REBASE(0x631330);
        const uintptr_t TxDatagram = REBASE(0x631510);
        const uintptr_t TxOutgoing = REBASE(0x6314B0);
        const uintptr_t UpdateNetworkLoop = REBASE(0x287F100);
    }
    namespace Signal
    {
        const uintptr_t Disconnect = REBASE(0x41C0290);
        const uintptr_t Index = REBASE(0x41BBE10);
    }
    namespace Connection
    {
        const uintptr_t Index = REBASE(0x41BFB20);
    }
    namespace RemoteEvent
    {
        const uintptr_t RemoteEventDelayedQueueLimit = REBASE(0x6B0230);
    }
    namespace RemoteFunction
    {
        const uintptr_t InvokeClient = REBASE(0x3351D0);
        const uintptr_t InvokeClientInternal = REBASE(0xC5B6D0);
        const uintptr_t InvokeServer = REBASE(0x3353D0);
        const uintptr_t InvokeServerInternal = REBASE(0xC5B530);
        const uintptr_t OnClientInvoke = REBASE(0x359CD70);
        const uintptr_t OnServerInvoke = REBASE(0x359CCA0);
        const uintptr_t RemoteFunctionDelayedQueueLimit = REBASE(0x6B5560);
        const uintptr_t RemoteFunctionMissingExecuteCall = REBASE(0x5859BF0);
    }
    namespace CollectionService
    {
        const uintptr_t CollectionServiceJob = REBASE(0x33A48A0);
    }
    namespace Task
    {
        const uintptr_t Cancel = REBASE(0x436A670);
        const uintptr_t Defer = REBASE(0x4369920);
        const uintptr_t Delay = REBASE(0x436A170);
        const uintptr_t Desynchronize = REBASE(0x4368CE0);
        const uintptr_t Spawn = REBASE(0x4369DE0);
        const uintptr_t Synchronize = REBASE(0x4368870);
        const uintptr_t Wait = REBASE(0x436A470);
        const uintptr_t checkForValidTaskScheduleState = REBASE(0x4368230);
        const uintptr_t cleanDelayDuration = REBASE(0x436A000);
        const uintptr_t getTaskLibraryFunction = REBASE(0x4367960);
        const uintptr_t getTaskScheduleStateName = REBASE(0x4368320);
        const uintptr_t setupTaskCallbackThread = REBASE(0x4369150);
    }
    namespace Input
    {
        const uintptr_t GetMouseButtonsPressed = REBASE(0x23B660);
        const uintptr_t IsMouseButtonPressed = REBASE(0x3C32800);
        const uintptr_t SendKey = REBASE(0x7D3A30);
        const uintptr_t SendKeyCore = REBASE(0x4C90160);
        const uintptr_t SendKeyEvent = REBASE(0x7D00F0);
        const uintptr_t SendMouseButton = REBASE(0x7D3480);
        const uintptr_t SendMouseButtonCore = REBASE(0x4C8FB30);
        const uintptr_t SendMouseDelta = REBASE(0x7D3270);
        const uintptr_t SendMouseDeltaCore = REBASE(0x4C8F9D0);
        const uintptr_t SendMouseMoveDeltaEvent = REBASE(0x7D0DD0);
        const uintptr_t SendMouseMoveEvent = REBASE(0x7D0BA0);
        const uintptr_t SendMousePosition = REBASE(0x7D3060);
        const uintptr_t SendMousePositionCore = REBASE(0x4C8F870);
        const uintptr_t SendPointerAction = REBASE(0x7D36D0);
        const uintptr_t SendPointerActionCore = REBASE(0x4C8FD30);
        const uintptr_t SendScroll = REBASE(0x7D1000);
        const uintptr_t SendTextInput = REBASE(0x7D3C80);
        const uintptr_t SendTextInputCharacterEvent = REBASE(0x7D1450);
        const uintptr_t SendTextInputCore = REBASE(0x4C90470);
        const uintptr_t SendTouchEvent = REBASE(0x7D1660);
    }
    namespace ClickDetector
    {
        const uintptr_t MouseClick = REBASE(0x254650);
        const uintptr_t MouseHoverEnter = REBASE(0x2546B0);
        const uintptr_t MouseHoverLeave = REBASE(0x254710);
        const uintptr_t RightMouseClick = REBASE(0x254770);
        const uintptr_t mouseClick = REBASE(0x2547D0);
    }
    namespace StringLib
    {
        const uintptr_t luaopen_string = REBASE(0x57A8160);
        const uintptr_t str_byte = REBASE(0x579F8D0);
        const uintptr_t str_capture = REBASE(0x57A06E0);
        const uintptr_t str_char = REBASE(0x579FB60);
        const uintptr_t str_classend = REBASE(0x579FC60);
        const uintptr_t str_find = REBASE(0x57A1770);
        const uintptr_t str_format = REBASE(0x57A3740);
        const uintptr_t str_gmatch = REBASE(0x57A1B60);
        const uintptr_t str_gsub = REBASE(0x57A3130);
        const uintptr_t str_len = REBASE(0x579E3E0);
        const uintptr_t str_lower = REBASE(0x579F040);
        const uintptr_t str_match = REBASE(0x57A00D0);
        const uintptr_t str_pack = REBASE(0x57A5320);
        const uintptr_t str_packsize = REBASE(0x57A5C90);
        const uintptr_t str_rep = REBASE(0x579F2E0);
        const uintptr_t str_reverse = REBASE(0x579EEF0);
        const uintptr_t str_split = REBASE(0x57A3D70);
        const uintptr_t str_sub = REBASE(0x579E490);
        const uintptr_t str_unpack = REBASE(0x57A6000);
        const uintptr_t str_upper = REBASE(0x579F190);
    }
    namespace TableLib
    {
        const uintptr_t tab_clear = REBASE(0x57966F0);
        const uintptr_t tab_clone = REBASE(0x5796990);
        const uintptr_t tab_concat = REBASE(0x57936F0);
        const uintptr_t tab_create = REBASE(0x5795A60);
        const uintptr_t tab_find = REBASE(0x5796250);
        const uintptr_t tab_foreach = REBASE(0x578E730);
        const uintptr_t tab_foreachi = REBASE(0x578D9C0);
        const uintptr_t tab_freeze = REBASE(0x5796740);
        const uintptr_t tab_getn = REBASE(0x578FB70);
        const uintptr_t tab_insert = REBASE(0x5790C80);
        const uintptr_t tab_isfrozen = REBASE(0x57968C0);
        const uintptr_t tab_maxn = REBASE(0x578F8B0);
        const uintptr_t tab_move = REBASE(0x57911F0);
        const uintptr_t tab_pack = REBASE(0x5793A10);
        const uintptr_t tab_remove = REBASE(0x5790E80);
        const uintptr_t tab_sort = REBASE(0x57958E0);
        const uintptr_t tunpack = REBASE(0x5794250);
    }
    namespace Bit32Lib
    {
        const uintptr_t bit32_arshift = REBASE(0x57BA5A0);
        const uintptr_t bit32_band = REBASE(0x57BA010);
        const uintptr_t bit32_bnot = REBASE(0x57BA2C0);
        const uintptr_t bit32_bor = REBASE(0x57BA0C0);
        const uintptr_t bit32_btest = REBASE(0x57BA040);
        const uintptr_t bit32_bxor = REBASE(0x57BA1C0);
        const uintptr_t bit32_byteswap = REBASE(0x57BAE00);
        const uintptr_t bit32_countlz = REBASE(0x57BAB60);
        const uintptr_t bit32_countrz = REBASE(0x57BACB0);
        const uintptr_t bit32_extract = REBASE(0x57BA960);
        const uintptr_t bit32_lrotate = REBASE(0x57BA790);
        const uintptr_t bit32_lshift = REBASE(0x57BA350);
        const uintptr_t bit32_replace = REBASE(0x57BAA20);
        const uintptr_t bit32_rrotate = REBASE(0x57BA810);
        const uintptr_t bit32_rshift = REBASE(0x57BA470);
    }
    namespace BufferLib
    {
        const uintptr_t buf_copy = REBASE(0x57B07C0);
        const uintptr_t buf_create = REBASE(0x57AF450);
        const uintptr_t buf_fill = REBASE(0x57B0AB0);
        const uintptr_t buf_fromstring = REBASE(0x57AF500);
        const uintptr_t buf_len = REBASE(0x57B0710);
        const uintptr_t buf_readbits = REBASE(0x57B0D00);
        const uintptr_t buf_readf32 = REBASE(0x57B1AC0);
        const uintptr_t buf_readf64 = REBASE(0x57B1C30);
        const uintptr_t buf_readi16 = REBASE(0x57B1540);
        const uintptr_t buf_readi32 = REBASE(0x57B1800);
        const uintptr_t buf_readi8 = REBASE(0x57B1280);
        const uintptr_t buf_readinteger = REBASE(0x57AFB50);
        const uintptr_t buf_readstring = REBASE(0x57AFDA0);
        const uintptr_t buf_readu16 = REBASE(0x57B16A0);
        const uintptr_t buf_readu32 = REBASE(0x57B1960);
        const uintptr_t buf_readu8 = REBASE(0x57B13E0);
        const uintptr_t buf_tostring = REBASE(0x57AF5D0);
        const uintptr_t buf_writebits = REBASE(0x57B0F40);
        const uintptr_t buf_writef32 = REBASE(0x57B2210);
        const uintptr_t buf_writef64 = REBASE(0x57B23A0);
        const uintptr_t buf_writeinteger = REBASE(0x57AFC60);
        const uintptr_t buf_writestring = REBASE(0x57B0470);
        const uintptr_t buf_writeu16 = REBASE(0x57B1F10);
        const uintptr_t buf_writeu32 = REBASE(0x57B2090);
        const uintptr_t buf_writeu8 = REBASE(0x57B1D90);
    }
    namespace CoroutineLib
    {
        const uintptr_t co_close = REBASE(0x57B6330);
        const uintptr_t co_create = REBASE(0x57B59B0);
        const uintptr_t co_isyieldable = REBASE(0x57B62A0);
        const uintptr_t co_running = REBASE(0x57B6230);
        const uintptr_t co_status = REBASE(0x57B3F30);
        const uintptr_t co_wrap = REBASE(0x57B5F60);
        const uintptr_t co_yield_fn = REBASE(0x57B61D0);
    }
    namespace Utf8Lib
    {
        const uintptr_t utf8_char = REBASE(0x579A6E0);
        const uintptr_t utf8_codepoint = REBASE(0x579A340);
        const uintptr_t utf8_codes = REBASE(0x579B3E0);
        const uintptr_t utf8_len = REBASE(0x579A060);
        const uintptr_t utf8_offset = REBASE(0x579AE90);
        const uintptr_t utf8_to_wideconvertstringerror_ = REBASE(0x284E910);
    }
    namespace MathLib
    {
        const uintptr_t math_abs = REBASE(0x57AA470);
        const uintptr_t math_acos = REBASE(0x57AABE0);
        const uintptr_t math_asin = REBASE(0x57AAAF0);
        const uintptr_t math_atan = REBASE(0x57AACD0);
        const uintptr_t math_atan2 = REBASE(0x57AADC0);
        const uintptr_t math_ceil = REBASE(0x57AAF20);
        const uintptr_t math_clamp = REBASE(0x57ACC00);
        const uintptr_t math_cos = REBASE(0x57AA730);
        const uintptr_t math_cosh = REBASE(0x57AA820);
        const uintptr_t math_deg = REBASE(0x57AB990);
        const uintptr_t math_exp = REBASE(0x57AB8A0);
        const uintptr_t math_floor = REBASE(0x57AB010);
        const uintptr_t math_fmod = REBASE(0x57AB100);
        const uintptr_t math_frexp = REBASE(0x57ABB70);
        const uintptr_t math_isfinite = REBASE(0x57AD6B0);
        const uintptr_t math_isinf = REBASE(0x57AD5C0);
        const uintptr_t math_isnan = REBASE(0x57AD4E0);
        const uintptr_t math_ldexp = REBASE(0x57ABC70);
        const uintptr_t math_lerp = REBASE(0x57AD300);
        const uintptr_t math_log = REBASE(0x57AB5F0);
        const uintptr_t math_log10 = REBASE(0x57AB7B0);
        const uintptr_t math_map = REBASE(0x57AD020);
        const uintptr_t math_max = REBASE(0x57ABF50);
        const uintptr_t math_min = REBASE(0x57ABDE0);
        const uintptr_t math_modf = REBASE(0x57AB260);
        const uintptr_t math_noise = REBASE(0x57AC8C0);
        const uintptr_t math_pow = REBASE(0x57AB490);
        const uintptr_t math_rad = REBASE(0x57ABA80);
        const uintptr_t math_random = REBASE(0x57AC0C0);
        const uintptr_t math_randomseed = REBASE(0x57AC3E0);
        const uintptr_t math_round = REBASE(0x57ACF30);
        const uintptr_t math_sign = REBASE(0x57ACE30);
        const uintptr_t math_sin = REBASE(0x57AA550);
        const uintptr_t math_sinh = REBASE(0x57AA640);
        const uintptr_t math_sqrt = REBASE(0x57AB390);
        const uintptr_t math_tan = REBASE(0x57AA910);
        const uintptr_t math_tanh = REBASE(0x57AAA00);
    }
    namespace OsLib
    {
        const uintptr_t os_clock = REBASE(0x579C2B0);
        const uintptr_t os_date = REBASE(0x579D820);
        const uintptr_t os_difftime = REBASE(0x579E230);
        const uintptr_t os_time = REBASE(0x579DF30);
    }
    namespace VectorLib
    {
        const uintptr_t vec_abs = REBASE(0x57A8AB0);
        const uintptr_t vec_angle = REBASE(0x57A86E0);
        const uintptr_t vec_ceil = REBASE(0x57A8A10);
        const uintptr_t vec_clamp = REBASE(0x57A8BE0);
        const uintptr_t vec_create = REBASE(0x57A8190);
        const uintptr_t vec_cross = REBASE(0x57A8510);
        const uintptr_t vec_dot = REBASE(0x57A85E0);
        const uintptr_t vec_floor = REBASE(0x57A8970);
        const uintptr_t vec_lerp = REBASE(0x57A9120);
        const uintptr_t vec_magnitude = REBASE(0x57A8340);
        const uintptr_t vec_max = REBASE(0x57A8E70);
        const uintptr_t vec_min = REBASE(0x57A8D50);
        const uintptr_t vec_normalize = REBASE(0x57A8430);
        const uintptr_t vec_sign = REBASE(0x57A8B20);
    }
    namespace Global
    {
        const uintptr_t Delay = REBASE(0x42AD8B0);
        const uintptr_t ElapsedTime = REBASE(0x42AE0E0);
        const uintptr_t PluginManager = REBASE(0x42AE7E0);
        const uintptr_t Spawn = REBASE(0x42AF850);
        const uintptr_t Stats = REBASE(0x42B0090);
        const uintptr_t UserSettings = REBASE(0x42B0840);
        const uintptr_t Version = REBASE(0x42B0CD0);
        const uintptr_t Wait = REBASE(0x42B0DB0);
        const uintptr_t collectgarbage = REBASE(0x42AD040);
        const uintptr_t getfenv = REBASE(0x42B1780);
        const uintptr_t loadstring = REBASE(0x42AE160);
        const uintptr_t print = REBASE(0x42AED20);
        const uintptr_t printInternal = REBASE(0x4284870);
        const uintptr_t require = REBASE(0x42AEE30);
        const uintptr_t setfenv = REBASE(0x42B18B0);
        const uintptr_t settings = REBASE(0x42AF340);
        const uintptr_t tick = REBASE(0x42B05C0);
        const uintptr_t time = REBASE(0x42B0640);
        const uintptr_t warn = REBASE(0x42B0F60);
    }
    namespace CreatorLog
    {
        const uintptr_t Print = REBASE(0x1D2EEB0);
        const uintptr_t print = REBASE(0x4284870);
    }
    namespace ClassDescriptor
    {
        const uintptr_t BucketIndices = 0x208;
        const uintptr_t BucketMask = 0x218;
        const uintptr_t PropertyMap = 0x200;
        const uintptr_t PropertyTable = 0x210;
        const uintptr_t GetProperty = REBASE(0x1D68D10);
    }
    namespace Properties
    {
        const uintptr_t BitFlags = 0x8C;
    }
    namespace Reflections
    {
        const uintptr_t CastArgs = REBASE(0x415A750);
    }
    namespace ContentProvider
    {
        const uintptr_t ContentProvider = REBASE(0x12791E0);
        const uintptr_t getDependencyContentIds = REBASE(0x1289220);
        const uintptr_t onServiceProvider = REBASE(0x1276E80);
        const uintptr_t registerEncryptedAsset = REBASE(0x128AB70);
    }
    namespace Security
    {
        const uintptr_t DMLockViolation = REBASE(0x4949C30);
    }
}