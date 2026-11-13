# UYA NTSC-U and PAL port investigation

## PAL online login and save export (2026-11-13)

User reported passing the DNAS stage and successfully logging in using the
loader-backed PAL payload. This is user-observed online-flow verification;
no claim of standalone save exploitation or uninterrupted resident state.

Closed the isolated emulator cleanly before copying its complete PAL save
folder, including the updated net.bin. Replaced main.ps2/BESCES-52456RATCHET
after backing up the prior directory. Exported a separate Desktop folder and
an EMS/PSU archive; verified all 8 game files byte-for-byte after decoding the
archive layout. PCSX2 metadata was retained in the folder copy only. Saved
network data differs from the original downloaded European save. Login data
and save archives remain outside the repositories and are not committed.

The exported save contains progress and saved network settings, not a DNAS
payload or loader. Real-PS2 testing still needs an independent launch/patch
mechanism; racdoor's UYA save bootstrap is not implemented.

## PAL DNAS payload executed in multiplayer (2026-11-13)

After user loaded the native PAL campaign save and entered multiplayer, live
inspection confirmed both intended instructions:
- 0x004d55d8 = 0x56220000
- 0x004d55e4 = 0x24020005

Full 11-word guard window matched the expected patched form. Telemetry at
0x000a6020: initCount 1, campaignTicks 0, multiplayerTicks 789, attempts 789,
applied 1, lastResult 2 (already patched), mode 2, context 2, level 39.
Emulator remained running. Snapshot: loader build/racdoor-dnas-test-host/live-check.json.

Critical lifecycle finding: emulog at 496.1543 reports loader starting in mode 2,
then reloading racdoor-dnas-test at 0x000a5d40. Campaign counters reset. The
PNACH-backed loader therefore REINITIALIZED across this transition; equal
module addresses and initCount 1 do not prove retained state. The payload ran
successfully after reloading (log at 502.7322), but a one-shot racdoor bootstrap
still needs a verified transition/persistence mechanism. Online DNAS flow,
DNS routing, and compatibility with the downloaded Horizon patch remain untested.

## PAL loader-backed DNAS test prepared (2026-11-13)

Added PAL-only `uya-mod-loader/modules/racdoor-dnas-test`, embedding the exact
384-byte raw worker built above via `.incbin`. The loader's existing lifecycle
runs the module in both campaign and multiplayer; only multiplayer level 39
calls the guarded worker. Exported counters track initialization, ticks,
attempts, successful applications, and the last worker result. PAL build and
existing relocation comparison passed.

Isolated test host root: `uya-mod-loader/build/racdoor-dnas-test-host`, containing
an ISO hard link and only this module. The probe emulator uses DnasTestCheats
with loader PNACH for campaign CRC 17125698 and MP CRC ede8b391. Direct DNAS
PNACH edits are not installed. Native PAL card remains selected. Previous
paused copy-loop breakpoint state preserved in isolated save-state slot 10;
pre-test INI saved as `inis/PCSX2.before-dnas-test.ini`.

Observed campaign startup: module 0x000a5d40, exports 0x000a6020, initCount 1,
increasing campaignTicks, zero MP ticks/attempts/writes. Multiplayer transition
verification pending. User will load native save, quit to title, select MP.

Important: PNACH rewrites separate campaign and MP sync-entry hooks. Its code
and persistent module state occupy low memory. This harness proves neither
save-bootstrap execution nor automatic hook reinstatement by a one-shot
payload; the UYA injector remains gated. DNS configuration and Horizon patch
low-memory compatibility remain separate tests.

## PAL DNAS callable payload built (2026-11-13)

User requested Horizon Launcher's PAL DNAS Skip/DNS bypass as a test payload.
Added `samples/uya-dnas-pal/` with guarded C payload, raw-image linker script,
existing EE Make integration, host tests, and README. This is a standalone
callable worker, NOT an RDX module, save bootstrap, or persistent overlay hook.
UYA injector rejection remains unchanged. No game memory or files in the test
card were modified; the campaign transition breakpoint state was left intact.

Copied behavior from PAL multiplayer entry in
`D:/Repos/Horizon/horizon-launcher/launcher/core/patches.cs` (also matches
Metroynome/UYALauncher). No separate DNS code patch was found in launcher C#.
- 0x004d55d8: 0x56220003 -> 0x56220000 (low byte only; NOT a NOP).
- 0x004d55e4: 0x24020006 -> 0x24020005 (low byte only).

Verified the full 11-word guard using stock extracted PAL multiplayer-menu
`overlay.elf`; local `pal.39.Lobby.bin` already has both edits. Worker rejects
wrong/partially patched windows, reports already patched, and flushes D/I
caches via verified PS2SDK syscall 0x64 modes 0 then 2 after writes.
Entry `uyaDnasEntry` at raw offset 0, no arguments; v0=0 mismatch, 1 applied,
2 already patched. Standard EE callable ABI; interrupted-register preservation,
placement, scheduling, and overlay lifetime belong to a future test loader.

Build/test command (from Windows):

```powershell
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src/samples/uya-dnas-pal uya-mod-loader make TARGET=ee ee test
```

Artifact: `samples/uya-dnas-pal/uya-dnas-pal.bin`, 384 bytes, SHA-256
`174ea4813514aa80d1d0e2c9a4397904affedb6595004bf41ed3b63aca74696e`.
Host tests passed exact writes/boundaries, idempotence, every guard mismatch,
and both partial-patch cases. Final ELF/raw inspection: text only, no data/BSS,
relocations, or absolute jumps; all conditional branches remain internal;
exactly two byte stores and two cache syscalls. `git diff --check` passed.

No runtime DNAS test or save-triggered payload execution has occurred. Next:
provide a validated loader/storage/call site in the PAL multiplayer menu,
verify stock words, invoke worker and inspect return/patches, then test DNAS
and normal continuation. Campaign travel reachability alone does not establish
multiplayer persistence or make these addresses safe in campaign overlays.

## PAL transition breakpoint verified (2026-11-13)

User reported the armed breakpoint tripped while traveling from native PAL
Phoenix. Read-only DebugServer capture confirms the emulator is PAUSED at
`0x0013d27c`; level global is 2 (Florana). The original instruction remains
`0x1440fffa` (`bnez v0,0x0013d268`), followed by delay-slot instruction
`0xa0830000` (`sb v1,0(a0)`). History count is 77 after the verified PAL
QuickSelect help event. No payload or modified branch was installed.

Captured low 32-bit register values:

| Register | Value |
| --- | --- |
| a0 | 0x001685e0 |
| a1 | 0x001f3340 |
| a2 | 0x00000060 |
| a3 | 0x00000001 |
| t0 | 0x001685e0 |
| v0 | 0x00000001 |
| v1 | 0x00000060 |
| ra | 0x0013d35c |
| sp | 0x01fffc70 |

This verifies PAL reachability of the candidate branch during the 96-byte
copy on planet travel, with a nonzero branch condition. It does not establish
modified-branch execution, trampoline lifetime, or save-triggered payload
execution. The delay slot and branch destination have not been stepped through
in this PAL capture.

Full registers, status, hook bytes, history count, and breakpoint list saved at:
`C:/Users/Troy/AppData/Local/Temp/racdoor-uya-probe-20261113/pal-native-transition-breakpoint.json`.

Left emulator paused and the persistent breakpoint enabled, as observed.
Before resuming normal gameplay, remove the breakpoint at 0x0013d27c to avoid
stopping on every loop iteration. For continuation verification, remove it and
set an observational breakpoint at the original destination 0x0013d268 before
resuming. Do not rely on this build's step() or chained temporary breakpoints
without checking the resulting PC.

## Native PAL help trigger verified live (2026-11-13)

User loaded native European MAX Phoenix save. Verified level 3 and actual PAL
call/wrapper/history instructions before setting breakpoints. Help records
0x11e/0x11f were zero, but block-5 flag byte at 0x001427bf was 4 and history
count 77 included QuickSelect. Backed up live state as
`pal-native-help-before-reset-1794604169366.json`, then cleared bit 2 and both
records and removed only the QuickSelect entry (77 -> 76). No disk save edit.

Observed call breakpoint 0x004cf720 with a0=0x14fb. Temporary breakpoints
were all absent after resume; do not rely on chaining temporary breakpoints in
this build. Condition requests also reported has_condition=false. The ordinary
call appended 0x14fb (76 -> 77), record 0x11e count became 1, flag returned 4.

Repeated targeted reset with persistent breakpoints, removed explicitly:
- Wrapper 0x00407a60: a0=0x14fb, a1=0x11e, ra=0x004cf728.
- History entry 0x004091f8: a0=0x14fb, ra=0x00407b10, history still 76.
Captured full registers in `pal-native-help-wrapper-hit.json` and
`pal-native-history-entry.json`. Removed both breakpoints and resumed.

Armed ONE persistent observational breakpoint at 0x0013d27c, original branch
0x1440fffa. Next: user travels from Phoenix to another planet; capture loop
registers and remove breakpoint before continuation (otherwise every iteration
stops). PAL help/history is now verified; PAL transition reachability and
save-triggered payload execution remain unproven. No payload/code installed.

User separately authorized copying original native MAX save0 into MAIN PAL
card; completed with hash verification. Old main PAL save backed up at
`C:/Users/Troy/AppData/Local/Temp/racdoor-main-pal-backup-20261113-160832/save0.bin`.
This supersedes older blanket statements that the main card was never changed.
Live help reset applies only to disposable emulator memory, not main save.

## Native European MAX save ready (2026-11-13)

User downloaded `C:/Users/Troy/Downloads/ratchet-and-clank-up-your-arsenal.17106.max`.
Header identifies `BESCES-52456RATCHET`. Validated container CRC32 and extracted
8 files using mymc format documentation/source and mymc+ LZARI decoder, kept
under `<probe root>/max-extractor`. Upstream sources:
https://github.com/ps2dev/mymc/blob/master/ps2save.py
https://github.com/thestr4ng3r/mymcplus/blob/master/mymcplus/save/lzari.py
No downloaded executable or global package installation.

Extraction is `<probe root>/european-max-extracted/BESCES-52456RATCHET`.
Native `save0.bin`: level 3 (Phoenix), history 77, SHA-256
`b00bfef58a94b713e42eb80b91a545441046bb6bf4795381b17b1e28d646c25c`.
Other slots are empty. Existing injector parsed/validated every section checksum
and produced separate `european-max-help-reset.bin` (one entry removed), but
this reset version is NOT installed. The test uses the UNMODIFIED extraction.

Prepared a fresh copied folder card `MemoryCards/uya-native-pal-max.ps2`, with
all extracted PAL files, preserving original cards and previous test cards.
Stopped only isolated emulator, selected this new card, and cold-booted PAL
visibly. TLB pause remains enabled; normal EE rate and 32 MB retained.
Next: user loads FIRST slot (native PAL Phoenix) and tests ship interaction.
No payload or runtime patch is installed in this fresh session.

### Florana rendering investigation findings (not a permanent fix)

Full saved fault is isolated savestate slot 9. Four cutscene actors at
`0x01cb3900`, `0x01cb4400`, `0x01cb6900`, `0x01cb6a00` selected animation
frames 40/41. Their sequence tables each have 41 frames (valid 0-40).
Reading entry 41 returns the first animation-data float `0x3ed55555`, causing
the repeated TLB accesses. Live rendering instructions match the extracted
PAL Florana overlay. Cutscene writer `0x00484868..0x00484878` unconditionally
sets next frame to `(timeline >> 1) + 1`, then calls initializer `0x00411730`.
Captured writer at `0x0048486c`, v1=40, v0=41; scene struct `0x001e5640`.
Why the scene timeline exceeds these assets is NOT yet established.

Disabling PauseOnTLBMiss failed (DMA errors and no progression); restored it.
Reversible live frame/pointer clamps allowed the initial rendering call to
return, but the next update recreated the invalid frame. Temporarily shortening
current scene length to 80 used normal completion, but a subsequent animation
faulted again. No persistent code patch or save edit resulted. All these runtime
changes were discarded by the native MAX cold boot. Evidence includes
`pal-florana-repeat.json`, `four-actor-clamp.json`, `frame-index-writer.json`,
`fault-moby.bin`, and `florana-animation-investigation-emulog.txt` in probe root.

## Native PAL control prepared after Florana fault (2026-11-13)

Unchanged live NTSC-U Florana save loaded, but exiting the ship caused another
load TLB miss: log PC `0x00417fa0`, invalid address `0x3ed5555b`, paused PC
`0x00418000`, level 2, original racdoor hook intact. Full registers/log are
`pal-florana-ship-tlb.json` and `pal-florana-ship-tlb-emulog.txt` in probe root.
Disassembly is the same rendering routine pattern as the earlier level 7/14
faults. Logged PC again differs from the apparent invalid t5-based load later
in the recompiled block; do not infer exact fault operands from logged PC alone.
Local GameIndex.yaml specifies EETimingHack for PAL; fresh log explicitly
confirms EETiming is enabled. No additional emulator settings were changed.

Prepared a control: stopped the isolated process, backed up copied PAL slot 0,
then replaced it with a verified exact copy of ORIGINAL PAL
`D:/PS2/PCSX2/memcards/main.ps2/BESCES-52456RATCHET/save0.bin` (level 1).
Second slot retains the unchanged live NTSC-U Florana conversion. Restarted
PAL visibly with same settings. Next user test: load FIRST slot (native PAL
Veldin) and exercise gameplay to see whether the rendering/TLB failure repeats.
A pass in this different level is useful but cannot alone prove a conversion
bug; compare the same ship/rendering interaction if possible. No payload runs.

## Correct live NTSC-U save1 source (2026-11-13)

User clarified that the desired save is from the live NTSC-U test card, not
original main.ps2. Correct source:
`C:/Users/Troy/AppData/Local/Temp/racdoor-manual-test-20261113-145755/memcards/uya-test.ps2/BASCUS-97353RATCHET/save1.bin`.
Verified level 2 (Florana), history 81, modified 2026-11-13 20:28:08 UTC.
SHA-256: `885f4dbbb6bee6a7e016e421f225c206becadb173412855eb178037bf4c67b5d`.

The previous Koros candidate loaded, but user entering the ship produced a
load TLB miss at `0x0042aa28`, address `0x3ed5555b`; paused PC `0x0042aa88`,
level 14, original hook intact. Preserved full registers and log as
`pal-koros-ship-tlb.json` and `pal-koros-ship-tlb-emulog.txt` in the probe root.
This is the same invalid address as the earlier Annihilation Nation failure.

Stopped only the isolated PAL emulator, backed up its current second slot,
then copied the correct live NTSC-U save1 BYTE-FOR-BYTE to copied PAL
`BESCES-52456RATCHET/save1.bin`. Verified matching hashes. No help reset or
payload modification in this candidate: first establish save/ship compatibility.
Restarted PAL visibly. User should load SECOND slot (Florana), then test ship
travel to Phoenix. Standard EE rate/32-MB settings retained. Original cards
and the live NTSC-U source remain untouched. Runtime outcome pending.

## NTSC-U save1 PAL retry (2026-11-13)

The normal-speed/32-MB retry reproduced exactly the same load TLB miss:
PC `0x004405a8`, address `0x3ed5555b`, paused PC `0x00440608`, level 7.
Original hook remained intact. Preserved `pal-conversion-tlb-baseline.json`
and `pal-conversion-tlb-baseline-emulog.txt` in the isolated test root.
Normalizing those two settings did not resolve the fault.

User clarified the NTSC-U test save was `save1.bin`. Read original
`BASCUS-97353RATCHET/save1.bin` through a read-only Docker mount and generated
`pal-from-ntsc-save1-help-reset.bin` with `--reset-uya-help`. This starts at
level 14 (Koros), history count 80 after removing one QuickSelect entry.
SHA-256: `465c5c8aca552e88c23d006eeb2ad30e20897f1da44ff19a7091eecf5c412fd6`.
Stopped the isolated emulator, backed up the copied PAL second slot to
`pal-save1-before-conversion.bin`, and installed the candidate as copied
`BESCES-52456RATCHET/save1.bin`. Restarted PAL visibly with normal EE rate
and extended RAM disabled. Load the SECOND menu slot for this retry.
First slot retains the earlier Annihilation Nation candidate for comparison.
Original memory cards remain untouched; no payload is installed.

## PAL converted-save TLB fault (2026-11-13)

User loaded the converted first slot into Annihilation Nation. DebugServer
confirmed level 7, history count 80, original hook `0x1440fffa`, and paused PC
`0x00440608`. Log reports load TLB miss at PC `0x004405a8`, address
`0x3ed5555b`. Paused registers are after the logged fault and must not be
assumed to describe its exact instruction: live disassembly at `0x004405a8`
is `lw t3,0x18(t7)` while paused t7 is `0x01f0c000`; paused t5 is
`0x3ed55565`. Rendering/geometry instructions surround the fault.
Full registers: `pal-conversion-tlb.json`; preserved log:
`pal-conversion-tlb-emulog.txt`, both in the isolated test root.
No payload, history corruption, or modified branch was installed.

The inherited test settings had EECycleRate=2 and ExtraMemory=true. Backed up
config as `PCSX2.ini.before-pal-baseline`, stopped only the isolated process,
changed these to 0 and false, and cold-booted the same PAL ISO/card visibly.
Awaiting user reload of slot 1 to determine whether the fault repeats with
normal EE cycle rate and memory size. Do not describe conversion as proven
compatible, or these settings as the established cause. If it repeats, compare
against original PAL save and an unreset NTSC-U copy under the same config.

## PAL test restart and save conversion candidate (2026-11-13)

This checkpoint supersedes the earlier active-emulator description. Shell access
works with reviewed `require_escalated`; sandbox exec and Node/Computer Use
still fail at startup. Docker build, arithmetic tests (including four expected
rejections), and `git diff --check` passed on this retry.

Booted the isolated emulator at
`C:/Users/Troy/AppData/Local/Temp/racdoor-uya-probe-20261113/pcsx2-qt.exe`
with the PAL ISO. Log verified SCES-52456, CRC 17125698, and the copied card
`MemoryCards/uya-pal-test-153732.ps2`. DebugServer at port 21512 responds.
Before conversion, live level was -1 (no campaign loaded), history count 0,
and hook bytes were `faff4014000083a0` (original branch plus delay slot).
No breakpoints or payload were installed.

User has no later PAL save and asked about alternate help triggers or converting
the NTSC-U save. Read-only comparison of original save0 files found identical
file sizes (134992), shared size (14216), level size (3264), and all block IDs,
sizes, and offsets across all 38 sections. Original NTSC-U slot starts at level
7; PAL slot 0 starts at level 1, and PAL slots 1-3 are empty. This establishes
layout compatibility, not successful runtime conversion.

Generated `pal-from-ntsc-help-reset.bin` in the isolated test root using:

```text
tools/inject/inject --reset-uya-help /original/BASCUS-97353RATCHET/save0.bin /test/pal-from-ntsc-help-reset.bin
```

Original card was mounted read-only. Removed one QuickSelect history entry;
no payload installed. SHA-256:
`44d616f475b9baf9f5782a23a166cd1a0a3b30aa108bf6e4bb583bd72ff51071`.
Stopped the verified isolated process before changing its card. Backed up the
copied PAL slot as `pal-save0-before-conversion.bin` in the test root, then
replaced only copied `BESCES-52456RATCHET/save0.bin` with the reset NTSC-U
candidate. Restarted PAL fresh. Original cards/settings remain untouched.

Next: user loads first PAL save slot manually (Computer Use unavailable),
verify loaded level and progression using DebugServer, then travel to Phoenix.
Only install Phoenix-specific observational breakpoints after confirming that
overlay and its instruction bytes. Conversion acceptance, PAL help/history
trigger, transition-hook reachability, and payload execution remain unverified.

## Latest handoff (2026-11-13, read this first)

### Help reset implemented; campaign call reached live

**Florana transition checkpoint:** user traveled from Phoenix toward Florana.
The temporary breakpoint at `0x0013d27c` fired and was automatically removed.
Level global was 2. Original instruction remained `0x1440fffa`.
Registers: `a0 = t0 = 0x001685e0`, `a1 = 0x001f33c0`, `a2 = 0x60`,
`a3 = 1`, `v0 = 1`, `v1 = 0x60`, `ra = 0x0013d35c`,
`sp = 0x01fffc60`. Thus the hook is on an actually taken branch on the first
iteration of the known 96-byte copy during planet travel, after the campaign
help test. Full GPR snapshot is saved in
`<manual test root>/florana-copy-branch.json`.

DebugServer `step()` reported success but left PC at `0x0013d27c`, including
on a later status query; do not count that as verified stepping. Set a temporary
breakpoint at the original target `0x0013d268` and resumed to verify continuation
without changing instructions. No payload or branch modification was installed.

The destination breakpoint then fired at `0x0013d268`, with `a3 = 2`.
This confirms the loop destination, but the earlier `step()` evidently did not
provide a simple one-instruction observation: do not claim the destination was
captured on iteration 1. Both temporary copy-loop breakpoints were consumed;
the existing Phoenix help breakpoint remains. Resumed normal gameplay after
capturing the destination. NTSC-U help/history and transition-hook reachability
are now observed; trampoline lifetime, modified-branch execution, a proof
payload, and PAL runtime verification remain outstanding.

**After the user resumed:** NTSC-U Phoenix continued running. History count
returned from 80 to 81 with `0x14fb` as the last entry; record `0x11e` now
begins with count 1, and block-5 byte `0xdf` returned to `0x05`. The active
help ID was subsequently `0xffffffff` (idle), with record index `0x11e`.
This verifies the normal campaign help path and history append after resetting
the saved prerequisites. It does not yet verify the out-of-bounds write or
payload execution. Original hook remains `0x1440fffa`.

Installed a temporary observational breakpoint at `0x0013d27c` to catch the
original copy loop. Existing help-call breakpoint `0x004cb5b0` was preserved.
Next: observe whether ordinary gameplay hits it; otherwise have the user travel
to another planet. If it pauses, inspect `a1`, `a2`, `a3`, `t0`, `v0`, and `ra`
before resuming, and verify the store delay slot / taken branch condition.

This subsection supersedes earlier statements that no runtime trigger was
observed or that no emulator is running. User requested resetting viewed help
or using a new save with an early message. Resetting the existing campaign
save's QuickSelect help is sufficient to reach its caller; a new game is not
needed for this route. Earliest-message bootstrapping is not established.

Active emulator: `D:/PS2/PCSX2-MCP-v1.0.0-win64/pcsx2-qt.exe`, NTSC-U,
normal Phoenix (level 3). Its slot 1 is the disposable folder card at
`C:/Users/Troy/AppData/Local/Temp/racdoor-manual-test-20261113-145755/memcards/uya-test.ps2`.
Original INI backup is in that same test root as `PCSX2.ini.before-test`;
restore over the installation's `inis/PCSX2.ini` only after closing PCSX2.
Cheats/patches and slot 2 were disabled for the manual test.

Before the live reset, records `0x11e` and `0x11f` were already all zero.
The blocking independent flag at `0x001427bf` was `0x05` (bit 2 set).
Backed up the complete affected live state to
`<test root>/help-before-reset-1794601259100.json`, then paused briefly,
removed one `0x14fb`/`0x14fc` history entry (81 -> 80), cleared both records,
changed the flag to `0x01`, and resumed. No code or payload was written.

**Observed result:** emulator reached an existing enabled breakpoint at
`0x004cb5b0`, with `a0 = 0x14fb`. Live disassembly shows the help call to
`0x004051e0` and delay-slot `li a1,0x11e`. Left this breakpoint and paused
state intact for the user's test. This confirms the campaign caller, not yet
the history write, message display, or exploit execution. Resume/step over the
call to inspect its result; do not mistake a paused emulator for a crash.

Added `inject --reset-uya-help INPUT OUTPUT` in `tools/inject/inject.c`.
It validates the UYA block layout and ordinary history bounds, clears only
records `0x11e`/`0x11f` and block-5 byte `0xdf` bit 2, removes IDs `0x14fb`
and `0x14fc` from active history while preserving other entries/order, and
updates checksums. It installs no payload and does not change gadget unlocks,
camera mode, planet, or unrelated help records. Use different input/output
files; never patch an attached card file while PCSX2 is running.

Generated files (not installed over the running card):

- `<test root>/ntscu-help-reset.bin`, based on original NTSC-U slot 0 (level 7).
  139 changed bytes, all inside the shared checksum and intended help fields;
  history 81 -> 80. SHA-256:
  `44d616f475b9baf9f5782a23a166cd1a0a3b30aa108bf6e4bb583bd72ff51071`.
- `<test root>/pal-help-reset.bin`, based on original PAL slot 0 (level 1).
  Already unviewed, so output is byte-identical; history remains 0. SHA-256:
  `656997b7da170246e69832b6feb94a80cd46d280c23db977ed6d9d495404047e`.
  This early save still lacks the demonstrated nine-eligible-gadget prerequisite;
  resetting flags alone does not make the QuickSelect route fire there.

Validation: host injector builds with `-Wall -Werror`; existing arithmetic
tests pass. Independent byte/checksum inspection validated every section of
both generated saves, exact change boundaries, and preservation of unrelated
history. A second reset is byte-identical; identical input/output path is
rejected. PAL runtime remains untested. No proof binary is installed.

Next: let the current help call complete, observe the history writer and
subsequent candidate copy-loop hook during a planet transition, then decide
trampoline storage/continuation before implementing the minimal proof.

User confirmed the immediate deliverable is **a minimal proof that code runs
from a UYA save**, not loading an existing external payload or porting the
complete module SDK. No working proof binary/save has been produced yet.

Completed this continuation: successful baseline build/tests, independent
NTSC-U assembly review, direct PAL ELF comparisons, saved-storage ownership
checks, and a broader scan for alternate help triggers. Implementation code
is unchanged; the existing UYA injector rejection remains in place.

Runtime attempt: created an isolated portable emulator copy at
`C:/Users/Troy/AppData/Local/Temp/racdoor-uya-probe-20261113`. Original game
saves, states, and emulator settings were not modified. Attempted to load
`D:/PS2/PCSX2/sstates/SCUS-97353 (45FE0CC4).05.p2s`, which contains Phoenix
(level 3) and original hook bytes. The emulator rejected it: state version
`v2.5.402` is incompatible with this debugger build (`Unknown`). The debugger
connection consequently returned ECONNREFUSED. Log:
`<probe directory>/logs/emulog.txt`. The isolated process (PID 40832) was
stopped after verifying its executable path. No test game remains running.

The existing states can be inspected read-only using `ZSTD_decompress` from
the installed `D:/PS2/PCSX2-MCP-v1.0.0-win64/zstd.dll` through Python ctypes;
no dependency installation is needed. ZIP local-header name/extra lengths
locate each compressed member. State `.05` has level 3; `.09` has level 7;
`.10` has level 9. This inspection is not runtime execution.

Fresh-boot UI automation is currently blocked: initializing `@oai/sky` in
Node REPL fails before JavaScript runs with Windows sandbox
`helper_unknown_error: setup refresh had errors` (also reproduced this turn).
Shell commands work with reviewed `require_escalated`, but that does not repair
the Node/Computer Use kernel. Do not claim an emulator test passed.

Next actionable sequence:

1. Restore working Computer Use / use a compatible debug-enabled PCSX2 build,
   then boot a disposable card copy fresh. Do not rely on the incompatible
   state. The isolated config has cheats/patches and both card slots disabled;
   enable only a copied card when setting up this fresh-boot test.
2. For the original route, observe the `0x14fb` / record `0x11e` trigger and
   watch `0x0013d27c` across a planet transition. A periodic help check is
   established statically; execution of the later hook still is not.
3. If that route cannot preserve live block-39 data, investigate the alternate
   trigger candidates recorded below. Identify an actually executed hook and
   safe trampoline bytes before enabling any injector path.
4. Implement an explicit minimal proof using the existing C/Make toolchain:
   preserve registers and overwritten save data, leave an observable marker,
   restore the instruction and help history, and resume the correct branch
   continuation. Keep full module lifecycle/import work out of this milestone.
5. Prove entry and clean continuation from a crafted save on both NTSC-U and
   PAL. Record payload size, marker, exact boot/save setup, and observations.

No commits, pushes, Ghidra mutations, or modified original memory cards.

## How to test

### 1. Run the checks available now

Run these separately in PowerShell and check that each exits with code 0:

```powershell
Set-Location D:\Repos\Metroynome\racdoor
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src uya-mod-loader make
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src uya-mod-loader make -C tests test
git diff --check
```

The test runs one successful arithmetic case group and four deliberately
failing cases: unaligned destination, unaligned log base, unsupported entry
width, and nonnegative byte index. Their `ERROR:` messages are expected; the
Make target must still exit successfully. The build produces existing R&C1
samples, **not a UYA proof binary**. UYA injection must still fail without
writing an output save. There is no implemented UYA generation command yet;
add its exact invocation here when the proof injector exists.

### 2. Prepare a disposable memory card

To regenerate the targeted reset using the existing tool (run from the repo;
the original card mount is read-only):

```powershell
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src uya-mod-loader make -C tools/inject TARGET=host host
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -v D:/PS2/PCSX2/memcards/main.ps2:/original:ro -v C:/Users/Troy/AppData/Local/Temp/racdoor-manual-test-20261113-145755:/test -w /src uya-mod-loader tools/inject/inject --reset-uya-help /original/BASCUS-97353RATCHET/save0.bin /test/ntscu-help-reset.bin
```

For PAL use `/original/BESCES-52456RATCHET/save0.bin` and a separate
`/test/pal-help-reset.bin` output. To test a generated reset save from disk,
close PCSX2, back up the copied card's regional `save0.bin`, and replace only
that copied file with the matching reset output. Fresh boot and load the slot.
Do not load an old emulator state: it restores the old in-memory help flags.
The live reset above is already ready to step/resume and needs no disk reload.

Close the isolated test emulator before copying cards or changing its config.
Use the test copy at
`C:\Users\Troy\AppData\Local\Temp\racdoor-uya-probe-20261113`, not the
ordinary PCSX2 installation. Copy the complete folder card so its metadata and
game directory structure are preserved. The following refuses to overwrite
an earlier test card:

```powershell
$probeRoot = Join-Path $env:TEMP 'racdoor-uya-probe-20261113'
$cardRoot = Join-Path $probeRoot 'MemoryCards'
$testCard = Join-Path $cardRoot 'uya-proof-test.ps2'
if (Test-Path -LiteralPath $testCard) { throw 'Test card exists; use a new test-card name.' }
New-Item -ItemType Directory -Path $cardRoot -Force | Out-Null
Copy-Item -LiteralPath 'D:\PS2\PCSX2\memcards\main.ps2' -Destination $testCard -Recurse
```

In the isolated emulator's `inis/PCSX2.ini`, verify these entries before boot:

```ini
[Folders]
MemoryCards = C:\Users\Troy\AppData\Local\Temp\racdoor-uya-probe-20261113\MemoryCards

[MemoryCards]
Slot1_Enable = true
Slot1_Filename = uya-proof-test.ps2
Slot2_Enable = false
```

Keep cheats and patches disabled. Verify that SaveStates, Snapshots, Cache,
and Cheats also point inside the isolated test directory. The test config
currently has `StartPaused = true`; resume manually when ready. Original
save inputs are under the original card's `BASCUS-97353RATCHET` (NTSC-U) and
`BESCES-52456RATCHET` (PAL) directories. Only replace `save0.bin` inside the
**copied** regional directory when a generated proof save is available.
Keep a pristine copy of that slot for comparison and repeat runs.

### 3. Fresh boot each region

Do not use `-statefile` with the known incompatible `.05.p2s`. Fresh boot,
then use the game's Load Game menu to select the copied slot. Run one region
at a time. These commands use the isolated emulator:

```powershell
$probeRoot = Join-Path $env:TEMP 'racdoor-uya-probe-20261113'
$probeExe = Join-Path $probeRoot 'pcsx2-qt.exe'
# NTSC-U
& $probeExe -portable -fastboot -- 'D:\PS2\ISOs\SCUS_973.53.Ratchet and Clank - UYA.iso'
# PAL: close the NTSC-U instance before running this command.
& $probeExe -portable -fastboot -- 'D:\PS2\ISOs\SCES_524.56.Ratchet and Clank 3 (PAL).iso'
```

Confirm the game serial and loaded planet from the debugger/log. The original
sample slots start on level 7 (NTSC-U) and level 1 (PAL), **not Phoenix**.
The help-function addresses below apply only while the normal Starship Phoenix
overlay (level 3) is loaded. Reach that planet using a suitable save/progression,
or verify addresses separately for another overlay. Do not force a planet value
in live memory and assume the corresponding overlay was loaded.

### 4. Establish the trigger before installing a payload

Use PCSX2's debugger UI or the installed DebugServer client (TCP 21512).
PINE (28011) supports memory inspection but does not provide these execution
breakpoints. Connect only after a game has initialized; ECONNREFUSED alone
does not establish anything about the exploit.

| Observation | NTSC-U Phoenix | PAL Phoenix |
| --- | --- | --- |
| Normal-camera help call, ID 0x14fb / record 0x11e | 0x004cb5b0 | 0x004cf720 |
| Alternate-camera help call, ID 0x14fc / record 0x11f | 0x004cb584 | 0x004cf6f4 |
| Help-start wrapper | 0x004051e0 | 0x00407a60 |
| History writer entry | 0x004068c0 | 0x004091f8 |
| Candidate copy-loop branch (both regions) | 0x0013d27c | 0x0013d27c |

With an unmodified copied save, first inspect the history writer using its
ordinary index. Record whether the intended help call occurs and which ID it
passes. At a breakpoint on a `jal`, its delay-slot assignment has not necessarily
executed: inspect `a1` at the callee entry when confirming the record index.

For the QuickSelect route, the eligible-gadget count must be at least nine,
block-5 byte `0xdf` bit 2 must be clear, the selected help record must not be
disabled, and the help system must be idle. The check occurs periodically.
An already-completed save may have consumed the message. Any temporary debugger
edits to establish these prerequisites are research only: the final proof must
reproduce them from the crafted save without debugger assistance.

Observe whether `0x0013d27c` executes after the help event, including when
traveling to another planet. Capture its registers and delay slot. Original
instruction is `0x1440fffa`; after a successful intended halfword write it would
be `0x144014fb` or `0x144014fc`. Do not intentionally corrupt the branch before
a validated trampoline and continuation exist. For the taken copy-loop route,
the store at `0x0013d280` executes before the trampoline; resume at `0x0013d268`.

### 5. Acceptance test once the minimal proof exists

For **each region**, start from a fresh disposable-card copy and cold boot:

1. Record the proof-generation command, payload byte size and SHA-256, input
   save SHA-256, output save SHA-256, emulator build, region, and starting planet.
2. Load the generated save through the game's menu. Do not inject code, change
   the PC, or call the payload manually through the debugger.
3. Perform the documented trigger action. A breakpoint may observe execution,
   but must not cause it. Show that execution reaches the saved trampoline and
   payload, and that the payload writes its declared marker value/address.
   Marker address/value are **not selected yet**; define them before this test.
4. Resume and verify the original branch, overwritten saved-data bytes, help
   history, stack pointer, and registers required by the interrupted function
   are restored. Confirm the copy completes and gameplay remains responsive.
5. Travel to another planet, then save to the disposable card and reload. Check
   QuickSelect/inventory behavior and absence of a crash or repeated unintended
   history writes. State explicitly whether the proof is one-shot or designed
   to re-arm on reload; test that declared behavior.
6. Repeat from the same pristine input at least once. Keep region results
   separate: an NTSC-U result does not establish PAL compatibility.

Pass means **save-triggered payload execution plus clean game continuation**,
not just a successful build, visible help message, changed branch, or emulator
process startup. Keep logs under the isolated test directory and summarize
observations here. If execution fails, record the last breakpoint reached,
PC, registers, hook bytes, marker contents, region/overlay, and reproduction
steps rather than describing the region as supported.

## Resume checkpoint: 2026-11-13

### Active continuation (later on 2026-11-13)

- Process startup works with `require_escalated`, explicit Windows PowerShell,
  and `login=false`; default sandbox startup still fails. Approval review
  allowed the read commands and documented Docker build.
- Read the implementation diffs, injector, arithmetic tests, and exploit
  documentation. The only implementation changes are the negative-index helper,
  its use by R&C1 `arm`, and explicit UYA rejection. No UYA injection path exists.
- Ran the documented Docker `make` successfully (incremental build), followed
  by `make -C tests test`: arithmetic and all four expected rejection cases
  passed. This does not establish UYA runtime support.
- Checked ancestor AGENTS.md paths at D:/, D:/Repos/, and Metroynome, and
  searched racdoor including hidden files: none returned.
- Ghidra is connected to shared project `Ratchet-RE` on
  `creepnt.stream:13100`. The relevant already-open program is
  `/Ratchet 3/PS2/NTSC-U/Single Player/ntscu.03.Starship1.bin`.
  Use read-only analysis; no Ghidra changes or check-ins have been made.
- Next operation: inspect the help caller at NTSC-U `0x004cb584` and locate
  regional extraction files under `D:/PS2/ISOs/uya` for PAL comparison.

The original checkpoint below records the preceding interrupted turn.

### Trigger trace checkpoint

### Regional comparison checkpoint

Recovered stock asset root: `D:/Repos/Horizon/wrench/games/`.
Regional directories are `uya_scus_973_53` and `uya_sces_524_56`.
In each, use `boot_elf.elf` and
`levels/singleplayer/3_starship_phoenix/overlay.elf`.
Phoenix SHA-256 values:

- NTSC-U: `307e3a50ebbf0d2435436e86037a050d69d0990168eb84c3f9142f6180996cb5`
- PAL: `eb75adb27ae4f56b0cfd2cb10a7dbd4c0fb9bf30a90e23180764c730a355cfeb`

Direct ELF-section byte comparisons (not assumed regional deltas):

- PAL `CheckAndStartHelpDeskMessage`: `0x00407a60`, 200 bytes. Four words
  differ from NTSC-U: help-state base `0x002e52f0`, the two instructions
  referencing it, frame-counter gp offset `0x2c10`, and the history `jal`.
- PAL history writer: `0x004091f8`, all 208 bytes identical to NTSC-U.
- PAL camera predicate: `0x00501978`, 228 bytes; only one word differs,
  loading a pointer at `0x001e2510` instead of NTSC-U `0x001e2590`.
- Compared trigger subsection NTSC-U `0x004cb3c0` to PAL `0x004cf530`,
  length `0x214`: exactly 12 words differ, all address loads or call targets.
  PAL frame counter is `0x001df4c0`; gadget table base is `0x002c49c0`.
  PAL calls are `0x004cf6f4` (ID `0x14fc`, record `0x11f`) and
  `0x004cf720` (ID `0x14fb`, record `0x11e`). Eligibility and bit tests match.
  Do NOT apply the subsection delta to the whole enclosing function: earlier
  PAL code has additional timer scaling instructions and different lengths.
- Both boot binaries contain branch `0x0013d27c = 0x1440fffa`.
  Writing `0x14fb` reaches `0x0014266c`; writing `0x14fc` reaches
  `0x00142670`. The first candidate branch at `0x0013d25c` similarly reaches
  `0x0014264c` or `0x00142650`. Both IDs need explicit treatment.

Current next step: inspect the save table for destination ownership/lifetime
and the copy-loop continuation. No payload or modified save was produced.

### Storage and continuation checkpoint

- Full 40-entry shared-save tables found at Phoenix ELF file offsets
  NTSC-U `0x1123f8`, PAL `0x1123b8`, by the first record
  `(0x001d545c, 4, 0, 0)`. Tables agree on block 39 at `0x00142660`, size
  `0x80`, and block 30 at `0x001425c0`, size `0xa0`.
- Scanning both boot ELFs against actual shared-save ranges confirms exactly
  the four `(message, branch)` candidates above for IDs `0x14fb`/`0x14fc`.
  ID `0x0b3e` yields no candidate in those ranges.
- `0x0013d258` is a byte-copy loop. At taken hook `0x0013d27c`, its delay
  slot `sb v1,0(a0)` at `0x0013d280` has executed and `a3` is incremented.
  Correct taken-path continuation is `0x0013d268`, preserving `a1`, `a2`,
  `a3`, `t0`, and `ra` (and any other registers the bootstrap changes).
  Returning to `0x0013d280` would repeat the store and then terminate early.
- Its caller `0x0013d310` invokes the copy with lengths `0x60`, `0x1818`,
  and `0x26f0`. Ghidra xrefs to this caller are `0x0049a024` in
  `FUN_00499ea0` and `0x004c12a8` in `StartPlanetLoad`. A per-frame trigger
  remains unproven; the first candidate branch requires nonpositive length.
- Block 39 is live data. In particular, `0x004f0d60` reads `0x00142670`,
  shifts subsequent nonzero words down, and writes the first word back to an
  indexed position. Do not treat it as padding or install a trampoline
  without checking this array's consumers and restoring original bytes.
- Original save samples recovered at
  `D:/PS2/PCSX2/memcards/main.ps2/BASCUS-97353RATCHET/save0.bin` (level 7)
  and `.../BESCES-52456RATCHET/save0.bin` (level 1). Both are 134992 bytes.
  Originals have only been read. NTSC-U block 39 offsets `0x0c..0x17` hold
  words `0`, `0x10`, `0x3f`; PAL holds zeros there.
- No PCSX2 game process was running. Installed debugger API is
  `D:/PS2/PCSX2-MCP-v1.0.0-win64/pcsx2-mcp-server/dist/debug-server-client.js`
  (JSON TCP port 21512); PINE is 28011. The existing emulator config enables
  cheats and points at real memory cards: isolate settings/cards before tests.
- Existing states under `D:/PS2/PCSX2/sstates` use ZIP compression unsupported
  by installed Python `zipfile`; `zstandard` is not installed. No state loaded.

Current work: find an existing decompressor / isolated runtime starting point.
User was asked whether the small binary is a minimal proof or an existing file;
pending an answer, the working assumption is a minimal proof. No implementation
edits, modified saves, emulator game runs, or Ghidra mutations yet this turn.

### Broader candidate scan (static leads, not validated hooks)

Scanned NTSC-U Phoenix direct calls to `0x004051e0` for nearby immediate
assignments to `a0` (delay slot, or preceding four instructions). Found 44
distinct IDs. This is a heuristic, not complete data-flow analysis; each call
must be checked before use. Scanned boot executable sections for integer
relative branches and required eight destination bytes inside an actual shared
save block. IDs with candidate counts:

| Message ID | Help call sites (NTSC-U Phoenix) | Candidate count |
| --- | --- | --- |
| 0x18cd | 0x002fc0c4 | 57 |
| 0x1911 | 0x00322c68, 0x00322e74 | 64 |
| 0x1627 | 0x003c969c | 17 |
| 0x1602 | 0x00321f2c | 15 |
| 0x15d6 | 0x003aa908 | 8 |
| 0x1519 | 0x0031d99c | 2 |
| 0x1517 | 0x0035cd80 | 1 |
| 0x1518 | 0x003cb78c, 0x003cbb0c | 2 |
| 0x14fb | 0x004cb5b0 | 2 |
| 0x14fc | 0x004cb584 | 2 |

The larger IDs reach more than the two byte-copy branches. Examples:
`0x1911` changes branch `0x0013c3b8` to target `0x00142800` (block 9 + 0x10);
`0x18cd` changes it to `0x001426f0` (block 5 + 0x10).
These belong to function `0x0013c348`, with numerous core callers. Do not infer
that it runs every frame: its call conditions and delay slot need tracing.

English message meanings were read from stock Phoenix
`help_messages_us_english.bin`: 16-byte records start at file offset 8, with
text offset and ID as the first two words; **add 8 to the text offset** to get
the string's actual file position.

- `0x18cd`: Thruster-Pack replacement/glide/boost-jump help. Call at
  `0x002fc0c4` passes record `0x161`; enclosing function `0x002fbe68`.
  It checks record flag at `0x001a8ea4` and hero/gadget conditions.
- `0x1911`: Titanium Bolts / purchasing skins help. Call at `0x00322c68`
  passes record `0x162`; enclosing function `0x00322968`. Likely requires a
  pickup interaction; do not assume a save flag alone triggers it.
- `0x14fb` and `0x14fc` are the same secondary QuickSelect-ring instruction
  with different controller buttons, confirming the camera-mode split.

Alternative candidates have NOT yet been compared to PAL or tested in game.

Ghidra `force_decompile` and `get_functions` currently fail with
`ghidra/program/model/lang/SleighLanguageDescription`; `disassemble_function`
works. Use `structuredContent.result` to avoid printing duplicate large results.

NTSC-U Phoenix assembly independently checked during this continuation:

- Caller function spans `0x004caf28..0x004cb75c`. At `0x004cb3c8`, the
  counter loaded from `0x001df540` must be positive and a multiple of 128.
- Block-5 byte at `0x001426e0 + 0xdf` must be below 7 to enter the gadget
  counting path. At `0x004cb4d0`, `s2 = (eligible_count < 9)`; `s2` must be
  zero to reach the camera-specific messages. The counting loop has eligibility
  tests and exclusions: simply counting nonzero unlock bytes is not equivalent.
- Block-5 byte `0xdf`, bit 2, must be clear. `0x004fd5b8` then selects ID
  `0x14fc` / record `0x11f` when nonzero, otherwise `0x14fb` / record `0x11e`.
  On successful help start, the caller sets bit 2 and calls `0x00421b20`.
- The camera predicate returns the byte at `0x001439f4` only after several
  hero/camera/action-state exclusions; otherwise it returns zero. Therefore
  the `0x14fc` branch displacement cannot be assumed for ordinary saves.
- `CheckAndStartHelpDeskMessage` (`0x004051e0..0x004052a4`) additionally checks
  help-state fields at `0x002e5330` and `0x001ccfd0`, excludes state 6 via a
  gp-relative global, and requires the frame counter to be at least 6. It
  rejects record flag `0xffff`, sets active message/record fields, then calls
  the history writer at `0x00405288`. No direct voice/text setting check occurs
  in this wrapper; caller/state initialization can still impose restrictions.

Next: locate PAL Wrench extraction (the `D:/PS2/ISOs/uya` directory contains
custom multiplayer map assets, not the needed stock extraction), compare the
caller and camera predicate, and evaluate BOTH `0x14fb` and `0x14fc` branch
destinations. No runtime trigger or bootstrap has been validated.

The follow-up agent read this handoff and inspected `git status --short` and
the last six commits. HEAD was `80dbe2b` (`Fix stack corruption bug`). The
UYA groundwork below is uncommitted work from the previous agent. Preserve it.
This follow-up has not changed implementation code or run builds, tests, or
the game. Its only edit is this checkpoint. The findings below remain the
previous agent's recorded evidence, not independently repeated verification.

The user's objective is UYA NTSC-U/PAL compatibility with a small binary
payload. A working UYA save/bootstrap has NOT been produced. Do not remove
the injector's UYA rejection just because the history-write arithmetic works.

Observed dirty files: `include/racdoor/exploit.h`, `sdk/exploit.c`,
`tools/inject/inject.c`, and numerous existing R&C1/Deadlocked CSV tables.
Untracked work includes `.gitattributes`, this file, `tests/`, and
`tables/gameglobals.SCUS-97353.csv` / `tables/gameglobals.SCES-52456.csv`.
Inspect the CSV diffs before editing; do not assume they are all UYA changes.

Exact next steps:

1. Read the diff of the three C/header files above, `tests/help_log.c`,
   `tests/Makefile`, `doc/exploit.texi`, and `tools/inject/inject.c` to recover
   the existing exploit and proposed halfword-write implementation.
2. Run the existing Docker build and arithmetic tests listed at the end of
   this document. These are baseline checks, not proof of game compatibility.
3. Locate the previously extracted NTSC-U/PAL Phoenix overlays and Ghidra
   programs. Their paths have not been recovered in this follow-up. Inspect
   the NTSC-U caller at `0x004cb584`, identify its PAL counterpart from actual
   bytes/control flow, and verify message ID versus record index separately.
4. Resolve the post-load trigger and branch continuation requirements in
   "Work still required" below before implementing a UYA bootstrap. Start
   with a minimal payload and disposable memory cards, then test both regions.

Tool blocker: Windows process startup intermittently fails with
`helper_unknown_error: setup refresh had errors`. Some read commands succeeded
with explicit `C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe`,
`login=false`, and working directory `D:\Repos\Metroynome`, but subsequent
attempts failed before execution. The Node REPL also failed to start with the
same sandbox error. The attempted source-diff reads therefore did not run.
This is an environment/tool startup failure, not a repository build failure.

Relevant local guidance read: the user's global C style preferences,
`C:/Users/Troy/.codex/skills/uya-mod-development/SKILL.md`, and
`D:/Repos/Metroynome/uya-mod-loader/AGENTS.md` (the latter belongs to the
neighboring loader repository). No racdoor-specific AGENTS.md was returned
by the initial tracked/nonignored filename search; ancestor/ignored instruction
files still need checking before implementation edits.

Status: groundwork only. Neither UYA region can produce a working Racdoor save
yet. The injector rejects UYA without writing an output file. The UYA symbol
tables contain verified globals, not a complete SDK or bootstrap profile.

Targets: SCUS-97353 (NTSC-U) and SCES-52456 (PAL).

## Verified save layout

Existing save0.bin files from both releases have the same layout:

- File size: 134992 bytes.
- Shared section: 14216 bytes, 40 blocks.
- Level section: 3264 bytes, 37 sections.
- Block 16: 4272 bytes, 356 help-message records.
- Block 17: 516 bytes, 43 gadget-help records.
- Block 18: 1872 bytes, 156 miscellaneous help records.
- Block 1010: 800 bytes, 400 16-bit history entries.
- Block 1011: 4-byte history index.
- Level block 3002: 2048 bytes per level, 75776 bytes in total.
- R&C1's separate HelpVoiceOn/HelpTextOn blocks 28 and 29 are absent.

The Starship Phoenix overlay save tables in both Wrench-extracted releases
independently identify the addresses in gameglobals.SCUS-97353.csv and
gameglobals.SCES-52456.csv. Their equality is observed, not assumed from a
regional address delta. No multiplayer lifetime or behavior is established.

## Verified history write

In the NTSC-U Starship Phoenix Ghidra program, TouchHelpDeskHistory starts at
0x004068c0. Its complete 208-byte body matches the PAL Starship Phoenix overlay
at 0x004091f8 byte for byte. These are overlay-specific function addresses.

The NTSC-U instruction sequence includes:

```text
0x004068dc  blez a0, 0x0040696c
0x00406970  lw v0, 0x5598(v0)    # HelpLogPos at 0x001d5598
0x00406978  sll v1, v0, 1
0x0040697c  addu v1, v1, a0      # HelpLog at 0x001a8ec8
0x00406984  sh a3, 0(v1)
```

For an aligned destination T and log base L, the 32-bit index bit pattern is:

```text
index = (((T - L) modulo 2^32) >> 1) | 0x80000000
```

The signed index is negative, avoiding the forward history scan. The MIPS sll
discards its sign bit, so L + (index << 1) still reaches T modulo 2^32.
helpLogIndex implements and tests this arithmetic. R&C1 arm still selects its
original byte-write mode; the new halfword mode is not a UYA bootstrap by itself.

CheckAndStartHelpDeskMessage at NTSC-U 0x004051e0 multiplies its record index
by 12 and checks the first 16-bit field against 0xffff. It passes a separate
message ID to the history writer. Thus the selected record index and the value
written to the branch instruction must not be conflated. R&C1's 8-byte
HelpDatum and hardcoded persistence array lengths cannot be reused for UYA.

## Candidate branches, not validated hooks

Scanning core.text in both extracted boot ELFs with the observed help ID 0x14fc
finds the same two branches reaching shared saved storage:

| Branch | Original instruction | Destination | Saved block / offset |
| --- | --- | --- | --- |
| 0x0013d25c | 0x18c00009 | 0x00142650 | 30 / 0x90 |
| 0x0013d27c | 0x1440fffa | 0x00142670 | 39 / 0x10 |

Both belong to a byte-copy routine starting at 0x0013d258. Its known caller,
0x0013d310, is called by StartPlanetLoad and another routine in the NTSC-U
Phoenix overlay. This is not evidence of a per-frame trigger. The first
branch requires a nonpositive copy length; the observed caller supplies
positive lengths. The second is taken during the copy loop, with an sb in
its delay slot. A bootstrap must preserve that copy's state and resume it.

The NTSC-U Phoenix overlay calls CheckAndStartHelpDeskMessage with ID 0x14fc
and record index 0x11f at 0x004cb584. It checks shared save block 5, byte 0xdf,
bit 2, and a camera-mode predicate at 0x004fd5b8, among other conditions.
The alternative path uses ID 0x14fb and record index 0x11e. These paths are
not yet validated as reliable post-load triggers, and the equivalent PAL
caller has not been verified. No candidate is enabled in the injector.

## Work still required

1. Verify a repeatable post-load help trigger, its message ID and record index,
   and its settings/prerequisites in both regions.
2. Select an executed branch whose overwritten 16-bit displacement reaches
   suitable saved storage. Verify its delay slot and continuation state.
3. Select trampoline/decryptor storage without corrupting live game state.
4. Adapt help-record preparation, RDX metadata, and the injector. Validate
   region compatibility, all destination block sizes, and payload capacity.
5. Map loader imports, module lifecycle hooks, and persistence for each overlay
   and region. Verify the required SDK calling conventions and memory lifetime.
6. Test a minimal payload on disposable memory-card copies in PCSX2 for both
   regions, followed by save/reload and planet transitions. No game execution
   has been tested yet.

## Build and arithmetic checks

From the repository directory, using the existing local Docker image:

```text
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src uya-mod-loader make
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src uya-mod-loader make -C tests test
```

The image provides ee-gcc 3.2.3 and the host compiler. CSV tables need LF line
endings; .gitattributes prevents Windows checkout conversion from breaking the
existing parser. A successful R&C1 sample build is a regression check, not
evidence that UYA bootstrapping works.
