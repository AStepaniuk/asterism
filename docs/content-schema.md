# Content Schema (v1)

All content lives under `assets/data/` as TOML files. Loader: `TomlContentProvider`
behind `IContentProvider`. Schema validated at startup; malformed content is a
startup error reported via `std::expected`.

**Conventions**
- All gameplay numeric values are **Fixed** (fixed-point, scale 1/1024).
  In TOML they are written as ordinary decimals (`1.5`) and converted at load.
  A value of `1` means `1.0` in game units.
- Content IDs are stable strings: `<kind>.<name>` (`chassis.humanoid`,
  `module.bronze_sword`, `tech.steam_power`). IDs never change once shipped.
- Cross-references are by ID (never by array index).
- Optional fields have a documented default. Unknown fields are a load error.

