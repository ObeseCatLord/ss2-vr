# Installed native weapon slots

The fingerprinted Sam2Game.dll `samGetWeaponParamsPath` switch has 17 slots; slot 14 is unused. Inventory membership and ammo come from native CPlayerInventory queries. Native hand enum 0 is left (`player+0x804`), 1 is right (`player+0x800`), verified using named left/right ammo-HUD methods.

| ID | Native parameter family | Wheel label |
|---|---|---|
| 0 | CircularSaw | Saw |
| 1 | ZapGun | Zap gun |
| 2 | AutoShotgun | Auto SG |
| 3 | DoubleShotgun | Double SG |
| 4 | Uzi | Uzi |
| 5 | MiniGun | Minigun |
| 6 | RocketLauncher | Rockets |
| 7 | GrenadeLauncher | Grenades |
| 8 | PlasmaRifle | Plasma |
| 9 | KlodovikGun | Klodovik |
| 10 | Cannon | Cannon |
| 11 | SeriousBomb | Bomb |
| 12 | Colt | Colt |
| 13 | Sniper | Sniper |
| 14 | unused | omitted |
| 15 | BeamGun | Beam |
| 16 | Flamer | Flamer |

Static vtable coverage: stock weapon classes use the base GetShootingPlacement slot at byte +0x1d0, except the sniper's separate override. Both are hooked. This does not establish runtime coverage of every special firing event or melee collision path.
