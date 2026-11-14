# PAL save execution probe (experimental)

This is a disposable diagnostic save, not a DNAS save or a completed UYA port.
The production injector still rejects UYA payload injection.

Build with the existing Docker image from this directory:
`make TARGET=ee ee host`.
Run `install-probe INPUT_SAVE probe.bin OUTPUT_SAVE` on a native Phoenix save.
Use a copied card and disable PCSX2 cheats/patches. Do not save progress from
this diagnostic card or replace the preserved main/login save with it.

The installer places a 152-byte routine at MapMask base 0x001a9318 and a
nop/jump/nop at shared block 39 offsets 0x0c..0x17. Message 0x14fb reaches
0x0014266c; 0x14fc reaches 0x00142670 after the halfword write to 0x0013d27c.
Other help records are disabled to limit negative-index writes to these two
messages. Native help display settings are preserved.

On entry the routine preserves its touched registers as full 128-bit values,
restores the original branch and overwritten block-39 words, restores an
in-bounds history count, writes 0x50524f42 at 0x001a9738, flushes D/I caches,
and resumes the copy loop at 0x0013d268 (its delay-slot store already ran).
Backup words and history count live at 0x001a9718..0x001a9727.

Unverified assumptions: block-39 trampoline lifetime (these are live gadget
queue words), MapMask residency/lifetime, actual fetch/cache behavior after
the help write, and continuation under real hardware. The first trial needs
breakpoints at the Phoenix help wrapper and at probe entry. A successful
build and save checksum check do not establish code execution.

Initial setup on 2026-11-13: probe built, generated native PAL save reparsed
successfully. PCSX2 exited during breakpoint installation before loading the
save; retry with emulator paused before setting breakpoints.
