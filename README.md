# MessageU
Instant messaging software. (Maman 15, Defensive Systems Programming).
* Client code written with C++.
* Server code written with Python.


## Project Configuration

### Client
* Developed with Visual Studio 2019.
* Client code written with ISO C++14 Standard. (Default by Visual Studio 2019).
* Boost Library 1.77.0 is used. https://www.boost.org
* Crypto++ Library 8.5 is used. https://www.cryptopp.com
* <b>These instructions apply for x86 configuration. Upon loading .sln, use x86 build!</b>

#### Client project configuration:

Both libraries Boost & Crypto++ are statically built in this guide.

<b>1. Boost 1.77.0 Installation & Configuration</b>

Boost 1.77.0 Installation Instructions are based on [Pattarapol Koosalapeerom's Boost Installation Instructions](https://tomkoos.github.io/cpp/boost-vs.html).

1.1. Get Boost
* Download the copy of Boost for Windows platform via http://www.boost.org/users/history/version_1_77_0.html. Either .7z or .zip is fine.
* Extract the archive file to your directory of choice. Example path: <i>"D:\boost_1_77_0\"</i>

1.2. Complie Boost library
* Run CMD <b>as administrator</b> inside boost folder.
* The following commands will take a while to build:
* Run <i><b>bootstrap.bat</b></i> 
* Run <i><b>b2 link=static runtime-link=static </b></i>

1.3. Include Boost library in Visual Studio's C++ Project

* Open Client's Project Properties.
* Add <i><b>"D:\boost_1_77_0\"</b></i> under <i>Project > Properties > C/C++ > General > Additional Include Directories</i>
* Add <i><b>"D:\boost_1_77_0\stage\lib"</b></i> under <i>Project > Properties > Linker > General > Additional Library Directories</i>
* Define <i><b>_WIN32_WINNT=0x0A00</b></i> under <i>Project > Properties > C/C++ > Preprocessor > Preprocessor Definitions</i> (Windows 10. For other OS see this [link](https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-160)).


<b>2. Crypto++ 8.5 Installation & Configuration</b>

2.1. Get Crypto++
* Download the copy of Crypto++ for Windows platform via https://www.cryptopp.com/#download. (ZIP).
* Extract the archive file to your directory of choice. Example path: <i>"D:\cryptopp850\"</i>

2.2. Complie Crypto++ library
* Open <i>"D:\cryptopp850\cryptest.sln"</i> with Visual Studio.
* Build the solution. Make sure build configuration matches. (For example, Debug, Win32).
* Close the solution.
* We will use the static library <i>cryptlib.lib</i>. (If Win32, Debug was built, the library will be located within <i>Win32\Output\Debug</i>).


2.3. Include Crypto++ library in Visual Studio's C++ Project

* Open Client's Project Properties.
* Add <i><b>"D:\cryptopp850\"</b></i> under <i>Project > Properties > C/C++ > General > Additional Include Directories</i>
* Add <i><b>"D:\cryptopp850\Win32\Output\Debug\cryptlib.lib"</b></i> under <i>Project > Properties > Linker > Input > Additional Dependencies</i>
* Make sure the project's Runtime Library is <i>Project > Properties > C/C++ > Code Generation > Runtime Library > <b>Multi-threaded Debug (/MTd)</b></i>

<b>3. Additional configurations</b>

The following configurations already set within the sln. Unlike above libraries, it doesn't need external references hence probably shouldn't be modifed.
* Not using precompiled headers. 
* Added additional include path $(ProjectDir)\cryptopp_wrapper\



### Server
* Developed with PyCharm 2021.1.2.
* Server code written with Python 3.9.6.
#### Server project onfiguration:
No special packages were required. Only the language's standard.


