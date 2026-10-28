Dark Pawns 3.0
==============

This is the Dark Pawns 3.0 development branch, preserved for reference. It is
not built by the top-level `configure`/`make` and is not compatible with the
2.x game in `src/` and `lib/`.

3.0 split off the 2.x line around 2000 and was developed mostly between 2000
and 2005 (see `src/changelog`). It was a redesign rather than an update:
100 mortal levels, religion, a reworked experience and skill system, weapon
types, mob classes, factions, player shops, notes, MySQL-backed accounts, and
MXP/MSP/MCCP client protocols. It never replaced 2.x in production.

Contents, taken from SVN `trunk/3.0` at r1330 (2007-08-05):

- `src/`     game source, from `circle/src`
- `scripts/` Lua scripts
- `world/`   world files (rooms, mobs, objects, zones, shops)
- `text/`    help files, MOTD, news, and other text, from `circle/lib/text`

The world files use a 3.0-specific format (letter-encoded bitvectors,
five object values, renumbered races, extra mob fields), and several zone
numbers hold different areas than they do in 2.x. They cannot be loaded by
the 2.x code without conversion.

Left out: compiled objects and executables, the prebuilt Lua static
libraries in `src/lib/` (the headers in `src/include/` remain), and two
Linux executables (`world/obj/act`, `world/obj/unact`) whose source was not
in the repository.
