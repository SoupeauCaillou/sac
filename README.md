sac
===

*sac* is [Soupe au Caillou](http://soupeaucaillou)'s 2D C++ game engine.

Features
--------
* Entity/System design. Game objects do not have to inherit from a base class like Node or GameObject but are instead a simple identifier (32 bit integer). We then use composition to add/remove features to individual objects. See ([0](http://t-machine.org/index.php/2007/09/03/entity-systems-are-the-future-of-mmog-development-part-1/)) and ([1](http://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/)) for more details on the concept.

* Multithreaded OpenGL renderer. Or more precisley: game logic updates happen on 1 thread, and rendering using OpenGL is done in another thread. The 2 threads only share 1 data structure: a rendering commands queue. The queue itself is built by transforming all renderable entities in render commands. In a second step this queue is sorted using a 64 bits key containing rendering states (which texture atlas, blending enabled or not, etc). See ([2](http://realtimecollisiondetection.net/blog/?p=86)) for a more in-depth article

* Debug UI using ([imgui](https://github.com/ocornut/imgui)). Allow inspection and manipulation of all entities/components as well as enabling debug info on various systems.

* Rewind: at any time you can pause the game and rewind (or move backward/forward frame by frame). This is really handy when debugging.

* The engine is multiplatform (Android, Linux, Mac, Windows, Web) but uses a single build system: cmake

* Music and Sound playing is supported as well. While Sound is really simple, Music can do some nice things: looping at specific time, blending, etc

* Entities can be defined in resources files, in a pseudo-ini format.

* Assets hot reload (only on desktop), allows to see modifications to assets while running the game

* Logging: the engine uses macros for logging - LOGI(), LOGE_IF(), LOGW_EVERY_N() etc. They can be easily disabled in release build

* Multitouch support

* Physics: we built our own physics engine - it's obviously not as good as Box2D and the like, but it can handle simple things well. Similarly we have a few Collision routines (and raycasting support).

* Minimal amount of c++ costly construction (std::string, std::map, etc). Where performance matters we replaced them by hashes or vector kind of storage.


Games using sac
---------------

<p align="center">
  <img src="http://soupeaucaillou.com/images/heriswap.png?raw=true" alt="Heriswap" title="Heriswap" />
  <img src="http://soupeaucaillou.com/images/rr.png?raw=true" alt="Recursive Runner" title="Recursive Runner"/>
</p>

* Heriswap is an Android match-3 game ([Google Play](https://play.google.com/store/apps/details?id=net.damsy.soupeaucaillou.heriswap) or [F-droid](https://f-droid.org/repository/browse/?fdid=net.damsy.soupeaucaillou.heriswap) or [direct link](http://soupeaucaillou.com/games/heriswap.apk)) which wants to be zen and smooth. Sources are available [here](https://github.com/SoupeauCaillou/recursive-runner).

* Recursive Runner is an Android runner game ([Google Play](https://play.google.com/store/apps/details?id=net.damsy.soupeaucaillou.recursiveRunner) or [F-droid soon](https://github.com/SoupeauCaillou/recursive-runner/issues/1) or [direct link](http://soupeaucaillou.com/games/recursive_runner.apk)) where your main obstacle is yourself! Sources are available [here](https://github.com/SoupeauCaillou/recursive-runner).

* Assault serie is a set of tactical turn-based wargames during the WWII. A demo Headquarters ([Google Play](https://play.google.com/store/apps/details?id=net.damsy.soupeaucaillou.warbler.assault.headquarters)) is available, first episode Normandy ([Google Play](https://play.google.com/store/apps/details?id=net.damsy.soupeaucaillou.warbler.assault.normandy)) and also second episode Bastogne ([Google Play](https://play.google.com/store/apps/details?id=net.damsy.soupeaucaillou.warbler.assault.bastogne)) are available on the market.


Documentation
-------------
See ([wiki](https://github.com/SoupeauCaillou/sac/wiki))

