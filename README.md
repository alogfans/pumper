# Pumper #
> Naive K/V Storage Server

This is one implementation of simplest key/value style storage system, written by C++11.

## Under Construction
To test the incompleted system, please use [Google Test](https://github.com/google/googletest). With my configuration in `Makefile`, you should do the following steps to test it.

1. Using `git clone` or other methods to fetch the current version of code to local disk. Remember that GCC Tools Chain is required.

2. According to the description of gtest, compile the code as a static library. Then copy this archive (`libgtest.a`) and folder `include` to `gtest` directory of this project. As a result, the structure of directories may be similar to this:

	```
	pumper
	-- build
	-- gtest
	   -- libgtest.a
	   -- include
	      -- gtest
	         -- ...
	-- include
	-- src
	-- test
	```

3. Create `build` folder, and execute `make` to compile ALL executives in `test` directory.

## MIT License
```
Copyright (C) 2015 Ren Feng.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

