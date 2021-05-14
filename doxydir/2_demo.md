
# Demo
You can clone the demo project, with examples for all features from the repository listed below:

\code
$ git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi-demo.git
\endcode

First build the libjapi, the demo uses, in the libjapi directory with:
\code
$ make
\endcode

Back in the libjapi-demo directory run
\code
$ make run-static
\endcode
to start the server.

\note for the next step python3 needs to be installed.

Start the python client script in a separate tab with
\code
$ ./test.py
\endcode

The server can manage multiple clients. Try and run multiple clients simultaneously.

To end the client(s) send a `CTRL-C`. Same for the server.