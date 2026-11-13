# PAL UYA DNAS test payload

Standalone callable EE payload for the PAL multiplayer menu. This is the
DNAS Skip from Horizon Launcher, not a DNS server redirect and not an RDX
module. No save/bootstrap or persistent multiplayer hook is installed.

## Provenance and exact behavior

Source: `D:/Repos/Horizon/horizon-launcher/launcher/core/patches.cs`, PAL
multiplayer DNAS Skip entry (also present in `UYALauncher/launcher/core/patches.cs`).
Verified the 11-word guard against the stock extracted PAL multiplayer-menu
`overlay.elf`. The existing `pal.39.Lobby.bin` dump already contains both
patched instructions and matches the remaining guard words.

| Address | Stock word | Patched word |
| --- | --- | --- |
| 0x004d55d8 | 0x56220003 | 0x56220000 |
| 0x004d55e4 | 0x24020006 | 0x24020005 |

The launcher writes the low byte at each address. The first edit is NOT a NOP.
The payload guards all 11 words at 0x004d55c8..0x004d55f0 before writing,
rejects partially patched or mismatching code, and accepts an already-patched
window without writes. It performs FlushCache(0), then FlushCache(2) after
applying. Syscall 0x64 was checked against the installed PS2SDK libkernel.a.

## Build and checks

From the racdoor root on Windows:

```powershell
docker run --rm -v D:/Repos/Metroynome/racdoor:/src -w /src/samples/uya-dnas-pal uya-mod-loader make TARGET=ee ee test
```

Outputs: `uya-dnas-pal.bin`, `.elf`, `.map`, and `.dis`. Entry is byte offset
zero, `uyaDnasEntry()`. No arguments; result is in v0:

- 0: instruction window mismatch; no writes.
- 1: applied and caches synchronized.
- 2: already applied; no writes.

This is relocatable raw code: internal branches are PC-relative, with no
absolute internal jumps, data sections, GP-relative accesses, external imports,
or outstanding ELF relocations in the inspected build. Absolute addresses in
the payload intentionally refer to the PAL patch window. It returns using ra.
Use an ordinary EE C-call boundary, valid return address, and game thread with
kernel syscalls available. A hook/bootstrap must preserve interrupted registers;
this callable function is not a register-preserving trampoline.

Host tests verify the exact two changed bytes, untouched surrounding bytes,
idempotence, each mismatching guard word, and both partial-patch combinations.
These tests establish patch logic, not online authentication or save execution.

## Runtime test requirements

Keep the original campaign-transition breakpoint capture intact. This payload
belongs to the PAL MULTIPLAYER MENU, not Phoenix or Florana. Run it after the
multiplayer overlay is loaded and before DNAS authentication. A future loader
must own safe executable storage, copy the raw code, synchronize caches BEFORE
calling it, and schedule/reapply it when the multiplayer overlay is reloaded.
No placement address or persistent hook is asserted by this sample.

For an isolated runtime test, verify original guard words first, invoke through
a validated test loader, record v0 and both resulting words, call again to check
v0=2, and exercise DNAS connection plus normal continuation. DNS/network server
configuration remains separate. A debugger/loader invocation does not prove
save-triggered execution. Do not pass this .bin to the RDX injector: UYA save
injection remains explicitly unsupported until its bootstrap is completed.
