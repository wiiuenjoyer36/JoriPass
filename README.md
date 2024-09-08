# NetPass: a new way to experience StreetPass!

[![Translation status](https://weblate.sorunome.de/widget/netpass/app/svg-badge.svg)](https://weblate.sorunome.de/engage/netpass/)

![QR Code](https://gitlab.com/Sorunome/3ds-streetpass/-/raw/release_builds/qr.png)  
Scan QR code to download!

GBAtemp thread: https://gbatemp.net/threads/a-new-way-to-experience-streetpass.653810/  
Discord guild: https://discord.gg/netpass

Introducing NetPass:

In the current state of the world, the 3DS is, sadly, on decline. That makes getting StreetPasses harder and harder, due to fewer and fewer people taking their 3DS with them on a walk. This is where NetPass comes in!  
Unlike StreetPass, NetPass works over the internet. Upon opening NetPass, you can pick various locations to go to, i.e. the train station, or a town plaza. Upon entering a location, you get passes of others who are in the same location! And, while you are at the location, others who enter it can get passes with you. But beware! You can only switch locations once every 10 hours!

## Licenses
The source code of this project is licensed under GPLv3 or later. See the included `LICENSE`.

Other assets, such as images and sounds, are licensed under CC-BY-NC. See the included `LICENSE-assets`.

## Translations
If you want to contribute to translations, we are doing that on [our Weblate](https://weblate.sorunome.de/projects/netpass)!

## Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started)

After installing devkitPro, you will need to download the following using [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman) or the devkitPro updater:

- 3ds-dev
- 3ds-curl

In other words, you'll need to run the following command in a Terminal/command prompt (with administrator/root privileges):

```bash
dkp-pacman -S 3ds-dev 3ds-curl
```

Furthermore you need to have `ffmpeg` and `python` and `python-pyyaml` installed.

Be sure to run `make codegen` first.
```bash
make codegen
make
```

### Additional prerequisites to build the `.CIA`
 - Makerom: You need the `makerom` executable in your `$PATH`
   You can get it precompiled on https://github.com/3DSGuy/Project_CTR/releases and then copy it to `$DEVKITPRO/tools/bin`
 - Bannertool: You need the `bannertool` executable in your `$PATH`
   You can get it precomiled on https://github.com/diasurgical/bannertool/releases

## Compilation

This project ships with a [Makefile](Makefile), which is meant to simplify the compilation process. If you're unfamiliar with them, you can find out more about GNU Make [here](https://www.gnu.org/software/make/).

```bash
make
```

To build `.cia` files, run

```bash
./build_release.sh
```

## Credits
### Research
 - [This gist](https://gist.github.com/wwylele/29a8caa6f5e5a7d88a00bedae90472ed) by wwylele, describing some cecd functionality
 - [This repo](https://github.com/NarcolepticK/CECDocs) by NarcolepticK documenting some more of the cecd sysmodule
 - [StreetPass 2](https://gbatemp.net/threads/streetpass-2-rise-from-the-ashes.526749/) for valuable data dumps
 - 3Dbrew and all its contributors, especially of the [cecd service](https://www.3dbrew.org/wiki/CECD_Services)

### Translations
 - English: Sorunome
 - German: Sorunome
 - Russian: [Rednorka](https://gbatemp.net/members/rednorka.575239/), Geo
 - Japanese: [Akira SUNADUKA](https://gitlab.com/Akira-SN)
 - Polish: [DanteyPL](https://gitlab.com/DanteyPL)
 - Spanish: [Gato-kun](https://gitlab.com/Gato-kun), [Amnesia1000](https://gitlab.com/Amnesia1000)
 - French: [Straky](https://straky.fr/en), [EssePeius](https://gitlab.com/Essepeius), Tourneur
 - Italian: [LNLenost](https://github.com/LNLenost)
 - Chinese (Traditional): ManLok Ho
 - Ukrainian: Geo
 - Portugese: Lia, arth
 - Dutch: Robbin12391, aiydn
