# Examples

To view the full example take a look [here](https://github.com/Fraunhofer-IIS/libjapi/blob/master/doxydir/demo.cpp).

\anchor serverExample
## Server example
\dontinclude demo.cpp
Include required headers:
\skipline stdio
\skip japi
\until utils
Register services and start server:
\skip main
\until return ret
\line }

\anchor clientExample
## Client request example
\include test.py

