# Purpose of this document

This tutorial is here to teach you, specifically, to mod an 07th-Expansion game.

If you are looking to create a new game with ponscripter, this will not help you. Nothing will. The [API docs](/ponscripter-fork/api/ ':ignore') will only make things worse. Just stop. Please.

If you are looking to modify a different ponscripter game, information here may be somewhat inaccurate, as it's written with Ryukishi's programming oddities in mind.


# The basics

ponscripter is the engine that 07th-Expansion has used to release all its games in the West so far, with the sole exception of Higurashi &mdash; that used a custom Unity-based engine. Some of the games that weren't officially released used an older iteration of the engine &mdash; ONScripter &mdash; but we don't expect that those older games will ever receive console releases, so they are largely irrelevant. We expect that future 07th-Expansion games will keep using ponscripter into the foreseeable future.

The [current version of ponscripter](https://github.com/07th-mod/ponscripter-fork) is maintained by the 07th-Mod team.

A [full API documentation](/ponscripter-fork/api/ ':ignore') is available as well, but it is somewhat difficult to parse and contains a lot of knowledge that you likely won't ever need when working on a mod.

## File encryption

All Ryukishi games use a single script file called `0.utf`. The 07th-Mod version of the engine modifies this filename to `0.u` instead, to prevent Steam upgrades from breaking scripts. If you cannot find a `0.utf` or a `0.u` in the game folder, you will likely instead see a `pscript.dat`. This is a trivially obfuscated script, and can be decoded with `nscdec` (rename `result.txt` to `0.u`). You may also find one or more `arc*.nsa` files. These are archived resources (images etc), and can be extracted with `nsadec`. **There is no need to ever re-obfuscate the script or re-pack the NSA archives**, the game will work fine in the decompiled state and always prefer plain files over encrypted/archived ones. `nscdec` and `nsadec` can be downloaded [here](/ponscripter-fork/onscrtools.zip ':ignore').

## Comments
ponscripter's comments start with a `;` and last until the end of the line. There are no block comments. Note that three actual commands start with a `;` too: `;value`, `;mode` and `;gameid`. Those are *not* comments, even though in any sane person's mind they would be.

## Command separation
Commands in ponscripter are separated with a newline, or with a `:` (that's a *colon*, not a semicolon &mdash; those are comments).

# Text display

A lot of work will likely involve editing text. ponscripter's text display is *slightly* insane. Any Japanese text will just be displayed as-is in most contexts, so a Japanese text line will mostly just consist of the text itself and of control characters (see below). For English, on the other hand, you need to use the "text mode". The text mode is entered whenever a `^` character is encountered, and exited upon encountering a clickwait/pagewait. A closing `^` is optional, but highly recommended as it makes text significantly easier to parse. In other words, English text usually looks like a weird quoted string:

```
…お酒を嗜まれましたな？ ; Japanese text example
^  ...So, you still haven't overcome your love of alcohol?"^ ; English text example
```

## Control characters

Text lines will tend to be interspersed with a few control characters that alter how text display works. There's a few of those that are relevant here.

- `@` will insert a "clickwait". This will wait for a user to click their mouse button or otherwise advance the text. After the click, the text will proceed without clearing the screen. If the line in the script ends with a `@`, a paragraph break is automatically inserted in the displayed text as well.
- `\` will insert a "pagewait". This is the same as above, except the screen is cleared of text after the click as well.
- `@/` is the same as `@`, except a paragraph break is never automatically inserted.
- `~` is for text tags such as bold, italic, etc. The syntax you'll see most usually is `~ib~some text~ib~`, which renders as ***some text***.
- `#` is for colours. These are just your usual HTML colour codes. `^#ff0000You are incompetent!#ffffff^` will render as `You are incompetent!` in red, switching back to white for the rest of the text. Note that colour *names* are not supported: `#red` will not do anything.

Be very, very careful with the `~` character when translating, especially when porting translations over from ONScripter (as it didn't have `~` as a special character). It might be tempting to use it to express playfulness or something similar to that, but it will also crash the engine. If you want to actually render a `~` in your text, double it, i.e. write `~~`. These mistakes tend to be hard to catch, as the line will look just fine at a glance, so I suggest just avoiding this cursed snake altogether.

The explanation about newlines might not make much sense without seeing it in action. So, let's give a few examples:

```
^A^@^B^@^C^ ; This will result in "ABC" after all clickwaits are triggered
^A^@
^B^@
^C^@ ; This will result in "A B C" (with linebreaks instead of spaces)
^A^@/
^B^@/
^C^@/ ; This will have the same effect as the first line
```

## Language support
ponscripter is built as an engine with bilingual games in mind: in other words, the same script will traditionally house text for both Japanese and English (adding more languages is, unfortunately, impossible). In games that use this feature, Japanese lines should be prefixed with `langjp` and English lines should be prefixed with `langen`. The text in these lines will only be displayed when the user has the appropriate language selected, although do note that any commands unrelated to text display will *not* take `langen` and `langjp` into account, and trigger for both languages.

## Voices
Voices are generally played with the `dwave` command. There is a multi-lingual version of that command that we created for our mod: `dwave_eng` will only play on `langen` lines, and `dwave_jp` will only play on `langjp` lines. If maintaining Japanese support is one of your goals, you *really* should use `dwave_jp` and `dwave_eng`, otherwise voices will turn into a mess.
The syntax of `dwave` is simple enough: `dwave channel_number,"voice_file"`. The channel number is usually set to `0`, it's only ever useful if you want to play multiple voices *at once*: in which case, use a syntax like `dwave 0,"voice_file_1":dwave 1,"voice_file_2":dwave 2,"voice_file_3"`

## Putting it all together... (text display)

Now that we've covered all of *that*, we can finally read some of Umineko's script! Let's give it a go!

```
langjp「この馬鹿がッ！！@　決まっておろうがッ！！@　殺したらそなたが歪めるだろうその表情が楽しいからの他に何の理由が必要なのかッ！！@　くっひゃはははははははは！！」\
langen:dwave_eng 0, "voice\27\50700001.ogg":^"You idiot!!^@:dwave_eng 0, "voice\27\50700002.ogg":^  Isn't it obvious?!!^@:dwave_eng 0, "voice\27\50700003.ogg":^  It'll be fun to kill her and see your face twist in pain, why else?!!^@:dwave_eng 0, "voice\27\50700004.ogg":^  Gahyahahahahahahaha!!"^\
```

You should now be able to read this wihtout significant issues. Congrats, you know how to edit text!

# The first lines of the script
The first lines of the Umineko script contain the following expressions:
```
;value2500,modew540@2x,localsave=mysav
;gameid Umineko4hdz
```
All of the three bits of the first line are optional, but nevertheless important. Let's discuss them in order:

- `value2500` is the global variable border. This is too terrifying of a concept to explain just yet, so for your own safety, it'll be tackled later.
- `mode` sets the resolution. A wide variety of resolutions are supported, but only a few are actually anything a sane person would use:
  - `modew720`, for 1280x720
  - `modew1080`, for 1920x1080
  - You may also encounter `modew540@2x`, for the main Umineko games (this is also 1080p, but with the weird auto-upscale enabled, see below). It's only around for historical reasons.
-  `localsave=mysav` forces the game to save in the `mysav` folder in the game directory. This is recommended for Steam releases, as ponscripter handles Steam saves in a very weird way (see below).

The second line is just `;gameid` followed by an identifier.

- This is ignored if `localsave` is set, in which case the folder specified in `localsave` is used.
- This is also ignored when running on the Steam build of the engine *from Steam*. If `localsave` is not set, but a Steam build is used and launched via Steam, the `saves` folder will be used for saves.
- If neither of those apply, this line will make game saves go to `%APPDATA%\specified_game_id`.

# Images
## Image transparency
tl;dr: Put `force-png-alpha` into `pns.cfg` in the game root, creating the file if it does not exist.

ponscripter features three modes for image transparency: `leftup`, `alphablend` and `copy`.

- `leftup` assumes that the top-left pixel is a "sacrificed" colour: all pixels with the colours identical to the top-left one become transparent.
- `alphablend` assumes that the image is doubled in width, and its right half is an inverted alpha-mask: pixels that are black on the mask are fully opaque, pixels that are white are fully transparent.
- `copy` assumes that the image contains no transparency information at all, and is just to be drawn as-is.

The transparency mode can be set per game (with `transmode`), but modifying that is not recommended -- it will, most likely, break things.

The transparency mode can also be set per image. This is done by prefixing the file path with `:l;`, `:a;` or `:c;`, e.g. `":a;myfile.bmp"`.

This implementation pre-dates support for the PNG format, and as such does not directly take into account the actual alpha channel of a PNG image.
However, if the top-left pixel of a transparent PNG is transparent, PNG transparency will be treated as expected. This is the case most of the time.
If you have transparent PNGs that have a non-transparent top-left pixel, however, transparency will break.
To fix it, add the aforementioned config parameter.

## Image sizes
Older ponscripter version will render all images at double their resolution. This can be fixed per-image by adding `:b;` before the path (if a transparency flag is used, it becomes something similar to `:ba;` instead).
In the latest releases, this is a config option passed to `mode` on the first line of your script (see below), e.g. `modew540@2x` will enable it.
You'll usually never want to have this enabled for a modded game, unless there's historical reasons for it. Use `modew1080`. If you want to re-use some images from the base game, upscale them.

## Effects and effect numbers
When showing and hiding images, ponscripter supports a variety of visual effects. The few built-in ones are documented [here](/ponscripter-fork/api/#effect ':ignore'), and the remaining will vary per game. Ryukishi references effects by numbers instead of giving them sensible names. Unfortunately, this is something you'll have to get used to for showing/hiding custom images. You may look up how custom effects work and what they are by searching the script of your game for `effect`, and you'll most likely find the definitions quickly enough. Most of the custom effects use image-based masks, which should be relatively simple to understand.

## The print statement
Effect number 0 is special. It merely loads the images in memory, storing them for later usage. When you are ready to use them, call `print`, followed by an actual effect number, e.g. `print 5`. Some commands don't take effect numbers as arguments at all, and instead *require* you to use `print`.
## Backgrounds

Backgrounds are fairly trivial: `bg "filename",effect_number`, e.g. `bg "backgrounds\my.png",2`. If you use effect `0`, you will need to `print` your background.

## Sprites
Sprites are more complex. This is mainly because Ryukishi has defined custom functions that handle sprite loading and placement, and those functions will vary greatly per game. As a general rule, we'll want to avoid those custom definitions (unless we're just copying a line that already works) and instead rely on the built-in ponscripter functionality, which does the job just the same, but doesn't vary per-game. As a general rule, we will only care about four commands: `_ld` (note the underscore), `cl`, `lsp` and `csp`.

### "_ld" and "cl"
The first two are the easiest to work with: they take a "side" (`l`eft, `c`enter or `r`ight), a filename (only for `_ld`) and an effect number, e.g.:

```
_ld l,"sprites\nat\1\nat_a11_zutuu1.png",80 ; Natsuhi has headache
; ...
cl l,14 ; Natsuhi dies
```
`cl` optionally takes "a" as a side, clearing all the sprites loaded with `_ld`.

Just like before, using effect `0` will require `print`.

### "lsp" and "csp"

`lsp` and `csp` are slightly more complicated. For one, they take a "sprite number" as an argument &mdash; this can be anything as long as there's no other `lsp` statements nearby that already take this number up. You make up a number for use with `lsp`, and then pass that same number to `csp` to get rid of the sprite (you **MUST DO THIS** -- do not leave these sprites hanging in memory!). These commands *must* also be followed by `print`, or they will have no effect. The syntaxes are:
```
lsp sprite_number,filename,x,y,opacity
print effect_number
; ...
csp sprite_number
print effect_number
```
The `opacity` argument is optional, and ranges from 0 to 255.

Example:
```
lsp 0,"bg\butterfly_3x.png",0,0
print 2
; ...
csp 0
print 3
```

### A note on "mld"
`mld` is a Ryukishi hack that allows the engine to display 2 sprites per side of `_ld`, with slight offsets (`mld l` is not the same position as `_ld l`, but it's still *on the left*). `mld` has largely the same syntax as `_ld`, but is fairly complex internally and will, once again, vary per game. If you want to do something similar to what `mld` does, I suggest just seeing what it does to coordinates in your game and then using the same equations for `lsp`, unless you understand exactly how `mld` works in your specific game.

# To be continued...
