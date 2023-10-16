# StormLibX

A reimplementation of Storm by wrapping other libraries, for Starcraft 1.17. Functions will need to be remapped or tweaked to work for other classic titles.

The implementations mostly wrap other libraries. Here is the list of supported Storm submodules:


| Submodule  | Description                | Implementation                                          |
|------------|----------------------------|---------------------------------------------------------|
| SBig       | Big Numbers                | [squall](https://github.com/whoahq/squall)              |
| SBlt       | Surface Blitting           | Not yet implemented                                     |
| SBmp       | Image reading/writing      | Not yet implemented                                     |
| SCmd       | Command Line               | No plans to implement                                   |
| SCode      | JIT Rendering Compiler     | Not yet implemented                                     |
| SDlg       | Dialogs and Popups         | Not yet implemented                                     |
| SDraw      | DirectDraw Wrapper         | [SDL2](https://github.com/libsdl-org/SDL)               |
| SErr       | Error Handling             | [squall](https://github.com/whoahq/squall)              |
| SEvt       | Event System               | No plans to implement                                   |
| SFile      | File System and MPQs       | [StormLib](https://github.com/ladislav-zezula/StormLib) |
| SGdi       | Windows GDI Wrapper        | Not yet implemented                                     |
| SLog       | Logging                    | No plans to implement                                   |
| SMem       | Memory Management          | [squall](https://github.com/whoahq/squall)              |
| SMsg       | Message Handling           | No plans to implement                                   |
| SNet       | Networking                 | Not yet implemented                                     |
| SReg       | Registry/Settings          | Not yet implemented                                     |
| SRgn       | Regions Library            | [squall](https://github.com/whoahq/squall)              |
| SSignature | Data Signatures/Validation | No plans to implement                                   |
| SStr       | String Management          | [squall](https://github.com/whoahq/squall)              |
| STrans     | Transparency               | Not yet implemented                                     |
| SUni       | Unicode Conversions        | Not yet implemented                                     |
| SVid       | Bink Video Wrapper         | Not yet implemented                                     |
