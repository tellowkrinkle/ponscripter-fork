This document is meant as a short crash-course into the `ponscripter` engine and cover the topics you are likely to deal with when working on mods.

ponscripter is the engine that 07th-Expansion has used to release all its games in the West so far, with the sole exception of Higurashi &mdash; that used a custom Unity-based engine. Some of the games that weren't officially released used an older iteration of the engine &mdash; ONScripter &mdash; but we don't expect that those older games will ever receive console releases, so they are largely irrelevant. We expect that future 07th-Expansion games will keep using ponscripter into the foreseeable future.

The [current version of ponscripter](https://github.com/07th-mod/ponscripter-fork) is maintained by the 07th-Mod team.

A [full API documentation](/api/ ':ignore') is available as well, but it is somewhat difficult to parse and contains a lot of knowledge that you likely won't ever need when working on a mod.

# On file encryption

All Ryukishi games use a single script file called `0.utf`. The 07th-Mod version of the engine modifies this filename to `0.u` instead, to prevent Steam upgrades from breaking scripts. If you cannot find a `0.utf` or a `0.u` in the game folder, you will likely instead see a `pscript.dat`. This is a trivially obfuscated script, and can be decoded with `nscdec` (rename `result.txt` to `0.u`). You may also find one or more `arc*.nsa` files. These are archived resources (images etc), and can be extracted with `nsadec`. **There is no need to ever re-obfuscate the script or re-pack the NSA archives**, the game will work fine in the decompiled state and always prefer plain files over encrypted/archived ones. `nscdec` and `nsadec` can be downloaded [here](/onscrtools.zip ':ignore').

# On comments
ponscripter's comments start with a `;` and last until the end of the line. There are no block comments. Note that three actual commands start with a `;` too: `;value`, `;mode` and `;gameid`. Those are *not* comments, even though in any sane person's mind they would be.

# On command separation
Commands in ponscripter are separated with a newline, or with a `:` (that's a *colon*, not a semicolon &mdash; those are comments).

# On text display

A lot of work will likely involve editing text. ponscripter's text display is *slightly* insane. Any Japanese text will just be displayed as-is in most contexts, so a Japanese text line will mostly just consist of the text itself and of control characters (see below). For English, on the other hand, you need to use the "text mode". The text mode is entered whenever a `^` character is encountered, and exited upon encountering another `^`. In other words, English text looks like a weird quoted string:

```
…お酒を嗜まれましたな？ ; Japanese text example
^  ...So, you still haven't overcome your love of alcohol?"^ ; English text example
```

# On control characters

Text lines will tend to be interspersed with a few control characters that alter how text display works. There's a few of those that are relevant here.

- `@` will insert a "clickwait". This will wait for a user to click their mouse button or otherwise advance the text. After the click, the text will proceed without clearing the screen. If the line in the script ends with a `@`, a paragraph break is automatically inserted in the displayed text as well.
- `\` will insert a "pagewait". This is the same as above, except the screen is cleared of text after the click as well.
- `@/` is the same as `@`, except a paragraph break is never automatically inserted.
- `~` is for text tags such as bold, italic, etc. The syntax you'll see most usually is `~ib~some text~ib~`, which renders as ***some text***.

This explanation about newlines might not make much sense without seeing it in action. So, let's give a few examples:

```
^A^@^B^@^C^ ; This will result in "ABC" after all clickwaits are triggered
^A^@
^B^@
^C^@ ; This will result in "A B C" (with linebreaks instead of spaces)
^A^@/
^B^@/
^C^@/ ; This will have the same effect as the first line
```

# On language support
ponscripter is built as an engine with bilingual games in mind: in other words, the same script will traditionally house text for both Japanese and English (adding more languages is, unfortunately, impossible). In games that use this feature, Japanese lines should be prefixed with `langjp` and English lines should be prefixed with `langen`. The text in these lines will only be displayed when the user has the appropriate language selected, although do note that any commands unrelated to text display will *not* take `langen` and `langjp` into account, and trigger for both languages.

# On voices
Voices are generally played with the `dwave` command. There is a multi-lingual version of that command that we created for our mod: `dwave_eng` will only play on `langen` lines, and `dwave_jp` will only play on `langjp` lines. If maintaining Japanese support is one of your goals, you *really* should use `dwave_jp` and `dwave_eng`, otherwise voices will turn into a mess.
The syntax of `dwave` is simple enough: `dwave channel_number,"voice_file"`. The channel number is usually set to `0`, it's only ever useful if you want to play multiple voices *at once*: in which case, use a syntax like `dwave 0,"voice_file_1":dwave 1,"voice_file_2":dwave 2,"voice_file_3"`

# Putting it all together... (text display)

Now that we've covered all of *that*, we can finally read some of Umineko's script! Let's give it a go!

```
langjp「この馬鹿がッ！！@　決まっておろうがッ！！@　殺したらそなたが歪めるだろうその表情が楽しいからの他に何の理由が必要なのかッ！！@　くっひゃはははははははは！！」\
langen:dwave_eng 0, "voice\27\50700001.ogg":^"You idiot!!^@:dwave_eng 0, "voice\27\50700002.ogg":^  Isn't it obvious?!!^@:dwave_eng 0, "voice\27\50700003.ogg":^  It'll be fun to kill her and see your face twist in pain, why else?!!^@:dwave_eng 0, "voice\27\50700004.ogg":^  Gahyahahahahahahaha!!"^\
```

You should now be able to read this wihtout significant issues. Congrats, you know how to edit text!

# To be continued...
