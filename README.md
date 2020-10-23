----------brought to you by rude_bwoy92------------

-----------MAIN.CPP IS THE ONLY FILE--------------

have cut all the extra features except irc client-server interaction & all the windows-exclusive tools.

have been trying to port the remains from winsock to socket.h in Mint.

have not yet finished, but expected to wrap it up soon.

api for mptcp lies somewhere in depth of hundreds of folders. sources for mptcp weigh 7 GBs. too much. i could install this on one of the machines, but i doubt if my small ssd would like it after copying them.

in beta v.1.1 i've got a buildable valid socket app, but at the cost of disabling some functions crucial for real operation.

in beta v1.2 i've tried to employ a tcp flood function. 
realised that I don't need api for mptcp since the kernel would do everything for me. 
installing mptcp on the kernel has not succeeded. have been trying numerous linux distros, failed for different reasons. 
and yes, mptcp source weighs about 800 mb., not 7 gb.
in doubts about the workability of the bot. hasn't checked yet.

in beta v2.1 code was fully rewritten from scratch with multithreading support. main.cpp not finished.
