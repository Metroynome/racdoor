# PAL campaign QuickSelect help trigger map

Cross-checked against `D:/PS2/ISOs/RAC Levels/rac-defs.json`. All 34 campaign gameplay overlays are represented; main menu (-1) and multiplayer are excluded. Extracted source: Wrench `uya_sces_524_56/levels/singleplayer/*/overlay.elf`.

These are statically matched code locations, not proof that the trigger is reachable in every mode (especially Clank and Vid-Comics). Only Phoenix has a verified save-triggered execution test. The shared branch target remains 0x0013d27c; trigger eligibility, record flags, and payload lifetime still matter.

| Level | Name | History writer | Help wrapper | Call: 0x14fb / 0x11e | Call: 0x14fc / 0x11f | Writer identical to Phoenix |
|---|---|---|---|---|---|---|
| 1 | Veldin | 0x0041bd48 | 0x0041a5b0 | 0x004e1d38 | 0x004e1d0c | yes |
| 2 | Florana | 0x003f49a0 | 0x003f3208 | 0x004b7c50 | 0x004b7c24 | yes |
| 3 | Starship Phoenix | 0x004091f8 | 0x00407a60 | 0x004cf720 | 0x004cf6f4 | yes |
| 4 | Marcadia | 0x0042cad0 | 0x0042b338 | 0x004f1e38 | 0x004f1e0c | yes |
| 5 | Daxx | 0x00415b30 | 0x00414398 | 0x004db1d8 | 0x004db1ac | yes |
| 6 | Starship Phoenix (Under Attack) | 0x00400808 | 0x003ff070 | 0x004c54f0 | 0x004c54c4 | yes |
| 7 | Annihilation Nation | 0x0041c5e8 | 0x0041ae50 | 0x004e0ae0 | 0x004e0ab4 | yes |
| 8 | Aquatos | 0x003ff568 | 0x003fddd0 | 0x004c40e8 | 0x004c40bc | yes |
| 9 | Tyhrranosis | 0x00435198 | 0x00433a00 | 0x004fb7d8 | 0x004fb7ac | yes |
| 10 | Zeldrin Starport | 0x003ff418 | 0x003fdc80 | 0x004c4f40 | 0x004c4f14 | yes |
| 11 | Obani Moons | 0x003f5690 | 0x003f3ef8 | 0x004b7eb8 | 0x004b7e8c | yes |
| 12 | Rilgar | 0x0041d7e0 | 0x0041c048 | 0x004e3480 | 0x004e3454 | yes |
| 13 | Holostar Studios - Ratchet | 0x003fd418 | 0x003fbc80 | 0x004c1cd8 | 0x004c1cac | yes |
| 14 | Koros | 0x00406908 | 0x00405170 | 0x004cc710 | 0x004cc6e4 | yes |
| 16 | Kerwan | 0x0042b9e8 | 0x0042a250 | 0x004f34c8 | 0x004f349c | yes |
| 17 | Crash Site | 0x003f7528 | 0x003f5d90 | 0x004bae40 | 0x004bae14 | yes |
| 18 | Aridia | 0x0042f220 | 0x0042da88 | 0x004f4b18 | 0x004f4aec | yes |
| 19 | Thran Asteroid Belt | 0x004111a0 | 0x0040fa08 | 0x004d6880 | 0x004d6854 | yes |
| 20 | Final Boss | 0x0043a6c8 | 0x00438f30 | 0x00501d80 | 0x00501d54 | yes |
| 21 | Obani Draco | 0x003f4918 | 0x003f3180 | 0x004b8b70 | 0x004b8b44 | yes |
| 22 | Mylon | 0x00421500 | 0x0041fd68 | 0x004e8a50 | 0x004e8a24 | yes |
| 23 | Holostar Studios - Clank | 0x003feb80 | 0x003fd3e8 | 0x004c3ed0 | 0x004c3ea4 | yes |
| 24 | Insomniac Museum | 0x00404098 | 0x00402900 | 0x004c6ba8 | 0x004c6b7c | yes |
| 26 | Kerwan Ranger Missions | 0x00425e18 | 0x00424680 | 0x004eacd0 | 0x004eaca4 | yes |
| 27 | Aquatos Base | 0x003f53d8 | 0x003f3c40 | 0x004b81d8 | 0x004b81ac | yes |
| 28 | Aquatos Sewers | 0x003e7fe8 | 0x003e6850 | 0x004a9a70 | 0x004a9a44 | yes |
| 29 | Tyhrranosis Ranger Missions | 0x00428720 | 0x00426f88 | 0x004ed488 | 0x004ed45c | yes |
| 30 | Vid-Comic Unamed | 0x003ef818 | 0x003ee080 | 0x004afbc8 | 0x004afb9c | yes |
| 31 | Vid-Comic 1 | 0x003f3bd8 | 0x003f2440 | 0x004af068 | 0x004af03c | yes |
| 32 | Vid-Comic 4 | 0x003fc458 | 0x003facc0 | 0x004b8558 | 0x004b852c | yes |
| 33 | Vid-Comic 2 | 0x003f4810 | 0x003f3078 | 0x004b0ba0 | 0x004b0b74 | yes |
| 34 | Vid-Comic 3 | 0x003f5f80 | 0x003f47e8 | 0x004b6108 | 0x004b60dc | yes |
| 35 | Vid-Comic 5 | 0x003f4f80 | 0x003f37e8 | 0x004b0248 | 0x004b021c | yes |
| 36 | Vid-Comic Special Edition | 0x003fa998 | 0x003f9200 | 0x004b5e28 | 0x004b5dfc | yes |
