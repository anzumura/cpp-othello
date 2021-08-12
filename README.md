# C++ othello game

I started this project in May 2021 in order to refresh my C++ knowledge
after close to 10 years using other languages (mainly Python and Scala). This project includes a main app called *othello* (human and computer player types are supported). The computer players can also be customized (choice of heuristics, allow randomization, how many moves to search, etc.). There is also an *othelloClient* app that can connect to the *othello* app when a remote player is specified.

I used this project to focus on learning the following things:
- **Modern C++**: I tried using lots of C++ features that were introduced in C++ 11 (or later) including *shared_ptr*, *optional*, *array*, *bitset*, *initializer_list*, *chrono*, *random*, ...
- **CMake and clang**: previously I used *make* and *gcc* (on Linux) and even older things in the early 90's.
- **VS Code**: I wanted to try using a 'modern', 'open' IDE that runs on *macOS* (I had mostly used *emacs* for C++ development before).
- **Automated Testing**: I chose *GoogleTest* and *GMock*
- **Code Analysis Tools**: I wanted automated checking of stack/memory type problems as well as memory leak detection. However, for *macOS* (on an *M1* processor) I was only able to get some *Address Sanitizer* options to work (see *othelloClientMain.cpp*) which was still helpful. I have used things like *purify* and *quantify* in the past which were pretty cool for this type of stuff.

I also tried to do some low-level *client-server* code. I have used various libraries for this before, but this time I tried using *boost/asio*. I hope to come back to this later and perhaps add a UI (instead of only supporting playing via the console).
