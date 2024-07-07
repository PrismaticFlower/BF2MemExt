Tool for patching SWBF2 (2005) to extend it's memory limits.

## Current Patches

- Runtime Heap Extension. This makes it **much** harder for modders to hit "Allocating X bytes failed - no free blocks left in Heap 5 (Runtime)" errors.
- SoundParameterized Layer Limit Extension. This keeps maps with lot's of flyers and entities that use `EngineSound` from crashing.

## Supported Versions

- [GoG Version](https://www.gog.com/en/game/star_wars_battlefront_ii)
- [Steam Version](https://store.steampowered.com/app/6060)
- BF2_modtools (The version of the game used to debug mods. Found in the modtools.)

If you're interested in seeing another version of the game supported feel free to open an Issue (or +1 an Issue if someone else has already asked for your version to be supported).


## Usage

The tool itself is a simple Win32 GUI app. Launch it, click "Patch Executable", browse to your game's executable (the one named `BattlefrontII.exe` and is in the same folder as your `Addon` folder) and click Open. The tool will then patch the executable, if it recognizes the executable and is able to patch it you'll get a success message.

If it fails the executable will left unmodified. Replacing it is the final step it does after everything else has succeeded.
