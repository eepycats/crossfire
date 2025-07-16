# crossfire

csgo crossplay between xbox 360, ps3 and pc (poc) 

~~how did we get here again?~~

## how to use

### server setup

1. get 731 and 740 [(manifest ids below, click!)](#manifest-ids) [(how to download)](#how-to-download-from-steam-small-guide)
2. extract/download both depots into the same directory
3. drop crossfire.dll and crossfire.vdf into `csgo/addons`
4. start the server with your args **(but don't forget -xlsp and -xlsp_insecure if you want working changelevel, im sorry i couldnt figure out what causes this one,,)**



### windows client setup

1. get 731 and 732 [(manifest ids below, click!)](#manifest-ids) [(how to download)](#how-to-download-from-steam-small-guide)
2. extract/download both depots into the same directory
3. drop crossfire.dll and crossfire.vdf into `csgo/addons`
4. if you start the game now, you should see a cat in the window title (it's supposed to be a cat but the windows window title renderer is uhhhhhh suboptimal to say the least)
5. get a cracked client.dll (you need this even though you own the game, thanks for CEG valve...) from [here](https://vaivesoftware.com/cracked-client.zip) and replace the binary in `csgo/bin` with this one (thanks to Mikko for this crack)
6. the game should work now (hopefully,,,)

### xbox ~~720~~ 360 setup

this one is a bit more complicated since as far as im aware there are no command execution thingies that just work on every source game (at some point im going to start working on something like that but blehh i dont have motivation to do much lately grrrr,rr sryyy)

for the time being i guess the only solution is to take the c# thing from my other repo and replace the ExecuteCommand function with this one

```c#
void ExecuteCommand(string command)
{
    // tu 0 0x860992A0
    // tu 5 0x8609A848


    // l4d -> 8642C060
    // csgo 0x86A1A330
    XDRPCExecutionOptions options = new XDRPCExecutionOptions(XDRPCMode.Title, 0x86A1A330);
    XDRPCArrayArgumentInfo<byte[]> cmd = new XDRPCArrayArgumentInfo<byte[]>(Encoding.ASCII.GetBytes(command), ArgumentType.ByRef);
    XDRPCArgumentInfo<int> dummy = new XDRPCArgumentInfo<int>(0);
    XDRPCArgumentInfo<int> dummy2 = new XDRPCArgumentInfo<int>(0);

    uint num = xbox.ExecuteRPC<uint>(options, [dummy,cmd,dummy2]);
}
```

i know its a bit suboptimal but i dont really have anything better for now sorry

so after you figure out how to execute commands, you need a [patched engine xex that opens UDP sockets instead of VDP or whatever the protocol ms imagined is](https://vaivesoftware.com/engine_360.dll)

simply replace the engine_360.dll and now you should be able to connect to the server

### ps3 setup

~~someone else write this part please please please and pr it kthxxxxxxxxx~~

as far as im aware on the ps3, all you need is a plugin called `CSGOPlugin` and the 1.01 version of the game then you need to spam the select button or something to open the console and from there you should be able to connect to the server

~~yes i know this explanation sucks but i hope someone will make a better one i don really do much ps3 stuff~~

## building

- get [xmake](https://xmake.io) (live, laugh, lua)
- get vs2022 build tools or install visual studio 2022 (needs c++23 support)
- run `xmake`
- copy `csgo` from dist to ur game root directory (so the addons directory ends up next to gameinfo.txt)

## bugsss

- radar is a bit broken because one of the scaleform usermessages causes the xbox to spontaneously combust so i had to filter it out (maybe i'll look into a proper solution later but i doubtt)

- voice chat doesn't and will probably never work (3 different codecs)

- cs_office crashes the consoles while loading for some reason

## artifacts!

this repo should have artifacts working, so check out [here](https://github.com/eepycats/crossfire/actions) for prebuilt binaries!

### manifest ids

| 731              | 732              | 733              | 740              |
|------------------|------------------|------------------|------------------|
| 291426208448326326 | 760024964360109585 | 4941625320971306498 | 4234207694164018948 |

### how to download from steam, small guide

the (probably) easiest way to download things from steam is by using a tool called DepotDownloader which you can get [here](https://github.com/SteamRE/DepotDownloader/releases)

#### Download commands

- 731: `./DepotDownloader -app 730 -depot 731 -manifest 291426208448326326`
- 732: `./DepotDownloader -app 730 -depot 732 -manifest 760024964360109585`
- 733: `./DepotDownloader -app 730 -depot 733 -manifest 4941625320971306498`
- 740: `./DepotDownloader -app 740 -depot 740 -manifest 1031872640143727699`

<sup><sub>🐈</sub></sup>
